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
*  Zane Starr
*******************************************************************************/


#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_events.h>

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

nrk_sig_t signal_one;
nrk_sig_t signal_two;


int
main ()
{
  uint8_t cnt;
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);

  printf( "Starting up...\r\n" );

  nrk_init();

  nrk_led_clr(ORANGE_LED);
  nrk_led_clr(BLUE_LED);
  nrk_led_clr(GREEN_LED);
  nrk_led_clr(RED_LED);
                    
  signal_one=nrk_signal_create();
  signal_two=nrk_signal_create();

  nrk_time_set(0,0);
  nrk_create_taskset ();
  nrk_start();
  
  return 0;
}


void Task1()
{
int8_t v;
nrk_sig_mask_t my_sigs;
uint8_t cnt;

cnt=0;
printf( "My node's address is %d\r\n",NODE_ADDR );

  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  v=nrk_signal_register(signal_two);
  if(v==NRK_ERROR) nrk_kprintf( PSTR( "T1 nrk_signal_register failed\r\n" ));
  while(1) {
	nrk_led_toggle(ORANGE_LED);  
	// After cnt 20, stop sending the signal
	// This will test the timeout	
	if(cnt<20) 
		{
		nrk_kprintf( PSTR("Task1 sending signal 1\r\n"));   
		v=nrk_event_signal( signal_one );
  		if(v==NRK_ERROR) 
		nrk_kprintf( PSTR( "T1 nrk_event_signal failed\r\n" ));
		cnt++;
		} else nrk_kprintf( PSTR( "Task1 not sending signal!\r\n" ));

	nrk_kprintf( PSTR("Task1 waiting on signal 2\r\n"));   
	my_sigs=nrk_event_wait( SIG(signal_two) );
  	if(my_sigs==0) nrk_kprintf( PSTR( "T1 nrk_event_wait failed\r\n" ));
	if(my_sigs & SIG(signal_two))
		nrk_kprintf( PSTR( "Task1 got signal 2\r\n") );
	
	nrk_wait_until_next_period();
	}
}


void Task2()
{
int8_t v;
nrk_sig_mask_t my_sigs;
nrk_sig_t t;
nrk_time_t timeout;

  timeout.secs=10;
  timeout.nano_secs=0;
  printf( "Task2 PID=%d\r\n",nrk_get_pid());
  v=nrk_signal_register(signal_one);
  if(v==NRK_ERROR) nrk_kprintf( PSTR( "T2 nrk_signal_register failed\r\n" ));

  // You can set a next wakeup signal which can be used as a  timeout 
  // for event_signal() 
  v=nrk_signal_register(nrk_wakeup_signal);
  if(v==NRK_ERROR) nrk_kprintf( PSTR( "T2 nrk_signal_register failed\r\n" ));

  while(1) {
	nrk_led_toggle(BLUE_LED); 
	nrk_kprintf( PSTR("Task2 sending signal 2\r\n"));   
	v=nrk_event_signal( signal_two);
  	if(v==NRK_ERROR) nrk_kprintf( PSTR( "T2 nrk_event_signal failed\r\n" ));
	nrk_kprintf( PSTR("Task2 waiting on signal 1\r\n"));  

	nrk_set_next_wakeup(timeout); 
	my_sigs=nrk_event_wait( SIG(signal_one) | SIG(nrk_wakeup_signal) );

  	if(my_sigs==0) nrk_kprintf( PSTR( "T2 nrk_event_wait failed\r\n" ));
	if(my_sigs & SIG(signal_one))
		nrk_kprintf( PSTR( "Task2 got signal 1\r\n") );
	if(my_sigs & SIG(nrk_wakeup_signal))
		nrk_kprintf( PSTR( "Task2 got timeout signal! \r\n") );
		
	nrk_wait_until_next_period();
	}
}

void Task3()
{
uint16_t cnt;
cnt=0;
while(1)
	{
	nrk_led_toggle(RED_LED);
	printf( "Task3: Tick %d\r\n",cnt );
	cnt++;
	nrk_wait_until_next_period();
	}


}

void
nrk_create_taskset()
{
  TaskOne.task = Task1;
  nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
  TaskOne.prio = 5;
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
  nrk_task_set_stk( &TaskTwo, Stack2, NRK_APP_STACKSIZE);
  TaskTwo.prio = 4;
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
  nrk_task_set_stk( &TaskThree, Stack3, NRK_APP_STACKSIZE);
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

