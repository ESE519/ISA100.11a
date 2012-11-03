/* This file implements the serial communication component of the Firefly node */

/* Authors:
 * Aditya Bhave
*/

/***************************** Include files *********************************************/
// network stack
#include "Serial.h"

// C library
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Nano-RK
#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <slip.h>

/************************** External variables and functions **************************/
// From Pack.c
extern void pack_Msg_SendNwInfo(uint8_t *, Msg_SendNwInfo*);
extern void pack_Msg_NwInfoAcquired(uint8_t *, Msg_NwInfoAcquired *);
extern void pack_Msg_SendNodeInfo(uint8_t *, Msg_SendNodeInfo *);
extern void pack_Msg_NodeInfoAcquired(uint8_t *, Msg_NodeInfoAcquired *);
extern void pack_Msg_RouteReply(uint8_t *, Msg_RouteReply *);
extern void pack_NodeToGatewaySerial_Packet_header(uint8_t *, NodeToGatewaySerial_Packet*);

extern void unpack_Msg_SendNwInfo(Msg_SendNwInfo*, uint8_t *);
extern void unpack_Msg_NwInfoAcquired(Msg_NwInfoAcquired*, uint8_t *);
extern void unpack_Msg_SendNodeInfo(Msg_SendNodeInfo *, uint8_t *);
extern void unpack_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *, uint8_t *);
extern void unpack_Msg_RouteReply(Msg_RouteReply *, uint8_t *);
extern void unpack_GatewayToNodeSerial_Packet_header(GatewayToNodeSerial_Packet *, uint8_t *);

// From NetworkLayer.c
extern void print_RoutingTable(Msg_RoutingTable *);
extern void set_RoutingTable(Msg_RoutingTable *);
extern uint16_t route_addr(uint16_t);
extern int8_t get_eviction_index();
extern int8_t sent_route_request(uint16_t dest);
extern int8_t remove_from_dest_requests(uint16_t dest);
extern void process_msg_route_reply(Msg_RouteReply *mrr);
extern void set_nw_pkt_header(NW_Packet *pkt, uint16_t src, uint16_t dest, uint16_t nextHop, uint16_t prevHop, uint16_t prevprevHop, int8_t ttl, uint8_t type, int8_t length, int8_t prio);
extern void send_nw_pkt_blocking(NW_Packet *pkt);
extern int8_t search_addr(uint16_t addr, uint16_t addrs[], int8_t size);
extern void set_dg(uint16_t);
extern nrk_sem_t *nl_sem;

// From CriticalRegion.c
extern void enter_cr(nrk_sem_t *, int8_t);
extern void leave_cr(nrk_sem_t *, int8_t);

// From BufferManager.c
extern int8_t insert_tx_aq(NW_Packet*);
extern nrk_sem_t *bm_sem;

/************************** Global data structures ************************/
nrk_task_type SERIAL_TASK;								// variables required to create the serial task
NRK_STK serial_task_stack[NRK_APP_STACKSIZE];
void serial_task(void);

static uint8_t rx_buf[SIZE_GATEWAYTONODESERIAL_PACKET];	// receive buffer to hold data received from attached node
static GatewayToNodeSerial_Packet gtn_pkt;				// to hold a packet received from attached node
static NodeToGatewaySerial_Packet ntg_pkt;				// to hold an ACK
static uint8_t tx_buf[SIZE_NODETOGATEWAYSERIAL_PACKET];

static Msg_RouteReply mrr;								// used by serial task to construct network messages
static Msg_NwInfoAcquired mnwia;						// to hold a NW_INFO_ACQUIRED message
static Msg_SendNwInfo msnwi;							// to hold a SEND_NW_INFO message
static Msg_SendNodeInfo msni;							// to hold a SEND_NODE_INFO message
static Msg_NodeInfoAcquired mnia;						// to hold a NODE_INFO_ACQUIRED message
static NW_Packet nw_pkt;								// network packet sent by the serial task
//static uint8_t tx_buf[SIZE_NW_PACKET];				// transmit buffer given to link layer

/************************* Function definitions ***************************/
int8_t sendToSerial(uint8_t *buf, uint8_t len)		// CHECKED
{
	while( slip_started () == NRK_ERROR )			// wait till the serial task starts the SLIPstream module
		nrk_wait_until_next_period();
		
	/*
	if(len > SIZE_SLIP_TX_BUF)
	{
		nrk_kprintf(PSTR("SR: sendToSerial(): Buffer length too big\r\n"));
		return NRK_ERROR;
	}
	*/
	
	if( slip_tx (buf, len) == NRK_ERROR )
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("SR: sendToSerial(): Error sending out message over serial port\r\n"));
	}
	return NRK_OK;
}
/**********************************************************************************************/
uint8_t serial_pkt_type(GatewayToNodeSerial_Packet *pkt)	// CHECKED
{
	switch(pkt -> type)
	{
		case SERIAL_APPLICATION:					// application layer packet
			return SERIAL_APPLICATION;
			
		case SERIAL_ROUTE_CONFIG:					// network control packet
		case SERIAL_ROUTE_REPLY:
		case SERIAL_SEND_NW_INFO:
		case SERIAL_NW_INFO_ACQUIRED:
		case SERIAL_SEND_NODE_INFO:
		case SERIAL_NODE_INFO_ACQUIRED:
			return SERIAL_NW_CONTROL;
		
		default:
			return SERIAL_INVALID;
	}
	return SERIAL_INVALID;									// unrecognized packet type 
}
/*************************************************************************************************/
void process_serial_app_pkt(GatewayToNodeSerial_Packet *pkt)	// CHECKED
{
	int8_t i;		// loop variable
	
	if(DEBUG_SR == 0)
		nrk_kprintf(PSTR("SR: process_serial_app_pkt(): Received an application layer packet\r\n"));
	for(i = 0; i < pkt -> length; i++)
	{
		printf("%d ", pkt -> data[i]);
	}
	nrk_kprintf(PSTR("\r\n"));	
	
	return;
}
/*************************************************************************************************/
void process_serial_nw_ctrl_pkt(GatewayToNodeSerial_Packet *pkt) // CHECKED SO
{
	if(DEBUG_SR == 1)
	{
		nrk_kprintf(PSTR("SR: process_serial_nw_ctrl_pkt(): Received a network control packet of type = \r\n"));
		printf("%x\r\n", pkt -> type);
	}
		
	
	switch(pkt -> type)
	{
		 case SERIAL_ROUTE_REPLY:
							
			unpack_Msg_RouteReply(&mrr, pkt -> data);
			if(DEBUG_SR == 0)
			{
				nrk_kprintf(PSTR("SR: RX SERIAL_ROUTE_REPLY: "));
				printf("%u\r\n", mrr.seq_no);
			}
			
			mrr.dg = NODE_ADDR;				// fill up the default gateway in any case
			
			process_Msg_RouteReply(&mrr); 	// look whether this message contains my address
											// and update routing tables accordingly
			
			// in any case send this packet into the network
			set_nw_pkt_header(&nw_pkt, NODE_ADDR, BCAST_ADDR, BCAST_ADDR, NODE_ADDR, NODE_ADDR, MAX_NETWORK_DIAMETER, ROUTE_REPLY, SIZE_MSG_ROUTE_REPLY, NORMAL_PRIORITY);
			// FIX ME, should nextHop = INVALID_ADDR?
			pack_Msg_RouteReply(nw_pkt.data, &mrr);
			send_nw_pkt_blocking(&nw_pkt);
					
			break;				
		
		case SERIAL_SEND_NW_INFO:
			
			unpack_Msg_SendNwInfo(&msnwi, pkt -> data); // unpack the message from the GatewayToNodeSerial_Packet packet
			if(DEBUG_SR == 0)
			{
				nrk_kprintf(PSTR("SR: RX SERIAL_SEND_NW_INFO: "));
				printf("%u\r\n", msnwi.seq_no);
			}
			
			msnwi.dg = NODE_ADDR;	// fill up the default gateway address in any case
			
			// Check if the gateway is requesting for my NGB_LIST information
			if(search_addr(NODE_ADDR, msnwi.addrs, MAX_SUBNET_SIZE) == TRUE)
				set_continue_sending_ngblist(1);
			
			// create a network layer packet to disseminate this information
			// the function route_addr() will return the correct 'nextHop' address regardless of whether
			// msnwi.addr equals BCAST_ADDR or some other node's address. Again it does not matter if the 
			// gateway knows the route to msnwi.addr or not.
			//set_nw_pkt_header(&nw_pkt, NODE_ADDR, msnwi.addr, route_addr(msnwi.addr), NODE_ADDR, NODE_ADDR, MAX_NETWORK_DIAMETER, SEND_NW_INFO, SIZE_MSG_SEND_NW_INFO, NORMAL_PRIORITY);
			set_nw_pkt_header(&nw_pkt, NODE_ADDR, BCAST_ADDR, BCAST_ADDR, NODE_ADDR, NODE_ADDR, MAX_NETWORK_DIAMETER, SEND_NW_INFO, SIZE_MSG_SEND_NW_INFO, NORMAL_PRIORITY);
			pack_Msg_SendNwInfo(nw_pkt.data, &msnwi);
			send_nw_pkt_blocking(&nw_pkt);
			break;
		
			
		case SERIAL_NW_INFO_ACQUIRED:
			
			unpack_Msg_NwInfoAcquired(&mnwia, pkt -> data); // unpack the message from the GatewayToNodeSerial_Packet packet
			if(DEBUG_SR == 0)
			{
				nrk_kprintf(PSTR("SR: RX SERIAL_NW_INFO_ACQUIRED: "));
				printf("%u\r\n", mnwia.seq_no);
			}
			mnwia.dg = NODE_ADDR;							// fill up the default gateway address in any case
			
			// the gateway has obtained my NGB_LIST information
			if(search_addr(NODE_ADDR, mnwia.addrs, MAX_SUBNET_SIZE) == TRUE)
				set_continue_sending_ngblist(0);
				
			// create a network layer packet to disseminate this information
			set_nw_pkt_header(&nw_pkt, NODE_ADDR, BCAST_ADDR, BCAST_ADDR, NODE_ADDR, NODE_ADDR, MAX_NETWORK_DIAMETER, NW_INFO_ACQUIRED, SIZE_MSG_NW_INFO_ACQUIRED, NORMAL_PRIORITY);
			pack_Msg_NwInfoAcquired(nw_pkt.data, &mnwia);
			send_nw_pkt_blocking(&nw_pkt);
			
			break;
			
		case SERIAL_SEND_NODE_INFO:
			
			unpack_Msg_SendNodeInfo(&msni, pkt -> data); // unpack the message from the GatewayToNodeSerial_Packet packet
			if(DEBUG_SR == 0)
			{
				nrk_kprintf(PSTR("SR: RX SERIAL_SEND_NODE_INFO: "));
				printf("%u\r\n", msni.seq_no);
			}
		
			msni.dg = NODE_ADDR;						// fill up the default gateway address in any case
			// Does the gateway wants my NODE_INFO information?
			if(search_addr(NODE_ADDR, msni.addrs, MAX_SUBNET_SIZE) == TRUE)
				set_continue_sending_nodeinfo(1);
			
			// create a network layer packet to disseminate this information
			// this packet is broadcast into the network
			set_nw_pkt_header(&nw_pkt, NODE_ADDR, BCAST_ADDR, BCAST_ADDR, NODE_ADDR, NODE_ADDR, MAX_NETWORK_DIAMETER, SEND_NODE_INFO, SIZE_MSG_SEND_NODE_INFO, NORMAL_PRIORITY);
			pack_Msg_SendNodeInfo(nw_pkt.data, &msni);
			send_nw_pkt_blocking(&nw_pkt);
			
			break;
		
		case SERIAL_NODE_INFO_ACQUIRED:
		
			unpack_Msg_NodeInfoAcquired(&mnia, pkt -> data); // unpack the message from the GatewayToNodeSerial_Packet packet
			if(DEBUG_SR == 0)
			{
				nrk_kprintf(PSTR("SR: RX SERIAL_NODE_INFO_ACQUIRED: "));
				printf("%u\r\n", mnia.seq_no);
			}
			mnia.dg = NODE_ADDR;							 // fill up the default gateway address in any case
			
			// Has the gateway has obtained my NODE_INFO information?
			if(search_addr(NODE_ADDR, mnia.addrs, MAX_SUBNET_SIZE) == TRUE)
				set_continue_sending_nodeinfo(0);
				
			// create a network layer packet to disseminate this information
			set_nw_pkt_header(&nw_pkt, NODE_ADDR, BCAST_ADDR, BCAST_ADDR, NODE_ADDR, NODE_ADDR, MAX_NETWORK_DIAMETER, NODE_INFO_ACQUIRED, SIZE_MSG_NODE_INFO_ACQUIRED, NORMAL_PRIORITY);
			pack_Msg_NodeInfoAcquired(nw_pkt.data, &mnia);
			send_nw_pkt_blocking(&nw_pkt);
			
			break;
					
		default:
			while(1)
				nrk_kprintf(PSTR("SR: process_serial_nw_ctrl_pkt(): Invalid nw_ctrl packet received: ERROR\r\n"));
	} // end switch()
	
	return;
}
/*************************************************************************************************/
void serial_task()												// CHECKED
{
	int8_t ret;			// to hold the return value of various function calls
	
	// initialise the SLIPstream module 
	slip_init (stdin, stdout, 0, 0);
	
	while(1)			// start processing forever
	{
		// wait for the gateway to send you a message
		if(DEBUG_SR >= 1)
		{
			nrk_kprintf(PSTR("SR: serial_task(): Waiting for a packet from the gateway\r\n"));
		}
		ret = slip_rx (rx_buf, SIZE_GATEWAYTONODESERIAL_PACKET);
		if (ret > 0)	// message received successfully
		{
			/*
			if(ret != SIZE_GATEWAYTONODESERIAL_PACKET)	// wrong-length packet received
			{
				nrk_kprintf(PSTR("SR: serial_task(): Incorrect packet length received from gateway: "));
				printf("%d\r\n",ret);
				continue;
			}
			*/
			if(ret < SIZE_GATEWAYTONODESERIAL_PACKET_HEADER)	// wrong length packet received
			{
				nrk_kprintf(PSTR("SR: serial_task(): Incorrect packet length received from gateway: "));
				printf("%d\r\n", ret);
				continue;
			}				 
			
			// got a valid packet. Send ACK
			send_ACK();
			//unpack the received data into a packet
			unpack_GatewayToNodeSerial_Packet_header(&gtn_pkt, rx_buf);
			//memcpy(gtn_pkt.data, rx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, MAX_GATEWAY_PAYLOAD);
			memcpy(gtn_pkt.data, rx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, gtn_pkt.length);
			
			switch(serial_pkt_type(&gtn_pkt))
			{
				case SERIAL_APPLICATION:
					process_serial_app_pkt(&gtn_pkt);
					break;
				
				case SERIAL_NW_CONTROL:
					process_serial_nw_ctrl_pkt(&gtn_pkt);
					break;
					
				default: 
					// drop the packet and go receive another one 
					if(DEBUG_SR == 0)
					{
						nrk_kprintf(PSTR("SR: serial_task(): Invalid packet type received = "));
						printf("%x\n", gtn_pkt.type);
					}	 
					break;
			} // end switch
			
		
		} // end if
		else	// message was corrupted
      	{
      		nrk_kprintf(PSTR("SR: serial_task(): Failed to receive a SLIP message from gateway: Length = "));
      		printf("%d\r\n", ret);
       	}
      	//nrk_wait_until_next_period();
   	} // end while 
		
	return;
}
/***************************************************************************************************/
void send_ACK()
{
	int8_t ret;
	
	ntg_pkt.type = SERIAL_ACK;
	ntg_pkt.length = 0;
	pack_NodeToGatewaySerial_Packet_header(tx_buf, &ntg_pkt);
	//memcpy(tx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, MAX_SERIAL_PAYLOAD);
	memcpy(tx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, ntg_pkt.length);
	//ret = sendToSerial(tx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
	ret = sendToSerial(tx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER + ntg_pkt.length);
	if(ret == NRK_OK)
		if(DEBUG_SR >= 1)
			nrk_kprintf(PSTR("SR: send_ACK(): Sent ACK to gateway\r\n"));
	
	return;
}

/****************************************************************************/
void create_serial_task()			// CHECKED
{
	SERIAL_TASK.task = serial_task;
	SERIAL_TASK.Ptos = (void *) &serial_task_stack[NRK_APP_STACKSIZE - 1];
	SERIAL_TASK.Pbos = (void *) &serial_task_stack[0];
	SERIAL_TASK.prio = 19;
	SERIAL_TASK.FirstActivation = TRUE;
	SERIAL_TASK.Type = BASIC_TASK;
	SERIAL_TASK.SchType = PREEMPTIVE;
	
	SERIAL_TASK.cpu_reserve.secs = 0;
	SERIAL_TASK.cpu_reserve.nano_secs = 200 * NANOS_PER_MS;  
	SERIAL_TASK.period.secs = 0;
	SERIAL_TASK.period.nano_secs = 250 * NANOS_PER_MS;
	   
  
  	SERIAL_TASK.offset.secs = 0;
  	SERIAL_TASK.offset.nano_secs= 0;
  	nrk_activate_task (&SERIAL_TASK);
  	
  	nrk_kprintf(PSTR("SR: create_serial_task(): Serial Task started\r\n"));
  	
  	return;
}
/*****************************************************************************/
void initialise_serial_communication()	// CHECKED
{
	create_serial_task();

	return;
}
/******************************************************************************/
void print_gtn_pkt(GatewayToNodeSerial_Packet *pkt)	
{
	int8_t i;					// loop variable
	print_gtn_pkt_header(pkt);
	
	for(i = 0; i < pkt -> length; i++)
	{
		printf("%d ", pkt -> data[i]);
	}
	nrk_kprintf(PSTR("\r\n"));

	return;
}

/********************************************************************************/
void print_gtn_pkt_header(GatewayToNodeSerial_Packet *pkt)	
{
	nrk_kprintf(PSTR("["));
	printf("%d %d]", pkt -> type, pkt -> length);
	
	return;
}
/**********************************************************************************/		
void printBuffer(uint8_t *buf, uint8_t len)		
{
	while(len > 0)
	{
		printf("%d ", *buf);
		buf++;
		len--;
	}
	nrk_kprintf(PSTR("\n\n"));
	return;
}

/***********************************************************************************************/	
		
