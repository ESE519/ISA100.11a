/* This file implements the network layer for the Firefly nodes 
*/

#include "TransportLayerUDP.h"
#include "NetworkLayer.h"
#include "BufferManager.h"
#include "Serial.h"
#include "NWErrorCodes.h"
#include "Debug.h"

#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <bmac.h>
#include <nrk_error.h>
#include <stdint.h>
#include <stdlib.h>

/******************************** External variables /functions ********************************/
// From TransportLayerUDP.c 
extern nrk_sem_t *tl_sem;
extern Port ports[];

extern int8_t is_port_associated(int16_t port);
extern void print_seg_header(Transport_Segment_UDP*);
extern void print_seg(Transport_Segment_UDP*);

//From BufferManager.c 
extern nrk_sem_t *bm_sem;
extern ReceiveBufferManager rx_buf_mgr[];

extern void insert_rx_pq(Transport_Segment_UDP*, int8_t, uint16_t, int8_t);
extern TransmitBuffer* remove_tx_aq();
extern void insert_tx_fq(TransmitBuffer*);
extern void enter_cr(nrk_sem_t *, int8_t);
extern void leave_cr(nrk_sem_t *, int8_t);
extern int8_t port_to_port_index(uint8_t);
extern void print_tx_buffer();


//From Pack.c 
extern void pack_Msg_NgbList(uint8_t *, Msg_NgbList *);
extern void pack_Msg_Hello(uint8_t *, Msg_Hello *); 
extern void pack_NodeToGatewaySerial_Packet_header(uint8_t *, NodeToGatewaySerial_Packet *);
extern void pack_TL_UDP_header(uint8_t *, Transport_Segment_UDP*);
extern void pack_NW_Packet_header(uint8_t *,NW_Packet*); 

extern void unpack_Msg_NgbList(Msg_NgbList*, uint8_t *);
extern void unpack_Msg_Hello(Msg_Hello *, uint8_t *);
extern void unpack_TL_UDP_header(Transport_Segment_UDP *, uint8_t *); 
extern void unpack_NW_Packet_header(NW_Packet*, uint8_t *);

// From Serial.c 
extern void sendToSerial(uint8_t *, int8_t);
extern void printBuffer(uint8_t *, int8_t);

// From Debug.c 
extern void go_into_panic(char *);

// From bmac.c
extern int8_t bmac_tx_packet_enqueue(uint8_t *, uint8_t);

/************************************ Global data structures *********************************/
static NeighborList nl;					 						// to hold the neighbors of this node
RoutingTable rt[MAX_NODES];										// holds the routing algorithm

static uint8_t rx_buf[RF_BUFFER_SIZE];						// receive buffer for network layer
static NW_Packet pkt_rx;										// to hold a received packet from link layer 

static uint8_t tx_buf[SIZE_NW_PACKET]; 					// to hold the packet to be multi-hopped
static NW_Packet pkt_tx;										// to hold the packet built by the network layer 

uint8_t to_gw_buf[SIZE_NODETOGATEWAYSERIAL_PACKET];
NodeToGatewaySerial_Packet ntg_pkt;							// to hold the packet to be sent to the gateway

uint16_t DEFAULT_GATEWAY;										// address of the default gateway 
int8_t ROUTING_ALGORITHM;										// to hold the type of routing algorithm 
int8_t FLOODING_TYPE;											// to hold the type of flooding desired 
int8_t P_DISTRIBUTION;											// to hold the type of probability distribution 

// used by the nl_rx_task 
static Transport_Segment_UDP udp_seg;						// to hold a transport layer segment
Msg_Hello mh;			   										// to hold a HELLO message
Msg_NgbList mnlist;												// to hold a NGB_LIST message 
NeighborList nlist;												// to hold the actual NeighborList

// used by the nl_tx task
Msg_Hello mhe;					
Msg_NgbList mn;
Transport_Segment_UDP seg;	   

Msg_RoutingTable mrt;									// needed to process ROUTE_CONFIG messages


// definitions of the tasks and the stacks in the network layer
nrk_task_type NL_RX_TASK;
NRK_STK nl_rx_task_stack[NRK_APP_STACKSIZE];
void nl_rx_task(void);

nrk_task_type NL_TX_TASK;
NRK_STK nl_tx_task_stack[NRK_APP_STACKSIZE];
void nl_tx_task(void);

nrk_sem_t *nl_sem;												// semaphore to access the above variables 

/******************************* FUNCTION DEFINITIONS ***************************************/
inline int8_t add_neighbor(Neighbor n)
{
	int8_t i;					// loop index
	int8_t found = FALSE;	// flag to indicate whether neighbor was found in array
	
	/* first pass through array checks to see 
		1. if the neighbor had been recorded before
		2. decrements the lastReport (and possibly removes) of each neighbor
	*/
	
	enter_cr(nl_sem, 19);
	for(i = 0; i < MAX_NGBS; i++)	
	{
		// search to see if this neighbor has been recorded before		
		if(nl.ngbs[i].addr == n.addr)
		{
			found = TRUE;
			// update last reported
			nl.ngbs[i].lastReport = TIMEOUT_COUNTER;
			nl.ngbs[i].isNew = FALSE;
			nl.ngbs[i].rssi = n.rssi;
		}
		else if(nl.ngbs[i].addr != BCAST_ADDR)		// entry at this position is valid
      {
			nl.ngbs[i].lastReport--;					// decrement lastReport
			if(nl.ngbs[i].lastReport == 0)			// should I remove this neighbor?
			{
				nl.ngbs[i].addr = BCAST_ADDR;			// invalidate this entry
				nl.ngbs[i].rssi = 0;						// zero out the remaining two entries 
				nl.ngbs[i].isNew = FALSE;
				nl.count--;									// decrement number of ngbs recorded
			}
		}
	} // end for 

	if(found == TRUE)			// neighbor was already present in array; do nothing further
	{
		leave_cr(nl_sem, 19);	
		return NRK_OK;
	}
	
	// check to see if the addition of a new neighbor is possible 	
	if(nl.count == MAX_NGBS)		// cannot store more than MAX_NGBS neighbors
	{
		_nrk_errno_set(MAX_NEIGHBOR_LIMIT_REACHED);
		
		leave_cr(nl_sem, 19);		
		return NRK_ERROR;
	}
	
	/* second pass through array adds the new neighbor */
	for(i = 0; i < MAX_NGBS; i++)	
	{
		if(nl.ngbs[i].addr == BCAST_ADDR)		// this position in array holds an invalid entry
		{
			n.lastReport = TIMEOUT_COUNTER;		// set lastReport to timeout value
			n.isNew = TRUE;							// a new neighbor has reported 
			nl.ngbs[i] = n;							// place the new neighbor at this array index
			nl.count++;									// increment the number of neighbors recorded
			break;
		}
	}
	
	if(i == MAX_NGBS)									// sanity check for debugging
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("add_neighbor(): Bug found in implementation of MAX_NGBS\r\n"));
	}
		
	leave_cr(nl_sem, 19);	
	return NRK_OK; 
}
/*********************************************************************************************/
// This function can be changed to support different multihop criteria 
inline int8_t shouldIMultihop(NW_Packet *pkt)
{
	pkt -> ttl--;					// decrement the ttl value of the packet	
	
	if(pkt -> ttl == 0)
		return MULTIHOP_NO;	
	
	return MULTIHOP_YES;
}
/*********************************************************************************************/
inline void multihop(NW_Packet *pkt)
{
	if( shouldIMultihop(pkt) == MULTIHOP_YES )// see whether this packet should be multi hopped
	{
		enter_cr(bm_sem, 21);
		if( insert_tx_aq(pkt) == NRK_ERROR )
		{
			record_tx_queue_full(pkt);				// record this fact for statistics collection
		}			
		leave_cr(bm_sem, 21);

		if(DEBUG_NL == 2)
		{
			nrk_kprintf(PSTR("NL: multihop(): Inserted packet. "));
			print_tx_buffer();
			//print_pkt_header(pkt);
		} 
	} 
	return;
}
/*********************************************************************************************/
inline uint16_t route_addr(uint16_t addr)
{
	int8_t i;
	
	// account for the BCAST case first
	if(addr == BCAST_ADDR)
		return BCAST_ADDR;
	
	enter_cr(nl_sem, 34);
	// go through the routing table to see if a mapping for the dest address exists
	for(i = 0; i < MAX_NODES; i++)
	{
		if(rt[i].dest != BCAST_ADDR)	// entry is valid
			if(rt[i].dest == addr)
			{
				leave_cr(nl_sem, 34);
				return rt[i].nextHop;
			}
	}
	
	// if the code reaches this point, it means a mapping was not found	
	leave_cr(nl_sem, 34);	
	return BCAST_ADDR;
}
/**********************************************************************************************/
void route_packet(NW_Packet *pkt)	
{
	pkt -> ttl--;
	if(pkt -> ttl == 0)				// the TTL value expired. Drop the packet
		return;						// this is to protect against corrupt routing tables
									// and to deal with BCAST messages
	
	pkt -> prevHop = NODE_ADDR;
	pkt -> nextHop = route_addr(pkt -> dest);
	
	enter_cr(bm_sem, 23);
	if( insert_tx_aq(pkt) == NRK_ERROR )
	{
		record_tx_queue_full(pkt);		// record this fact for statistics collection
	}			
	leave_cr(bm_sem, 23);
		
	return;
}
/**********************************************************************************************/
int8_t sendToGateway(uint8_t *ptr, int8_t len)	// user API 
{
	uint8_t gw_buf[SIZE_NODETOGATEWAYSERIAL_PACKET] = {0};
		
	if(len <= 0 || len > MAX_SERIAL_PAYLOAD || ptr == NULL || CONNECTED_TO_GATEWAY == FALSE)	// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		return NRK_ERROR;
	}
	
	// fill up the members of the transmit buffer 
	gw_buf[0] = SERIAL_APPLICATION;
	gw_buf[1] = len;
	memcpy(gw_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ptr, len);
	
	sendToSerial(gw_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
	//printBuffer(gw_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
	
	return NRK_OK;
}
/**********************************************************************************************/
inline uint8_t pkt_type(NW_Packet *pkt)
{
	switch(pkt -> type)
	{
		case UDP:
			return APPLICATION;
			
		case HELLO:
		case NGB_LIST:
		case ROUTE_CONFIG:
			return NW_CONTROL;
				
	}
	return INVALID;	// invalid packet header received 
}
/***********************************************************************************************/
inline uint8_t tl_type(uint8_t type)
{
	if(type == UDP)
		return UDP;
		
	return INVALID;		// invalid transport layer type received 
}
/***********************************************************************************************/
inline uint8_t nw_ctrl_type(uint8_t type)
{
	switch(type)
	{
		case HELLO:
			return HELLO;
			
		case NGB_LIST:
			return NGB_LIST;
			
		case ROUTE_CONFIG:
			return ROUTE_CONFIG;
	}
	return INVALID;
}
/********************************************************************************************/
void process_app_pkt(NW_Packet *pkt, int8_t rssi)
{
	int8_t ret;
	
	if(DEBUG_NL >= 1)
	{
		nrk_kprintf(PSTR("NL: process_app_pkt(): Entered\r\n"));
		nrk_kprintf(PSTR("NL process_app_pkt(): Received from "));
		print_pkt_header(pkt);
		
		/*
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("What the hell am I doing here\r\n"));
		*/
	}
		
	if(tl_type(pkt -> type) == UDP)				// the TL protocol is UDP 
	{
		// unpack the UDP header					
		unpack_TL_UDP_header(&udp_seg, pkt -> data); 
		// copy the application payload into the data section of the UDP segment					
		memcpy(udp_seg.data, pkt -> data + SIZE_TRANSPORT_UDP_HEADER, MAX_APP_PAYLOAD);  
		
		// check to see if the destination port in the header is associated with any socket 
		if(is_port_associated(udp_seg.destPort) == TRUE)	// yes there is 
		{
			int8_t port_index;
			
			enter_cr(bm_sem, 28);
			enter_cr(tl_sem, 28);
				
			
			if(DEBUG_NL == 2)
			{
				//nrk_kprintf(PSTR("NL: process_app_pkt(): Received segment = "));
				//print_seg(&udp_seg);
				nrk_kprintf(PSTR("NL: process_app_pkt(): Before inserting into port queue\r\n"));
			}			
			insert_rx_pq(&udp_seg, pkt -> prio, pkt -> src, rssi);
			if(DEBUG_NL == 2)
			{
				nrk_kprintf(PSTR("NL: process_app_pkt(): After inserting into port queue\r\n"));
			}				
			
			port_index = port_to_port_index(udp_seg.destPort); // extract the relevant port element 
			if(port_index == NRK_ERROR)	//sanity check for debugging
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("NL: process_app_pkt(): Bug detected in implementation of port/rbm element array\r\n"));
			}			
			ret = nrk_event_signal(ports[port_index].data_arrived_signal);	// signal 'data arrived'
						
			if(ret == NRK_ERROR)
			{
				if(nrk_errno_get() == 1)	// this means the signal was not created. This is a bug
				{
					nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
						nrk_kprintf(PSTR("NL: process_app_pkt(): Bug detected in implementation of port signals\r\n"));
				}				
			} // end if(ret == NRK_ERROR)
		} // end if(is_port_associated())
	
		else // if there is no socket associated with this port, simply drop the packet
		{
			if(DEBUG_NL == 0)
			{
				nrk_kprintf(PSTR("Unassociated port found: "));
				printf("%u\n", udp_seg.destPort);
			}
				
			record_unassociated_socket_pkt(pkt);	// record this fact 
		}
		
		leave_cr(tl_sem, 28);	
		leave_cr(bm_sem, 28);
		
	} // end if(tl_layer == UDP)
	else // as of now UDP is the only supported transport layer, hence print an error
	{
		nrk_kprintf(PSTR("NL: process_app_pkt(): Unsupported transport layer type detected = "));
		printf("%d\r\n", pkt -> type);
	}
	return;
}
/**********************************************************************************************/
void process_nw_ctrl_pkt(NW_Packet *pkt, int8_t rssi)
{
	int8_t ret;	// to hold the return value of various function calls  
	int8_t i;
	
	if(DEBUG_NL >= 1)
		nrk_kprintf(PSTR("NL: Entered process_nw_ctrl_pkt()\n"));
		
	switch( nw_ctrl_type(pkt -> type) )
	{
		case HELLO:		// HELLO msg
			
			// unpack the Msg_Hello from the packet payload 
			unpack_Msg_Hello(&mh, pkt -> data);
			mh.n.rssi = rssi;
			ret = add_neighbor(mh.n);	
			if(ret == NRK_ERROR)
			{
				record_max_ngb_limit_reached(pkt);	// record this fact
			} 
			if(DEBUG_NL >= 1)
			{
				nrk_kprintf(PSTR("Received HELLO msg from: "));
				printf("%d ", mh.n.addr);
				nrk_kprintf(PSTR("with RSSI = "));
				printf("%d\r\n",mh.n.rssi);
			}
			
			break;

		case NGB_LIST:		// NGB_LIST msg
			// if the node gets it's own NgbList message back, drop it
			if(pkt -> src == NODE_ADDR)	
				break;
						
			// unpack the Msg_NgbList from the packet payload						
			unpack_Msg_NgbList(&mnlist, pkt -> data);
			nlist = mnlist.nl;
			
			if(DEBUG_NL >= 1)
			{
				int8_t i;	// loop index 
				printf("NL: Received NGB_LIST msg from %d with count = %d\n", nlist.my_addr, nlist.count);
				for(i = 0; i < MAX_NGBS; i++)		
				{
					if(nlist.ngbs[i].addr != BCAST_ADDR)	// valid entry
						printf("%u, ", nlist.ngbs[i].addr);
				}
				printf("\r\n");
			}
			
			//multihop(pkt);		// multihop this NGB_LIST message.
			route_packet(pkt); 
								
			if(CONNECTED_TO_GATEWAY == TRUE)	// this node is connected to the gateway 				
			{					
				// construct a packet to be sent to the gateway over the serial connection 
				ntg_pkt.type = SERIAL_NGB_LIST;
				ntg_pkt.length = SIZE_MSG_NGB_LIST;
				// pack the message in the data field of the NodeToGatewaySerial_Packet 
				pack_Msg_NgbList(ntg_pkt.data, &mnlist);
				
				// pack the NodeToGatewaySerial_Packet header into the serial transmit buffer 
				pack_NodeToGatewaySerial_Packet_header(to_gw_buf, &ntg_pkt);
				// append the payload into the serial transmit buffer 
				memcpy(to_gw_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, MAX_SERIAL_PAYLOAD); // CHECK 
				
				//send the packet to gateway
				if(DEBUG_NL >= 1)
				{
					nrk_kprintf(PSTR("Sending packet to gateway\r\n"));
				}
				sendToSerial(to_gw_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
				//printBuffer(to_gw_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
			} 
			break;
			
		case ROUTE_CONFIG:
			
			unpack_Msg_RoutingTable(&mrt, pkt -> data);
			enter_cr(nl_sem, 34);
			DEFAULT_GATEWAY = mrt.dg;	// get the value of DEFAULT_GATEWAY in any case
			leave_cr(nl_sem, 34);
			
			if(DEBUG_NL == 0)
			{
				nrk_kprintf(PSTR("Received a ROUTE_CONFIG message\r\n"));
				for(i = 0; i < MAX_NODES; i++)
				{
					printf("%d -> %d [%d, %d]\r\n", mrt.node, mrt.rt[i].dest, mrt.rt[i].nextHop, mrt.rt[i].cost);
				}
			}
			
			if(mrt.node == NODE_ADDR)	// this is my routing table
			{
				enter_cr(nl_sem, 34);
				initialise_routing_table();	// invalidate all entries
				for(i = 0; i < MAX_NODES; i++)
				{
					rt[i] = mrt.rt[i];
				}
				leave_cr(nl_sem, 34);
			}
			else						// some other node's routing table. Route it
			{
				route_packet(pkt);
			}
				
			break;
			
		default: 
			nrk_kprintf(PSTR("NL: process_nw_ctrl_pkt(): Unsupported network control message received = "));
			printf("%u\n", pkt -> type);
			break;			
			
	} // end switch
	
	return;
}
/**********************************************************************************************/
void process_other_pkt(NW_Packet *pkt, int8_t rssi)
{
	return;
}

/*********************************************************************************************/
inline void build_Msg_Hello(Msg_Hello *m)
{
	(m -> n).addr = (uint16_t)NODE_ADDR;
	(m -> n).rssi = (m -> n).lastReport = (m -> n).isNew = 0;
	
	// Build the network packet which will hold the HELLO message
  	pkt_tx.src = (uint16_t)NODE_ADDR;
  	pkt_tx.dest = (uint16_t)BCAST_ADDR;
  	pkt_tx.nextHop = (uint16_t)BCAST_ADDR;
  	pkt_tx.prevHop = (uint16_t)NODE_ADDR;
  	
  	pkt_tx.ttl = 1;		
  	pkt_tx.type = (uint8_t)HELLO;
  	pkt_tx.length = SIZE_MSG_HELLO;
  	pkt_tx.prio = NORMAL_PRIORITY;
  	
  	pack_Msg_Hello(pkt_tx.data, m);
	return;
}
/*********************************************************************************************/
inline void build_Msg_NgbList(Msg_NgbList *m)
{
	enter_cr(nl_sem, 32);
	m -> nl = nl;	
	leave_cr(nl_sem, 32);
	
	// Build the network packet which will hold the NGB_LIST message
  	pkt_tx.src = (uint16_t)NODE_ADDR;
  	enter_cr(nl_sem, 25);
  	pkt_tx.dest = DEFAULT_GATEWAY;
  	leave_cr(nl_sem, 25);
  	pkt_tx.nextHop = route_addr(pkt_tx.dest);
  	pkt_tx.prevHop = (uint16_t)NODE_ADDR;
  	
  	pkt_tx.ttl = MAX_NETWORK_DIAMETER;		
  	pkt_tx.type = (uint8_t)NGB_LIST;
  	pkt_tx.length = SIZE_MSG_NGB_LIST;
  	pkt_tx.prio = NORMAL_PRIORITY;
  	
  	pack_Msg_NgbList(pkt_tx.data, m);
  	
  	return;
}				
/**********************************************************************************************/	

void nl_rx_task()
{
	uint8_t len; 				   // to hold the size of the received packet, always = sizeof(NW_Packet) 
	int8_t rssi;  			   	// to hold rssi of received packet
	uint8_t *local_rx_buf;	   // pointer to receive buffer of link layer
	int8_t val;						// status variable to hold the return type of function calls
	int8_t flag;
	
	nrk_time_t start, end, elapsed;		// needed by nl_rx_task()
										// to decide when to send own NGB_LIST
	if(DEBUG_NL >= 1)
	{
		nrk_kprintf(PSTR("NL_RX_TASK PID = "));
		printf("%d\r\n",nrk_get_pid());
	}
	
	// initialise the timer
	nrk_time_get(&start);
	end.secs = start.secs;
	end.nano_secs = start.nano_secs;

	// initialise the link layer	
	val = bmac_init(25);
	if(val == NRK_ERROR)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("NL: Error returned by bmac_init()\r\n"));
	}	
		
	// give the link layer a rx buffer 	
	val = bmac_rx_pkt_set_buffer(rx_buf, RF_BUFFER_SIZE);
	if(val == NRK_ERROR)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("NL: Error returned by bmac_rx_pkt_set_buffer()\r\n"));
	}
		
	// start processing forever 
	while(1)
	{
		// decide whether it is time to send your own Ngb_List message
		if(CONNECTED_TO_GATEWAY == TRUE)
		{
			nrk_time_get(&end);
			val = nrk_time_sub(&elapsed, end, start);
			nrk_time_compact_nanos(&elapsed);
			if(elapsed.secs >= NGB_LIST_PERIOD)
			{
				ntg_pkt.type = SERIAL_NGB_LIST;
				ntg_pkt.length = SIZE_MSG_NGB_LIST;
				enter_cr(nl_sem, 34);
				pack_Msg_NgbList(ntg_pkt.data, &nl);
				leave_cr(nl_sem, 34);
				pack_NodeToGatewaySerial_Packet_header(to_gw_buf, &ntg_pkt);
				memcpy(to_gw_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, MAX_SERIAL_PAYLOAD);
				if(DEBUG_NL == 2)
				{
					nrk_kprintf(PSTR("Sending own NGB_LIST message to gateway\r\n"));
				}
				sendToSerial(to_gw_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
				
				// reset the timer
				start.secs = end.secs;
				start.nano_secs = end.nano_secs;
			} // end if
		} // end if
				
		if(DEBUG_NL >= 1)
		{
			nrk_kprintf(PSTR("Waiting for next pkt from link layer\r\n"));
		}
		
		flag = 0;
		// wait for the next packet 		
		while(bmac_rx_pkt_ready() == 0)
		{
			val = bmac_wait_until_rx_pkt();
			if(DEBUG_NL == 2)
			{
				nrk_kprintf(PSTR("NL: bmac_wait_until_rx_packet() returned "));
				printf("%d\n", val);
			}
		}
		
		// Get the packet 
	   	do
	   	{
	   		local_rx_buf = bmac_rx_pkt_get(&len,&rssi);
	   		if(local_rx_buf == NULL)
	   		{
	   			nrk_kprintf(PSTR("NL: NULL returned by bmac_rx_pkt_get()\r\n"));
	   		}
	   		
	   	} while(local_rx_buf == NULL);
	   	
	   	// sanity check for debugging 
	   	if(len != SIZE_NW_PACKET)	// this should not happen 
	   	{
	   		/* 
	   		nrk_int_disable();
	   		nrk_led_set(RED_LED);
	   		while(1)
	   		{
	   			nrk_kprintf(PSTR("NL: Wrong length of packet received: "));
	   			printf("%d\r\n", len);
	   		}
	   		*/
	   		if(DEBUG_NL >= 1)
	   		{
	   			nrk_kprintf(PSTR("NL: nl_rx_task(): Wrong length of packet received: "));
	   			printf("%d\r\n", len);
	   		}
	   		flag = 1;
	   	}
	   	
	   	nrk_led_set(GREEN_LED);
	   	
	   	if(DEBUG_NL == 2)// || flag == 1)
	   	{
				int8_t i; 
				nrk_kprintf(PSTR("NL: Contents of received packet are\r\n"));
				printf("[");
				for(i = 0; i < len; i++)
					printf("%d ", local_rx_buf[i]);
				printf("]\r\n");
		}  	
		if(flag == 1)
		{
			bmac_rx_pkt_release();	// drop the packet and go receive another
			nrk_led_clr(GREEN_LED);
			continue;
		}
			   	
			// unpack the packet header from the received buffer 
			unpack_NW_Packet_header(&pkt_rx, local_rx_buf);
			// copy the packet payload to the data field of the local packet
			memcpy(pkt_rx.data, local_rx_buf + SIZE_NW_PACKET_HEADER, MAX_NETWORK_PAYLOAD);
			// Release the RX buffer quickly so future packets can arrive
			bmac_rx_pkt_release();	
			
			// begin processing this packet
			if(pkt_type(&pkt_rx) == APPLICATION)	// its an application layer packet 
			{
				// case 1: Destination is NODE_ADDR or BCAST_ADDR
				if(pkt_rx.dest == NODE_ADDR || pkt_rx.dest == BCAST_ADDR)
					process_app_pkt(&pkt_rx, rssi);
					
				// case 2: I am enroute to the destination
				else if(pkt_rx.nextHop == NODE_ADDR)
					 {
					 	if(pkt_rx.src == NODE_ADDR)		// routing table corrupted
						{
							nrk_int_disable();
							nrk_led_set(RED_LED);
							while(1)
							{
								nrk_kprintf(PSTR("Routing table corrupted at "));
								printf("%d\r\n", NODE_ADDR);
							} // end while 
						} // end if 
						else
							route_packet(&pkt_rx);
					 } // end if 		
					 // case 3: Routing tables still not made
					 else if(pkt_rx.nextHop == BCAST_ADDR)	
					 			route_packet(&pkt_rx);
					 	  else
					 	  	;		 // drop all other packets								
			} // end if(type == APPLICATION)
			else 
			{
				if(pkt_type(&pkt_rx) == NW_CONTROL)	// its a network control packet 
				{
				  	// case 1: Destination is NODE_ADDR or BCAST_ADDR
				  	if(pkt_rx.dest == NODE_ADDR || pkt_rx.dest == BCAST_ADDR)
				  		process_nw_ctrl_pkt(&pkt_rx, rssi);
				  	
				  	// case 2: I am enroute to a destination
				  	else if(pkt_rx.nextHop == NODE_ADDR)
				  		 {
				  		 	if(pkt_rx.src == NODE_ADDR)		// routing table corrupted
				  		 	{
				  		 		nrk_int_disable();
								nrk_led_set(RED_LED);
								while(1)
								{
									nrk_kprintf(PSTR("Routing table corrupted at "));
									printf("%d\r\n", NODE_ADDR);
								} // end while 
							} // end if
							else
				  		 		route_packet(&pkt_rx);
				  		 } // end if
				  		// case 3: Routing tables still not made 
				  		 else if(pkt_rx.nextHop == BCAST_ADDR)
				  		 		route_packet(&pkt_rx);
				  		 	  else
				  		 		;	// drop all other packets
				  } // end if(type == NW_CONTROL)
				  else	// unknown packet type 
				  {
				  		nrk_kprintf(PSTR("NL: Unknown pkt type received = "));
				  		printf("%d\r\n", pkt_type(&pkt_rx));
				  }
			}
				
			nrk_led_clr(GREEN_LED);			
	 } // end while(1)
	
	return;
}	// end nl_rx_task
	
/*********************************************************************************************/
void nl_tx_task()
{
	TransmitBuffer *ptr = NULL; 				// pointer to the buffer to be transmitted 
	nrk_sig_t tx_done_signal;					// to hold the tx_done signal from the link layer 
	int8_t ret;										// to hold the return value of various functions 
	int8_t port_index;							// to store the index of the corresponding port element 
	int8_t sent;									// to count the number of times the HELLO msg was sent 
	int8_t isApplication;						// flag to indicate whether the packet in the transmit 
														// buffer is an APPLICATION / NW_CONTROL packet
	 
	nrk_time_t timeout;
	nrk_time_t start;								// used for sending network control messages 
	nrk_time_t end;	
	nrk_time_t elapsed;			
	
	// wait for the nl_rx_task to start bmac 	
	while(!bmac_started()) nrk_wait_until_next_period();
	
	// retrieve and register for the 'transmit done' signal from bmac 	
	tx_done_signal = bmac_get_tx_done_signal();
  	
  	if( nrk_signal_register(tx_done_signal) == NRK_ERROR )
  	{
  		nrk_int_disable();
  		nrk_led_set(RED_LED);
  		while(1)
  			nrk_kprintf(PSTR("NL: Error while registering for the bmax_tx_done_signal\r\n"));
  	}
	
	// initialise the timer
	nrk_time_get(&start);
	end.secs = start.secs;
	end.nano_secs = start.nano_secs;
	sent = 0;
	
	// set the radio power
	if( bmac_set_rf_power(20) == NRK_ERROR)
  	{
  		nrk_led_set(RED_LED);
  		nrk_int_disable();
  		while(1)
  			nrk_kprintf(PSTR("Error setting the transmit power\r\n"));
  	}
	while(1)
	{
		isApplication = FALSE;	// assume at the beginning that a nw_ctrl pkt will be transmitted 
				
		ret = nrk_time_sub(&elapsed, end, start);
		if(ret == 0)
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("NL: Error returned by nrk_time_sub\r\n"));
		}
		
		nrk_time_compact_nanos(&elapsed);
		
		if(elapsed.secs >= HELLO_PERIOD)
		{	
			sent++;
			build_Msg_Hello(&mhe);				// build the 'HELLO' message
			if(DEBUG_NL == 2)
			{
				nrk_kprintf(PSTR("After building Msg_Hello, packet = "));
				print_pkt(&pkt_tx);
			}
				
			enter_cr(bm_sem, 34);
			ret = insert_tx_aq(&pkt_tx);  			// insert it into the transmit queue			
			leave_cr(bm_sem, 34);
			
			if(DEBUG_NL == 2)
			{
				nrk_kprintf(PSTR("build_Msg_Hello() inserted packet."));
				print_tx_buffer();
				//print_pkt_header(&pkt_tx);
			}
			
			if(ret == NRK_ERROR && DEBUG_NL == 2)
			{
				nrk_kprintf(PSTR("HELLO msg was not inserted into the transmit queue\r\n"));
			}
			
			start.secs = end.secs;	
			start.nano_secs = end.nano_secs; // reinitialise the timer 		
		}

		if(sent >= 3) //NGB_LIST_PERIOD / HELLO_PERIOD)	// NGB_LIST period is always a multiple of HELLO_PERIOD		
		{
			build_Msg_NgbList(&mn);				// build the 'NGB_LIST' message 
									
			enter_cr(bm_sem, 34);
			ret = insert_tx_aq(&pkt_tx);  			// insert it into the transmit queue			
			leave_cr(bm_sem, 34);
			
			if(DEBUG_NL == 2)
			{
				nrk_kprintf(PSTR("build_Msg_NgbList() inserted packet."));
				print_tx_buffer();
				//print_pkt_header(&pkt_tx);
			}
			
			if(ret == NRK_ERROR && DEBUG_NL == 2)
			{
				nrk_kprintf(PSTR("NGB_LIST msg was not inserted into the transmit queue\r\n"));
			}
			
			sent = 0;								// reset the value of 'sent'
		}					
		 
	
		if(rand() % 2 == 0)						// random number generator
			collect_queue_statistics();
		
		enter_cr(bm_sem, 34);		
		ptr = remove_tx_aq();		
		leave_cr(bm_sem, 34);
		
		if(ptr == NULL)							// transmit queue is empty 
		{
			if(DEBUG_NL == 2)
				nrk_kprintf(PSTR("NL:Transmit queue is empty\r\n"));
			
			// update the end time 
			nrk_time_get(&end);		
				
			nrk_wait_until_next_period();	// FIX ME 
			continue;
		} 

		if(DEBUG_NL ==  2)
		{
			nrk_kprintf(PSTR("NL: nl_tx_task(): Packet removed. Packet = "));
			//print_pkt_header( &(ptr -> pkt) );
			print_pkt( &(ptr -> pkt) );
			//print_tx_buffer();
		}		
		
		// check to see the type of packet. It should be of type APPLICATION and be sent by this 
		// node  		
		if( (pkt_type(&(ptr -> pkt)) == APPLICATION) && ((ptr -> pkt).src == NODE_ADDR) )		 
		{ 		
			// remove the encapsulated TL segment from the packet 
			unpack_TL_UDP_header(&seg, (ptr -> pkt).data);
			memcpy(seg.data, (ptr -> pkt).data + SIZE_TRANSPORT_UDP_HEADER, MAX_APP_PAYLOAD);
			
			if(DEBUG_NL == 2)
			{
				nrk_kprintf(PSTR("NL: nl_tx_task(): Segment Removed = "));
				print_seg(&seg);
			}
			isApplication = TRUE;
		}	
			
		// pack the network packet header into the transmit buffer 
		pack_NW_Packet_header(tx_buf, &(ptr -> pkt));
		// append the network payload into the transmit buffer
		memcpy(tx_buf + SIZE_NW_PACKET_HEADER, (ptr -> pkt).data, MAX_NETWORK_PAYLOAD);
		
		enter_cr(bm_sem, 34);		
		insert_tx_fq(ptr);	// release the transmit buffer into the free queue 
		leave_cr(bm_sem, 34);
		if(DEBUG_NL == 2)
		{
			nrk_kprintf(PSTR("NL: nl_tx_task(): Released transmit buffer back into queue\n"));
			print_tx_buffer();
		}
		
		do
		{
			ret =  bmac_tx_pkt_nonblocking(tx_buf, SIZE_NW_PACKET);	// try to queue the buffer in link layer
			
			if(ret == NRK_ERROR)
				if(nrk_event_wait(SIG(tx_done_signal)) == 0)
				{
					nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
						nrk_kprintf(PSTR("NL: Error returned by nrk_event_wait(tx_done_signal)\r\n"));
				}
		}while(ret == NRK_ERROR);
		
		// packet is queued at link layer
		timeout.secs = 10;				// set a wait period of maximum 10 seconds
		timeout.nano_secs = 0;
		if( nrk_signal_register(nrk_wakeup_signal) == NRK_ERROR )
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("NL:nl_tx(): Error registering for nrk_wakeup_signal\r\n"));
		}
		if( nrk_set_next_wakeup(timeout) == NRK_ERROR)
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("NL: nl_tx(): Error returned by nrk_set_next_wakeup()\r\n"));
		}		 
		nrk_led_set(BLUE_LED);
  		ret = nrk_event_wait (SIG(tx_done_signal) | SIG(nrk_wakeup_signal));	// wait for its transmission  				
		if(ret == 0)
  		{
  			nrk_int_disable();
  			nrk_led_set(RED_LED);
  			while(1)
  				nrk_kprintf(PSTR("NL: Error returned by nrk_event_wait(tx_done_signal)\r\n"));
  		}
	   if(ret & SIG(tx_done_signal)) 	// bmac has successfully sent the packet over the radio 
		{
			if(isApplication == TRUE)		// it was an application layer packet 
			{
				enter_cr(tl_sem, 34);				
				port_index = port_to_port_index(seg.srcPort);
						
				if(port_index == NRK_ERROR)	// sanity check
				{
					nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
						nrk_kprintf(PSTR("NL: nl_tx_task: Bug detected in implementation of port element array\r\n"));
				}
				// signal 'send done' signal 			
				if(nrk_event_signal(ports[port_index].send_done_signal) == NRK_ERROR)
				{
					if(nrk_errno_get() == 1)	// sanity check. This means signal was not created 
					{
						nrk_int_disable();
						nrk_led_set(RED_LED);
						while(1)
							nrk_kprintf(PSTR("NL: nl_tx_task: Bug detected in creating signals in port element array\r\n"));
					}
			   }					
				leave_cr(tl_sem, 34);	
				
			}// end if(isApplication == TRUE)
			
			else	// a network control message was transmitted. Nothing to signal
				;	// do nothing 
		
		} // end if(signal received = tx_done_signal)
		else if(ret & SIG(nrk_wakeup_signal))	
			 {
			 	//nrk_led_set(RED_LED);
			 	//nrk_int_disable();
			 	//while(1)			
			 	//{
			 		nrk_kprintf(PSTR("BMAC did not transmit the packet within specified time\r\n"));
			 	//}
			 }			
			else // unknown signal caught 
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("NL: nl_tx_task(): Unknown signal caught\r\n"));
			}
		
		nrk_led_clr(BLUE_LED);		
		// update the end time 
		nrk_time_get(&end);
	
   } // end while(1) 
   
	return;
}
/******************************************************************************************/
int8_t set_routing_algorithm(int8_t pref, int8_t type, int8_t pdist)
{
	switch(pref)
	{
		case LINK_STATE:
			break;
			
		case FLOODING:
		{
			switch(type)
			{
				case TTL_BASED:
					break;
				
				case PROBABILISTIC:
				{
					switch(pdist)
					{
						case RANDOM:
						case GAUSSIAN:
							break;
							
						default:
						{
							_nrk_errno_set(INVALID_ARGUMENT);
							return NRK_ERROR;
						}
  					} // end switch(pdist)
					break;  					
  				} // end case PROBABILISTIC:
  				  				
  				default:	// invalid flooding type 
  				{
  					_nrk_errno_set(INVALID_ARGUMENT);
  					return NRK_ERROR;
  				}
  			} // end switch(type)
			break;
		} // end case FLOODING 
		
		default: // invalid routing algorithm preference
		{
			_nrk_errno_set(INVALID_ARGUMENT);
			return NRK_ERROR;
		}
	} // end switch(pref)
	
	enter_cr(nl_sem, 19);
	ROUTING_ALGORITHM = pref;
	if(pref == FLOODING)
	{
		FLOODING_TYPE = type;
		if(type == PROBABILISTIC)
		{
			P_DISTRIBUTION = pdist;
		}
	}
	leave_cr(nl_sem, 19);
	
	return NRK_OK;
}
/******************************************************************************************/
void create_network_layer_tasks()
{
  NL_RX_TASK.task = nl_rx_task;
  NL_RX_TASK.Ptos = (void *) &nl_rx_task_stack[NRK_APP_STACKSIZE - 1];
  NL_RX_TASK.Pbos = (void *) &nl_rx_task_stack[0];
  NL_RX_TASK.prio = 19;
  NL_RX_TASK.FirstActivation = TRUE;
  NL_RX_TASK.Type = BASIC_TASK;
  NL_RX_TASK.SchType = PREEMPTIVE;
  
  NL_RX_TASK.cpu_reserve.secs = 0;
  NL_RX_TASK.cpu_reserve.nano_secs = 700 * NANOS_PER_MS;  
  NL_RX_TASK.period.secs = 1;
  NL_RX_TASK.period.nano_secs = 0;
   
  
  NL_RX_TASK.offset.secs = 0;
  NL_RX_TASK.offset.nano_secs= 0;
  nrk_activate_task (&NL_RX_TASK);

/*****************************************************************************************/  
  NL_TX_TASK.task = nl_tx_task;
  NL_TX_TASK.Ptos = (void *) &nl_tx_task_stack[NRK_APP_STACKSIZE - 1];
  NL_TX_TASK.Pbos = (void *) &nl_tx_task_stack[0];
  NL_TX_TASK.prio = 18;
  NL_TX_TASK.FirstActivation = TRUE;
  NL_TX_TASK.Type = BASIC_TASK;
  NL_TX_TASK.SchType = PREEMPTIVE;
  
  NL_TX_TASK.cpu_reserve.secs = 0;
  NL_TX_TASK.cpu_reserve.nano_secs = 500 * NANOS_PER_MS;	
  NL_TX_TASK.period.secs = 1;
  NL_TX_TASK.period.nano_secs = 0;
  
  NL_TX_TASK.offset.secs = 0;
  NL_TX_TASK.offset.nano_secs= 0;
  nrk_activate_task (&NL_TX_TASK);

  if(DEBUG_NL == 2)
	nrk_kprintf(PSTR("create_network_layer_tasks(): Network layer task creation done\r\n"));
}

/*******************************************************************************************/
void initialise_network_layer()
{
	int8_t i;	// loop index
		
	/* initialise the data structures required for the network layer */ 
	nl.count = 0;									// no neighbors recorded yet 
	nl.my_addr = NODE_ADDR;						// assign my network layer address 
	for(i = 0; i < MAX_NGBS; i++)				// an addr = BCAST_ADDR indicates an invalid entry
	   nl.ngbs[i].addr = BCAST_ADDR;
		
	DEFAULT_GATEWAY = BCAST_ADDR;				// initially the default gateway is unknown 
	ROUTING_ALGORITHM = DEFAULT_ROUTING_ALGORITHM; // set the routing algorithm 
	FLOODING_TYPE = DEFAULT_FLOODING_TYPE;			  // set the flooding type to be ttl-based 
	P_DISTRIBUTION = DEFAULT_PDISTRIBUTION;		  // set the probability distribution 
	
	initialise_routing_table();		
	
	nl_sem = nrk_sem_create(1,MAX_TASK_PRIORITY);	// create the mutex  
	if(nl_sem == NULL)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("NL: Error creating semaphore in initialise_network_layer()\r\n"));
	}
	
	create_network_layer_tasks();	// create the two tasks 
	
	return;
}	
/******************************************************************************************/
void initialise_routing_table()
{
	int8_t i;
	
	for(i = 0; i < MAX_NODES; i++)
	{
		rt[i].dest = BCAST_ADDR;				  // indicates invalid entry 
		rt[i].nextHop = BCAST_ADDR;				  // initially no routes are known
		rt[i].cost = INFINITY;				  	  // no distances are known
	}
	return;
}
/*******************************************************************************************/
void set_RoutingTable(Msg_RoutingTable *m)
{
	int8_t i;
	
	enter_cr(nl_sem, 34);
	initialise_routing_table();					// make all entries invalid	
	DEFAULT_GATEWAY = m -> dg;
	for(i = 0; i < MAX_NODES; i++)
	{
		rt[i] = m -> rt[i];
	}
	leave_cr(nl_sem, 34);
	return;
}
/******************************************************************************************/
void print_pkt_header(NW_Packet *pkt)
{
	printf("[%u %u %u %u %d %u %d %d]", pkt -> src, pkt -> dest, pkt -> prevHop, pkt -> nextHop, pkt -> ttl, pkt -> type, pkt -> length, pkt -> prio);
	
	return;
}
/*******************************************************************************************/
void print_pkt(NW_Packet *pkt)
{
	int8_t i;
		
	print_pkt_header(pkt);
	for(i = 0; i < pkt -> length; i++)
		printf("%u ", (pkt -> data)[i]);
		
	printf("\r\n");

	return;
}
/******************************************************************************************/
void print_RoutingTable(Msg_RoutingTable *mrtbl)
{
	int8_t i;
	
	nrk_kprintf(PSTR("Routing table for "));
	printf("%d\r\n", mrtbl -> node);
	
	for(i = 0; i < MAX_NODES; i++)
	{
		printf("%d -> %d [nh = ", mrtbl -> node, (mrtbl -> rt[i]).dest);
		if((mrtbl -> rt[i]).nextHop == INVALID_ADDRESS)
			printf("INV, ");
		else
			printf("%d, ", (mrtbl -> rt[i]).nextHop);
					
		if((mrtbl -> rt[i]).cost != INFINITY)
			printf("dist = %d]\r\n", (mrtbl -> rt[i]).cost);
		else
			printf("dist = INF]\r\n");
	}
	return;
}
/*************************** STATISTICS COLLECTION functions ******************************/
void record_tx_queue_full(NW_Packet *pkt)
{
	return;
	
}
/*******************************************************************************************/
void record_unassociated_socket_pkt(NW_Packet *pkt)
{
	return;
}
/********************************************************************************************/
void record_max_ngb_limit_reached(NW_Packet *pkt)
{
	return;
}
/********************************************************************************************/
void collect_queue_statistics()
{
	return;
}
/*******************************************************************************************/

/************************** PRINTING functions *********************************************/
	
	
	
	
	
	

