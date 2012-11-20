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


#define COORDINATOR 
#define NODE_ID	    4

#if NODE_ID == 1
	#define MY_TX_SLOT  2 
	#define MY_RX_SLOT  0 
#endif

#if NODE_ID == 2
	#define MY_TX_SLOT  4 
	#define MY_RX_SLOT  2 
#endif

#if NODE_ID == 3
	#define MY_TX_SLOT  6 
	#define MY_RX_SLOT  4 
#endif


#if NODE_ID == 4
	#define MY_TX_SLOT  8 
	#define MY_RX_SLOT  6 
#endif



NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);


void nrk_create_taskset();

uint8_t tx_buf[MAX_RTL_PKT_SIZE];
uint8_t rx_buf[MAX_RTL_PKT_SIZE];

int
main ()
{
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);

  nrk_kprintf( PSTR("Starting up...\r\n") );

	
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
  uint8_t cnt;
  int8_t rssi;
  uint8_t length,slot;
  uint16_t counter;
  volatile nrk_time_t t;
  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  counter=0;
  cnt=0;

#ifdef COORDINATOR
  nrk_kprintf( PSTR( "Coordinator\r\n")); 
  rtl_init (RTL_COORDINATOR);
  nrk_led_set(RED_LED);  
#else
  nrk_kprintf( PSTR( "Mobile\r\n")); 
  rtl_init (RTL_MOBILE);
#endif

  printf( "Node %d ",NODE_ID );
  printf( "TX %d ",MY_TX_SLOT);
  printf( "RX %d\r\n",MY_RX_SLOT);

  rtl_set_schedule( RTL_TX, MY_TX_SLOT, 1 ); 
  rtl_set_schedule( RTL_RX, MY_RX_SLOT, 1 ); 
  
  rtl_start();
  
  rtl_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE);
  nrk_kprintf( PSTR("start done\r\n") );
  while(!rtl_ready())  nrk_wait_until_next_period(); 
  while(1) {
	

#if NODE_ID==1 
          
		if( rtl_tx_pkt_check(MY_TX_SLOT)!=0 )
	       		{
		  		printf( "Pending on slot %d\r\n",MY_TX_SLOT );
	       		}
	  	else 	{
	       			nrk_led_set(2);
                		cnt++;
                		sprintf( &tx_buf[PKT_DATA_START], "Hello World %d", cnt );
                		length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
                		rtl_tx_pkt( tx_buf, length, MY_TX_SLOT );
                		printf( "Sending Packet on slot %d\r\n",MY_TX_SLOT );
                		nrk_led_clr(2);

			}
	
#else 
	  if( rtl_rx_pkt_check()!=0 )
               {
                   nrk_led_set(1);
		   local_rx_buf=rtl_rx_pkt_get(&length, &rssi, &slot);
                   printf( "Got Packet on slot %d %d: ",slot,length );
                   for(i=PKT_DATA_START; i<length; i++ )
                   {
			// copy the rx buffer to the tx buffer
			// tx_buf[i]=local_rx_buf[i];
                        printf( "%c",local_rx_buf[i] );
                   }
                   nrk_kprintf( PSTR("\r\n") );
               
		if( rtl_tx_pkt_check(MY_TX_SLOT)!=0 )
	       		{
		  	printf( "Pending on slot %d\r\n",MY_TX_SLOT );
	       		}
	  	else 	{
			nrk_led_set(2);
			sprintf( &tx_buf[PKT_DATA_START], "%s %d",&local_rx_buf[PKT_DATA_START], NODE_ID );
                	length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
			rtl_tx_pkt( tx_buf, length, MY_TX_SLOT );
			printf( "Sending [%d] %s\r\n",length, &tx_buf[PKT_DATA_START] );
			//printf( "Sending Packet on slot %d\r\n",MY_TX_SLOT );
			nrk_led_clr(2);
	       		}
                   rtl_rx_pkt_release();
                   nrk_led_clr(1);
		}
#endif 
	  	
	 
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


  nrk_kprintf ( PSTR("Create done\r\n") );
}


