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
#define DEBUG_APP 1

nrk_task_type TASK;
NRK_STK task_stack[NRK_APP_STACKSIZE];
void task(void);


/**************************************************************************************/
/**************************************************************************************/

void task ()
{
  if(DEBUG_APP >= 1)
  		printf ("task PID=%d\r\n", nrk_get_pid ());
  
  
  while(1)
  {
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
  TASK.cpu_reserve.nano_secs = 300 * NANOS_PER_MS;	
  TASK.period.secs = 0;
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
	
	// initialise the network stack  	
	nrk_init_nw_stack();
		
	nrk_kprintf(PSTR("Entire network stack initialised\r\n"));
		
	nrk_create_taskset();		// create the set of tasks
	nrk_start();					// start this node
	
	return 0;
}
