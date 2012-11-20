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

/*
 * Contributing authors (this file):
 * Anthony Rowe
 * Mark Hamilton
 */

#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_stack_check.h>
#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <adc_driver.h>

NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void Task2(void);

void nrk_create_taskset();
void nrk_register_drivers();

int
main ()
{
  uint16_t i;
	nrk_init_hardware();
	nrk_setup_ports();
	nrk_setup_uart(UART_BAUDRATE_115K2);

	nrk_init();
	nrk_time_set(0,0);
	nrk_register_drivers();
	nrk_create_taskset();

	printf("Starting up...\r\n");
  P1DIR = 0xf0;
  P2DIR = 0x7e;
  P2OUT |= 0x02; // ASIC START_UP high
  P2OUT &= ~0x08; // ASIC UP_RESET low
  for(i=0; i<30000; i++); // Delay
  P1OUT |= 0x40; // Set ECG gain
  for(i=0; i<30000; i++); // Delay
  P2OUT &= ~0x02; // ASIC START_UP low
  for(i=0; i<30000; i++); // Delay
  P2OUT |= 0x08; // ASIC UP_RESET high (disabled - active low)


	nrk_start();

	return 0;
}

void Task1()
{
	uint16_t cnt;
	int8_t fd,val,chan;
	uint16_t buf[8];
  int last;

  //EEG init
  /*
  nrk_gpio_clr(UP_RESET);
  nrk_gpio_set(UP_RESET);
  nrk_gpio_clr(UP_RESET);
  nrk_gpio_clr(UP_A0_ECG);
  nrk_gpio_clr(UP_A1_ECG);
  nrk_gpio_clr(UP_A2_ECG);
  nrk_gpio_set(ENABLE1);
  */

	fd=nrk_open(ADC_DEV_MANAGER,READ);
  if(fd==NRK_ERROR) nrk_kprintf( "Failed to open ADC driver\r\n");
  
  cnt=0;
  chan=0;
  last=0;
  val=nrk_set_status(fd,ADC_CHAN,0);
  if(val==NRK_ERROR) nrk_kprintf( "Failed to set ADC status\r\n");


	while(1) {
		nrk_gpio_toggle(NRK_MISC1);

    P1OUT |= 0x80; //clock asic
    if((P2IN & 0x01) && !last) { //if we're reading the ecg channel
      val=nrk_read(fd,&buf[chan],2);
      if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to read ADC\r\n" ));
      printf( "%d,",buf[chan]);
      last=1;
    } else {
      last=0;
    }
    P1OUT &= ~0x80;

		//nrk_wait_until_next_period();
	}
}

void Task2()
{
	while(1) {
		nrk_gpio_toggle(NRK_MISC2);
		printf("\r\n\r\n\r\nTask 2\r\n\r\n\r\n");
		nrk_wait_until_next_period();
	}
}

void
nrk_create_taskset()
{
	nrk_task_set_entry_function(&TaskOne, Task1);
	TaskOne.Ptos = (void *) &Stack1[NRK_APP_STACKSIZE];
	TaskOne.Pbos = (void *) &Stack1[0];
	TaskOne.prio = 1;
	TaskOne.FirstActivation = TRUE;
	TaskOne.Type = BASIC_TASK;
	TaskOne.SchType = PREEMPTIVE;
	TaskOne.period.secs = 0;
	TaskOne.period.nano_secs = 10*NANOS_PER_MS;
	TaskOne.cpu_reserve.secs = 0;
	TaskOne.cpu_reserve.nano_secs =  50*NANOS_PER_MS;
	TaskOne.offset.secs = 0;
	TaskOne.offset.nano_secs = 0;
	nrk_activate_task (&TaskOne);

/*
	nrk_task_set_entry_function(&TaskTwo, Task2);
	TaskTwo.Ptos = (void *) &Stack2[NRK_APP_STACKSIZE];
	TaskTwo.Pbos = (void *) &Stack2[0];
	TaskTwo.prio = 2;
	TaskTwo.FirstActivation = TRUE;
	TaskTwo.Type = BASIC_TASK;
	TaskTwo.SchType = PREEMPTIVE;
	TaskTwo.period.secs = 5;
	TaskTwo.period.nano_secs = 0;
	TaskTwo.cpu_reserve.secs = 0;
	TaskTwo.cpu_reserve.nano_secs =  50*NANOS_PER_MS;
	TaskTwo.offset.secs = 0;
	TaskTwo.offset.nano_secs = 0;
	nrk_activate_task (&TaskTwo);
*/
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
if(val==NRK_ERROR) nrk_kprintf( "Failed to load my ADC driver\r\n");
}
