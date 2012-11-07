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
#include "mmc.h" 


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

// Don't put the MMC buffer in a task or it will go onto the stack!
uint8_t sectorbuffer[512];


int
main ()
{
  uint16_t div;
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);

  printf( "Starting up...\r\n" );

  nrk_init();

  nrk_led_clr(0);
  nrk_led_clr(1);
  nrk_led_clr(2);
  nrk_led_clr(3);
  
  nrk_time_set(0,0);
  nrk_create_taskset ();
  nrk_start();
  
  return 0;
}


void Task1()
{
uint16_t cnt;
int8_t val;
uint32_t sector = 0;


  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  cnt=0;
  val=mmc_init();
  printf("mmc_init returns %d\n\r", val );
  if(val!=0 ) {
	printf( "card init failed\r\n" );
	while(1);
	}
  
	printf("\nsector %ld\n\r",sector);                // show sector number
        val=mmc_readsector(sector,sectorbuffer);    // read a data sector
       	printf( "readsector returned %d\n",val );
	for(cnt=0; cnt<32; cnt++ )
		printf( "%d ",sectorbuffer[cnt] );
	printf( "\n\r" ); 

	val=sectorbuffer[0];
	val++;
	for(cnt=0; cnt<512; cnt++ )
	{
	sectorbuffer[cnt]=val;
	}

	printf( "Writting\r\n" );
	val=mmc_writesector(sector,sectorbuffer);    // read a data sector
       	printf( "writesector returned %d\n",val );
	printf( "After write:\r\n" );
	val=mmc_readsector(sector,sectorbuffer);    // read a data sector
       	printf( "readsector returned %d\n",val );
       	if(val==0)
	{
	 for(cnt=0; cnt<32; cnt++ )
		printf( "%d ",sectorbuffer[cnt] );
	printf( "\n\r" ); 
	}


  	while(1) {
	nrk_wait_until_next_period();
	}
}

void Task2()
{
  uint8_t cnt;
  printf( "Task2 PID=%d\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
	nrk_led_set(BLUE_LED);
	printf( "Task2 cnt=%d\r\n",cnt );
	cnt++;
	//if(cnt>=10) while(1);   // This will test the reservation
	//if(cnt>=10) kill_stack(100);
	nrk_wait_until_next_period();
        nrk_led_clr(BLUE_LED);
	nrk_wait_until_next_period();
	}
}



void Task3()
{
uint16_t cnt;
nrk_time_t my_time;

  printf( "Task3 PID=%d\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
	
	nrk_led_set(GREEN_LED);
	nrk_time_get(&my_time);
	printf( "Task3 cnt=%d\r\n",cnt );
	nrk_wait_until_next_period();
	nrk_led_clr(GREEN_LED);
	cnt++;
	nrk_wait_until_next_period();
	}
}

void Task4()
{
uint16_t cnt;
nrk_time_t my_time;

  printf( "Task4 PID=%d\r\n",nrk_get_pid());
  cnt=0;
  while(1) {
	
	nrk_led_set(RED_LED);
	nrk_time_get(&my_time);
	printf( "Task4 cnt=%d\r\n",cnt );
	nrk_wait_until_next_period();
	nrk_led_clr(RED_LED);
	cnt++;
	nrk_wait_until_next_period();
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
  TaskOne.period.secs = 10;
  TaskOne.period.nano_secs = 0;
  TaskOne.cpu_reserve.secs = 10;
  TaskOne.cpu_reserve.nano_secs =  0;
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
  TaskThree.period.secs = 0;
  TaskThree.period.nano_secs = 750*NANOS_PER_MS;
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
  TaskFour.period.secs = 1;
  TaskFour.period.nano_secs = 0;
  TaskFour.cpu_reserve.secs = 0;
  TaskFour.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskFour.offset.secs = 0;
  TaskFour.offset.nano_secs= 0;
  nrk_activate_task (&TaskFour);



  //printf ("Create done\r\n");
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


