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
*  Karthik Lakshmanan 
*******************************************************************************/


#include <nrk.h>
#include <basic_rf.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <bmac.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <ff_basic_sensor.h>
#include "flash_flooding.h"


#define NODE_NO 			0

#define DATA_MODE			0
#define DIAG_MODE			1

#define FLOOD_RATE			24
#define TIME_TO_FLOOD_IN_SECS		4
#define BMAC_CHECK_RATE_IN_MS		100
#define MAXIMUM_DEPTH			4	
#define DELAY_AT_EACH_LEVEL_IN_SECS	2
#define NEXT_CONTROL_TIME		0


/* Setting for the gateway		*/
#define MODE				DATA_MODE
#define SENSOR				LIGHT



// Definitions for the gateway task
nrk_task_type GATEWAY_TASK;
NRK_STK gateway_task_stack[NRK_APP_STACKSIZE];
void gateway_task (void);

void nrk_create_taskset ();


// Separate buffers for diagnostic packets and data packets
struct message_packet diag_data_pkt;
struct message_packet data_pkt;
struct message_packet pkt_buf;


// Common receive and transmit buffers
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];


// State variables reflecting the current control parameters
uint8_t current_mode;
uint8_t control_count;
uint8_t hop_number;
uint8_t time_to_flood;
uint8_t bmac_check_rate;
uint8_t maximum_depth;
uint8_t delay_at_each_level;
uint8_t next_control_time;
uint8_t data_push_rate;
uint8_t my_control[WORDS_PER_NODE]; 

 

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



void gateway_task ()
{
  uint8_t i, len;
  int8_t rssi, val;
  uint8_t *local_rx_tx_buf;
  uint16_t batt;
  nrk_time_t check_period, wait_time;
  uint16_t buf;
  int8_t fd;

  uint8_t j, cnt, level;
  nrk_sig_t tx_done_signal;
  nrk_sig_mask_t ret;
  nrk_time_t next_transmit_time;
  nrk_time_t current_time;
  nrk_time_t prev_time;
  char tmp[4];

  printf ("gateway_task PID=%d\r\n", nrk_get_pid ());

  next_transmit_time.secs 	= 0;
  next_transmit_time.nano_secs 	= 0;

  // init bmac on channel 25 
  bmac_init (26);

  // By default the RX check rate is 100ms
  // below shows how to change that
  check_period.secs=0;
  check_period.nano_secs=100*NANOS_PER_MS;
  val=bmac_set_rx_check_rate(check_period);

  // The default Clear Channel Assement RSSI threshold is -32
  // Setting this value higher means that you will only trigger
  // receive with a very strong signal.  Setting this lower means
  // bmac will try to receive fainter packets.  If the value is set
  // too high or too low performance will suffer greatly.
  // bmac_set_cca_thresh(-36); 
  
  if(val==NRK_ERROR) nrk_kprintf( PSTR("ERROR setting bmac rate\r\n" ));

  // This sets the next RX buffer.
  // This can be called at anytime before releaseing the packet
  // if you wish to do a zero-copy buffer switch
  bmac_rx_pkt_set_buffer (rx_buf, RF_MAX_PAYLOAD_SIZE);
  
  // Get and register the tx_done_signal if you want to
  // do non-blocking transmits
  tx_done_signal = bmac_get_tx_done_signal ();
  nrk_signal_register (tx_done_signal);

  nrk_time_set(0, 0);
  cnt = 0;
  // Initially clear all the data buffers
  for(i=0; i<(MAX_NODES);i++)
  {
	for(j=0; j<(WORDS_PER_NODE); j++)
	{
		data_pkt.node_specific_data[i][j] = 0;		
	}
  }
  while (1) 
  {

  	nrk_time_get(&current_time);

	// If it is time to flood the network, send the control packets
	if(current_time.secs%FLOOD_RATE == 0 && prev_time.secs != current_time.secs || cnt == 0)
   	{
			
		prev_time = current_time;

		cnt ++;
	  	SET_PACKET_COUNT(pkt_buf.pkt_type_cnt, cnt);
#if MODE==DATA_MODE
		SET_PACKET_TYPE(pkt_buf.pkt_type_cnt, PKT_TYPE_CONTROL); 
#else
		SET_PACKET_TYPE(pkt_buf.pkt_type_cnt, PKT_TYPE_DIAG_CONTROL);
#endif
		pkt_buf.hop_number 		= 0;
	  	pkt_buf.time_to_flood		= TIME_TO_FLOOD_IN_SECS;
  		pkt_buf.bmac_check_rate 	= BMAC_CHECK_RATE_IN_MS;
	  	pkt_buf.maximum_depth 		= MAXIMUM_DEPTH;
  		pkt_buf.delay_at_each_level	= DELAY_AT_EACH_LEVEL_IN_SECS;
	  	pkt_buf.next_control_time	= NEXT_CONTROL_TIME;
  		pkt_buf.data_push_rate		= 0;	
	  	for(i=0; i<(MAX_NODES);i++)
  		{
			for(j=0; j<(WORDS_PER_NODE); j++)
			{
#if MODE==DATA_MODE
				pkt_buf.node_specific_data[i][j] = SENSOR;
#else
				pkt_buf.node_specific_data[i][j] = 0;
#endif
			}
	  	}	
	
  		memcpy(tx_buf, &pkt_buf, sizeof(pkt_buf));

		// Non-blocking send
	  	nrk_led_set (BLUE_LED);
  		val = bmac_tx_pkt_nonblocking (tx_buf, sizeof(struct message_packet));
	  	nrk_kprintf (PSTR ("Tx packet enqueued\r\n"));
  		ret = nrk_event_wait (SIG(tx_done_signal));
	  	if(ret & SIG(tx_done_signal) == 0 ) 
    			nrk_kprintf (PSTR ("TX done signal error\r\n"));
	  	nrk_led_clr (BLUE_LED);
	  }
  
	  // If a new packet has been received, then update the information
	  if(bmac_rx_pkt_ready())
	  {	
    		// Wait until an RX packet is received
		nrk_led_set (ORANGE_LED);
    	
		 // Get the RX packet 
		local_rx_tx_buf = bmac_rx_pkt_get (&len, &rssi);
    
    		for (i = 0; i < len; i++)
	      	printf ("%d", rx_buf[i]);
   		printf ("\r\n");

	    	memcpy(&pkt_buf, rx_buf, sizeof(pkt_buf));
	
    		nrk_led_clr (ORANGE_LED);
   
		// Release the RX buffer so future packets can arrive 
    		bmac_rx_pkt_release ();

		// If the received packet is a data packet then update the information, gateway just ignores the control packets
		if(GET_PACKET_TYPE(pkt_buf.pkt_type_cnt) == PKT_TYPE_DIAG_DATA || GET_PACKET_TYPE(pkt_buf.pkt_type_cnt) == PKT_TYPE_DATA)
		{
 
  	
			printf("Type 	= %d\n\r", GET_PACKET_TYPE(pkt_buf.pkt_type_cnt));
			printf("Count 	= %d\n\r", GET_PACKET_COUNT(pkt_buf.pkt_type_cnt));
			printf("Hop #	= %d\n\r", pkt_buf.hop_number);
			printf("TTF	= %d\n\r", pkt_buf.time_to_flood);
		  	printf("BCR	= %d\n\r", pkt_buf.bmac_check_rate);
		  	printf("Nmax 	= %d\n\r", pkt_buf.maximum_depth);
		  	printf("Dlev	= %d\n\r", pkt_buf.delay_at_each_level);
		  	printf("Nxt	= %d\n\r", pkt_buf.next_control_time);
		  	printf("Prate	= %d\n\r", pkt_buf.data_push_rate);		
  	
			for(i=0; i<(MAX_NODES);i++)
		  	{
				for(j=0; j<(WORDS_PER_NODE); j++)
				{
					if(pkt_buf.node_specific_data[i][j] != 0)
					data_pkt.node_specific_data[i][j] = pkt_buf.node_specific_data[i][j];		
				}
			}
			for(i=0; i<(MAX_NODES);i++)
		  	{
				printf("Value @ Node %d is %d  (cnt is %d)", i, ((int16_t)data_pkt.node_specific_data[i][1]<<4), (data_pkt.node_specific_data[i][0]&0x3F));
				printf("\n\r");
		  	}	
		}
	  }
   }	
}


void nrk_create_taskset ()
{

  GATEWAY_TASK.task = gateway_task;
  GATEWAY_TASK.Ptos = (void *) &gateway_task_stack[NRK_APP_STACKSIZE - 1];
  GATEWAY_TASK.Pbos = (void *) &gateway_task_stack[0];
  GATEWAY_TASK.prio = 2;
  GATEWAY_TASK.FirstActivation = TRUE;
  GATEWAY_TASK.Type = BASIC_TASK;
  GATEWAY_TASK.SchType = PREEMPTIVE;
  GATEWAY_TASK.period.secs = 0;
  GATEWAY_TASK.period.nano_secs = 0;
  GATEWAY_TASK.cpu_reserve.secs = 0;
  GATEWAY_TASK.cpu_reserve.nano_secs =0 ;
  GATEWAY_TASK.offset.secs = 0;
  GATEWAY_TASK.offset.nano_secs = 0;
  nrk_activate_task (&GATEWAY_TASK);

  printf ("Create done\r\n");
}
