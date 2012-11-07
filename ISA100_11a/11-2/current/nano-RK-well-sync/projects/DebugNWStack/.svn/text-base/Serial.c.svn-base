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
/************************** Global data structures ************************/
nrk_task_type SERIAL_TASK;								// variables required to create the serial task
NRK_STK serial_task_stack[NRK_APP_STACKSIZE];
void serial_task(void);

static uint8_t rx_buf[SIZE_GATEWAYTONODESERIAL_PACKET];	// receive buffer to hold data received from attached node
static GatewayToNodeSerial_Packet gtn_pkt;				// to hold a packet received from attached node

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
		ret = slip_rx(rx_buf, SIZE_GATEWAYTONODESERIAL_PACKET);
		if (ret > 0)	// message received successfully
		{
			if(ret != SIZE_GATEWAYTONODESERIAL_PACKET)	// wrong-length packet received
			{
				nrk_kprintf(PSTR("SR: serial_task(): Incorrect packet length received from gateway: "));
				printf("%d\r\n",ret);
				continue;
			}
			if(DEBUG_SR == 0)
			{
				nrk_kprintf(PSTR("SR: serial_task(): Received a message from the gateway: Length =   "));
				printf("%d\r\n", ret);
			}
			
			
		} // end if
		else	// message was corrupted
      	{
      		nrk_kprintf(PSTR("SR: serial_task(): Failed to receive a SLIP message from gateway: Length = "));
      		printf("%d\r\n", ret);
      	}
      	nrk_wait_until_next_period();
   	} // end while 
		
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
  	
  	return;
}
/*****************************************************************************/
void initialise_serial_communication()	// CHECKED
{
	create_serial_task();

	return;
}
/******************************************************************************/
