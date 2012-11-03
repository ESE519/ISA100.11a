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
*  Anthony Rowe
*******************************************************************************/


#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <rt_link.h>
#include <nrk_error.h>

// A single node must be a coordinatior
#define COORDINATOR

#define MY_TX_SLOT  RTL_CONTENTION 



NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);


void nrk_create_taskset();

RF_TX_INFO rfTxInfo;
uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];

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

  
  rtl_task_config();
  
  nrk_create_taskset ();

  nrk_start();
  
  return 0;
}


void Task1()
{
  uint8_t j, i;
  uint8_t *local_rx_buf;
  uint8_t length,slot;
  int8_t rssi;
  uint8_t cnt;
  uint16_t counter;
  volatile nrk_time_t t;
  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  counter=0;
  cnt=0;
 

//  nrk_led_set(0);
#ifdef COORDINATOR
  rtl_init (RTL_COORDINATOR);
#else
  rtl_init (RTL_MOBILE);
#endif

//  rtl_set_schedule( RTL_RX, MY_RX_SLOT, 1 );
  //rtl_set_schedule( RTL_RX, 12, 1 );
//  rtl_set_schedule( RTL_TX, MY_TX_SLOT, 1 );
  rtl_set_contention( 8, 1 );
  rtl_start();
  
  rtl_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE);
 
  while(!rtl_ready())  nrk_wait_until_next_period(); 
  nrk_led_clr(0);
  
  while(1) {

          if( rtl_rx_pkt_check()!=0 )
               {
                   nrk_led_set(BLUE_LED);
                   local_rx_buf=rtl_rx_pkt_get(&length, &rssi, &slot);
                   printf( "Got Packet on slot %d len %d: ",slot,length );
                   for(i=PKT_DATA_START; i<length; i++ )
                   {
                        printf( "%c",local_rx_buf[i] );
                   }
                   nrk_kprintf( PSTR("\r\n") );
                   rtl_rx_pkt_release();
                   nrk_led_clr(BLUE_LED);
               }

          if( rtl_tx_pkt_check(RTL_CONTENTION)!=0 )
               {
                  nrk_kprintf( PSTR("Pending on slot RTL_CONTENTION\r\n"));
               }
          else {
                nrk_led_set(GREEN_LED);
                cnt++;
                sprintf( &tx_buf[PKT_DATA_START], "Hello World %d", cnt );
                length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START;
                nrk_kprintf( PSTR("Sending on slot RTL_CONTENTION\r\n"));
                rtl_tx_pkt( tx_buf, length, MY_TX_SLOT );
                nrk_led_clr(GREEN_LED);
               }

          rtl_wait_until_rx_or_tx();


        }


}

void
nrk_create_taskset()
{


  TaskOne.task = Task1;
  TaskOne.Ptos = (void *) &Stack1[NRK_APP_STACKSIZE-1];
  TaskOne.Pbos = (void *) &Stack1[0];
  TaskOne.prio = 2;
  TaskOne.FirstActivation = TRUE;
  TaskOne.Type = BASIC_TASK;
  TaskOne.SchType = PREEMPTIVE;
  TaskOne.period.secs = 1;
  TaskOne.period.nano_secs = 0;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);


  printf ("Create done\r\n");
}


