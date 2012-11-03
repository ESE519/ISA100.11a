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
*  Nuno Pereira - Polytechnic Institute of Porto; nap@isep.ipp.pt
*******************************************************************************/




/*
//------------------------------------------------------------------------------
/
//      DESCRIPTION:
//          The basic implementation of WiDom for Single Broadcast Domain (SBD)
//		requires that a node in the network acts as a sync. master.
//		This is the implementation of a simple sync master. All the node does is
//		sending synchronization packets periodically.
//		
//		The period is defined according to the parameters defined for the 
//		WiDom protocol in widom.h. Any change to these parameters can cause
//		a change in the sync period and thus require that the sync. master
//		is reprogrammed 
//
//------------------------------------------------------------------------------
*/

#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <nrk.h>
#include <hal.h>
#include <basic_rf.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <widom_rf.h>
#include <widom.h>

#define SYNC_TASK_PRIO 1 

// define sync period based on value from widom include file
const uint32_t WD_SYNC_PERIOD_ms=((WD_SYNC_PERIOD_us/1000) + 1);
// task reserve equal to period
#define WD_SYNC_TASK_RESERVE_ms WD_SYNC_PERIOD_ms

NRK_STK StackTaskSndSyncFrame[NRK_APP_STACKSIZE];
nrk_task_type TTaskSndSyncFrame;
void TaskSndSyncFrame(void);

void nrk_create_taskset();

RF_TX_INFO rfTxInfo;
RF_RX_INFO rfRxInfo;
uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];

//------------------------------------------------------------------------------
//      void main (void)
//
//      DESCRIPTION:
//              Startup routine and main loop
//------------------------------------------------------------------------------
int main (void)
{
    nrk_setup_ports(); 
    nrk_setup_uart (UART_BAUDRATE_115K2);

    printf( "Sync. Node Starting up. WD_SYNC_PERIOD=%lu ms\r\n", WD_SYNC_PERIOD_ms);

    nrk_init();

	  nrk_led_set(ORANGE_LED);
	  nrk_led_clr(BLUE_LED);
	  nrk_led_clr(GREEN_LED);
	  nrk_led_clr(RED_LED);
	

    nrk_time_set(0,0);
/*
    rfRxInfo.pPayload = rx_buf;
    rfRxInfo.max_length = RF_MAX_PAYLOAD_SIZE;
*/
    wdrf_init (&rfRxInfo, WD_SYNC_CHANNEL);

    nrk_create_taskset ();
    nrk_start();

    return 0;
}

void TaskSndSyncFrame()
{
  while(1) {
	nrk_led_toggle(BLUE_LED);
	nrk_gpio_toggle(NRK_DEBUG_0);

	wdrf_tx_sync_packet();
	nrk_wait_until_next_period();
	}
}

void
nrk_create_taskset()
{
  TTaskSndSyncFrame.task = TaskSndSyncFrame;
  TTaskSndSyncFrame.Ptos = (void *) &StackTaskSndSyncFrame[NRK_APP_STACKSIZE];
  TTaskSndSyncFrame.Pbos = (void *) &StackTaskSndSyncFrame[0];
  TTaskSndSyncFrame.prio = SYNC_TASK_PRIO;
  TTaskSndSyncFrame.FirstActivation = TRUE;
  TTaskSndSyncFrame.Type = BASIC_TASK;
  TTaskSndSyncFrame.SchType = PREEMPTIVE;
  TTaskSndSyncFrame.period.secs = 0;
  TTaskSndSyncFrame.period.nano_secs = WD_SYNC_PERIOD_ms*NANOS_PER_MS;
  TTaskSndSyncFrame.cpu_reserve.secs = 0;
  TTaskSndSyncFrame.cpu_reserve.nano_secs =  WD_SYNC_TASK_RESERVE_ms*NANOS_PER_MS;
  TTaskSndSyncFrame.offset.secs = 0;
  TTaskSndSyncFrame.offset.nano_secs= 0;
  nrk_activate_task (&TTaskSndSyncFrame);
}
