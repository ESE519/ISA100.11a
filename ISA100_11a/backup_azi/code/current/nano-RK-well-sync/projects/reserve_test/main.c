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
#include<nrk_reserve.h>
#include <nrk_driver.h>
#include <nrk_driver_list.h>
#include <dev_adc.h>
#include <nrk_cfg.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_events.h>



NRK_STK Stack1[NRK_APP_STACKSIZE];
NRK_STK Stack2[NRK_APP_STACKSIZE];

nrk_task_type TaskOne;
nrk_task_type TaskTwo;
void Task1(void);
void Task2(void);
int count=0;
uint8_t mine;
void nrk_create_taskset();
void error();
int main ()
{
	uint16_t div;

	nrk_setup_ports();
	nrk_setup_uart(UART_BAUDRATE_115K2);
        //nrk_register_driver((*dev_manager_adc)(0,0,NULL,0),ADC_DEV_MANAGER);
        nrk_register_driver(&dev_manager_adc,ADC_DEV_MANAGER);
       mine =nrk_set_reserve(1,0,3,1,99,&error);  // (sec, nano_sec, # access, type, id, err func ptr)
	printf( "Starting up...\r\n" );


	nrk_init();

	nrk_led_clr(0);
	nrk_led_clr(1);
	nrk_led_clr(2);

	nrk_time_set(0,0);


	nrk_create_taskset ();

	nrk_start();

	return 0;
}



void Task1()
{
               uint8_t fd;
	       uint8_t buf[10];
               
      //  fd=nrk_open(ADC_DEV_MANAGER,READ);
	while(1)
	{
	        printf("T1 start\r\n");
        	printf("access %i\r\n",nrk_status(mine,0));
                printf("ticks %li\r\n",nrk_status(mine,1));
                nrk_consume_reserve(mine);
        	printf("access1 %i\r\n",nrk_status(mine,0));
	        printf("ticks %li\r\n",nrk_status(mine,1));
	       nrk_consume_reserve(mine);
		printf("access2 %i\r\n",nrk_status(mine,0));
	        printf("ticks %li\r\n",nrk_status(mine,1));
	        nrk_consume_reserve(mine);
		printf("access3 %i\r\n",nrk_status(mine,0));
	        printf("ticks %li\r\n",nrk_status(mine,1));
		//nrk_set_status(fd,2,0);
	       // nrk_read(fd,buf,1);   
		nrk_wait_until_next_period();
	}
}


	void
Task2 ()
{                                   

	while(1)
	{

		printf("T2\r\n");
		nrk_wait_until_next_period();
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
	TaskOne.period.secs = 2;
	//TaskOne.period.nano_secs = 0;
	TaskOne.period.nano_secs = 0*NANOS_PER_MS;
	TaskOne.cpu_reserve.secs = 0;
	TaskOne.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
	TaskOne.offset.secs = 2;
	TaskOne.offset.nano_secs= 100*NANOS_PER_MS;
	nrk_activate_task (&TaskOne);

         TaskTwo.task = Task2;
	TaskTwo.Ptos = (void *) &Stack2[NRK_APP_STACKSIZE];
	TaskTwo.Pbos = (void *) &Stack2[0];
	TaskTwo.prio = 2;
	TaskTwo.FirstActivation = TRUE;
	TaskTwo.Type = BASIC_TASK;
	TaskTwo.SchType = PREEMPTIVE;
	TaskTwo.period.secs = 60;
	//TaskOne.period.nano_secs = 0;
	TaskTwo.period.nano_secs = 0*NANOS_PER_MS;
	TaskTwo.cpu_reserve.secs =55;
	TaskTwo.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
	TaskTwo.offset.secs = 60;
	TaskTwo.offset.nano_secs= 100*NANOS_PER_MS;
	nrk_activate_task (&TaskTwo);

	printf ("Create done\n\r");
}

void error()
{

         printf("generic error no 1 !!!\n");

}
