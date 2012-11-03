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




//------------------------------------------------------------------------------
//
//      DESCRIPTION:
//		Widom Test Application
//		
//		Just two taks: 
//			- TaskSnd sends messages using NODE_ADDR fo their priority;
//			- TaskRcv Handles received messages.
//
//
//    NOTE: Define the widom version used in $(ROOT_DIR)/src/net/widom/widom.h
//
//------------------------------------------------------------------------------

#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_error.h>
#include <widom_rf.h>
#include <widom.h>

#define MSG_PRIO 1 // NODE_ADDR is used as the priority 

#define SND_TASK_PRIO 5 
#define RCV_TASK_PRIO 6

#define SND_TASK_PERIOD_ms 1000
#define SND_TASK_RESERVE_ms SND_TASK_PERIOD_ms

#define RCV_TASK_PERIOD_ms 100
#define RCV_TASK_RESERVE_ms RCV_TASK_PERIOD_ms

NRK_STK StackTaskSnd[NRK_APP_STACKSIZE];
nrk_task_type TTaskSnd;
void TaskSnd(void);

NRK_STK StackTaskRcv[NRK_APP_STACKSIZE];
nrk_task_type TTaskRcv;
void TaskRcv(void);

void nrk_create_taskset();

uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];

//------------------------------------------------------------------------------
//      void main (void)
//
//      DESCRIPTION:
//              Startup routine
//------------------------------------------------------------------------------
int main (void)
{
	nrk_setup_ports(); 
	nrk_setup_uart (UART_BAUDRATE_115K2);

	nrk_init();

	printf( "WiDom Test Starting up... (MSG_PRIO=%d)\r\n", MSG_PRIO );

	nrk_led_set(ORANGE_LED);
	nrk_led_clr(BLUE_LED);
	nrk_led_clr(GREEN_LED);
	nrk_led_clr(RED_LED);
	
	nrk_time_set(0,0);
	nrk_create_taskset ();
	nrk_start();

	return 0;
}

//------------------------------------------------------------------------------
//      void TaskSnd (void)
//
//      DESCRIPTION:
//              Task that periodically sends a packet
//------------------------------------------------------------------------------
void TaskSnd()
{
  uint8_t ret;
  
  // Wait until the rx_task starts up the protocol
  while (!wd_started ())
    nrk_wait_until_next_period ();
    
  
	while(1) {
		nrk_led_toggle(BLUE_LED);

		// put just two bytes of payload in the packet...
		tx_buf[0]=0xCB;
		tx_buf[1]=MSG_PRIO; // put MSG_PRIO in the payload also

    // For blocking transmits, just use the following function call.
    // wd_tx_packet(tx_buf, 2, MSG_PRIO); 

    // This function transmits packets in a non-blocking manner  
    ret = wd_tx_packet_enqueue (tx_buf, 2, MSG_PRIO); 
    nrk_kprintf (PSTR ("Tx packet enqueued\r\n"));
    // This function waits on the tx_done_signal   
    //ret = wd_wait_until_tx_packet();

    // Just check to be sure signal is okay
    if(ret != NRK_OK ) 
    	nrk_kprintf (PSTR ("TX done error!\r\n"));
	
		nrk_wait_until_next_period();
	}	
}

//------------------------------------------------------------------------------
//      void TaskRcv (void)
//
//      DESCRIPTION:
//              Task that periodically checks to receive a packet
//------------------------------------------------------------------------------
void TaskRcv()
{
  uint8_t len;
  int8_t rssi;
  uint8_t *local_rx_buf;
  
	// widom init must be in a task
	wd_init (WD_CHANNEL);
      
  // This sets the next RX buffer.
  // This can be called at anytime before releasing the packet
  // if you wish to do a zero-copy buffer switch
  wd_rx_pkt_set_buffer (rx_buf, RF_MAX_PAYLOAD_SIZE);
    
	while(1) {
		nrk_led_toggle(GREEN_LED);
		
		if (wd_wait_until_rx_packet() == NRK_OK) {
      // Get the RX packet 
      local_rx_buf = wd_rx_pkt_get (&len, &rssi);
  		printf ("Rx Packet len=%u, prio=%u\r\n", len, local_rx_buf[1]);
  		//
  		// do something with packet here ...
  		//	
      //for (i = 0; i < len; i++)
      //  printf ("%c", rx_buf[i]);
      //printf ("]\r\n");
  
  		wd_rx_pkt_release();
  	}
 	
		nrk_wait_until_next_period();
	}
}

void nrk_create_taskset()
{
	TTaskSnd.task = TaskSnd;
	TTaskSnd.Ptos = (void *) &StackTaskSnd[NRK_APP_STACKSIZE-1];
	TTaskSnd.Pbos = (void *) &StackTaskSnd[0];
	TTaskSnd.prio = SND_TASK_PRIO;
	TTaskSnd.FirstActivation = TRUE;
	TTaskSnd.Type = BASIC_TASK;
	TTaskSnd.SchType = PREEMPTIVE;
	TTaskSnd.period.secs = 0;
	TTaskSnd.period.nano_secs = SND_TASK_PERIOD_ms*NANOS_PER_MS;
	TTaskSnd.cpu_reserve.secs = 0;
	TTaskSnd.cpu_reserve.nano_secs =  SND_TASK_RESERVE_ms*NANOS_PER_MS;
	TTaskSnd.offset.secs = 0;
	TTaskSnd.offset.nano_secs= 0;
	nrk_activate_task (&TTaskSnd);

	TTaskRcv.task = TaskRcv;
	TTaskRcv.Ptos = (void *) &StackTaskRcv[NRK_APP_STACKSIZE-1];
	TTaskRcv.Pbos = (void *) &StackTaskRcv[0];
	TTaskRcv.prio = RCV_TASK_PRIO;
	TTaskRcv.FirstActivation = TRUE;
	TTaskRcv.Type = BASIC_TASK;
	TTaskRcv.SchType = PREEMPTIVE;
	TTaskRcv.period.secs = 0;
	TTaskRcv.period.nano_secs = RCV_TASK_PERIOD_ms*NANOS_PER_MS;
	TTaskRcv.cpu_reserve.secs = 0;
	TTaskRcv.cpu_reserve.nano_secs =  RCV_TASK_RESERVE_ms*NANOS_PER_MS;
	TTaskRcv.offset.secs = 0;
	TTaskRcv.offset.nano_secs= 0;
	nrk_activate_task (&TTaskRcv);
}
