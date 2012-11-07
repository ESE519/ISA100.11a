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
#include <nrk_events.h>
#include <nrk_timer.h>


NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void Task2 (void);

void nrk_create_taskset();

nrk_sem_t *my_semaphore;

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

  my_semaphore = nrk_sem_create(1,4);
  if(my_semaphore==NULL) nrk_kprintf( PSTR("Error creating sem\r\n" ));
  nrk_start();
  
  return 0;
}


void Task1()
{
uint16_t cnt;
int8_t v;

printf( "My node's address is %d\r\n",NODE_ADDR );

  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
	nrk_led_toggle(ORANGE_LED);
	printf( "Task1 cnt=%d\r\n",cnt );
	nrk_kprintf( PSTR("Task1 accessing semaphore\r\n"));
	v = nrk_sem_pend(my_semaphore);
	if(v==NRK_ERROR) nrk_kprintf( PSTR("T1 error pend\r\n"));
	nrk_kprintf( PSTR("Task1 holding semaphore\r\n"));
	// wait some time inside semaphore to show the effect
	nrk_wait_until_next_period();
	v = nrk_sem_post(my_semaphore);
	if(v==NRK_ERROR) nrk_kprintf( PSTR("T1 error post\r\n"));
	nrk_kprintf( PSTR("Task1 released semaphore\r\n"));
	nrk_wait_until_next_period();
	cnt++;
	}
}

void Task2()
{
  uint8_t cnt;
  int8_t v;

  printf( "Task2 PID=%d\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
        nrk_led_toggle(ORANGE_LED);
        printf( "Task1 cnt=%d\r\n",cnt );
        nrk_kprintf( PSTR("Task2 accessing semaphore\r\n"));
        v = nrk_sem_pend(my_semaphore);
	if(v==NRK_ERROR) nrk_kprintf( PSTR("T2 error pend\r\n"));
        nrk_kprintf( PSTR("Task2 holding semaphore\r\n"));
	// wait some time inside semaphore to show the effect
        nrk_wait_until_next_period();
        v = nrk_sem_post(my_semaphore);
	if(v==NRK_ERROR) nrk_kprintf( PSTR("T2 error post\r\n"));
        nrk_kprintf( PSTR("Task2 released semaphore\r\n"));
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
  TaskTwo.period.secs = 3;
  TaskTwo.period.nano_secs = 0;
  TaskTwo.cpu_reserve.secs = 0;
  TaskTwo.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskTwo.offset.secs = 0;
  TaskTwo.offset.nano_secs= 0;
  nrk_activate_task (&TaskTwo);


}



