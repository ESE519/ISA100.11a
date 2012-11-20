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
#include <bmac.h>
#include <nrk_defs.h>
#include <nrk_error.h>


nrk_task_type RX_TASK;
NRK_STK rx_task_stack[NRK_APP_STACKSIZE];
void rx_task (void);


nrk_task_type TX_TASK;
NRK_STK tx_task_stack[NRK_APP_STACKSIZE];
void tx_task (void);

void nrk_create_taskset ();

uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_data_ok;
uint8_t tx_data_ok;
int8_t rx_rssi;

int main ()
{
  uint16_t div;
  nrk_setup_ports ();
  nrk_setup_uart (UART_BAUDRATE_115K2);

  nrk_init ();

  nrk_led_clr (0);
  nrk_led_clr (1);
  nrk_led_clr (2);
  nrk_led_clr (3);

  nrk_time_set (0, 0);

  bmac_task_config ();

  nrk_create_taskset ();
  nrk_start ();

  return 0;
}

void rx_task ()
{
  uint8_t i, len;
  int8_t rssi, val;
  uint8_t *local_rx_buf;
  nrk_time_t check_period;
  printf ("rx_task PID=%d\r\n", nrk_get_pid ());

  // init bmac on channel 25 
  bmac_init (25);
  rx_data_ok=0;
  // By default the RX check rate is 100ms
  // below shows how to change that
  //check_period.secs=0;
  //check_period.nano_secs=200*NANOS_PER_MS;
  //val=bmac_set_rx_check_rate(check_period);

  // The default Clear Channel Assement RSSI threshold is -45
  // Setting this value higher means that you will only trigger
  // receive with a very strong signal.  Setting this lower means
  // bmac will try to receive fainter packets.  If the value is set
  // too high or too low performance will suffer greatly.
  // bmac_set_cca_thresh(-45); 


  if(val==NRK_ERROR) nrk_kprintf( PSTR("ERROR setting bmac rate\r\n" ));
  // This sets the next RX buffer.
  // This can be called at anytime before releaseing the packet
  // if you wish to do a zero-copy buffer switch
  bmac_rx_pkt_set_buffer (rx_buf, RF_MAX_PAYLOAD_SIZE);

  while (1) {
    // Wait until an RX packet is received
    val = bmac_wait_until_rx_pkt ();
    // Get the RX packet 
    nrk_led_set (ORANGE_LED);
    local_rx_buf = bmac_rx_pkt_get (&len, &rssi);
    rx_data_ok=1;
    rx_rssi=rssi;
    //printf ("Got RX packet len=%d RSSI=%d [", len, rssi);
    //for (i = 0; i < len; i++)
    //  printf ("%c", rx_buf[i]);
    //printf ("]\r\n");
    nrk_led_clr (ORANGE_LED);
    // Release the RX buffer so future packets can arrive 
    bmac_rx_pkt_release ();
  }

}


void tx_task ()
{
  uint8_t j, i, val, len, cnt;
  volatile uint8_t start;
  uint16_t ticks,ticks_min,ticks_max;
  uint16_t iterations;
  nrk_sig_t tx_done_signal;
  nrk_sig_mask_t ret;

  iterations=0;
  ticks_min=-1;
  ticks_max=0;
  tx_data_ok=0;
  printf ("tx_task PID=%d\r\n", nrk_get_pid ());

  // Wait until the tx_task starts up bmac
  // This should be called by all tasks using bmac that
  // do not call bmac_init()...
  while (!bmac_started ())
    nrk_wait_until_next_period ();

  // Get and register the tx_done_signal if you want to
  // do non-blocking transmits
  tx_done_signal = bmac_get_tx_done_signal ();
  nrk_signal_register (tx_done_signal);

  cnt = 0;
  while (1) {
    // Build a TX packet
    sprintf (tx_buf, "This is a test %d", cnt);
    cnt++;
    nrk_led_set (GREEN_LED);

    // For blocking transmits, use the following function call.
    // For this there is no need to register  
    // val=bmac_tx_packet(tx_buf, strlen(tx_buf));

    // This function shows how to transmit packets in a
    // non-blocking manner  
    val = bmac_tx_pkt_nonblocking(tx_buf, strlen (tx_buf));
    // This functions waits on the tx_done_signal
    ret = nrk_event_wait (SIG(tx_done_signal));

    // Just check to be sure signal is okay
    if(ret & SIG(tx_done_signal) == 0 ) 
    	nrk_kprintf (PSTR ("TX done signal error\r\n"));
    else tx_data_ok=1;
    // Task gets control again after TX complete
    //nrk_kprintf (PSTR ("Tx task sent data!\r\n"));
    nrk_led_clr (GREEN_LED);
    nrk_wait_until_next_period ();

   nrk_kprintf( PSTR( "Self Test Cycle: ") );
   printf( "%d\r\n",iterations);
   iterations++;
   nrk_kprintf( PSTR( "TX status: ") );
   if(tx_data_ok==1) nrk_kprintf( PSTR( "OK\r\n" ));
	else  nrk_kprintf( PSTR( "NONE\r\n" ));

   nrk_kprintf( PSTR( "RX status: ") );
   if(rx_data_ok==1) {
		nrk_led_clr(RED_LED);
		nrk_kprintf( PSTR( "OK  RSSI:" ));
		printf( "%d\r\n",rx_rssi );
	}
	else  {
	nrk_led_set(RED_LED);
	nrk_kprintf( PSTR( "NO PKT\r\n" ));
	}
   rx_data_ok=0;
   nrk_kprintf( PSTR( "Max wakeup time: " ));
   printf( "%d\r\n",nrk_max_sleep_wakeup_time );
   
   nrk_wait_until_next_period ();
   _nrk_set_next_wakeup(25);
   nrk_int_disable(); 
   	_nrk_os_timer_reset();
   	_nrk_high_speed_timer_reset(); 
   	do{
   	} while((volatile)_nrk_os_timer_get()<16);
   	ticks=_nrk_high_speed_timer_get(); 
   	
	ticks=ticks/16;
   	if(ticks<ticks_min) ticks_min=ticks;
   	if(ticks>ticks_max) ticks_max=ticks;
   	nrk_kprintf( PSTR( "OS tick time: " ));
   	printf( "%u %u %u\r\n\r\n",ticks_min,ticks,ticks_max);
   nrk_int_enable(); 

   nrk_wait_until_next_period ();
   

  }

}

void nrk_create_taskset ()
{


  RX_TASK.task = rx_task;
  nrk_task_set_stk( &RX_TASK, rx_task_stack, NRK_APP_STACKSIZE);
  RX_TASK.prio = 2;
  RX_TASK.FirstActivation = TRUE;
  RX_TASK.Type = BASIC_TASK;
  RX_TASK.SchType = PREEMPTIVE;
  RX_TASK.period.secs = 1;
  RX_TASK.period.nano_secs = 0;
  RX_TASK.cpu_reserve.secs = 1;
  RX_TASK.cpu_reserve.nano_secs = 500 * NANOS_PER_MS;
  RX_TASK.offset.secs = 0;
  RX_TASK.offset.nano_secs = 0;
  nrk_activate_task (&RX_TASK);

  TX_TASK.task = tx_task;
  nrk_task_set_stk( &TX_TASK, tx_task_stack, NRK_APP_STACKSIZE);
  TX_TASK.prio = 25;
  TX_TASK.FirstActivation = TRUE;
  TX_TASK.Type = BASIC_TASK;
  TX_TASK.SchType = PREEMPTIVE;
  TX_TASK.period.secs = 1;
  TX_TASK.period.nano_secs = 0;
  TX_TASK.cpu_reserve.secs = 0;
  TX_TASK.cpu_reserve.nano_secs = 0;
  TX_TASK.offset.secs = 0;
  TX_TASK.offset.nano_secs = 0;
  nrk_activate_task (&TX_TASK);

}
