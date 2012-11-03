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
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_stack_check.h>
#include <nrk_stats.h>


NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void Task2 (void);

NRK_STK Stack3[NRK_APP_STACKSIZE];
nrk_task_type TaskThree;
void Task3 (void);


NRK_STK Stack4[NRK_APP_STACKSIZE];
nrk_task_type TaskFour;
void Task4 (void);

void nrk_create_taskset();
uint8_t kill_stack(uint8_t val);

int
main ()
{
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);

  nrk_init();

  nrk_led_clr(ORANGE_LED);
  nrk_led_clr(BLUE_LED);
  nrk_led_clr(GREEN_LED);
  nrk_led_clr(RED_LED);
 
  nrk_time_set(0,0);
  nrk_create_taskset ();
  nrk_start();
  
  return 0;
}

// This structure should be global since it is quite large!
nrk_task_stat_t my_stats;

void Task1()
{
nrk_time_t t;
uint16_t cnt;
cnt=0;
nrk_kprintf( PSTR("Nano-RK Version ") );
printf( "%d\r\n",NRK_VERSION );

printf( "My node's address is %u\r\n",NODE_ADDR );
  
printf( "Task1 PID=%u\r\n",nrk_get_pid());

  while(1) {
	nrk_led_toggle(ORANGE_LED);
	nrk_gpio_toggle(NRK_DEBUG_0);
	printf( "Task1 cnt=%u\r\n",cnt );
	nrk_wait_until_next_period();
        // Uncomment this line to cause a stack overflow
	// if(cnt>20) kill_stack(10);

	// At time 50, the OS will halt and print statistics
	// This requires the NRK_STATS_TRACKER #define in nrk_cfg.h
	 if(cnt==50)  {
		nrk_stats_display_all();
		// This will induce a kernel panic on purpose	
		nrk_halt();
		}

        // This is an example of how to access the task execution data
	if( cnt==10 ) {
	nrk_stats_get(nrk_get_pid(), &my_stats);
	nrk_kprintf( PSTR( "\r\n   Total CPU: "));
	t=_nrk_ticks_to_time(my_stats.total_ticks);
	printf( "%lu secs %lu ms", t.secs, t.nano_secs/NANOS_PER_MS );
	nrk_kprintf( PSTR( "\r\n   Time [Min,Last,Max]: "));
	t=_nrk_ticks_to_time(my_stats.min_exec_ticks);
	printf( "%lu secs %lu ms, ", t.secs, t.nano_secs/NANOS_PER_MS );
	t=_nrk_ticks_to_time(my_stats.last_exec_ticks);
	printf( "%lu secs %lu ms, ", t.secs, t.nano_secs/NANOS_PER_MS );
	t=_nrk_ticks_to_time(my_stats.max_exec_ticks);
	printf( "%lu secs %lu ms", t.secs, t.nano_secs/NANOS_PER_MS );
	nrk_kprintf( PSTR( "\r\n   Swap-ins: "));
	printf( "%lu",my_stats.swapped_in );
	nrk_kprintf( PSTR( "\r\n   Preemptions: "));
	printf( "%lu",my_stats.preempted);
	nrk_kprintf( PSTR( "\r\n   Kernel Violations: "));
	printf( "%u",my_stats.violations);
	nrk_kprintf( PSTR( "\r\n   Overflow Error Status: "));
	printf( "%u",my_stats.overflow);
	nrk_kprintf( PSTR("\r\n") );
	}

	cnt++;
	}
}

void Task2()
{
  int16_t cnt;
  printf( "Task2 PID=%u\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
	nrk_led_toggle(BLUE_LED);
	nrk_gpio_toggle(NRK_DEBUG_1);
	printf( "Task2 signed cnt=%d\r\n",cnt );
	nrk_wait_until_next_period();
	//nrk_stats_display_pid(nrk_get_pid());
	cnt--;
	}
}

void Task3()
{
uint16_t cnt;
  printf( "Task3 PID=%u\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
	nrk_led_toggle(GREEN_LED);
	nrk_gpio_toggle(NRK_DEBUG_2);
	printf( "Task3 cnt=%u\r\n",cnt );
	nrk_wait_until_next_period();
	cnt++;
	}
}

void Task4()
{
uint16_t cnt;

  printf( "Task4 PID=%u\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
	nrk_led_toggle(RED_LED);
	nrk_gpio_toggle(NRK_DEBUG_3);
	printf( "Task4 cnt=%u\r\n",cnt );
	nrk_wait_until_next_period();
	cnt++;
	}
}

void
nrk_create_taskset()
{
  nrk_task_set_entry_function( &TaskOne, Task1);
  nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
  TaskOne.prio = 1;
  TaskOne.FirstActivation = TRUE;
  TaskOne.Type = BASIC_TASK;
  TaskOne.SchType = PREEMPTIVE;
  TaskOne.period.secs = 0;
  TaskOne.period.nano_secs = 250*NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 1;
  TaskOne.cpu_reserve.nano_secs = 50*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);

  nrk_task_set_entry_function( &TaskTwo, Task2);
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


  nrk_task_set_entry_function( &TaskThree, Task3);
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


  nrk_task_set_entry_function( &TaskFour, Task4);
  nrk_task_set_stk( &TaskFour, Stack4, NRK_APP_STACKSIZE);
  TaskFour.prio = 4;
  TaskFour.FirstActivation = TRUE;
  TaskFour.Type = BASIC_TASK;
  TaskFour.SchType = PREEMPTIVE;
  TaskFour.period.secs = 2;
  TaskFour.period.nano_secs = 0;
  TaskFour.cpu_reserve.secs = 0;
  TaskFour.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskFour.offset.secs = 0;
  TaskFour.offset.nano_secs= 0;
  nrk_activate_task (&TaskFour);


}

uint8_t kill_stack(uint8_t val)
{
char bad_memory[10];
uint8_t i;
for(i=0; i<10; i++ ) bad_memory[i]=i;
for(i=0; i<10; i++ ) printf( "%d ", bad_memory[i]);
   printf( "Die Stack %d\r\n",val );
if(val>1) kill_stack(val-1);
return 0;
}


