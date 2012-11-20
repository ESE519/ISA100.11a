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
*******************************************************************************/


#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>


NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void rx_task(void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void tx_task(void);


void nrk_create_taskset();

int
main ()
{
  uint8_t t;
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);

  printf( "Starting up...\r\n" );

  nrk_init();

  nrk_led_clr(ORANGE_LED);
  nrk_led_clr(BLUE_LED);
  nrk_led_set(GREEN_LED);
  nrk_led_clr(RED_LED);
 
  nrk_time_set(0,0);
  nrk_create_taskset ();
  nrk_start();
  
  return 0;
}


void rx_task()
{
char c;
nrk_sig_t uart_rx_signal;
nrk_sig_mask_t sm;

  printf( "My node's address is %d\r\n",NODE_ADDR );
  printf( "rx_task PID=%d\r\n",nrk_get_pid());

  // Get the signal for UART RX  
  uart_rx_signal=nrk_uart_rx_signal_get();
  // Register your task to wakeup on RX Data 
  if(uart_rx_signal==NRK_ERROR) nrk_kprintf( PSTR("Get Signal ERROR!\r\n") );
  nrk_signal_register(uart_rx_signal);

  while(1) {

	// Wait for UART signal
	while(nrk_uart_data_ready(NRK_DEFAULT_UART)!=0)
                {
		// Read Character
                c=getchar();
		printf( "%c",c);
		if(c=='x') nrk_led_set(GREEN_LED);
		else nrk_led_clr(GREEN_LED);
		}
	sm=nrk_event_wait(SIG(uart_rx_signal));
	if(sm != SIG(uart_rx_signal))
	nrk_kprintf( PSTR("RX signal error") );
	nrk_kprintf( PSTR("\r\ngot uart data: ") );
	}
}

void tx_task()
{
  uint8_t cnt;
  printf( "tx_task PID=%d\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
	nrk_led_toggle(BLUE_LED);
//	printf( "Task2 cnt=%d\r\n",cnt );
	nrk_wait_until_next_period();
	cnt++;
	}
}

void
nrk_create_taskset()
{
  TaskOne.task = rx_task;
  nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
  TaskOne.prio = 1;
  TaskOne.FirstActivation = TRUE;
  TaskOne.Type = BASIC_TASK;
  TaskOne.SchType = PREEMPTIVE;
  TaskOne.period.secs = 10;
  TaskOne.period.nano_secs = 250*NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs =  50*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);

  TaskTwo.task = tx_task;
  nrk_task_set_stk( &TaskTwo, Stack2, NRK_APP_STACKSIZE);
  TaskTwo.prio = 2;
  TaskTwo.FirstActivation = TRUE;
  TaskTwo.Type = BASIC_TASK;
  TaskTwo.SchType = PREEMPTIVE;
  TaskTwo.period.secs = 0;
  TaskTwo.period.nano_secs = 500*NANOS_PER_MS;
  TaskTwo.cpu_reserve.secs = 0;
  TaskTwo.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskTwo.offset.secs = 0;
  TaskTwo.offset.nano_secs= 0;
  nrk_activate_task (&TaskTwo);


}



