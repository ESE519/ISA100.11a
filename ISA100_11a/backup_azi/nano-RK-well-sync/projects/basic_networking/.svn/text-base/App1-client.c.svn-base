/******************************************************************************
*  Nano-RK, a real-time operating system for sensor networks.
*  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
*  All rights reserved.
*
*  This is the Open Source Version of Nano-RK included as part of a Dual
*  Licensing Model. If you are unsure which license to use please refer to:
*  http://www.nanork.org/nano-RK/wiki/Licensing
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, version 2.0 of the License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Contributing Authors (specific to this file):
*  Aditya Bhave 
*******************************************************************************/


// This is an example application to test the buffer manager and the network stack 

#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_error.h>
#include <stdint.h>


#include "NWStackConfig.h"
#include "TransportLayerUDP.h"
#include "NetworkLayer.h"
#include "Pack.h"
#include "Serial.h"
#include "BufferManager.h"
#include "Debug.h"
  			
#define SERVER_ADDR 5   
#define SERVER_PORT 60 
#define SERVER_RQS 4  

#define PAYLOAD MAX_APP_PAYLOAD 
#define DEBUG_APP 2 

nrk_task_type TASK;
NRK_STK task_stack[NRK_APP_STACKSIZE];
void task(void);


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

void task ()
{
  int8_t sock;
  
  int8_t ret;

  uint8_t *rx_buf;
  int8_t len;
  uint16_t srcAddr;
  uint8_t srcPort;
  int8_t rssi;

  int8_t cnt;
  
  printf ("task PID=%d\r\n", nrk_get_pid ());
  
  sock = create_socket(SOCK_DGRAM);
  if(sock == NRK_ERROR)
  {
   	nrk_kprintf(PSTR("Error creating socket in task: "));;
   	print_nw_stack_errno(nrk_errno_get());
  }
  
  printf("App1-client: sock: %d\r\n", sock);
  
  cnt = 0;
  
  while(1)
  {
		cnt++;
		if(cnt == 125)
			cnt = 1; 
			  		
  		s.temperature = cnt;			// build the message 
		s.humidity = cnt + 1;
		s.pressure = cnt + 2;
		
		// pack these readings into the tx buffer  
		tx_buf[0] = s.temperature;
		tx_buf[1] = s.humidity;
		tx_buf[2] = s.pressure;
		
		//do
		//{
			// this while loop controls enqueuing the packet in the TX queue 		
		 
			while( (ret = send(sock, tx_buf, 3, SERVER_ADDR, SERVER_PORT, NORMAL_PRIORITY)) == NRK_ERROR )
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
			else	
				printf("App1 sent: %d %d %d \r\n", tx_buf[0], tx_buf[1], tx_buf[2]);				
				
			set_timeout(sock, 10, 0);			
			rx_buf = receive(sock, &len, NULL, NULL, NULL);		// wait for server reply 
							
		//}while(rx_buf == NULL && nrk_errno_get() == SOCKET_TIMEOUT);
				
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
				nrk_kprintf(PSTR("App1-client: receive: Socket timed out\r\n"));
				continue;
			}
		}			
		
		// got something in the buffer 
		// retrieve the sensor readings sent back from server			
		s.temperature = rx_buf[0];
		s.humidity = rx_buf[1];
		s.pressure = rx_buf[2];

		// release the buffer back to buffer manager			
		release_buffer(sock, rx_buf);
		
		// print what server sent back
		nrk_kprintf(PSTR("Server sent: "));
		printf("%d %d %d	", s.temperature, s.humidity, s.pressure);			
		printf("Cnt: %d\r\n\r\n", cnt); 
		
		
		nrk_wait_until_next_period();
		
	} // end infinite while 

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
  TASK.cpu_reserve.nano_secs = 500 * NANOS_PER_MS;	
  TASK.period.secs = 1;
  TASK.period.nano_secs = 0 * NANOS_PER_MS;
  
  TASK.offset.secs = 0;
  TASK.offset.nano_secs= 0;
  nrk_activate_task (&TASK);
  
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
	nrk_kprintf(PSTR("Network stack initialised\r\n"));
	
	nrk_create_taskset();		// create the set of tasks
	nrk_start();					// start this node
	
	return 0;
}
