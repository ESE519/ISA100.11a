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

extern void initialise_serial_communication();

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

  int8_t cnt = 0;
  
  if(DEBUG_APP >= 1)
  		printf ("task PID=%d\r\n", nrk_get_pid ());
  
  while(1)
  {
  	if(cnt % 2 == 0)
  		nrk_led_set(BLUE_LED);
  	else
  		nrk_led_clr(BLUE_LED);
  	//nrk_kprintf(PSTR("App task: "));
  	//printf("%d\r\n", cnt);
  	cnt = (cnt + 1) % 128;
  	
  	nrk_wait_until_next_period();
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
  TASK.cpu_reserve.nano_secs = 300 * NANOS_PER_MS;	
  TASK.period.secs = 1;
  TASK.period.nano_secs = 500 * NANOS_PER_MS;
  
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
	//printf("here");

	initialise_serial_communication();	
	nrk_create_taskset();		// create the set of tasks
	nrk_start();					// start this node
	
	return 0;
}
