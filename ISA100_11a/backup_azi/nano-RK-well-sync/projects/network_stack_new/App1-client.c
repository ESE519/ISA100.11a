// This is an example application to test the buffer manager and the network stack 

#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_stack_check.h>
#include <avr/sleep.h>
#include <nrk_events.h>
#include <nrk_defs.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "NWStackConfig.h"
#include "NetworkLayer.h"
#include "TransportLayerUDP.h"
#include "Pack.h"
#include "BufferManager.h"
#include "Serial.h"
#include "Debug.h"
  			
#define SERVER_ADDR 3  
#define SERVER_PORT 100 
#define SERVER_RQS 4
#define RX_PORT 200  

#define PAYLOAD MAX_APP_PAYLOAD 
#define DEBUG_APP 1

nrk_task_type TASK;
NRK_STK task_stack[NRK_APP_STACKSIZE];
void task(void);

nrk_task_type NW_TASK;
NRK_STK nw_task_stack[NRK_APP_STACKSIZE];
void nw_task(void);

/**************************************************************************************/
int8_t tx_buf[PAYLOAD];

typedef struct
{
   int8_t temperature; 
   int8_t humidity;
   uint8_t pressure;

}SensorReadings;

SensorReadings s;
/**************************************************************************************/
/*
void nw_task()
{
	int8_t sock;
	int8_t ret;
  	uint8_t cnt;
  	uint8_t *rx_buf;
  	int8_t len;
  	int8_t data;
	uint16_t srcAddr;
	uint8_t srcPort;
	  	  	  
	sock = create_socket(SOCK_DGRAM);
  	if(sock == NRK_ERROR)
  	{
  		nrk_kprintf(PSTR("TX: Error in creating socket in task: "));
  		print_nw_stack_errno(nrk_errno_get());
  	}
  	
  	cnt = 0;
  	while(1)
  	{
  		cnt = (cnt + 1) % 120;
  		tx_buf[0] = cnt;
  	
  		// send data
  		ret = send(sock, tx_buf, 1, SERVER_ADDR, SERVER_PORT, NORMAL_PRIORITY);
  		if(ret == NRK_ERROR)
  		{
  			nrk_kprintf(PSTR("TX: Error in sending message: "));
  			print_nw_stack_errno(nrk_errno_get());
  		}
  		else if(DEBUG_APP == 1)
  			 {
  				nrk_kprintf(PSTR("TX: Sent message: "));
  				printf("%d\r\n", cnt);
  			 }
  	
  		
  		// receive data
  		set_timeout(sock, 5, 0);			
		rx_buf = receive(sock, &len, &srcAddr, &srcPort, NULL);		// wait for server reply
	
		if(rx_buf == NULL)		// receive() failed  
		{	
			nrk_kprintf(PSTR("RX: Error in receiving data: "));
			print_nw_stack_errno(nrk_errno_get());
		}
		else
		{
			// retrieve the data
			data = rx_buf[0];
			if(DEBUG_APP == 0)
			{
				nrk_kprintf(PSTR("RX: Releasing buffer back to buffer manager\r\n"));
			}
			// release the buffer back to buffer manager			
			release_buffer(sock, rx_buf);
		
			// print what server sent back
			if(DEBUG_APP == 1)
			{
				nrk_kprintf(PSTR("RX: Got message: "));
				printf("%d from %u %u\n", data, srcAddr, srcPort);
			}
		} 
		
		nrk_wait_until_next_period();
  	} // end while
  	
  	return;
}
*/

/*void task_IPC_send()
{
	int8_t sock;
  	int8_t ret;
  	uint8_t cnt;
  	
  
	sock = create_socket(SOCK_IPC);
  	if(sock == NRK_ERROR)
  	{
  		nrk_kprintf(PSTR("TX: Error in creating socket in task: "));
  		print_nw_stack_errno(nrk_errno_get());
  	}
  
 	cnt = 0;
  	while(1)
  	{
  		cnt = (cnt + 1) % 120;
  		tx_buf[0] = cnt;
  	
  		ret = send(sock, tx_buf, 1, NODE_ADDR, RX_PORT, NORMAL_PRIORITY);
  		if(ret == NRK_ERROR)
  		{
  			nrk_kprintf(PSTR("TX: Error in sending message: "));
  			print_nw_stack_errno(nrk_errno_get());
  		}
  		else if(DEBUG_APP == 1)
  			 {
  				nrk_kprintf(PSTR("TX: Sent message: "));
  				printf("%d\r\n", cnt);
  			 }
  		  	
 	 	nrk_wait_until_next_period();
  	}
  	
  	return;
}

void task_IPC_receive()
{
	int8_t sock;
	int8_t ret;
	uint8_t *rx_buf;
	int8_t len;
	int8_t data;
	uint16_t srcAddr;
	uint8_t srcPort;
	
	sock = create_socket(SOCK_IPC);
	if(sock == NRK_ERROR)
	{
		nrk_kprintf(PSTR("RX: Error in creating socket in task: "));
		print_nw_stack_errno(nrk_errno_get());
	}
	
	ret = bind(sock, RX_PORT);
	if(ret == NRK_ERROR)
	{	
		nrk_kprintf(PSTR("RX: Error in binding to port: "));
		print_nw_stack_errno(nrk_errno_get());
	}
	
	ret = set_rx_queue_size(sock, SERVER_RQS);
	if(ret == NRK_ERROR)
	{
		nrk_kprintf(PSTR("RX: Error in setting rx queue size: "));
		print_nw_stack_errno(nrk_errno_get());
	}
	
	ret = set_excess_policy(NORMAL_PRIORITY, DROP);
	if(ret == NRK_ERROR)
	{
		nrk_kprintf(PSTR("RX: Error in setting excess policy: "));
		print_nw_stack_errno(nrk_errno_get());
	}
	
	while(1)
	{
		set_timeout(sock, 10, 0);			
		rx_buf = receive(sock, &len, &srcAddr, &srcPort, NULL);		// wait for server reply
	
		if(rx_buf == NULL)		// receive() failed  
		{	
			nrk_kprintf(PSTR("RX: Error in receiving data: "));
			print_nw_stack_errno(nrk_errno_get());
		}
		else
		{
			// retrieve the data
			data = rx_buf[0];
			if(DEBUG_APP == 0)
			{
				nrk_kprintf(PSTR("RX: Releasing buffer back to buffer manager\r\n"));
			}
			// release the buffer back to buffer manager			
			release_buffer(sock, rx_buf);
		
			// print what server sent back
			if(DEBUG_APP == 1)
			{
				nrk_kprintf(PSTR("RX: Got message: "));
				printf("%d from %u %u\n", data, srcAddr, srcPort);
			}
		} 
		
		nrk_wait_until_next_period();
	} //end while(1)
	
	return;
}	
*/

void task()
{
  
  int8_t sock;
  int8_t ret;
  uint8_t *rx_buf;
  int8_t len;
  uint16_t srcAddr;
  uint8_t srcPort;
  int8_t rssi;
  int8_t cnt;
  
  if(DEBUG_APP >= 1)
  {
 	printf ("task PID=%d\r\n", nrk_get_pid ());
 	printf("App1-client: Value of CTG = %d\r\n", CONNECTED_TO_GATEWAY);
 
  }
  
  
  while(1)
  {
  	nrk_wait_until_next_period();
  }
  
  /*
  sock = create_socket(SOCK_DGRAM);
  if(sock == NRK_ERROR)
  {
   	nrk_kprintf(PSTR("Error creating socket in task: "));;
   	print_nw_stack_errno(nrk_errno_get());
  }
  
  cnt = 0;
  while(1)		// loop forever
  {
		cnt = (cnt + 1) % 120;
			  		
  		s.temperature = cnt;			// build the message 
		//s.humidity = cnt + 1;
		//s.pressure = cnt + 2;
		
		// pack these readings into the tx buffer  
		tx_buf[0] = s.temperature;
		//tx_buf[1] = s.humidity;
		//tx_buf[2] = s.pressure;
		do
		{
			// this while loop controls sending the packet over the radio		
		 
			while( (ret = send(sock, tx_buf, 1, SERVER_ADDR, SERVER_PORT, NORMAL_PRIORITY)) == NRK_ERROR )
			{
				if(nrk_errno_get() == INVALID_ARGUMENT || nrk_errno_get() == INVALID_SOCKET)	// bug 
				{
					nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
					{
						nrk_kprintf(PSTR("App1-client: Error in sending the message: "));
						print_nw_stack_errno(nrk_errno_get());
					}
				}
				else if(nrk_errno_get() == NO_PORTS_AVAILABLE || nrk_errno_get() == NO_RX_BUFFERS_AVAILABLE)
					  {
					  		nrk_int_disable();
					  		nrk_led_set(RED_LED);
					  		while(1)
					  		{
					  			nrk_kprintf(PSTR("App1-client: send() returned: "));
					  			print_nw_stack_errno(nrk_errno_get());
					  		}
					  }
					  else
					  		nrk_wait_until_next_period();		// TX queue is full, wait for some time 
			} // end while()
			if(DEBUG_APP == 1)
				printf("App1 enqueued: %d %d %d \r\n", tx_buf[0], tx_buf[1], tx_buf[2]);			

			ret = wait_until_send_done(sock);	
			if(ret == NRK_ERROR)
			{
				if(nrk_errno_get() == INVALID_ARGUMENT || nrk_errno_get() == INVALID_SOCKET)	// bug 
				{
					nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
					{
						nrk_kprintf(PSTR("App1-client: Error in waiting for message to be sent: "));
						print_nw_stack_errno(nrk_errno_get());
					}
				}
			} //end if()
				
			if(DEBUG_APP == 1)
				printf("App1 sent: %d %d %d \r\n", tx_buf[0], tx_buf[1], tx_buf[2]);				
				
			set_timeout(sock, 10, 0);			
			rx_buf = receive(sock, &len, NULL, NULL, NULL);		// wait for server reply 
							
		}while(rx_buf == NULL && nrk_errno_get() == SOCKET_TIMEOUT);
				
		if(rx_buf == NULL)		// receive() failed  
		{	
			if(nrk_errno_get() != SOCKET_TIMEOUT)		// bug 
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
				{
					nrk_kprintf(PSTR("App1-client: Error in receiving server reply: "));
					print_nw_stack_errno(nrk_errno_get());
				}
			}
			else												// socket timed out, send the next packet 
			{
				if(DEBUG_APP == 2)
					nrk_kprintf(PSTR("App1-client: receive: Socket timed out\r\n"));
				continue;
			}
		}			
		
		// got something in the buffer 
		// retrieve the sensor readings sent back from server			
		s.temperature = rx_buf[0];
		//s.humidity = rx_buf[1];
		//s.pressure = rx_buf[2];

		// release the buffer back to buffer manager			
		release_buffer(sock, rx_buf);
		
		// print what server sent back
		if(DEBUG_APP == 1)
		{
			nrk_kprintf(PSTR("Server sent: "));
			printf("%d ", s.temperature);
			//printf("%d %d %d	", s.temperature, s.humidity, s.pressure);			
			printf("Cnt: %d\r\n", cnt);
		} 
		
		nrk_wait_until_next_period();
		
	} // end infinite while 
	
	*/	
	return;
} // end task
 

/**********************************************************************************************/	
void nrk_create_taskset()
{
  TASK.task = task;
  TASK.Ptos = (void *) &task_stack[NRK_APP_STACKSIZE - 1];
  TASK.Pbos = (void *) &task_stack[0];
  TASK.prio = 3;
  TASK.FirstActivation = TRUE;
  TASK.Type = BASIC_TASK;
  TASK.SchType = PREEMPTIVE;
  
  TASK.cpu_reserve.secs = 0;
  TASK.cpu_reserve.nano_secs = 300 * NANOS_PER_MS;	
  TASK.period.secs = 0;
  TASK.period.nano_secs = 500 * NANOS_PER_MS;
  
  TASK.offset.secs = 0;
  TASK.offset.nano_secs= 0;
  nrk_activate_task (&TASK);
  
  
  /*
  TX_TASK.task = task_IPC_send;
  TX_TASK.Ptos = (void *) &tx_task_stack[NRK_APP_STACKSIZE - 1];
  TX_TASK.Pbos = (void *) &tx_task_stack[0];
  TX_TASK.prio = 3;
  TX_TASK.FirstActivation = TRUE;
  TX_TASK.Type = BASIC_TASK;
  TX_TASK.SchType = PREEMPTIVE;
  
  TX_TASK.cpu_reserve.secs = 0;
  TX_TASK.cpu_reserve.nano_secs = 300 * NANOS_PER_MS;	
  TX_TASK.period.secs = 2;
  TX_TASK.period.nano_secs = 500 * NANOS_PER_MS;
  
  TX_TASK.offset.secs = 0;
  TX_TASK.offset.nano_secs= 0;
  nrk_activate_task (&TX_TASK);
  
  RX_TASK.task = task_IPC_receive;
  RX_TASK.Ptos = (void *) &rx_task_stack[NRK_APP_STACKSIZE - 1];
  RX_TASK.Pbos = (void *) &rx_task_stack[0];
  RX_TASK.prio = 6;
  RX_TASK.FirstActivation = TRUE;
  RX_TASK.Type = BASIC_TASK;
  RX_TASK.SchType = PREEMPTIVE;
  
  RX_TASK.cpu_reserve.secs = 0;
  RX_TASK.cpu_reserve.nano_secs = 300 * NANOS_PER_MS;	
  RX_TASK.period.secs = 0;
  RX_TASK.period.nano_secs = 500 * NANOS_PER_MS;
  
  RX_TASK.offset.secs = 0;
  RX_TASK.offset.nano_secs= 0;
  nrk_activate_task (&RX_TASK);
  */
  
  /*
  NW_TASK.task = nw_task;
  NW_TASK.Ptos = (void *) &nw_task_stack[NRK_APP_STACKSIZE - 1];
  NW_TASK.Pbos = (void *) &nw_task_stack[0];
  NW_TASK.prio = 3;
  NW_TASK.FirstActivation = TRUE;
  NW_TASK.Type = BASIC_TASK;
  NW_TASK.SchType = PREEMPTIVE;
  
  NW_TASK.cpu_reserve.secs = 0;
  NW_TASK.cpu_reserve.nano_secs = 300 * NANOS_PER_MS;	
  NW_TASK.period.secs = 2;
  NW_TASK.period.nano_secs = 500 * NANOS_PER_MS;
  
  NW_TASK.offset.secs = 4;
  NW_TASK.offset.nano_secs= 0;
  nrk_activate_task (&NW_TASK);
  */
  
  if(DEBUG_APP == 2)
  	nrk_kprintf(PSTR("Created the two application layer tasks\r\n"));
}
/***********************************************************************************************/
int main()
{
	// initialise the UART	
	nrk_setup_ports();  
	nrk_setup_uart(UART_BAUDRATE_115K2);
	
	
	// initialise the OS
	nrk_init();

	// clear all LEDs	
	nrk_led_clr(ORANGE_LED);		
    nrk_led_clr(BLUE_LED);			// will toggle whenever a packet is sent
    nrk_led_clr(GREEN_LED);			// will toggle whenever a packet is received
    nrk_led_clr(RED_LED);			// will light up on error
	  
	// start the clock	
	nrk_time_set(0,0);
	
	// initialise the network stack  	
	nrk_init_nw_stack();
		
	if(DEBUG_APP == 1)
		nrk_kprintf(PSTR("Entire network stack initialised\r\n"));
		
	nrk_create_taskset();		// create the set of tasks
	nrk_start();					// start this node
	
	return 0;
}
