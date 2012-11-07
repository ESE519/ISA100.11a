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
#include <nrk_stack_check.h>


NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void Task2 (void);

NRK_STK Stack3[NRK_APP_STACKSIZE];
nrk_task_type TaskThree;
void Task3 (void);


void nrk_create_taskset();

int
main ()
{
  uint8_t t;
  
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);
  
  nrk_kprintf( PSTR("Starting up...\r\n") );

  nrk_init();

 
  nrk_time_set(0,0);
  nrk_create_taskset ();
  nrk_start();
  return 0;
}

void my_timer_callback()
{
	nrk_led_toggle(ORANGE_LED);
	nrk_gpio_toggle(NRK_DEBUG_0);
	// Normally you should not call long functions like printf
	// inside a interrupt callback
	nrk_kprintf( PSTR("*** Timer interrupt!\r\n"));
}

void Task1()
{
uint16_t cnt;
uint8_t val;

printf( "My node's address is %d\r\n",NODE_ADDR );

  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  cnt=0;

  // Setup application timer with:
  //       Prescaler = 5 
  //       Compare Match = 25000
  //       Sys Clock = 7.3728 MHz
  // Prescaler 5 means divide sys clock by 1024
  // 7372800 / 1024 = 7200 Hz clock
  // 1 / 7200 = 0.138 ms per tick
  // 0.138 ms * 25000 = ~3472 ms / per interrupt callback

  val=nrk_timer_int_configure(NRK_APP_TIMER_0, 5, 25000, &my_timer_callback );
  if(val==NRK_OK) nrk_kprintf( PSTR("Callback timer setup\r\n"));
  else nrk_kprintf( PSTR("Error setting up timer callback\r\n"));

  // Zero the timer...
  nrk_timer_int_reset(NRK_APP_TIMER_0);
  // Start the timer...
  nrk_timer_int_start(NRK_APP_TIMER_0);

  while(1) {
	cnt=nrk_timer_int_read(NRK_APP_TIMER_0);
	printf( "Task1 timer=%u\r\n",cnt );
	nrk_wait_until_next_period();
	}
}

void Task2()
{
  uint8_t cnt;
  printf( "Task2 PID=%d\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
	nrk_led_toggle(GREEN_LED);
	printf( "Task2 cnt=%d\r\n",cnt );
	nrk_wait_until_next_period();
	cnt++;
  	if(cnt==25) 
		{
		nrk_led_clr(ORANGE_LED);
		nrk_kprintf( PSTR("*** Timer stopped by Task2!\r\n" ));
		nrk_timer_int_stop(NRK_APP_TIMER_0);
		}
	}
}

void Task3()
{
uint16_t cnt;
uint16_t i;
  printf( "Task3 PID=%d\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
	printf( "Task3 cnt=%d\r\n",cnt );
	nrk_wait_until_next_period();
	cnt++;
	}
}



void
nrk_create_taskset()
{
  TaskOne.task = Task1;
  TaskOne.Ptos = (void *) &Stack1[NRK_APP_STACKSIZE];
  TaskOne.Pbos = (void *) &Stack1[0];
  TaskOne.prio = 1;
  TaskOne.FirstActivation = TRUE;
  TaskOne.Type = BASIC_TASK;
  TaskOne.SchType = PREEMPTIVE;
  TaskOne.period.secs = 0;
  TaskOne.period.nano_secs = 250*NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs =  50*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);

  TaskTwo.task = Task2;
  TaskTwo.Ptos = (void *) &Stack2[NRK_APP_STACKSIZE];
  TaskTwo.Pbos = (void *) &Stack2[0];
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


  TaskThree.task = Task3;
  TaskThree.Ptos = (void *) &Stack3[NRK_APP_STACKSIZE];
  TaskThree.Pbos = (void *) &Stack3[0];
  TaskThree.prio = 3;
  TaskThree.FirstActivation = TRUE;
  TaskThree.Type = BASIC_TASK;
  TaskThree.SchType = PREEMPTIVE;
  TaskThree.period.secs = 1;
  TaskThree.period.nano_secs = 0;
  TaskThree.cpu_reserve.secs = 0;
  TaskThree.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskThree.offset.secs = 0;
  TaskThree.offset.nano_secs= 0;
  nrk_activate_task (&TaskThree);




}
