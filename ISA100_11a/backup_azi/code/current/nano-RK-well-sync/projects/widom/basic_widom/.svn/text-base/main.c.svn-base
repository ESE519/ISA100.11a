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
*  Nuno Pereira 
*******************************************************************************/


#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_stack_check.h>
#include <widom.h>

#define MSG_PRIO NODE_ADDR // NODE_ADDR is used as the priority 

#define SND_TASK_PRIO 2 
#define RCV_TASK_PRIO 3

#define SND_TASK_PERIOD_ms 100
#define SND_TASK_RESERVE_ms SND_TASK_PERIOD_ms

#define RCV_TASK_PERIOD_ms 100
#define RCV_TASK_RESERVE_ms RCV_TASK_PERIOD_ms

NRK_STK stack_task_snd[NRK_APP_STACKSIZE];
nrk_task_type type_task_snd;
void task_snd(void);

NRK_STK stack_task_rcv[NRK_APP_STACKSIZE];
nrk_task_type type_task_rcv;
void task_rcv(void);

void nrk_create_taskset();

uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];

int main ()
{
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);
  nrk_init();

  printf( "Starting up... (NODE_ADDR=%d)\r\n", NODE_ADDR);

  nrk_led_clr(ORANGE_LED);
  nrk_led_clr(BLUE_LED);
  nrk_led_set(GREEN_LED);
  nrk_led_clr(RED_LED);

	// widom init (must be in main unless wd_timer_cpu_clk.h uses timer/counter1)
	wd_init(WD_CHANNEL);  

#ifdef CONTINUOUS_SEND
  // Testing purposes only; The node will try to access the medium 
  // continuously, acting like it is always backlogged.
  // defined in widom.h 

	// put just two bytes of payload in the packet...
	tx_buf[0]=0xCB;
	tx_buf[1]=MSG_PRIO; // put MSG_PRIO in the payload also

  // This function transmits packets in a non-blocking manner  
  wd_tx_packet_enqueue (tx_buf, 2, MSG_PRIO);
  
  while (1); 
#endif

  nrk_time_set(0,0);
  nrk_create_taskset();  
  nrk_start();
  
  return 0;
}


void task_snd()
{
  uint8_t ret;
  nrk_sig_t tx_done_signal;
  uint16_t count=1;
  
  // Wait until the rx_task starts up the protocol
  while (!wd_started ())
    nrk_wait_until_next_period ();
    
  nrk_wait_until_next_period (); // wait one more period after init
  tx_done_signal = wd_get_tx_signal ();
  nrk_signal_register (tx_done_signal);  

	while(1) {
		nrk_led_toggle(ORANGE_LED);

		// put just two bytes of payload in the packet...
		tx_buf[0]=0xCB;
		tx_buf[1]=MSG_PRIO; // put MSG_PRIO in the payload also

    // For blocking transmits, just use the following function call.
    // wd_tx_packet(tx_buf, 2, MSG_PRIO); 

    // This function transmits packets in a non-blocking manner  
    ret = wd_tx_packet_enqueue (tx_buf, 2, MSG_PRIO); 
//    if (ret == NRK_OK) printf ("(%u) Tx packet enqueued\r\n", count);
//    else printf ("(%u) Tx packet NOT enqueued\r\n", count);
    if (ret == NRK_OK) nrk_kprintf (PSTR ("t"));
    else nrk_kprintf (PSTR ("\r\nTx Enqueue error.\r\n"));

		// This function waits on the tx_done_signal   
  	//ret = wd_wait_until_tx_packet();
		//if(ret != NRK_OK ) nrk_kprintf (PSTR ("TX error!\r\n"));  // Just check result
		
	  // Or, we do it here...
  	ret = nrk_event_wait (SIG(tx_done_signal) | SIG(nrk_wakeup_signal));
		//if(ret & SIG(tx_done_signal)) 
    //		printf ("(%u) TX send done \r\n", count++); // Just check result (signal is okay)
    
		nrk_wait_until_next_period();
	}	 
}

void task_rcv()
{
  uint8_t len;
  int8_t rssi;
  uint8_t *local_rx_buf;
  uint16_t count=1;
      
  // Wait until the rx_task starts up the protocol
  while (!wd_started ()) nrk_wait_until_next_period ();
	//printf("ok \r\n");
      
  // This sets the next RX buffer.
  // This can be called at anytime before releasing the packet
  // if you wish to do a zero-copy buffer switch
  wd_rx_pkt_set_buffer (rx_buf, RF_MAX_PAYLOAD_SIZE);
    
	while(1) {
	        //printf("ok 1 \r\n");
		if (wd_wait_until_rx_packet() == NRK_OK) {
		  	nrk_led_toggle(BLUE_LED);
		  
      // Get the RX packet 
      local_rx_buf = wd_rx_pkt_get (&len, &rssi);
      //if (local_rx_buf!=NULL)	printf ("(%u) Rx Packet len=%u, prio=%u\r\n", count++, len, local_rx_buf[1]);
      nrk_kprintf (PSTR ("r"));
      
      
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
  type_task_snd.task = task_snd;
  nrk_task_set_stk( &type_task_snd, stack_task_snd, NRK_APP_STACKSIZE);
  type_task_snd.prio = SND_TASK_PRIO;
  type_task_snd.FirstActivation = TRUE;
  type_task_snd.Type = BASIC_TASK;
  type_task_snd.SchType = PREEMPTIVE;
  type_task_snd.period.secs = 0;
  type_task_snd.period.nano_secs = SND_TASK_PERIOD_ms*NANOS_PER_MS;
  type_task_snd.cpu_reserve.secs = 0;
  type_task_snd.cpu_reserve.nano_secs = 0; //SND_TASK_RESERVE_ms*NANOS_PER_MS;
  type_task_snd.offset.secs = 0;
  type_task_snd.offset.nano_secs= 0;
  nrk_activate_task (&type_task_snd);

  type_task_rcv.task = task_rcv;
  nrk_task_set_stk( &type_task_rcv, stack_task_rcv, NRK_APP_STACKSIZE);
  type_task_rcv.prio = RCV_TASK_PRIO;
  type_task_rcv.FirstActivation = TRUE;
  type_task_rcv.Type = BASIC_TASK;
  type_task_rcv.SchType = PREEMPTIVE;
  type_task_rcv.period.secs = 0;
  type_task_rcv.period.nano_secs = RCV_TASK_PERIOD_ms*NANOS_PER_MS;
  type_task_rcv.cpu_reserve.secs = 0;
  type_task_rcv.cpu_reserve.nano_secs = 0; //RCV_TASK_RESERVE_ms*NANOS_PER_MS;
  type_task_rcv.offset.secs = 0;
  type_task_rcv.offset.nano_secs= 0;
  nrk_activate_task (&type_task_rcv);
}
