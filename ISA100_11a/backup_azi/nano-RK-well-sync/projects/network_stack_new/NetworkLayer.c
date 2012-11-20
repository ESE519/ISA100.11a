/* This file implements the network layer of the network stack
 
 Authors:
 Aditya Bhave
*/

/**************************************** HEADERS ************************************************/
#include "TransportLayerUDP.h"
#include "NetworkLayer.h"
#include "BufferManager.h"
#include "Serial.h"
#include "NWErrorCodes.h"
#include "Debug.h"
#include "LinkLayer.h"

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
#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <ff_basic_sensor.h>

/******************************** EXTERNAL VARIABLES / FUNCTIONS ********************************/
// From TransportLayerUDP.c 
extern nrk_sem_t *tl_sem;
extern Port ports[];

extern int8_t is_port_associated(int16_t port);
extern void print_seg_header(Transport_Segment_UDP*);
extern void print_seg(Transport_Segment_UDP*);

//From BufferManager.c 
extern nrk_sem_t *bm_sem;
extern ReceiveBufferManager rx_buf_mgr[];
extern TransmitBufferManager tx_buf_mgr;
extern int8_t num_bufs_free;

extern void insert_rx_pq(Transport_Segment_UDP*, int8_t, uint16_t, int8_t);
extern TransmitBuffer* remove_tx_aq();
extern void insert_tx_fq(TransmitBuffer*);
extern void enter_cr(nrk_sem_t *, int8_t);
extern void leave_cr(nrk_sem_t *, int8_t);
extern int8_t port_to_port_index(uint8_t);
extern void print_tx_buffer();
extern void collect_queue_statistics();
extern void record_tx_queue_full(NW_Packet *pkt);
extern int8_t get_tx_aq_size();

//From Pack.c 
extern void pack_Msg_NgbList(uint8_t *, Msg_NgbList *);
extern void pack_Msg_Hello(uint8_t *, Msg_Hello *); 
extern void pack_TL_UDP_header(uint8_t *, Transport_Segment_UDP*);
extern void pack_NW_Packet_header(uint8_t *,NW_Packet*); 
extern void pack_Msg_RouteRequest(uint8_t *, Msg_RouteRequest *);
extern void pack_NodeToGatewaySerial_Packet_header(uint8_t *, NodeToGatewaySerial_Packet *);
extern void pack_Msg_NodeInfo(uint8_t *, Msg_NodeInfo *);

extern void unpack_Msg_NgbList(Msg_NgbList*, uint8_t *);
extern void unpack_Msg_Hello(Msg_Hello *, uint8_t *);
extern void unpack_TL_UDP_header(Transport_Segment_UDP *, uint8_t *); 
extern void unpack_NW_Packet_header(NW_Packet*, uint8_t *);
extern void unpack_Msg_RouteReply(Msg_RouteReply *, uint8_t *);
extern void unpack_Msg_NwInfoAcquired(Msg_NwInfoAcquired*, uint8_t *);
extern void unpack_Msg_SendNwInfo(Msg_SendNwInfo*, uint8_t *);
extern void unpack_Msg_SendNodeInfo(Msg_SendNodeInfo*, uint8_t *);
extern void unpack_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *, uint8_t *);
extern void unpack_Msg_NodeInfo(Msg_NodeInfo *, uint8_t *);

// From Serial.c 
extern int8_t sendToSerial(uint8_t *, uint8_t);
extern void printBuffer(uint8_t *, uint8_t);

// From Debug.c 
extern void go_into_panic(char *);

// From bmac.c
extern int8_t bmac_tx_packet_enqueue(uint8_t *, uint8_t);

/************************************ DATA STRUCTURES *********************************/
static NeighborList nl;					 				// to hold the neighbors of this node
RoutingTable rt;										// holds the routing table of the node
														
static uint8_t rx_buf[RF_BUFFER_SIZE];					// receive buffer for network layer
static NW_Packet pkt_rx;								// to hold a received packet from link layer 

static uint8_t tx_buf[SIZE_NW_PACKET]; 					// transmit buffer given to link layer
static NW_Packet pkt_tx;								// to hold a packet to be transmitted 

uint16_t DEFAULT_GATEWAY;								// address of the default gateway 

// used by the nl_rx_task
static Transport_Segment_UDP udp_seg;					// to build a segment to be given to the TL
Msg_Hello mh;			   								// to hold a HELLO message
Msg_NgbList mnlist;										// to hold a NGB_LIST message 
Msg_NodeInfo mni;
NeighborList nlist;										// to hold the actual NeighborList
Neighbor ngb;											// to hold information about a neighbor
Msg_RouteRequest mrrq;									// to hold ROUTE_REQUEST message
Msg_RoutingTable mrt;									// to hold a ROUTE_CONFIG message
Msg_RouteReply mrr;										// to hold a ROUTE_REPLY message
Msg_NwInfoAcquired mnwia;								// to hold a NW_INFO_ACQUIRED message
Msg_SendNwInfo msnwi;									// to hold a SEND_NW_INFO message
Msg_SendNodeInfo msni;									// to hold a SEND_NODE_INFO message
Msg_NodeInfoAcquired mnia;								// to hold a NODE_INFO_ACQUIRED message
uint8_t to_gw_buf_rx[SIZE_NODETOGATEWAYSERIAL_PACKET];	// transmit buffer used by nl_rx task for serial communication
uint8_t to_gw_buf_tx[SIZE_NODETOGATEWAYSERIAL_PACKET];	// transmit buffer used by nl_tx_task for serial communication
uint8_t gw_buf[SIZE_NODETOGATEWAYSERIAL_PACKET];		// another transmit buffer for serial communication
NodeToGatewaySerial_Packet ntg_pkt;						// to hold the packet to be sent to the gateway

// used by the nl_tx task
Msg_Hello mhe;											// to hold a HELLO message	
Msg_NgbList mn;											// to hold a NGB_LIST message
Msg_NodeInfo mnode;										// to hold a NODE_INFO message
Transport_Segment_UDP seg;	   							// to retrieve the TL segment from the TX buffer


// definitions of the tasks and the stacks in the network layer
nrk_task_type NL_RX_TASK;								// for the NL_TX task
NRK_STK nl_rx_task_stack[NRK_APP_STACKSIZE];
void nl_rx_task(void);

nrk_task_type NL_TX_TASK;								// for the NL_RX task
NRK_STK nl_tx_task_stack[NRK_APP_STACKSIZE];
void nl_tx_task(void);

nrk_sem_t *nl_sem;									// semaphore to access the above variables 
int8_t continue_sending_ngblist;					// flag to indicate whether to continue sending NGB_LIST messages
int8_t ngb_list_pkt_queued;							// to store whether the TX queue contains a NGB_LIST message
int8_t ngb_list_pkt_dequeued;						// to store when a NGB_LIST message is dequeued

int8_t continue_sending_nodeinfo;					// flag to indicate whether to continue sending NODE_INFO messages
int8_t nodeinfo_pkt_queued;							// to store whether the TX queue contains a NODE_INFO message
int8_t nodeinfo_pkt_dequeued;						// to store when a NODE_INFO message is dequeued

NodeState ns;										// to hold one entry in the NodeStateQueue
NodeStateQueue nsq;									// to store state variables for the node

/******************************* FUNCTION DEFINITIONS ***************************************/
int8_t add_neighbor(Neighbor n)		// CHECKED
{
	int8_t i;					// loop index
	int8_t free_index = -1;		// initally assume no free positions exist within the array
	
	/* first pass through array checks to see 
		1. if the neighbor had been recorded before
	*/
	
	enter_cr(nl_sem, 1);		// Acquire the NL semaphore
	for(i = 0; i < MAX_NGBS; i++)	
	{
		// search to see if this neighbor has been recorded before		
		if(nl.ngbs[i].addr == n.addr)	// neighbor is found
		{
			// update state variables of this neighbor
			nl.ngbs[i].lastReport = TIMEOUT_COUNTER;
			nl.ngbs[i].rssi = n.rssi;
			nl.ngbs[i].isNew = FALSE;
			leave_cr(nl_sem, 1);		// release the NL semaphore
			return NRK_OK;				// return;
		} // end if
		else if(nl.ngbs[i].addr == INVALID_ADDR)
				free_index = i;
	} // end for
	
	// At this point, we know that we need to add a new neighbor
	// check to see if the addition is possible 	
	if(nl.count == MAX_NGBS)		// cannot store more than MAX_NGBS neighbors
	{
		_nrk_errno_set(MAX_NEIGHBOR_LIMIT_REACHED);	// set the error number
		leave_cr(nl_sem, 1);						// release the NL semaphore	
		return NRK_ERROR;							// return
	}
	
	/* Addition is possible. Assign the new neighbor to the empty position within the array */
	if(free_index == -1)	// sanity check. This should never happen
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("PANIC: NL: add_neighbor(): Bug found in implementation of MAX_NGBS\r\n"));
	}
	
	nl.ngbs[free_index].addr = n.addr;
	nl.ngbs[free_index].rssi = n.rssi;
	nl.ngbs[free_index].lastReport = TIMEOUT_COUNTER;
	nl.ngbs[free_index].isNew = TRUE;
	nl.count++;			// increment number of neighbors recorded
	printf("Added a neighbor: %d\r\n", n.addr);
		
	leave_cr(nl_sem, 1);	// release the NL semaphore
	return NRK_OK; 			// return 
}
/*********************************************************************************************/
void update_NgbList()
{
	int8_t i;
	
	enter_cr(nl_sem, 2);
	for(i = 0; i < MAX_NGBS; i++)
		if(nl.ngbs[i].addr != INVALID_ADDR)		// valid entry in the Ngb_List array
		{
			nl.ngbs[i].lastReport--;
			if(nl.ngbs[i].lastReport == 0)		// time to remove this neighbor
			{
				nrk_kprintf(PSTR("NL: Removed a neighbor: "));
				printf("%d\r\n", nl.ngbs[i].addr);
				nl.ngbs[i].addr = INVALID_ADDR;
				nl.ngbs[i].rssi = 0;
				nl.ngbs[i].isNew = FALSE;
				nl.count--;
				//continue_sending_ngblist = 1;	// ALLOWED here since Im already in the critical region
			}
		}
	leave_cr(nl_sem, 2);
	
	return;
}		
/*********************************************************************************************/
void adjust_routing_table(int8_t index)
{
	// this function is already called from a thread-safe function. No need to use semaphores
	int8_t i;
	RoutingTableEntry temp = rt.rte[index];
	
	// this implements the LRU policy
	for(i = index; i > 0; i--)
		rt.rte[i] = rt.rte[i-1];
		
	rt.rte[0] = temp;
	
	return;
}
				
/*********************************************************************************************/
uint16_t route_addr(uint16_t addr)					// CHECKED
{
	int8_t i;			// loop index
	uint16_t nextHop;   // to hold the address of the next-hop node
		
	// account for the BCAST case first
	if(addr == BCAST_ADDR)
		return BCAST_ADDR;
	
	enter_cr(nl_sem, 3);	// acquire the NL semaphore
	
	// go through the routing table to see if a mapping for the dest address exists
	for(i = 0; i < NUM_ROUTING_TABLE_ENTRIES; i++)
	{
		if(rt.rte[i].dest != INVALID_ADDR)		// entry is valid
			if((rt.rte)[i].dest == addr)		// an entry exists for the requested address
			{
				nextHop = rt.rte[i].nextHop;	// store the next hop address to be returned
				if(nextHop == BCAST_ADDR)		// sanity check
				{
					nrk_int_disable();
					nrk_led_set(RED_LED);
					nrk_kprintf(PSTR("NL: route_addr(): Fatal bug detected in the routing table\r\n"));
				}
				adjust_routing_table(i);		// put the entry at this index position at the top of the table
				
				leave_cr(nl_sem, 3);			// release the NL semaphore
				return nextHop;					// return the next hop
			}
	}
	// if the code reaches this point, it means a mapping was not found	
	leave_cr(nl_sem, 3);	// release the NL semaphore	
	return BCAST_ADDR;		// if no mapping exists, the packet has to be broadcast.
}
/**********************************************************************************************/
void route_packet(NW_Packet *pkt)					// CHECKED	
{
	int8_t ret;										// holds the return type of calls
	
	// this function is only entered when nextHop is either equal to NODE_ADDR or BCAST_ADDR
	if( (pkt -> nextHop != NODE_ADDR) && (pkt -> nextHop != BCAST_ADDR) )
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
	
		nrk_kprintf(PSTR("NL: route_packet(): Bug2 detected in implementation of routing\r\n"));
		print_pkt(pkt);
		nrk_led_toggle(ORANGE_LED);
	}		
	
	pkt -> ttl--;									// decrement the TTL value
	if(pkt -> ttl == 0)								// the TTL value expired. Drop the packet
		return;										// this is to protect against corrupt routing tables
		  											// and to deal with BCAST messages
	pkt -> prevprevHop = pkt -> prevHop;		    // this node sent the packet to me
	pkt -> prevHop = NODE_ADDR;						// Now I am multi-hopping the packet
	
	// nextHop could be BCAST_ADDR or NODE_ADDR
	if(pkt -> nextHop != BCAST_ADDR)				// if nextHop == BCAST_ADDR, gateway should know about this
		pkt -> nextHop = route_addr(pkt -> dest);	// which neighbor should receive it?
	
	else if(CONNECTED_TO_GATEWAY == TRUE) 			// i.e. the nextHop is BCAST_ADDR.  
		 {
		 	// construct a ROUTE_REQUEST packet to be sent to the gateway
		 	mrrq.src = pkt -> src;
		 	mrrq.dest = pkt -> dest;
		 	
		 	// construct a packet to be sent to the gateway over the serial connection 
			ntg_pkt.type = SERIAL_ROUTE_REQUEST;
			ntg_pkt.length = SIZE_MSG_ROUTE_REQUEST;
			// pack the message in the data field of the NodeToGatewaySerial_Packet 
			pack_Msg_RouteRequest(ntg_pkt.data, &mrrq);
			// pack the NodeToGatewaySerial_Packet header into the serial transmit buffer 
			pack_NodeToGatewaySerial_Packet_header(to_gw_buf_rx, &ntg_pkt);
			// append the payload into the serial transmit buffer 
			memcpy(to_gw_buf_rx + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, MAX_SERIAL_PAYLOAD);  
			sendToSerial(to_gw_buf_rx, SIZE_NODETOGATEWAYSERIAL_PACKET);
		 } // end if
		 	
	enter_cr(bm_sem, 4);							// acquire the BM semaphore
	ret = insert_tx_aq(pkt);						// insert the packet into the TX queue
	leave_cr(bm_sem, 4);							// release the BM semaphore
	
	if(ret == NRK_ERROR)
		record_tx_queue_full(pkt);					// record this fact for statistics collection
		
	return;
}
/**********************************************************************************************/
int8_t sendToGateway(uint8_t *ptr, int8_t len)	// user API // CHECKED
{
	
	if(len <= 0 || len > MAX_SERIAL_PAYLOAD || ptr == NULL || CONNECTED_TO_GATEWAY == FALSE)	// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);	// set the error number
		return NRK_ERROR;					// return
	}
	
	// fill up the members of the transmit buffer 
	gw_buf[0] = SERIAL_APPLICATION;
	gw_buf[1] = len;
	memcpy(gw_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ptr, len);
	
	sendToSerial(gw_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);		// send the message to the gateway
	
	return NRK_OK;
}
/**********************************************************************************************/
uint8_t pkt_type(NW_Packet *pkt)					// CHECKED
{
	switch(pkt -> type)
	{
		case UDP:
			return APPLICATION;
			
		case HELLO:
		case NGB_LIST:
		case ROUTE_CONFIG:
		case ROUTE_REPLY:
		case NW_INFO_ACQUIRED:
		case SEND_NW_INFO:
		case SEND_NODE_INFO:
		case NODE_INFO_ACQUIRED:
		case NODE_INFO:
			return NW_CONTROL;
				
	}
	return INVALID;	// invalid packet header received 
}
/***********************************************************************************************/
uint8_t tl_type(uint8_t type)						// CHECKED
{
	if(type == UDP)
		return UDP;
	if(type == TCP)
		return TCP;
				
	return INVALID;		// invalid transport layer type received 
}
/***********************************************************************************************/
/********************************************************************************************/
void process_app_pkt(NW_Packet *pkt, int8_t rssi)	
{
	int8_t ret;			// to hold the return value of various function calls
	
	if(tl_type(pkt -> type) == UDP)				// the TL protocol is UDP 
	{
		// unpack the UDP header					
		unpack_TL_UDP_header(&udp_seg, pkt -> data); 
		// copy the application payload into the data section of the UDP segment					
		memcpy(udp_seg.data, pkt -> data + SIZE_TRANSPORT_UDP_HEADER, MAX_APP_PAYLOAD);  
		
		// check to see if the destination port in the header is associated with any socket 
		if(is_port_associated(udp_seg.destPort) == TRUE)	// yes there is 
		{
			int8_t port_index;				// to hold the index of the relevant port element
			
			enter_cr(bm_sem, 5);			// acquire the semaphore (buffer manager)
			enter_cr(tl_sem, 6);			// acquire the semaphore (transport layer)
				
			insert_rx_pq(&udp_seg, pkt -> prio, pkt -> src, rssi);	// insert the packet into the appropriate rx queue
						
			port_index = port_to_port_index(udp_seg.destPort); 		// extract the relevant port element 
			if(port_index == NRK_ERROR)	//sanity check for debugging
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("NL: process_app_pkt(): Bug detected in implementation of port/rbm element array: ERROR\r\n"));
			}			
			ret = nrk_event_signal(ports[port_index].data_arrived_signal);	// signal 'data arrived'
			if(ret == NRK_ERROR)
			{
				if(nrk_errno_get() == 1)	// this means the signal was not created. This is a bug
				{
					nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
						nrk_kprintf(PSTR("NL: process_app_pkt(): Bug detected in implementation of port signals: ERROR\r\n"));
				}				
			} // end if(ret == NRK_ERROR)
			
			leave_cr(tl_sem, 6);						// release the semaphore (transport layer)
			leave_cr(bm_sem, 5);						// release the semaphore (buffer manager)
		} // end if(is_port_associated())
	
		else // if there is no socket associated with this port, simply drop the packet
		{
			record_unassociated_socket_pkt(pkt);	// record this fact 
		}
		
		
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
	int8_t i;
	int8_t ret;
	
	//if(CONNECTED_TO_GATEWAY == TRUE)
	//	printf("NL: Entered process_nw_ctrl_pkt(): %u\r\n", pkt -> type);
	switch(pkt -> type)
	{
		case HELLO:		// HELLO msg
			// The neighbor information has already been updated in nl_rx_task. Just retrieve the 
			// address of the default gateway
			// unpack the Msg_Hello from the packet payload 
			unpack_Msg_Hello(&mh, pkt -> data);
			enter_cr(nl_sem, 7);				// acquire the NL semaphore
			if(DEFAULT_GATEWAY == BCAST_ADDR)	// I do not know address of DG as yet
				DEFAULT_GATEWAY = mh.dg;		// get the address of the default gateway
			leave_cr(nl_sem, 7);				// release the NL semaphore
			
			if(DEBUG_NL >= 1)
			{
				nrk_kprintf(PSTR("NL: process_nw_ctrl_pkt(): Received HELLO msg from: "));
				printf("%d ", mh.addr);
				//nrk_kprintf(PSTR("with RSSI = "));
				//printf("%d\r\n",rssi);
				print_dg();
			}
			
			break;

		// this part of the code will only be executed by the gateway node
		case NGB_LIST:		// NGB_LIST msg
			
			// unpack the Msg_NgbList from the packet payload. Only required for display purposes						
			unpack_Msg_NgbList(&mnlist, pkt -> data);
			nlist = mnlist.nl;
			
			if(DEBUG_NL >= 0)
			{
				//printf("Destination = %u   ", pkt -> dest);
				nrk_kprintf(PSTR("NL: RX NGB_LIST from "));
				//nrk_kprintf(PSTR("NL: process_nw_ctrl_pkt(): Received NGB_LIST msg from "));
				printf("%d with count = %d\n[", nlist.addr, nlist.count);
				for(i = 0; i < MAX_NGBS; i++)		
				{
					if(nlist.ngbs[i].addr != INVALID_ADDR)				// valid entry
						printf("%u, ", nlist.ngbs[i].addr);
				}
				printf("]\r\n");
			}
			
			if(CONNECTED_TO_GATEWAY == TRUE)	// this node is connected to the gateway 				
			{		
				// construct a packet to be sent to the gateway over the serial connection 
				ntg_pkt.type = SERIAL_NGB_LIST;
				ntg_pkt.length = SIZE_MSG_NGB_LIST;
				// pack the message in the data field of the NodeToGatewaySerial_Packet 
				pack_Msg_NgbList(ntg_pkt.data, &mnlist);
				// pack the NodeToGatewaySerial_Packet header into the serial transmit buffer 
				pack_NodeToGatewaySerial_Packet_header(to_gw_buf_rx, &ntg_pkt);
				// append the payload into the serial transmit buffer 
				memcpy(to_gw_buf_rx + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, MAX_SERIAL_PAYLOAD); 
				// send this buffer to the gateway
				sendToSerial(to_gw_buf_rx, SIZE_NODETOGATEWAYSERIAL_PACKET);
				
				if(pkt -> nextHop == BCAST_ADDR)	// sender's routing tables need to be made
		 		{
		 			mrrq.src = pkt -> src;			// construct a Msg_RouteRequest
		 			mrrq.dest = pkt -> dest;
		 	
		 			// construct a packet to be sent to the gateway over the serial connection 
					ntg_pkt.type = SERIAL_ROUTE_REQUEST;
					ntg_pkt.length = SIZE_MSG_ROUTE_REQUEST;
					// pack the message in the data field of the NodeToGatewaySerial_Packet 
					pack_Msg_RouteRequest(ntg_pkt.data, &mrrq);
					// pack the NodeToGatewaySerial_Packet header into the serial transmit buffer 
					pack_NodeToGatewaySerial_Packet_header(to_gw_buf_rx, &ntg_pkt);
					// append the payload into the serial transmit buffer 
					memcpy(to_gw_buf_rx + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, MAX_SERIAL_PAYLOAD);  
					sendToSerial(to_gw_buf_rx, SIZE_NODETOGATEWAYSERIAL_PACKET); //for debugging
		 		} // end inner if
			} 
			else		// this node is not connected to the gateway 
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				printf("NL: process_nw_ctrl_pkt(): Bug detected in implementation of nl_rx_task()1\r\n");
				
				//route_packet(pkt);		// multihop this packet into the network
			}			
			break;
			
		// this part of the code is only executed by the gateway
		case NODE_INFO:
			// unpack the Msg_NodeInfo from the packet payload. Only required for display purposes						
			unpack_Msg_NodeInfo(&mni, pkt -> data);
			
			if(DEBUG_NL >= 0)
			{
				nrk_kprintf(PSTR("NL: RX NODE_INFO from "));
				printf("%u\r\n", mni.addr);
				print_NodeStateQueue(&(mni.nsq));
			}
			
			if(CONNECTED_TO_GATEWAY == TRUE)	// this node is connected to the gateway 				
			{		
				// construct a packet to be sent to the gateway over the serial connection 
				ntg_pkt.type = SERIAL_NODE_INFO;
				ntg_pkt.length = SIZE_MSG_NODE_INFO;
				// pack the message in the data field of the NodeToGatewaySerial_Packet 
				pack_Msg_NodeInfo(ntg_pkt.data, &mni);
				// pack the NodeToGatewaySerial_Packet header into the serial transmit buffer 
				pack_NodeToGatewaySerial_Packet_header(to_gw_buf_rx, &ntg_pkt);
				// append the payload into the serial transmit buffer 
				memcpy(to_gw_buf_rx + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, MAX_SERIAL_PAYLOAD); 
				// send this buffer to the gateway
				sendToSerial(to_gw_buf_rx, SIZE_NODETOGATEWAYSERIAL_PACKET);
				
				if(pkt -> nextHop == BCAST_ADDR)	// sender's routing tables need to be made
		 		{
		 			mrrq.src = pkt -> src;			// construct a Msg_RouteRequest
		 			mrrq.dest = pkt -> dest;
		 	
		 			// construct a packet to be sent to the gateway over the serial connection 
					ntg_pkt.type = SERIAL_ROUTE_REQUEST;
					ntg_pkt.length = SIZE_MSG_ROUTE_REQUEST;
					// pack the message in the data field of the NodeToGatewaySerial_Packet 
					pack_Msg_RouteRequest(ntg_pkt.data, &mrrq);
					// pack the NodeToGatewaySerial_Packet header into the serial transmit buffer 
					pack_NodeToGatewaySerial_Packet_header(to_gw_buf_rx, &ntg_pkt);
					// append the payload into the serial transmit buffer 
					memcpy(to_gw_buf_rx + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, MAX_SERIAL_PAYLOAD);  
					sendToSerial(to_gw_buf_rx, SIZE_NODETOGATEWAYSERIAL_PACKET); //for debugging
		 		} // end inner if
			} 
			else		// this node is not connected to the gateway 
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				printf("NL: process_nw_ctrl_pkt(): Bug detected in implementation of nl_rx_task()2\r\n");
				
			}			
			break;
			
		case ROUTE_REPLY:
			if(DEBUG_NL == 0)
				nrk_kprintf(PSTR("NL: RX ROUTE_REPLY msg \r\n"));
			unpack_Msg_RouteReply(&mrr, pkt -> data);
			enter_cr(nl_sem, 34);		// acquire the semaphore
			DEFAULT_GATEWAY = mrr.dg;	// get the value of DEFAULT_GATEWAY in any case
			leave_cr(nl_sem, 34);		// release the semaphore
			
			ret = process_Msg_RouteReply(&mrr);
			if(ret == 1)
			{
				print_Msg_RouteReply(&mrr);
				if(DEBUG_NL == 0)
					print_RoutingTable();
				
				route_packet(pkt);		// finally route this packet
			}
			
			break;			
			
		case SEND_NW_INFO:
			//the gateway has requested for network topology information to be sent
			unpack_Msg_SendNwInfo(&msnwi, pkt -> data);
			if(DEBUG_NL == 0)
			{
				nrk_kprintf(PSTR("NL: RX SEND_NW_INFO: "));
				printf("%u\r\n", msnwi.seq_no);
			}
					
			ret = process_Msg_SendNwInfo(&msnwi);
			if(DEBUG_NL >= 1)
			{
				nrk_kprintf(PSTR("NL: process_nw_ctrl_pkt(): Calling route_packet from NW1\r\n"));
				printf("%x\r\n", pkt -> type);
			}
			if(ret == 1)
				route_packet(pkt);
			break;	
			
		case NW_INFO_ACQUIRED:
			// the gateway has received information about the network. 
			unpack_Msg_NwInfoAcquired(&mnwia, pkt -> data);
			if(DEBUG_NL == 0)
			{
				nrk_kprintf(PSTR("NL: RX NW_INFO_ACQUIRED: "));
				printf("%u\r\n", mnwia.seq_no);
			}
			ret = process_Msg_NwInfoAcquired(&mnwia);
			
			if(DEBUG_NL >= 1)
			{
				nrk_kprintf(PSTR("NL: process_nw_ctrl_pkt(): Calling route_packet from NW2\r\n"));
				printf("%x\r\n", pkt -> type);
			}
			if(ret == 1)
				route_packet(pkt);
			break;	
			
		case SEND_NODE_INFO:
			
			unpack_Msg_SendNodeInfo(&msni, pkt -> data);
			if(DEBUG_NL == 0)
			{
				nrk_kprintf(PSTR("NL: RX SEND_NODE_INFO: "));
				printf("%u\r\n", msni.seq_no);
			}
			
			ret = process_Msg_SendNodeInfo(&msni);
			if(DEBUG_NL >= 1)
			{
				nrk_kprintf(PSTR("NL: process_nw_ctrl_pkt(): Calling route_packet from NW3\r\n"));
				printf("%x\r\n", pkt -> type);
			}
			if(ret == 1)
				route_packet(pkt);
			break;	
			
		case NODE_INFO_ACQUIRED:
			// the gateway has received information about node state. 
			unpack_Msg_NodeInfoAcquired(&mnia, pkt -> data);
			if(DEBUG_NL == 0)
			{
				nrk_kprintf(PSTR("NL: RX NODE_INFO_ACQUIRED: "));
				printf("%u\r\n", mnia.seq_no);
			}
			
			ret = process_Msg_NodeInfoAcquired(&mnia);
			if(DEBUG_NL >= 1)
			{
				nrk_kprintf(PSTR("NL: process_nw_ctrl_pkt(): Calling route_packet from NW4\r\n"));
				printf("%x\r\n", pkt -> type);
			}
			if(ret == 1)
				route_packet(pkt);
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
void build_Msg_Hello(Msg_Hello *m)	// CHECKED
{
	m -> addr = (uint16_t)NODE_ADDR;
	
	//nrk_kprintf(PSTR("NL: build_Msg_Hello(): "));
	//print_dg();
	
	enter_cr(nl_sem, 8);
	m -> dg = DEFAULT_GATEWAY;
	leave_cr(nl_sem, 8);
	
	// Build the network packet which will hold the HELLO message
  	pkt_tx.src = (uint16_t)NODE_ADDR;
  	pkt_tx.dest = (uint16_t)BCAST_ADDR;
  	pkt_tx.nextHop = (uint16_t)BCAST_ADDR;	// FIX ME.. Should nextHop = INVALID_ADDR
  	pkt_tx.prevHop = (uint16_t)NODE_ADDR;
  	pkt_tx.prevprevHop = (uint16_t)NODE_ADDR;
  	
  	pkt_tx.ttl = 1;		
  	pkt_tx.type = (uint8_t)HELLO;
  	pkt_tx.length = SIZE_MSG_HELLO;
  	pkt_tx.prio = NORMAL_PRIORITY;
  	
  	pack_Msg_Hello(pkt_tx.data, m);
	return;
}
/*********************************************************************************************/
void build_Msg_NgbList(Msg_NgbList *m)	// CHECKED
{
	enter_cr(nl_sem, 9);	// acquire the semaphore
	m -> nl = nl;			// copy the NeighborList into the message
	leave_cr(nl_sem, 9);	// release the semaphore
	
	// Build the network packet which will hold the NGB_LIST message
  	pkt_tx.src = (uint16_t)NODE_ADDR;
  	enter_cr(nl_sem, 10);	// acquire the semaphore
  	pkt_tx.dest = DEFAULT_GATEWAY;	// assign the default gateway
  	if(DEFAULT_GATEWAY == BCAST_ADDR)	// for debugging
  	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
  		/*while(1)
  		{
  			nrk_kprintf(PSTR("BMNL: Error in DG = "));
  			printf("%u\n", DEFAULT_GATEWAY);
  			nrk_led_toggle(ORANGE_LED);
  		}
  		*/
  		nrk_kprintf(PSTR("BMNL:Error in DG = "));
  		printf("%u\n", DEFAULT_GATEWAY);
  	}
  	leave_cr(nl_sem, 10);	// release the semaphore
  	
  	pkt_tx.nextHop = route_addr(pkt_tx.dest);
  	pkt_tx.prevHop = (uint16_t)NODE_ADDR;
  	pkt_tx.prevprevHop = (uint16_t)NODE_ADDR;
  	
  	pkt_tx.ttl = MAX_NETWORK_DIAMETER;		
  	pkt_tx.type = (uint8_t)NGB_LIST;
  	pkt_tx.length = SIZE_MSG_NGB_LIST;
  	pkt_tx.prio = NORMAL_PRIORITY;
  	
  	pack_Msg_NgbList(pkt_tx.data, m);
  	
  	return;
}				
/**********************************************************************************************/
void build_Msg_NodeInfo(Msg_NodeInfo *m)	// CHECKED
{
	// Build the network packet which will hold the NODE_INFO message
  	m -> addr = NODE_ADDR;
  	m -> nsq = nsq;
  	  	
  	pkt_tx.src = (uint16_t)NODE_ADDR;
  	enter_cr(nl_sem, 10);	// acquire the semaphore
  	pkt_tx.dest = DEFAULT_GATEWAY;	// assign the default gateway
  	leave_cr(nl_sem, 10);	// release the semaphore
  	
  	if(pkt_tx.dest == BCAST_ADDR)	// for debugging
  	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
  		/*while(1)
  		{
  			nrk_kprintf(PSTR("BMNL: Error in DG = "));
  			printf("%u\n", DEFAULT_GATEWAY);
  			nrk_led_toggle(ORANGE_LED);
  		}
  		*/
  		nrk_kprintf(PSTR("BMNI:Error in DG = "));
  		printf("%u\n", DEFAULT_GATEWAY);
  	}
  	
  	pkt_tx.nextHop = route_addr(pkt_tx.dest);
  	pkt_tx.prevHop = (uint16_t)NODE_ADDR;
  	pkt_tx.prevprevHop = (uint16_t)NODE_ADDR;
  	
  	pkt_tx.ttl = MAX_NETWORK_DIAMETER;		
  	pkt_tx.type = (uint8_t)NODE_INFO;
  	pkt_tx.length = SIZE_MSG_NODE_INFO;
  	pkt_tx.prio = NORMAL_PRIORITY;
  	
  	pack_Msg_NodeInfo(pkt_tx.data, m);
  	
  	return;
}				
/**********************************************************************************************/	
void nl_rx_task()	// CHECKED
{
	/**** Variable declarations *****/
	uint8_t len; 					// to hold the size of the received packet, always = sizeof(NW_Packet) 
	int8_t rssi;  			   		// to hold rssi of received packet
	uint8_t *local_rx_buf;	   		// pointer to receive buffer of link layer
	int8_t val;						// status variable to hold the return type of function calls
	int8_t pkt_len_flag;			// to indicate whether a wrong-length packet was received from the link layer
	uint8_t packet_type;			// to hold the type of packet received
	
	/***** Start processing *****/
	
	// initialise the link layer	
	val = initialise_link_layer((uint8_t)TX_CHANNEL);
	if(val == NRK_ERROR)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("NL:nl_rx_task(): Error returned by initialise_link_layer()\r\n"));
	}	
	
	nrk_kprintf(PSTR("NL: nl_rx_task(): Started BMAC\r\n"));
		
	// give the link layer a rx buffer 	
	val = bmac_rx_pkt_set_buffer(rx_buf, RF_BUFFER_SIZE);
	if(val == NRK_ERROR)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("NL:nl_rx_task(): Error returned by bmac_rx_pkt_set_buffer()\r\n"));
	}
		
	// start processing forever 
	while(1)
	{
		pkt_len_flag = VALID_PKT_LENGTH;	// assume initially a valid-length packet will be received
		
		// wait for the next packet 		
		while(bmac_rx_pkt_ready() == 0)
			val = bmac_wait_until_rx_pkt();
						
		// Get the packet 
	   	do
	   	{
	   		local_rx_buf = bmac_rx_pkt_get(&len, &rssi);
	   		if(local_rx_buf == NULL)
	   		{
	   			nrk_int_disable();
	   			nrk_led_set(RED_LED);
	   			while(1)
	   				nrk_kprintf(PSTR("NL:nl_rx_task(): NULL returned by bmac_rx_pkt_get(): ERROR\r\n"));
	   		}
	   		
	   	}while(local_rx_buf == NULL);
	   	
	   	if(len != SIZE_NW_PACKET)	// bad packet 
	   	{
	   		if(DEBUG_NL >= 0)
	   		{
	   			nrk_kprintf(PSTR("NL: nl_rx_task(): Wrong length of packet received: "));
	   			printf("%d: ERROR\r\n", len);
	   		}
	   		pkt_len_flag = INVALID_PKT_LENGTH;			// mark this event with the flag variable
	   	}
	   	
	   	if(DEBUG_NL >= 0)
	   		nrk_led_set(GREEN_LED);		// set the GREEN LED to indicate the packet is received
	   			  	
		if(pkt_len_flag == INVALID_PKT_LENGTH)
		{
			bmac_rx_pkt_release();	// drop the packet and go wait for another
			nrk_led_clr(GREEN_LED);
			continue;
		}
			   	
		// unpack the packet header from the received buffer 
		unpack_NW_Packet_header(&pkt_rx, local_rx_buf);
		// copy the packet payload to the data field of the local packet
		memcpy(pkt_rx.data, local_rx_buf + SIZE_NW_PACKET_HEADER, MAX_NETWORK_PAYLOAD);
		// Release the RX buffer quickly so future packets can arrive
		bmac_rx_pkt_release();	
			
		// update neighbor information from this packet header if possible
		ngb.addr = pkt_rx.prevHop;
		ngb.rssi = rssi;
		val = add_neighbor(ngb);
		if(val == NRK_ERROR)					// not possible to add the new neighbor
		{
			record_max_ngb_limit_reached(&pkt_rx);	// record this fact
		} 
		
		// begin processing this packet
		// It is necessary to seperate the cases based on packet type to filter out bogus packets
		packet_type = pkt_type(&pkt_rx);
		if(packet_type == APPLICATION)	// its an application layer packet 
		{
			// case 1: prevprevHop/src is NODE_ADDR (Did I send the packet?)
			if(pkt_rx.prevprevHop == NODE_ADDR || pkt_rx.src == NODE_ADDR)
				; // do nothing, drop this packet. It is on its way through the network
			else  // prevprevHop and src is someone else
			{
				// case 2: Destination is NODE_ADDR or BCAST_ADDR
				if(pkt_rx.dest == NODE_ADDR || pkt_rx.dest == BCAST_ADDR)
					process_app_pkt(&pkt_rx, rssi);
					
				// case 3: I am enroute to the destination
				else if(pkt_rx.nextHop == NODE_ADDR)
					 {
					 	nrk_kprintf(PSTR("NL: Calling route_packet from APP1\r\n"));
					 	route_packet(&pkt_rx);		// route this packet
					 }
					 // case 4: Sender's routing tables still not made
					 else if(pkt_rx.nextHop == BCAST_ADDR)	
						  {
						  	nrk_kprintf(PSTR("NL: Calling route_packet from APP2\r\n"));
					 	  	route_packet(&pkt_rx);
						  }
					 	  else
						  	;		 // drop all other packets
			} // end else								
		} // end if(type == APPLICATION)
		else if(packet_type == NW_CONTROL)	// its a network control packet 
			 {
				// case 1: prevprevHop/src is NODE_ADDR (Did I send this packet?)
				if(pkt_rx.prevprevHop == NODE_ADDR || pkt_rx.src == NODE_ADDR)
					;	// do nothing, drop this packet
				else
				{
					// case 2: Destination is NODE_ADDR or BCAST_ADDR
				  	if(pkt_rx.dest == NODE_ADDR || pkt_rx.dest == BCAST_ADDR)
				  		process_nw_ctrl_pkt(&pkt_rx, rssi);
				  	
				  	// case 3: I am enroute to a destination
				  	else if(pkt_rx.nextHop == NODE_ADDR)
				  		 {
				  		 	if(DEBUG_NL == 1)
				  		 	{
				  		 		nrk_kprintf(PSTR("NL: nl_rx_task(): Calling route_packet from NW1: "));
				  		 		printf("%x\r\n", pkt_rx.type);
				  		 	}
					 	 	route_packet(&pkt_rx);
				  		 }
					  	 // case 4: Sender's routing tables still not made 
					    else if(pkt_rx.nextHop == BCAST_ADDR)
					  	 	 {
					  	 	 	if(DEBUG_NL == 1)
					  	 	 	{
					  	 	 		nrk_kprintf(PSTR("NL: nl_rx_task(): Calling route_packet from NW2: "));
					  	 	 		printf("%x\r\n", pkt_rx.type);
					  	 	 	}
					 	 	 	route_packet(&pkt_rx);
					  	 	 }					  	 	 	
					  	     else
					  			;	// drop all other packets
				 } // end else
					 
			  } // end if(type == NW_CONTROL)
			  else	// unknown packet type 
			  {
			  		if(DEBUG_NL >= 0)
			  		{
			  			nrk_kprintf(PSTR("NL:nl_rx_task(): Unknown pkt type received = "));
			  			printf("%d\r\n", packet_type);
			  		}
			  }
			  nrk_led_clr(GREEN_LED);		// finished processing this packet			
	 } // end while(1)
	
	return;
}	// end nl_rx_task
	
/*********************************************************************************************/
void nl_tx_task()	// CHECKED
{
	TransmitBuffer *ptr = NULL; 				// pointer to the buffer to be transmitted 
	nrk_sig_t tx_done_signal;					// to hold the tx_done signal from the link layer 
	int8_t ret;									// to hold the return value of various functions 
	int8_t port_index;							// to store the index of the corresponding port element 
	int8_t isApplication;						// flag to indicate whether the packet in the transmit 
												// buffer is an APPLICATION / NW_CONTROL packet
	 
	nrk_time_t ll_timeout;						// used for blocking while LL sends one packet
	
	nrk_time_t start_hello;						// used for sending network control messages (HELLO) 
	nrk_time_t end_hello;	
	nrk_time_t elapsed_hello;
	
	nrk_time_t start_ngblist;	 				// used for sending network control messages (NGB_LIST) 
	nrk_time_t end_ngblist;				
	nrk_time_t elapsed_ngblist;
	
	nrk_time_t start_update_NgbList;			// timer used to update NgbList
	nrk_time_t end_update_NgbList;
	nrk_time_t elapsed_update_NgbList;
	
	nrk_time_t start_nodeinfo;					// used for sending  network control messages (NODE_INFO)
	nrk_time_t end_nodeinfo;
	nrk_time_t elapsed_nodeinfo;
	
	//uint32_t i = 0;
	
	// wait for the nl_rx_task to start bmac 	
	while(!bmac_started())
	{
		if(DEBUG_NL >= 0)
		{
			nrk_kprintf(PSTR("NL: nl_tx_task(): Waiting for the link layer to start\r\n"));
		    nrk_wait_until_next_period();
		}
	}	
	// retrieve and register for the 'transmit done' signal from bmac 	
	tx_done_signal = bmac_get_tx_done_signal();
  	
  	if( nrk_signal_register(tx_done_signal) == NRK_ERROR )
  	{
  		nrk_int_disable();
  		nrk_led_set(RED_LED);
  		while(1)
  			nrk_kprintf(PSTR("NL: nl_tx_task(): Error while registering for the bmax_tx_done_signal\r\n"));
  	}
  	// set the radio power
	if( bmac_set_rf_power(TX_RADIO_POWER) == NRK_ERROR)
  	{
  		nrk_led_set(RED_LED);
  		nrk_int_disable();
  		while(1)
  			nrk_kprintf(PSTR("NL: nl_tx_task(): Error setting the transmit power\r\n"));
  	}
	
	// set the HELLO_msg timer
	nrk_time_get(&start_hello);
	end_hello.secs = start_hello.secs;
	end_hello.nano_secs = start_hello.nano_secs;
	
	// set the NGB_LIST timer
	nrk_time_get(&start_ngblist);
	end_ngblist.secs = start_ngblist.secs;
	end_ngblist.nano_secs = start_ngblist.nano_secs;
	
	// set the update_NgbList timer
	nrk_time_get(&start_update_NgbList);
	end_update_NgbList.secs = start_update_NgbList.secs;
	end_update_NgbList.nano_secs = start_update_NgbList.nano_secs;
	
	// set the NODE_INFO timer
	nrk_time_get(&start_nodeinfo);
	end_nodeinfo.secs = start_nodeinfo.secs;
	end_nodeinfo.nano_secs = start_nodeinfo.nano_secs;
		
	while(1)	// begin processing forever
	{
		isApplication = FALSE;	// assume at the beginning that a nw_ctrl pkt will be transmitted 
		
		// still need to send HELLO messages?
		if( get_tx_aq_size() == 0 )				// this node is not going to send any message
		{
			ret = nrk_time_sub(&elapsed_hello, end_hello, start_hello);	// how much time has passed?
			if(ret == 0)
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("NL: nl_tx_task(): Error returned by nrk_time_sub(HELLO time)\r\n"));
			}
			nrk_time_compact_nanos(&elapsed_hello);
			if(elapsed_hello.secs >= HELLO_PERIOD)		
			{	
				build_Msg_Hello(&mhe);					// build the 'HELLO' message
					
				enter_cr(bm_sem, 11);					// acquire the semaphore (buffer manager)				
				ret = insert_tx_aq(&pkt_tx);  			// insert it into the transmit queue			
				leave_cr(bm_sem, 11);					// release the semaphore (buffer manager)
				if(ret == NRK_ERROR)
					nrk_kprintf(PSTR("Unable to insert HELLO message into trasmit queue\r\n"));
			
				start_hello.secs = end_hello.secs;		// reset the HELLO_MSG timer
				start_hello.nano_secs = end_hello.nano_secs; 
				if(DEBUG_NL >= 1)
					nrk_kprintf(PSTR("NL: nl_tx_task(): Sending HELLO message\r\n"));
			}
		}		
		// still need to send NGB_LIST messages?
		if(get_continue_sending_ngblist() == 1 && ngb_list_pkt_queued == 0 )		
		{
			ret = nrk_time_sub(&elapsed_ngblist, end_ngblist, start_ngblist);	// how much time has passed?
			if(ret == 0)
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("NL: nl_tx_task(): Error returned by nrk_time_sub(NGB_LIST time)\r\n"));
			}
			nrk_time_compact_nanos(&elapsed_ngblist);
			if(elapsed_ngblist.secs >= NGB_LIST_PERIOD)
		  	{
				if(CONNECTED_TO_GATEWAY == FALSE)		// send the NGB_LIST message over the air
				{
					build_Msg_NgbList(&mn);				// build the 'NGB_LIST' message
					
					if(DEBUG_NL >= 1)
					{
						printf("NL: TX NGB_LIST air: %u\r\n", pkt_tx.dest);
						//nrk_kprintf(PSTR("NL: nl_tx_task(): Sending NGB_LIST message over the air\r\n"));
					 	//printf("NL: nl_tx_task(): NeighborList: ");
						print_NgbList(&(mn.nl));
					}			
					enter_cr(bm_sem, 12);				// acquire the semaphore (buffer manager)
					ret = insert_tx_aq(&pkt_tx);  		// insert it into the transmit queue
					leave_cr(bm_sem, 12);				// release the semaphore (buffer manager)
					if(ret == NRK_ERROR)
						nrk_kprintf(PSTR("Unable to insert NGB_LIST into transmit queue\r\n"));
					else
						ngb_list_pkt_queued = 1;		// a NGB_LIST message is in the queue		
				}// end if
				else 									// send the NGB_LIST message over the serial port
				{
					if(DEBUG_NL >= 1)
						nrk_kprintf(PSTR("NL: nl_tx_task(): Sending NGB_LIST message over the serial port\r\n"));
					
					ntg_pkt.type = SERIAL_NGB_LIST;		// prepare the SERIAL_NGB_LIST message to be sent to gateway
					ntg_pkt.length = SIZE_MSG_NGB_LIST;
			
					enter_cr(nl_sem, 13);
					mn.nl = nl;
					leave_cr(nl_sem, 13);
					pack_Msg_NgbList(ntg_pkt.data, &mn);
						
					pack_NodeToGatewaySerial_Packet_header(to_gw_buf_tx, &ntg_pkt);
					memcpy(to_gw_buf_tx + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, MAX_SERIAL_PAYLOAD);
					sendToSerial(to_gw_buf_tx, SIZE_NODETOGATEWAYSERIAL_PACKET);
				} // end else
			
				start_ngblist.secs = end_ngblist.secs;		// reset the NGB_LIST message timer
				start_ngblist.nano_secs = end_ngblist.nano_secs; 
			 }// end if	
		}// end if
		
		// still need to send NODE_INFO messages?
		if(get_continue_sending_nodeinfo() == 1 && nodeinfo_pkt_queued == 0 )		
		{
			ret = nrk_time_sub(&elapsed_nodeinfo, end_nodeinfo, start_nodeinfo);	// how much time has passed?
			if(ret == 0)
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("NL: nl_tx_task(): Error returned by nrk_time_sub(NODE_INFO time)\r\n"));
			}
			nrk_time_compact_nanos(&elapsed_nodeinfo);
			if(elapsed_nodeinfo.secs >= NODE_INFO_PERIOD)
		  	{
				if(CONNECTED_TO_GATEWAY == FALSE)		// send the NODE_INFO message over the air
				{
					build_Msg_NodeInfo(&mnode);			// build the 'NODE_INFO' message
					
					if(DEBUG_NL >= 0)
					{
						printf("NL: TX NODE_INFO air: %u\r\n", pkt_tx.dest);
						//nrk_kprintf(PSTR("NL: nl_tx_task(): Sending NODE_INFO message over the air\r\n"));
						print_NodeStateQueue(&nsq);
					} 
					
					enter_cr(bm_sem, 12);				// acquire the semaphore (buffer manager)
					ret = insert_tx_aq(&pkt_tx);  		// insert it into the transmit queue
					leave_cr(bm_sem, 12);				// release the semaphore (buffer manager)
					if(ret == NRK_ERROR)
						nrk_kprintf(PSTR("Unable to insert NODE_INFO into transmit queue\r\n"));
					else
						nodeinfo_pkt_queued = 1;		// a NODE_INFO message is in the queue		
				}// end if
				else 									// send the NODE_INFO message over the serial port
				{
					if(DEBUG_NL >= 0)
						nrk_kprintf(PSTR("NL: nl_tx_task(): Sending NODE_INFO message over the serial port\r\n"));
					
					ntg_pkt.type = SERIAL_NODE_INFO;		// prepare the SERIAL_NODE_INFO message to be sent to gateway
					ntg_pkt.length = SIZE_MSG_SEND_NODE_INFO;
					
					mnode.addr = NODE_ADDR;
					mnode.nsq = nsq;				
					pack_Msg_NodeInfo(ntg_pkt.data, &mnode);
							
					pack_NodeToGatewaySerial_Packet_header(to_gw_buf_tx, &ntg_pkt);
					memcpy(to_gw_buf_tx + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, MAX_SERIAL_PAYLOAD);
					sendToSerial(to_gw_buf_tx, SIZE_NODETOGATEWAYSERIAL_PACKET);
				} // end else
			
				start_nodeinfo.secs = end_nodeinfo.secs;		// reset the NODE_INFO message timer
				start_nodeinfo.nano_secs = end_nodeinfo.nano_secs; 
			 }// end if	
		}// end if
		
		// do I need to update NeighborList information
		ret = nrk_time_sub(&elapsed_update_NgbList, end_update_NgbList, start_update_NgbList);
		if(ret == 0)
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("NL: nl_tx_task(): Error returned by nrk_time_sub(UPDATE_NGB_LIST)\r\n"));
		}
		nrk_time_compact_nanos(&elapsed_update_NgbList);
		if(elapsed_update_NgbList.secs >= UPDATE_NGB_LIST_PERIOD)
		{
			update_NgbList();
			start_update_NgbList.secs = end_update_NgbList.secs;		// reset the UPDATE_NGB_LIST message timer
			start_update_NgbList.nano_secs = end_update_NgbList.nano_secs;
		}		
		
		if(rand() % 2 == 0)						// random number generator
			collect_node_statistics();
		
		/************ PHASE 2 of the tx task **************************/
		enter_cr(bm_sem, 14);					// acquire the semaphore (buffer manager)
		ptr = remove_tx_aq();					// remove the next packet from the TX queue
		leave_cr(bm_sem, 14);					// release the semaphore (buffer manager)
		
		if(ptr == NULL)							// transmit queue is empty 
		{
			if(DEBUG_NL >= 1)
				nrk_kprintf(PSTR("NL: nl_tx_task(): Transmit queue is empty\r\n"));
			
			// update the end times 
			if(get_tx_aq_size() == 0)
				nrk_time_get(&end_hello);
			if( (get_continue_sending_ngblist() == 1) && (ngb_list_pkt_queued == 0) )
				nrk_time_get(&end_ngblist);
			if( (get_continue_sending_nodeinfo() == 1) && (nodeinfo_pkt_queued == 0) )
				nrk_time_get(&end_nodeinfo);
			nrk_time_get(&end_update_NgbList);
					
			continue;						// go back to the start of the while(1) loop
		} 

		// check the type of packet. Is it of type APPLICATION and sent by this 
		// node?  		
		if( (pkt_type(&(ptr -> pkt)) == APPLICATION) && ((ptr -> pkt).src == NODE_ADDR) )		 
		{ 		
			if(DEBUG_NL >= 0)
				nrk_kprintf(PSTR("NL: nl_tx_task(): Dequeued a self-inserted application layer packet"));
			// remove the encapsulated TL segment from the packet. This is only required so that 
			// the corresponding port number on this node can be signaled
			unpack_TL_UDP_header(&seg, (ptr -> pkt).data);
			isApplication = TRUE;	// a self-generated application layer packet is to be transmitted
		}	
		
		if((ptr -> pkt).type == NGB_LIST)	// a NGB_LIST message is going to be transmitted
			ngb_list_pkt_dequeued = 1;
		else if((ptr -> pkt).type == NODE_INFO)	// a NODE_INFO message is going to be transmitted
			nodeinfo_pkt_dequeued = 1;	
		
		// pack the network packet header into the transmit buffer to be given to link layer
		pack_NW_Packet_header(tx_buf, &(ptr -> pkt));
		// append the network payload into the transmit buffer
		memcpy(tx_buf + SIZE_NW_PACKET_HEADER, (ptr -> pkt).data, MAX_NETWORK_PAYLOAD);
		
		enter_cr(bm_sem, 15);	// acquire the semaphore (buffer manager)	
		insert_tx_fq(ptr);		// release the transmit buffer into the free queue 
		leave_cr(bm_sem, 15);	// release the semaphore (buffer manager)
		
		do
		{
			ret =  bmac_tx_pkt_nonblocking(tx_buf, SIZE_NW_PACKET);	// try to queue the buffer in link layer
			
			if(ret == NRK_ERROR)									// link layer could not accept it
			{
				nrk_kprintf(PSTR("NL: nl_tx_task(): Link layer did not accept packet\r\n"));
				nrk_wait_until_next_period();
				// FIX me..wait on tx_done_signal AND on a timeout to possibly try again
				/*if(nrk_event_wait(SIG(tx_done_signal)) == 0)		// wait for LL to finish transmitting prev packet
				{
					nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
						nrk_kprintf(PSTR("NL: nl_tx_task(): Error returned by nrk_event_wait(tx_done_signal)\r\n"));
				}*/
			}
		}while(ret == NRK_ERROR);
		
		// packet is queued at link layer
		// reset the HELLO_msg timer. This packet will serve as an implicit HELLO message
		start_hello.secs = end_hello.secs;
		start_hello.nano_secs = end_hello.nano_secs;
		
		ll_timeout.secs = TX_WAIT_PERIOD;				// set a wait period of TX_WAIT_PERIOD seconds
		ll_timeout.nano_secs = 0;
		if( nrk_signal_register(nrk_wakeup_signal) == NRK_ERROR )	// register for the 'wakeup' signal
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("NL:nl_tx_task(): Error registering for nrk_wakeup_signal\r\n"));
		}
		if( nrk_set_next_wakeup(ll_timeout) == NRK_ERROR)			// be prepared to wait for TX_WAIT_PERIOD seconds
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("NL: nl_tx(): Error returned by nrk_set_next_wakeup()\r\n"));
		}		 
		nrk_led_set(BLUE_LED);						// set the BLUE LED till LL sends the packet
  		ret = nrk_event_wait (SIG(tx_done_signal) | SIG(nrk_wakeup_signal));	// wait for its transmission  				
		if(ret == 0)
  		{
  			nrk_int_disable();
  			nrk_led_set(RED_LED);
  			while(1)
  				nrk_kprintf(PSTR("NL: nl_tx_task(): Error returned by nrk_event_wait(tx_done_signal)\r\n"));
  		}
	   if(ret & SIG(tx_done_signal)) 	// bmac has successfully sent the packet over the radio 
	   {
			if(isApplication == TRUE)		// it was an application layer packet 
			{
				enter_cr(tl_sem, 16);				
				port_index = port_to_port_index(seg.srcPort);
						
				if(port_index == NRK_ERROR)	// sanity check
				{
					nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
						nrk_kprintf(PSTR("NL: nl_tx_task: Bug detected in implementation of port element array: ERROR\r\n"));
				}
				// signal 'send done' signal 			
				if(nrk_event_signal(ports[port_index].send_done_signal) == NRK_ERROR)
				{
					if(nrk_errno_get() == 1)	// sanity check. This means signal was not created 
					{
						nrk_int_disable();
						nrk_led_set(RED_LED);
						while(1)
							nrk_kprintf(PSTR("NL: nl_tx_task: Bug detected in creating signals in port element array: ERROR\r\n"));
					}
			    }					
	           leave_cr(tl_sem, 16);	
			}// end if(isApplication == TRUE)
			
			else if(ngb_list_pkt_dequeued == 1) 	
				 {
				 	ngb_list_pkt_dequeued = 0;
				 	ngb_list_pkt_queued = 0;
				 }
				 else if(nodeinfo_pkt_dequeued == 1)
				 {
				 	nodeinfo_pkt_dequeued = 0;
				 	nodeinfo_pkt_queued = 0;
				 }
				 else  // another type of NW_CONTROL pkt OR another node's application message was sent. Do nothing
				 	; 
		} // end if(signal received == tx_done_signal)
	
		else if(ret & SIG(nrk_wakeup_signal))	
			 {
			 	nrk_kprintf(PSTR("LL did not transmit the packet within specified time\r\n"));
			 }			
			else // unknown signal caught 
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("NL: nl_tx_task(): Unknown signal caught: ERROR\r\n"));
			}
		
		nrk_led_clr(BLUE_LED);			// packet was either sent or timeout occurred	
		
		// update the end times 
		if(get_tx_aq_size() == 0)
			nrk_time_get(&end_hello);
		if( (get_continue_sending_ngblist() == 1) && (ngb_list_pkt_queued == 0) )
			nrk_time_get(&end_ngblist);
		if( (get_continue_sending_nodeinfo() == 1) && (nodeinfo_pkt_queued == 0) )
			nrk_time_get(&end_nodeinfo);
		nrk_time_get(&end_update_NgbList);
		
		nrk_wait_until_next_period();
	
   } // end while(1) 
   
	return;
}
/******************************************************************************************/
void create_network_layer_tasks()	// CHECKED
{
  NL_RX_TASK.task = nl_rx_task;
  NL_RX_TASK.Ptos = (void *) &nl_rx_task_stack[NRK_APP_STACKSIZE - 1];
  NL_RX_TASK.Pbos = (void *) &nl_rx_task_stack[0];
  NL_RX_TASK.prio = 19;
  NL_RX_TASK.FirstActivation = TRUE;
  NL_RX_TASK.Type = BASIC_TASK;
  NL_RX_TASK.SchType = PREEMPTIVE;
  
  NL_RX_TASK.cpu_reserve.secs = 1;
  NL_RX_TASK.cpu_reserve.nano_secs = 700 * NANOS_PER_MS;  
  NL_RX_TASK.period.secs = 2;
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
  
  NL_TX_TASK.cpu_reserve.secs = 3;
  NL_TX_TASK.cpu_reserve.nano_secs = 700 * NANOS_PER_MS;	
  NL_TX_TASK.period.secs = 4;
  NL_TX_TASK.period.nano_secs = 0;
  
  NL_TX_TASK.offset.secs = 0;
  NL_TX_TASK.offset.nano_secs= 0;
  nrk_activate_task (&NL_TX_TASK);

  if(DEBUG_NL == 2)
	nrk_kprintf(PSTR("NL: create_network_layer_tasks(): Network layer task creation done\r\n"));
}

/*******************************************************************************************/
void initialise_network_layer()	// CHECKED
{
	int8_t i;	// loop index
	
	nl_sem = nrk_sem_create(1,MAX_TASK_PRIORITY);	// create the mutex  
	if(nl_sem == NULL)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("NL: initialise_network_layer(): Error creating semaphore\r\n"));
	}
	
		
	/* initialise the data structures required for the network layer */ 
	nl.count = 0;									// no neighbors recorded yet 
	nl.addr = NODE_ADDR;							// assign my network layer address 
	for(i = 0; i < MAX_NGBS; i++)					// an addr = INVALID_ADDR indicates an invalid entry
	   nl.ngbs[i].addr = INVALID_ADDR;
	   
	DEFAULT_GATEWAY = BCAST_ADDR;					// initially the default gateway is unknown 
	if(CONNECTED_TO_GATEWAY == TRUE)				// I am connected to the gateway
		DEFAULT_GATEWAY = NODE_ADDR;
	printf("DG = %u\r\n", DEFAULT_GATEWAY);
	
		
	initialise_routing_table();						// initialise the routing table		
	initialise_node_state();						// initialise the node state
	
	continue_sending_ngblist = 0;				    // wait for SEND_NW_INFO message from gateway
	ngb_list_pkt_queued = 0;						// No NGB_LIST messages are currently queued
	ngb_list_pkt_dequeued = 0;		
	
	continue_sending_nodeinfo = 0;					// wait for SEND_NODE_INFO message from gateway
	nodeinfo_pkt_queued = 0;						// No NODE_INFO messages are currently queued
	nodeinfo_pkt_dequeued = 0;						
	
	create_network_layer_tasks();					// create the two tasks 
	
	if(DEBUG_NL == 0)
	{
		nrk_kprintf(PSTR("From NetworkLayer \r\n"));
		nrk_kprintf(PSTR("SIZE_NEIGHBOR: "));
		printf("%u\r\n", SIZE_NEIGHBOR);
		nrk_kprintf(PSTR("SIZE_NEIGHBORLIST: "));
		printf("%u\r\n", SIZE_NEIGHBORLIST);
		nrk_kprintf(PSTR("SIZE_MSG_HELLO: "));
		printf("%u\r\n", SIZE_MSG_HELLO);
		nrk_kprintf(PSTR("SIZE_MSG_NGB_LIST: "));
		printf("%u\r\n", SIZE_MSG_NGB_LIST);
		nrk_kprintf(PSTR("SIZE_NW_PACKET_HEADER: "));
		printf("%u\r\n", SIZE_NW_PACKET_HEADER);
		nrk_kprintf(PSTR("SIZE_NW_PACKET: "));
		printf("%u\r\n", SIZE_NW_PACKET);
		nrk_kprintf(PSTR("SIZE_ROUTING_TABLE_ENTRY: "));
		printf("%u\r\n", SIZE_ROUTING_TABLE_ENTRY);
		nrk_kprintf(PSTR("SIZE_ROUTING_TABLE: "));
		printf("%u\r\n", SIZE_ROUTING_TABLE);
		nrk_kprintf(PSTR("SIZE_ROUTE_REPLY_ELEMENT: "));
		printf("%u\r\n", SIZE_ROUTE_REPLY_ELEMENT);
		nrk_kprintf(PSTR("NUM_ROUTE_REPLY_ELEMENTS: "));
		printf("%u\r\n", NUM_ROUTE_REPLY_ELEMENTS);
		nrk_kprintf(PSTR("SIZE_MSG_ROUTE_REPLY: "));
		printf("%u\r\n", SIZE_MSG_ROUTE_REPLY);
		nrk_kprintf(PSTR("SIZE_MSG_ROUTE_REQUEST: "));
		printf("%u\r\n", SIZE_MSG_ROUTE_REQUEST);
		nrk_kprintf(PSTR("SIZE_NODE_STATE: "));
		printf("%u\r\n", SIZE_NODE_STATE);
		nrk_kprintf(PSTR("SIZE_NODE_STATE_QUEUE: "));
		printf("%u\r\n", SIZE_NODE_STATE_QUEUE);
		nrk_kprintf(PSTR("SIZE_MSG_SEND_NW_INFO: "));
		printf("%u\r\n", SIZE_MSG_SEND_NW_INFO);
		nrk_kprintf(PSTR("SIZE_MSG_NW_INFO_ACQUIRED: "));
		printf("%u\r\n", SIZE_MSG_NW_INFO_ACQUIRED);
		nrk_kprintf(PSTR("SIZE_MSG_SEND_NODE_INFO: "));
		printf("%u\r\n", SIZE_MSG_SEND_NODE_INFO);
		nrk_kprintf(PSTR("SIZE_MSG_NODE_INFO_ACQUIRED: "));
		printf("%u\r\n", SIZE_MSG_NODE_INFO_ACQUIRED);
		nrk_kprintf(PSTR("SIZE_MSG_NODE_INFO: "));
		printf("%u\r\n", SIZE_MSG_NODE_INFO);
		nrk_kprintf(PSTR("RF_BUFFER_SIZE: "));
		printf("%u\r\n", RF_BUFFER_SIZE);
		nrk_kprintf(PSTR("Size occupied by NL: "));
		printf("%u\r\n", SIZE_NEIGHBOR + 2 * SIZE_NEIGHBORLIST + SIZE_ROUTING_TABLE + RF_BUFFER_SIZE + 3 * SIZE_NW_PACKET + 2 * MAX_TRANSPORT_UDP_SEG + 2 * SIZE_MSG_HELLO + 2 * SIZE_MSG_NGB_LIST + 2 * SIZE_MSG_NODE_INFO + SIZE_MSG_ROUTE_REQUEST + SIZE_MSG_ROUTE_REPLY + SIZE_MSG_NW_INFO_ACQUIRED + SIZE_MSG_SEND_NW_INFO + SIZE_MSG_SEND_NODE_INFO + SIZE_MSG_NODE_INFO_ACQUIRED + 4 * SIZE_NODETOGATEWAYSERIAL_PACKET + SIZE_NODE_STATE + SIZE_NODE_STATE_QUEUE);
	} 
	
	return;
}	
/******************************************************************************************/
void initialise_routing_table()		// CHECKED
{
	int8_t i;
	
	for(i = 0; i < NUM_ROUTING_TABLE_ENTRIES; i++)
	{
		rt.rte[i].dest = INVALID_ADDR;				  // indicates invalid entry 
		rt.rte[i].nextHop = INVALID_ADDR;			  // initially no routes are known
		rt.rte[i].cost = INFINITY;				  	  // no distances are known
	}
	return;
}
/*******************************************************************************************/
void initialise_node_state()		// CHECKED
{
	nsq.front = nsq.rear = 0;			// initialise the circular queue
	return;
}
/*******************************************************************************************/
void add_to_node_state(NodeState *p)
{
	
	if( (nsq.rear + 1) % MAX_STATE_VALUES == nsq.front ) // node's state queue is full
		remove_from_node_state();				   // remove the oldest entry
	
	nsq.ns[nsq.rear] = *p;
	
	nsq.rear = (nsq.rear + 1) % MAX_STATE_VALUES;
	return;
}	
/********************************************************************************************/
void remove_from_node_state()
{
	if(nsq.front == nsq.rear)	// queue is empty
		return;
		
	nsq.front = (nsq.front + 1) % MAX_STATE_VALUES;
	return;
}
/********************************************************************************************/
void clear_node_state()
{
	initialise_node_state();
	return;
}
/*******************************************************************************************/
int8_t process_Msg_RouteReply(Msg_RouteReply *mrr)
{
	int8_t i, j;		// loop indices
	int8_t index = -1;	// temporary variable to hold position at which new entry is to be made
						// in the routing table
	static uint8_t last_seq_no = 0;
	
	if(last_seq_no != mrr -> seq_no)	// its a different packet. Process it
	{
		for(i = 0; i < NUM_ROUTE_REPLY_ELEMENTS; i++)		// go through each RouteReplyElement
		{
			if( (mrr -> rre)[i].src == NODE_ADDR )			// I need to configure my routing table
			{
				enter_cr(nl_sem, 34);					// acquire the semaphore (nw layer)
				for(j = 0; j < NUM_ROUTING_TABLE_ENTRIES; j++)			// go through my routing table
					if(rt.rte[j].dest == INVALID_ADDR)	// entry in the routing table is free
						index = j;						// store this index
					else if(rt.rte[j].dest == mrr -> dest)	// otherwise check if destination node is found
						 {
					 		rt.rte[j].nextHop = (mrr -> rre)[i].nextHop;	// update the nextHop
					 		rt.rte[j].cost = (mrr -> rre)[i].cost;			// update the cost
					 		break;
					 	}//end if
		
				if(j == NUM_ROUTING_TABLE_ENTRIES)						// destination was not found in the routing table
				{
					if(index == -1)		// no free entry in the routing table
					{
						index = get_eviction_index();	// need to remove an entry from the routing table
						rt.rte[index].dest = mrr -> dest;	// fill up the now-empty position in the routing table
						rt.rte[index].nextHop = (mrr -> rre)[i].nextHop;
						rt.rte[index].cost = (mrr -> rre)[i].cost;
					}					
					else // index holds the value of a free entry in the routing table
					{
						rt.rte[index].dest = mrr -> dest;		// update the entry
						rt.rte[index].nextHop = (mrr -> rre)[i].nextHop;
						rt.rte[index].cost = (mrr -> rre)[i].cost;
					}//end else
				} // end if
					
				leave_cr(nl_sem, 34);
				break;									// I found my address in the list of nodes
			}// end if
		} // end for
		
		last_seq_no = mrr -> seq_no;						// update the last_seq_no variable
		return 1;
	} // end outer if
	
	return 0;
}			
/*******************************************************************************************/
int8_t process_Msg_NwInfoAcquired(Msg_NwInfoAcquired *m)
{
	static uint8_t last_seq_no = 0;
	
	if(last_seq_no != m -> seq_no)
	{
		// capture the address of the default gateway in any case
		enter_cr(nl_sem, 17);
		DEFAULT_GATEWAY = m -> dg;
		if(DEFAULT_GATEWAY == BCAST_ADDR)
	  	{
			nrk_int_disable();
			nrk_led_set(RED_LED);
	  		nrk_kprintf(PSTR("PMNWIA:Error in DG = "));
	  		printf("%u\n", DEFAULT_GATEWAY);
	  		
	  	}
		leave_cr(nl_sem, 17);
		
		if(search_addr(NODE_ADDR, m -> addrs, MAX_SUBNET_SIZE) == TRUE)
			set_continue_sending_ngblist(0);	// gateway has acquired my NGB_LIST message
		
		else
		// if the code reaches this point, there are two possibilities
		// 1. We are already sending NGB_LIST messages: In this case, nothing is to be done
		// 2. We are not sending NGB_LIST messages: Again nothing is to be done
		;	// do nothing
		
		last_seq_no = m -> seq_no;
		return 1;
	}
		
	return 0;
}
/********************************************************************************************/
int8_t process_Msg_SendNwInfo(Msg_SendNwInfo *m)	// CHECKED
{
	static uint8_t last_seq_no = 0;
	
	if(last_seq_no != m -> seq_no)		// a new message
	{
		// capture the address of the default gateway from this message
		enter_cr(nl_sem, 18);
		DEFAULT_GATEWAY = m -> dg;
		if(DEFAULT_GATEWAY == BCAST_ADDR)
  		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
  			nrk_kprintf(PSTR("PMSNWI:Error in DG = "));
  			printf("%u\n", DEFAULT_GATEWAY);
  		}
		leave_cr(nl_sem, 18);
	
		if(search_addr(NODE_ADDR, m -> addrs, MAX_SUBNET_SIZE) == TRUE)
			set_continue_sending_ngblist(1);			// gateway wants my NGB_LIST information
		else
			set_continue_sending_ngblist(0);			// needed to guard against lost NW_INFO_ACQUIRED messages
			
		last_seq_no = m -> seq_no;
		
		return 1;										// should route this packet
	}
	
	return 0;											// no need to process or route this packet
}
/*********************************************************************************************/
int8_t process_Msg_SendNodeInfo(Msg_SendNodeInfo *m)
{
	static uint8_t last_seq_no = 0;
	
	if(last_seq_no != m -> seq_no)
	{
		// capture the address of the default gateway from this message
		enter_cr(nl_sem, 18);
		DEFAULT_GATEWAY = m -> dg;
		if(DEFAULT_GATEWAY == BCAST_ADDR)	// for debugging
	  	{
			nrk_int_disable();
			nrk_led_set(RED_LED);
	  		nrk_kprintf(PSTR("PMSNI:Error in DG = "));
	  		printf("%u\n", DEFAULT_GATEWAY);
	  	}
	  	leave_cr(nl_sem, 18);
			
		if(search_addr(NODE_ADDR, m -> addrs, MAX_SUBNET_SIZE) == TRUE)
			set_continue_sending_nodeinfo(1);			// gateway wants my NODE_INFO information
		else
			set_continue_sending_nodeinfo(0);			// needed to guard against lost NODE_INFO_ACQUIRED messages
			
		last_seq_no = m -> seq_no;
		return 1;
	}
		
	return 0;
}
/*************************************************************************************************/
int8_t process_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *m)
{
	static uint8_t last_seq_no = 0;
	
	if(last_seq_no != m -> seq_no)	// its a different packet. Process it
	{
		// capture the address of the default gateway in any case
		enter_cr(nl_sem, 17);
		DEFAULT_GATEWAY = m -> dg;
		if(DEFAULT_GATEWAY == BCAST_ADDR)
	  	{
			nrk_int_disable();
			nrk_led_set(RED_LED);
	  		nrk_kprintf(PSTR("PMNIA:Error in DG = "));
	  		printf("%u\n", DEFAULT_GATEWAY);
	  		
	  	}
		leave_cr(nl_sem, 17);
		
		if(search_addr(NODE_ADDR, m -> addrs, MAX_SUBNET_SIZE) == TRUE)
			set_continue_sending_nodeinfo(0);	// gateway has acquired my NODE_INFO message
		
		else
		// if the code reaches this point, there are two possibilities
		// 1. We are already sending NODE_INFO messages: In this case, nothing is to be done
		// 2. We are not sending NODE_INFO messages: Again nothing is to be done
		;	// do nothing
		
		last_seq_no = m -> seq_no;
		return 1;
	}
			
	return 0;
}
/**************************************************************************************************/		
int8_t get_eviction_index()
{
	// the function adjust_routing_table() implements the LRU policy.
	// always return the last element in the routing table
	return (NUM_ROUTING_TABLE_ENTRIES - 1);
	
}	
/*************************************************************************************************/
// CHECKED
void set_nw_pkt_header(NW_Packet *pkt, uint16_t src, uint16_t dest, uint16_t nextHop, uint16_t prevHop, uint16_t prevprevHop, int8_t ttl, uint8_t type, int8_t length, int8_t prio)
{
	pkt -> src = src;
	pkt -> dest = dest;
	pkt -> nextHop = nextHop;
	pkt -> prevHop = prevHop;
	pkt -> prevprevHop = prevprevHop;
	pkt -> ttl = ttl;
	pkt -> type = type;
	pkt -> length = length;
	pkt -> prio = prio;
	
	return;
}	
/*******************************************************************************************/
void print_pkt_header(NW_Packet *pkt)	// CHECKED
{
	printf("[%u %u %u %u %u %d %u %d %d]", pkt -> src, pkt -> dest, pkt -> nextHop, pkt -> prevHop, pkt -> prevprevHop, pkt -> ttl, pkt -> type, pkt -> length, pkt -> prio);
	
	return;
}
/*******************************************************************************************/
void print_pkt(NW_Packet *pkt)		// CHECKED
{
	int8_t i;
		
	print_pkt_header(pkt);
	for(i = 0; i < pkt -> length; i++)
		printf("%u ", pkt -> data[i]);
		
	printf("\r\n");

	return;
}
/******************************************************************************************/
void print_nrk_time_t(nrk_time_t time)
{
	printf("%lu %2lu ", time.secs, time.nano_secs);
	return;
}
/******************************************************************************************/
void print_NodeState(NodeState *ns)
{
	print_nrk_time_t(ns -> timestamp);
	printf("%u %d %d\r\n", ns -> battery, ns -> tx_queue_size, ns -> rx_queue_size);
	
	return;
}
/******************************************************************************************/
void print_NodeStateQueue(NodeStateQueue *nsq)
{
	int8_t i;
	
	for(i = nsq -> front; ;i = ((i + 1) % MAX_STATE_VALUES))
	{
		if(i == nsq -> rear)	// reached the end of the circular queue
			break;
		print_NodeState( &(nsq -> ns[i]) );
	}
	
	printf("\r\n");
	
	return;
}	
/******************************************************************************************/
void print_Msg_RouteReply(Msg_RouteReply *m)
{
	int8_t i;
	
	printf("Msg_RouteReply is:\n");
	
	if(m -> dest == INVALID_ADDR)
		printf("dest = INV\n");
	else
		printf("dest = %d\n", m -> dest);
	for(i = 0; i < NUM_ROUTE_REPLY_ELEMENTS; i++)
		if(m -> rre[i].src != INVALID_ADDR)
			printf("%d[%d %d]\n", m -> rre[i].src, m -> rre[i].nextHop, m -> rre[i].cost);
		else
			printf("INV[]\n");

	printf("\n");
}
/******************************************************************************************/
void print_NgbList(NeighborList *nlt)
{
	int8_t i;
	
	printf("Addr = %u Count = %d ", nlt -> addr, nlt -> count);
	for(i = 0; i < MAX_NGBS; i++)		
	{
		if(nlt -> ngbs[i].addr != INVALID_ADDR)				// valid entry
			printf("%u[%d], ", nlt -> ngbs[i].addr, nlt -> ngbs[i].rssi);
	}
	printf("\r\n");
	
	return;
}
/******************************************************************************************/
void print_RoutingTable()
{
	int8_t i;
		
	nrk_kprintf(PSTR("My Routing Table\r\n"));
	nrk_kprintf(PSTR("Dest\tNextHop\tCost\r\n"));
	
	enter_cr(nl_sem, 34);
	for(i = 0; i < NUM_ROUTING_TABLE_ENTRIES; i++)
	{
		if(rt.rte[i].dest == INVALID_ADDR)	// ignore invalid destination
			continue;
			
		printf("%u\t", rt.rte[i].dest);
		
		if(rt.rte[i].nextHop == INVALID_ADDR)
			printf("INV\t");
		else
			printf("%u\t", rt.rte[i].nextHop);
					
		if(rt.rte[i].cost == INFINITY)
			printf("INF\r\n");
		else
			printf("%d\r\n", rt.rte[i].cost);
	}
	leave_cr(nl_sem, 34);
	return;
}
/******************************************************************************************/
void print_dg()
{
	uint16_t addr;
	
	enter_cr(nl_sem, 34);
	addr = DEFAULT_GATEWAY;
	leave_cr(nl_sem, 34);
	
	printf("NL: DG = %u\r\n", addr);
	return;
}	
/******************************************************************************************/
void print_raw(uint8_t *buf, int8_t len)	// CHECKED
{
	int8_t i; 
	
	printf("[");
	for(i = 0; i < len; i++)
		printf("%d ", buf[i]);
	printf("]\r\n");
	
	return;
}
/******************************************************************************************/
void set_continue_sending_nodeinfo(int8_t value)	// CHECKED
{
	enter_cr(nl_sem, 34);
	continue_sending_nodeinfo = value;
	leave_cr(nl_sem, 34);
	
	return;
}
/************************************************************************************************/
int8_t get_continue_sending_nodeinfo()		// CHECKED
{
	int8_t value;
	enter_cr(nl_sem, 34);
	value = continue_sending_nodeinfo;
	leave_cr(nl_sem, 34);
	
	return value;
}
/***********************************************************************************************/
void set_continue_sending_ngblist(int8_t value)		// CHECKED
{
	//printf("NL: set_continue_sending_ngblist() = %d\r\n", value);
	//printf("NL: scnl: Acquiring\r\n");
	enter_cr(nl_sem, 23);
	continue_sending_ngblist = value;
	leave_cr(nl_sem, 23);
	//printf("NL: scnl: Leaving\r\n");
	
	return;
}
/***********************************************************************************************/
int8_t get_continue_sending_ngblist()	// CHECKED
{
	int8_t value;
	
	enter_cr(nl_sem, 24);
	value = continue_sending_ngblist;
	leave_cr(nl_sem, 24);
	
	//printf("NL: get_continue_sending_ngblist() = %d\r\n", value);
	
	return value;
}
/**********************************************************************************************/
int8_t search_addr(uint16_t addr, uint16_t addrs[], int8_t size)	// CHECKED
{
	int8_t i;	// loop index
	
	// special case. Used by the gateway to signify that it has received/requires everybody's information 
	if(addrs[0] == BCAST_ADDR)	
		return TRUE;
		
	for(i = 0; i < size; i++)
		if(addrs[i] == addr)		// I am present in this address list
		{
			if(DEBUG_NL >= 1)
				nrk_kprintf(PSTR("NL: search_addr(): I am in the list\r\n"));
			return TRUE;
		}
			
	return FALSE;					// I am not present in this address list
}	
/*******************************************************************************************/
void send_nw_pkt_blocking(NW_Packet *pkt)		// CHECKED
{
	int8_t ret;		// to hold the return value of function calls
	
	if(DEBUG_NL >= 1)
		nrk_kprintf(PSTR("NL: Entered send_nw_pkt_blocking()\r\n"));
	
	while(1)
	{
		enter_cr(bm_sem, 20);				// acquire the BM semaphore
		ret = insert_tx_aq(pkt);			// insert this packet into the TX queue
		leave_cr(bm_sem, 20);				// release the BM semaphore
		if(ret == NRK_ERROR)
			nrk_wait_until_next_period();	// this packet cannot be dropped. Has to enter the network
		else								// packet inserted successfully 
			break;
	} // end while(1)
	
	if(DEBUG_NL >= 1)
		nrk_kprintf(PSTR("NL: Left send_nw_pkt_blocking()\r\n"));
	return;
}
/*******************************************************************************************/
void set_dg(uint16_t addr)
{
	printf("NL: set_dg(): %d\r\n", addr);
	enter_cr(nl_sem, 21);
	DEFAULT_GATEWAY = addr;
	leave_cr(nl_sem, 21);
}
/*******************************************************************************************/
uint16_t get_dg()
{
	uint16_t addr;
	
	enter_cr(nl_sem, 22);
	addr = DEFAULT_GATEWAY;
	leave_cr(nl_sem, 22);
	
	return addr;
}
/*************************** STATISTICS COLLECTION functions ******************************/
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
void collect_node_statistics()
{
	int8_t fd;
	uint16_t buf;
	//uint8_t buf[2];
	
	nrk_time_get(&(ns.timestamp));							// record the timestamp of collection
	nrk_time_compact_nanos(&(ns.timestamp));
	
	enter_cr(bm_sem, 34);									// collect queue statistics
	ns.tx_queue_size = tx_buf_mgr.count_aq;
	ns.rx_queue_size = MAX_RX_QUEUE_SIZE - num_bufs_free;
	leave_cr(bm_sem, 34);
	
	fd = nrk_open(FIREFLY_SENSOR_BASIC, READ);				// record the battery level
 	if(fd == NRK_ERROR)
 	{
 		nrk_int_disable();
 		nrk_led_set(RED_LED);
 		nrk_kprintf(PSTR("Failed to open sensor driver\r\n"));
 	}
 	nrk_set_status(fd, SENSOR_SELECT, BAT);
 	nrk_read(fd, &buf, 2);
 	nrk_close(fd);
 	ns.battery = buf;
 	
	add_to_node_state(&ns);	
	return;	
}
/*********************************************************************************************/

