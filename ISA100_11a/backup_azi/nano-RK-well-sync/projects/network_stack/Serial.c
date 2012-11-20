#include "Serial.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <slip.h>

#define INFINITY 100
#define INVALID_ADDRESS 0 

/************************** External variables and functions **************************/
// From Pack.c
extern void unpack_GatewayToNodeSerial_Packet_header(GatewayToNodeSerial_Packet *, uint8_t *);
extern void pack_Msg_RoutingTable(uint8_t *, Msg_RoutingTable*);

// From NetworkLayer.c
extern void print_RoutingTable(Msg_RoutingTable *);
extern void set_RoutingTable(Msg_RoutingTable *);
extern uint16_t route_addr(uint16_t);

// From BufferManager.c
extern void enter_cr(nrk_sem_t *, int8_t);
extern void leave_cr(nrk_sem_t *, int8_t);
extern int8_t insert_tx_aq(NW_Packet*);
extern nrk_sem_t *bm_sem;


/************************** Global data structures ************************/
nrk_task_type SERIAL_TASK;
NRK_STK serial_task_stack[NRK_APP_STACKSIZE];
void serial_task(void);

uint8_t rx_buf[SIZE_GATEWAYTONODESERIAL_PACKET];
GatewayToNodeSerial_Packet gtn_pkt;

Msg_RoutingTable mrt;
NW_Packet nw_pkt;
uint8_t tx_buf[SIZE_NW_PACKET];


/************************* Function definitions ***************************/
/*
void sendToSerial(uint8_t *buf, int8_t length)
{
   putchar(END);
   if(DEBUG_SR == 2)
   	printf("%d ", END);

   while(length > 0)
	{
   	switch(*buf)
		 {
       		
            case END:
               putchar(ESC);
               putchar(ESC_END);
               if(DEBUG_SR == 2)
               	printf("%d %d ",ESC, ESC_END); 
               
               break;

             case ESC:
                putchar(ESC);
                putchar(ESC_ESC);
                if(DEBUG_SR == 2)
                	printf("%d %d ",ESC, ESC_ESC);
                break;

             default:
 	              putchar(*buf);
 	              if(DEBUG_SR == 2)	
 	              	 printf("%d ", *buf);
        } // end switch
        buf++;
        length--;
   } // end while
   
	putchar(END);
    if(DEBUG_SR == 2)
    	printf("%d ", END);
	 return;
}
*/
void sendToSerial(uint8_t *buf, int8_t len)
{
	while( slip_started () == NRK_ERROR )
	{
		nrk_wait_until_next_period();
	}
	
	if(DEBUG_SR == 2)
	{
		nrk_kprintf(PSTR("Calling slip_tx\r\n"));
	}
	
	if( slip_tx (buf, len) == NRK_ERROR )
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
		{
			nrk_kprintf(PSTR("Error sending out NGB_LIST message over serial\r\n"));
		}
	}
	
	return;
}
/**********************************************************************************************/
void printBuffer(uint8_t *buf, int8_t len)
{
	while(len > 0)
	{
		printf("%d ", *buf);
		buf++;
		len--;
	}
	printf("\n\n");
	return;
}

/***********************************************************************************************/
uint8_t serial_pkt_type(GatewayToNodeSerial_Packet *pkt)
{
	switch(pkt -> type)
	{
		case SERIAL_APPLICATION:
			return SERIAL_APPLICATION;
			
		case SERIAL_ROUTE_CONFIG:
			return SERIAL_NW_CONTROL;
			
	}
	return INVALID;									// unrecognized packet type 
}
/*************************************************************************************************/
uint8_t serial_nw_ctrl_type(GatewayToNodeSerial_Packet *pkt)
{
	switch(pkt -> type)
	{
		case SERIAL_ROUTE_CONFIG:
			return SERIAL_ROUTE_CONFIG;
	}
	
	while(1)
		printf("Bug detected in implementation of packet type\n");

	return INVALID;									// this should never happen
}
/************************************************************************************************/
void process_serial_app_pkt(GatewayToNodeSerial_Packet *pkt)
{
	int8_t i;
	printf("Received an application layer packet\n");
	for(i = 0; i < pkt -> length; i++)
	{
		printf("%c", pkt -> data[i]);
	}
	printf("\r\n");
	
	return;
}
/*************************************************************************************************/
void process_serial_nw_ctrl_pkt(GatewayToNodeSerial_Packet *pkt)
{
	int8_t i;
	int8_t ret;
	
	if(DEBUG_SR == 0)
	{
		printf("Inside process_serial_nw_ctrl_pkt()\r\n");
	}

	switch(serial_nw_ctrl_type(pkt))
	{
		case SERIAL_ROUTE_CONFIG:
			unpack_Msg_RoutingTable(&mrt, pkt -> data);
			mrt.dg = NODE_ADDR;		// fill up the default gateway address in any case
						
			if(DEBUG_SR == 0)
			{
				print_RoutingTable(&mrt);
			}					
			
			if(mrt.node == NODE_ADDR)	// this is my own routing table
			{
				set_RoutingTable(&mrt);
			}
			else // this is some other node's routing table
			{
				// create a network layer packet to disseminate this routing information
				nw_pkt.src = (uint16_t)NODE_ADDR;
			  	nw_pkt.dest = mrt.node;
			  	nw_pkt.nextHop = route_addr(mrt.node);
			  	nw_pkt.prevHop = (uint16_t)NODE_ADDR;
			  	nw_pkt.ttl = MAX_NETWORK_DIAMETER;		
			  	nw_pkt.type = (uint8_t)ROUTE_CONFIG;
			  	nw_pkt.length = SIZE_MSG_ROUTING_TABLE;
			  	nw_pkt.prio = NORMAL_PRIORITY;
			  	
			  	pack_Msg_RoutingTable(nw_pkt.data, &mrt);
			  	while(1)
			  	{
			  		enter_cr(bm_sem, 34);
			  		ret = insert_tx_aq(&nw_pkt);
			  		if(ret == NRK_ERROR)
			  		{
			  			leave_cr(bm_sem, 34);
			  			nrk_wait_until_next_period();
			  		}
			  		else	// packet inserted successfully 
			  		{
			  			leave_cr(bm_sem, 34);
			  			break;
			  		}
			  	} // end while(1)
			} //end else
			  		
			break;	// end case 
			
	} // end switch()
	
	return;
}
	
/*************************************************************************************************/
void serial_task()
{
	int8_t ret;
	
	if(DEBUG_SR == 0)
	{
			nrk_kprintf(PSTR("Inside serial_task. Task PID = "));
			printf("%d\r\n", nrk_get_pid());
	}
	
	// initialise the SLIP module 
	slip_init (stdin, stdout, 0, 0);
	
	while(1)
	{
		// wait for the gateway to send you a message
		if(DEBUG_SR == 0)
		{
			nrk_kprintf(PSTR("SL: Waiting for a packet from the gateway\r\n"));
		}
		ret = slip_rx (rx_buf, SIZE_GATEWAYTONODESERIAL_PACKET);
		if (ret > 0)	// message received successfully
		{
			if(DEBUG_SR == 0)
			{
				nrk_kprintf(PSTR("Received a message from the gateway\r\n"));
			}
			unpack_GatewayToNodeSerial_Packet_header(&gtn_pkt, rx_buf);
			
			switch(serial_pkt_type(&gtn_pkt))
			{
				case SERIAL_APPLICATION:
					process_serial_app_pkt(&gtn_pkt);
					break;
				
				case SERIAL_NW_CONTROL:
					process_serial_nw_ctrl_pkt(&gtn_pkt);
					break;
					
				case INVALID: 
					// drop the packet and go receive another one 
					//printf("serial_task(): Invalid packet type received = %d\n", gtn_pkt.type);	 
					break;
			} // end switch
		} // end if
      	else	// message was corrupted
      	{
      		nrk_kprintf(PSTR("Failed to receive a SLIP message from gateway\r\n"));
      		//nrk_wait_until_next_period ();
      	}
	} // end while 
		
	return;
}

/****************************************************************************/
void create_serial_task()
{
	SERIAL_TASK.task = serial_task;
	SERIAL_TASK.Ptos = (void *) &serial_task_stack[NRK_APP_STACKSIZE - 1];
	SERIAL_TASK.Pbos = (void *) &serial_task_stack[0];
	SERIAL_TASK.prio = 17;
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
  	
  	if(DEBUG_SR == 0)
  	{
  			nrk_kprintf(PSTR("Serial task activated\r\n"));
  	}
}
/*****************************************************************************/
void initialise_serial_communication()
{
	create_serial_task();

	return;
}
/******************************************************************************/
void print_gtn_pkt(GatewayToNodeSerial_Packet *pkt)
{
	int8_t i;
	print_gtn_pkt_header(pkt);
	
	for(i = 0; i < pkt -> length; i++)
	{
		printf("%d ", pkt -> data[i]);
	}
	printf("\r\n");

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
		
