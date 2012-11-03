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
  			
#define SERVER_ADDR 80
#define SERVER_PORT 60 
#define SERVER_RQS 4  

#define PAYLOAD MAX_APP_PAYLOAD 
#define DEBUG_APP 0  

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
  
  if(DEBUG_APP == 2)
  		printf ("task PID=%d\r\n", nrk_get_pid ());
  
  sock = create_socket(SOCK_DGRAM);
  if(sock == NRK_ERROR)
  {
   	nrk_kprintf(PSTR("Error creating socket in task: "));;
   	print_nw_stack_errno(nrk_errno_get());
  }
  
  ret = bind(sock, SERVER_PORT);
  if(ret == NRK_ERROR)
  {
   	while(1)
  		{
  			nrk_int_disable();
  			nrk_led_set(RED_LED);
  			nrk_kprintf(("App1-server: Error in binding to socket: "));
     		print_nw_stack_errno(nrk_errno_get());
     	}
  }
  ret = set_rx_queue_size(sock, SERVER_RQS);
  if(ret == NRK_ERROR)
  {
  		while(1)
  		{
  			nrk_int_disable();
  			nrk_led_set(RED_LED);
  			nrk_kprintf(("App1-server: Error in setting rx queue size: "));
      	print_nw_stack_errno(nrk_errno_get());
      }
  }
  
  cnt = -1;
  while(1)
  {
		rx_buf = receive(sock, &len, &srcAddr, &srcPort, &rssi);
			
		// retrieve the sensor readings			
		s.temperature = rx_buf[0];
		s.humidity = rx_buf[1];
		s.pressure = rx_buf[2];
		
		release_buffer(sock, rx_buf);
		
		if(DEBUG_APP == 2)
		{
			nrk_kprintf(PSTR("Server received from "));
			printf("(%d,%d)..%d %d %d\r\n", srcAddr, srcPort, s.temperature, s.humidity, s.pressure);
		}
		
		// echo these readings back to sender 
		tx_buf[0] = s.temperature; 
		tx_buf[1] = s.humidity;
		tx_buf[2] = s.pressure;
			
		while( (ret = send(sock, tx_buf, len, srcAddr, srcPort, NORMAL_PRIORITY)) == NRK_ERROR )
		{
			if(nrk_errno_get() == INVALID_ARGUMENT || nrk_errno_get() == INVALID_SOCKET)	// bug 
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
				{
					nrk_kprintf(PSTR("App1-server: Error in sending the message: "));
					print_nw_stack_errno(nrk_errno_get());
				}
			}
			else if(nrk_errno_get() == NO_PORTS_AVAILABLE || nrk_errno_get() == NO_RX_BUFFERS_AVAILABLE)
		   	  {
		  				nrk_int_disable();
		  				nrk_led_set(RED_LED);
		  				while(1)
		  				{
		  					nrk_kprintf(PSTR("App1-server: send() returned: "));
		  					print_nw_stack_errno(nrk_errno_get());
		  				}
		   	  }
		   	  else
						nrk_wait_until_next_period();	// TX queue is full wait for some time 
		} // end while()
		
		// at this point the packet is enqueued in the transmit queue
	}
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
  TASK.cpu_reserve.nano_secs = 800 * NANOS_PER_MS;	
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
	if(DEBUG_APP == 2)
		nrk_kprintf(PSTR("Network stack initialised\r\n"));
	
	nrk_create_taskset();		// create the set of tasks
	nrk_start();					// start this node
	
	return 0;
}
