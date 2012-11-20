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
#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <adc_driver.h>


NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);


void nrk_create_taskset();
void nrk_register_drivers();
uint8_t kill_stack(uint8_t val);

int
main ()
{
  uint8_t t;
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);

  printf( "Starting up...\r\n" );

  nrk_init();
  nrk_time_set(0,0);

  nrk_register_drivers();
  nrk_create_taskset ();
  nrk_start();
  
  return 0;
}


void Task1()
{
uint16_t cnt;
int8_t fd,val,chan;
uint16_t buf[8];

printf( "My node's address is %d\r\n",NODE_ADDR );

  printf( "Task1 PID=%d\r\n",nrk_get_pid());

  // Open ADC device as read 
  fd=nrk_open(ADC_DEV_MANAGER,READ);
  if(fd==NRK_ERROR) nrk_kprintf( PSTR("Failed to open ADC driver\r\n"));
  
  cnt=0;
  chan=0;
  val=nrk_set_status(fd,ADC_CHAN,5);
  if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to set ADC status\r\n" ));
  while(1) {
	nrk_led_toggle(BLUE_LED);
/*	for(chan=0; chan<8; chan++ )
	{
	  // Example of setting the ADC channel
	  val=nrk_set_status(fd,ADC_CHAN,chan);
	  if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to set ADC status\r\n" ));
	  val=nrk_read(fd,&buf[chan],2);
	  if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to read ADC\r\n" ));

	}
	printf( "Task1 " );
 	for(chan=0; chan<8; chan++ )
	{
	printf( "chan:%d=%d ",chan,buf[chan]);
	}
	nrk_kprintf( PSTR("\r\n" ));
*/
	  val=nrk_read(fd,&buf[chan],2);
	  if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to read ADC\r\n" ));
	printf( "%d\r\n",buf[chan]);

	nrk_wait_until_next_period();
	cnt++;
	}
}


void
nrk_create_taskset()
{
  TaskOne.task = Task1;
  nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
  TaskOne.prio = 1;
  TaskOne.FirstActivation = TRUE;
  TaskOne.Type = BASIC_TASK;
  TaskOne.SchType = PREEMPTIVE;
  TaskOne.period.secs = 0;
  TaskOne.period.nano_secs = 5*NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs =  50*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);

}

void nrk_register_drivers()
{
int8_t val;

// Register the ADC device driver
// Make sure to add: 
//     #define NRK_MAX_DRIVER_CNT  
//     in nrk_cfg.h
// Make sure to add: 
//     SRC += $(ROOT_DIR)/src/drivers/platform/$(PLATFORM_TYPE)/source/adc_driver.c
//     in makefile
val=nrk_register_driver( &dev_manager_adc,ADC_DEV_MANAGER);
if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to load my ADC driver\r\n") );

}


