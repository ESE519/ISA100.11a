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
#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <adc_driver.h>
#include <eeg_driver.h>

NRK_STK ECGReaderTaskStack[NRK_APP_STACKSIZE];
nrk_task_type ECGReaderTask;
void ECGBuffer(void);

NRK_STK ECGProcessorTaskStack[NRK_APP_STACKSIZE];
nrk_task_type ECGProcessorTask;
void ECGProcessor(void);

NRK_STK RadioTaskStack[NRK_APP_STACKSIZE];
nrk_task_type RadioTask;
void Radio(void);

void nrk_create_taskset();
void nrk_register_drivers();

int main ()
{
	nrk_init_hardware();
	nrk_setup_ports();
	nrk_setup_uart(UART_BAUDRATE_115K2);

	nrk_init();

	nrk_time_set(0,0);
  nrk_register_drivers();
	nrk_create_taskset ();
	nrk_start();

	return 0;
}

void ECGReader() {
	uint16_t cnt;
	int8_t fd,val,chan;
	uint16_t buf[8];

	fd=nrk_open(IMEC_EEG_MANAGER,READ);
  if(fd==NRK_ERROR) nrk_kprintf( "Failed to open EEG driver\r\n");
  
  cnt=0;
  chan=0;
  val=nrk_set_status(fd,EEG_CHAN,1);
  if(val==NRK_ERROR) nrk_kprintf( "Failed to set EEG status\r\n");
  val=nrk_set_status(fd,ECG_GAIN,5);
  if(val==NRK_ERROR) nrk_kprintf( "Failed to set EEG gain\r\n");

	while(1) {
		nrk_gpio_toggle(NRK_MISC1);
		//printf("Task 1\r\n");
	  val=nrk_read(fd,&buf[chan],2);
	  if(val==NRK_ERROR) nrk_kprintf("Failed to read EEG\r\n");
		printf( "EEG: %d\r\n",buf[chan]);

		nrk_wait_until_next_period();
		cnt++;
	}

}

void ECGProcessor() {
}

void Radio() {
}

void nrk_create_taskset() {
	nrk_task_set_entry_function(&ECGReaderTask, ECGReader);
	ECGReaderTask.Ptos = (void *) &ECGReaderTaskStack[NRK_APP_STACKSIZE];
	ECGReaderTask.Pbos = (void *) &ECGReaderTaskStack[0];
	ECGReaderTask.prio = 1;
	ECGReaderTask.FirstActivation = TRUE;
	ECGReaderTask.Type = BASIC_TASK;
	ECGReaderTask.SchType = PREEMPTIVE;
	ECGReaderTask.period.secs = 0;
	ECGReaderTask.period.nano_secs = 600*NANOS_PER_MS;
	ECGReaderTask.cpu_reserve.secs = 0;
	ECGReaderTask.cpu_reserve.nano_secs =  50*NANOS_PER_MS;
	ECGReaderTask.offset.secs = 0;
	ECGReaderTask.offset.nano_secs= 0;
	nrk_activate_task (&ECGReaderTask);

/*
	nrk_task_set_entry_function(&ECGProcessorTask, ECGProcessor);
	ECGProcessorTask.Ptos = (void *) &ECGProcessorTaskStack[NRK_APP_STACKSIZE];
	ECGProcessorTask.Pbos = (void *) &ECGProcessorTaskStack[0];
	ECGProcessorTask.prio = 1;
	ECGProcessorTask.FirstActivation = TRUE;
	ECGProcessorTask.Type = BASIC_TASK;
	ECGProcessorTask.SchType = PREEMPTIVE;
	ECGProcessorTask.period.secs = 0;
	ECGProcessorTask.period.nano_secs = 600*NANOS_PER_MS;
	ECGProcessorTask.cpu_reserve.secs = 0;
	ECGProcessorTask.cpu_reserve.nano_secs =  50*NANOS_PER_MS;
	ECGProcessorTask.offset.secs = 0;
	ECGProcessorTask.offset.nano_secs= 0;
	nrk_activate_task (&ECGProcessorTask);

	nrk_task_set_entry_function(&RadioTask, Radio);
	RadioTask.Ptos = (void *) &RadioTaskStack[NRK_APP_STACKSIZE];
	RadioTask.Pbos = (void *) &RadioTaskStack[0];
	RadioTask.prio = 1;
	RadioTask.FirstActivation = TRUE;
	RadioTask.Type = BASIC_TASK;
	RadioTask.SchType = PREEMPTIVE;
	RadioTask.period.secs = 0;
	RadioTask.period.nano_secs = 600*NANOS_PER_MS;
	RadioTask.cpu_reserve.secs = 0;
	RadioTask.cpu_reserve.nano_secs =  50*NANOS_PER_MS;
	RadioTask.offset.secs = 0;
	RadioTask.offset.nano_secs= 0;
	nrk_activate_task (&RadioTask);
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

val=nrk_register_driver( &dev_manager_eeg,IMEC_EEG_MANAGER);
if(val==NRK_ERROR) nrk_kprintf( "Failed to load EEG driver\r\n");
}
