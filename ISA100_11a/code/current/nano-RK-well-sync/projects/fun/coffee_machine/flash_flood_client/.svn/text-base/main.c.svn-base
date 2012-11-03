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
#include <hal.h>
#include <bmac.h>
#include <ulib.h>
#include <stdio.h>
#include <include.h>
#include <basic_rf.h>
#include <avr/sleep.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_driver.h>
#include <ff_basic_sensor.h>
#include <nrk_driver_list.h>

#include "flash_flooding.h"

#define NODE_NO  	3	

#define DATA_MODE	0
#define DIAG_MODE	1

#define RSSI_THRESHOLD	(-35)

// Definitions for the flash flood task
nrk_task_type FLASH_FLOOD_TASK;
NRK_STK flash_flood_task_stack[NRK_APP_STACKSIZE];
void flash_flood_task (void);

void nrk_create_taskset ();

// Separate buffers for holding diagnostic packets and data packets
struct message_packet diag_data_pkt;
struct message_packet data_pkt;
struct message_packet pkt_buf;

// Common receive and transmit buffers
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];

// State variables to store the latest control information
uint8_t current_mode;
uint8_t control_count;
uint8_t hop_number;
uint8_t time_to_flood;
uint8_t bmac_check_rate;
uint8_t current_check_rate;
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

  nrk_register_drivers();

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

void flash_flood_task ()
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
  char tmp[4];

  printf ("flash flood task PID=%d\r\n", nrk_get_pid ());

  next_transmit_time.secs 	= 0;
  next_transmit_time.nano_secs 	= 0;

  // init bmac on channel 26 
  bmac_init (26);

  // By default the RX check rate is 100ms
  // below shows how to change that
  current_check_rate = 100;
  check_period.secs = 0;
  check_period.nano_secs = current_check_rate*NANOS_PER_MS;
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

  cnt = 0; 

  // Initially clear all the values in the data packet and the diagnostics packet
  for(i=0; i<MAX_NODES; i++)
  {
	diag_data_pkt.node_specific_data[i][0] = 0;
	diag_data_pkt.node_specific_data[i][1] = 0;
	
	data_pkt.node_specific_data[i][0] = 0;
	data_pkt.node_specific_data[i][1] = 0;
  }


  while (1) 
  {

    // Check if someone has sent a packet
    if(bmac_rx_pkt_ready())
    {
	nrk_led_set (ORANGE_LED);
    
	// Get the RX packet 
	local_rx_tx_buf = bmac_rx_pkt_get (&len, &rssi);
   	if(len != sizeof(pkt_buf))
	{
		nrk_kprintf( PSTR("Received unknown packet!!!\n\r") );
	
    		nrk_led_clr (ORANGE_LED);
		// Release the RX buffer so future packets can arrive 
    		bmac_rx_pkt_release ();
		continue;
	} 

    	memcpy(&pkt_buf, rx_buf, sizeof(pkt_buf));

    	nrk_led_clr (ORANGE_LED);
   
	// Release the RX buffer so future packets can arrive 
    	bmac_rx_pkt_release ();



	if(GET_PACKET_TYPE(pkt_buf.pkt_type_cnt) == PKT_TYPE_CONTROL || GET_PACKET_TYPE(pkt_buf.pkt_type_cnt) == PKT_TYPE_DIAG_CONTROL)
	{
		// Prune the weak command links ... BE A MAN
		if(rssi < RSSI_THRESHOLD)
		{
			printf("Packet too weak!!!\n\r");
			continue;
		
		}
	}	
	
	nrk_kprintf( PSTR("Packet details:\n\r"));
    	printf("Count\t= %d\n\r", GET_PACKET_COUNT(pkt_buf.pkt_type_cnt));
    	printf("Hop #\t= %d\n\r", pkt_buf.hop_number);
    	printf("TTF\t= %d\n\r", pkt_buf.time_to_flood);
	printf("BCR\t= %d\n\r", pkt_buf.bmac_check_rate);
	printf("Nmax\t= %d\n\r", pkt_buf.maximum_depth);
	printf("Dlev\t= %d\n\r", pkt_buf.delay_at_each_level);
	printf("Nxt\t= %d\n\r", pkt_buf.next_control_time);
	printf("Prate\t= %d\n\r", pkt_buf.data_push_rate);		


   	// Deal with the particular kind of packet	
	switch (GET_PACKET_TYPE(pkt_buf.pkt_type_cnt))
    	{

		case PKT_TYPE_DIAG_CONTROL:
		nrk_kprintf( PSTR("Type\t= Diagnostics Control Packet\n\r") );
		current_mode = DIAG_MODE;
		if(GET_PACKET_COUNT(pkt_buf.pkt_type_cnt) == control_count)
		{
			// Control packet from the same level so ignore it
			continue;
		}	
		control_count = GET_PACKET_COUNT(pkt_buf.pkt_type_cnt);
	
		// Update the state variables	
		pkt_buf.hop_number	= pkt_buf.hop_number + 1;
		hop_number 		= pkt_buf.hop_number;
		time_to_flood 		= pkt_buf.time_to_flood;
		bmac_check_rate		= pkt_buf.bmac_check_rate;
		maximum_depth		= pkt_buf.maximum_depth;
		delay_at_each_level	= pkt_buf.delay_at_each_level;
		next_control_time	= pkt_buf.next_control_time;
		data_push_rate		= pkt_buf.data_push_rate;
		for(i=0; i<(WORDS_PER_NODE);i++)
		{
			// Get the control message specific to the node
			my_control[i] = pkt_buf.node_specific_data[NODE_NO][i];
		}
		for(i=0; i<MAX_NODES; i++)
		{
			// The data present in the buffers is no longer fresh
			CLEAR_DATA_NEW(diag_data_pkt.node_specific_data[i][0]);
		}
		break;



		case PKT_TYPE_DIAG_DATA:
		nrk_kprintf( PSTR("Type\t= Diagnostics Data Packet\n\r") );
		for(i=0; i<MAX_NODES; i++)
		{
			// Update the data, only if it is valid and new
			if(IS_DATA_VALID(pkt_buf.node_specific_data[i][0]) == 1)
			{
				if(IS_DATA_NEW(pkt_buf.node_specific_data[i][0]) == 1)
				{
					for(j=0; j<WORDS_PER_NODE; j++)
					diag_data_pkt.node_specific_data[i][j] = pkt_buf.node_specific_data[i][j];
				}
			}
		}	
		break;



		case PKT_TYPE_CONTROL:
		nrk_kprintf( PSTR("Type\t= Control Packet\n\r") );
		current_mode = DATA_MODE;
		if(GET_PACKET_COUNT(pkt_buf.pkt_type_cnt) == control_count)
		{
			// Control packet from the same level so ignore it
			continue;
		}	
		control_count = GET_PACKET_COUNT(pkt_buf.pkt_type_cnt);

		// Update the state variables
		pkt_buf.hop_number	= pkt_buf.hop_number + 1;
		hop_number 		= pkt_buf.hop_number;
		time_to_flood 		= pkt_buf.time_to_flood;
		bmac_check_rate		= pkt_buf.bmac_check_rate;
		maximum_depth		= pkt_buf.maximum_depth;
		delay_at_each_level	= pkt_buf.delay_at_each_level;
		next_control_time	= pkt_buf.next_control_time;
		data_push_rate		= pkt_buf.data_push_rate;		
		for(i=0; i<(WORDS_PER_NODE);i++)
		{
			// Get the control messages specific to the node
			my_control[i] = pkt_buf.node_specific_data[NODE_NO][i];
		}	
		for(i=0; i<MAX_NODES; i++)
		{
			// The date currently in the buffers is no longer fresh
			CLEAR_DATA_NEW(data_pkt.node_specific_data[i][0]);
		}
		break;



		case PKT_TYPE_DATA:
		nrk_kprintf( PSTR("Type\t= Data Packet\n\r") );
		for(i=0; i<MAX_NODES; i++)
		{
			// Update the data, only if it is valid and new
			if(IS_DATA_VALID(pkt_buf.node_specific_data[i][0]) == 1)
			{
				if(IS_DATA_NEW(pkt_buf.node_specific_data[i][0]) == 1)
				{
					for(j=0; j<WORDS_PER_NODE; j++)
					data_pkt.node_specific_data[i][j] = pkt_buf.node_specific_data[i][j];
				}
			}
		}
		break;
        } 
		    

	if(GET_PACKET_TYPE(pkt_buf.pkt_type_cnt) == PKT_TYPE_CONTROL || GET_PACKET_TYPE(pkt_buf.pkt_type_cnt) == PKT_TYPE_DIAG_CONTROL)
	{
		// Control packets need to be sent to the children
		if(current_check_rate != bmac_check_rate)
		{
  			val=bmac_set_rx_check_rate(check_period);
  			if(val==NRK_ERROR) nrk_kprintf( PSTR("ERROR setting bmac rate\r\n" ));
		}
	
		memcpy(tx_buf, &pkt_buf, sizeof(pkt_buf));
		nrk_time_set(0, 0);

		// Compute the time when we must reply to the gateway
		next_transmit_time.secs = time_to_flood - ((hop_number * bmac_check_rate * 2)/1000) + (maximum_depth-hop_number)*delay_at_each_level;

		printf("Hop Number %d Next transmit time : %d . %d\n", hop_number, next_transmit_time.secs, next_transmit_time.nano_secs);
		
		// Non-blocking send
		// Rebroadcasts the control packet to the children
    		nrk_led_set (BLUE_LED);
    		val = bmac_tx_pkt_nonblocking (tx_buf, sizeof(struct message_packet));
    		nrk_kprintf (PSTR ("Tx packet enqueued\r\n"));

    		printf("Type\t= %d\n\r", GET_PACKET_TYPE(pkt_buf.pkt_type_cnt));
    		printf("Count\t	= %d\n\r", GET_PACKET_COUNT(pkt_buf.pkt_type_cnt));
    		printf("Hop #\t= %d\n\r", pkt_buf.hop_number);
    		printf("TTF\t= %d\n\r", pkt_buf.time_to_flood);
		printf("BCR\t= %d\n\r", pkt_buf.bmac_check_rate);
		printf("Nmax\t= %d\n\r", pkt_buf.maximum_depth);
		printf("Dlev\t= %d\n\r", pkt_buf.delay_at_each_level);
    		printf("Nxt\t= %d\n\r", pkt_buf.next_control_time);
		printf("Prate\t= %d\n\r", pkt_buf.data_push_rate);		
		for(i=0; i<(MAX_NODES);i++)
		{
			for(j=0; j<(WORDS_PER_NODE); j++)
			{
				printf("%d [%d][%d]", pkt_buf.node_specific_data[i][j], i, j);
			}
			printf("\t");
		}

    		ret = nrk_event_wait (SIG(tx_done_signal));
    		if(ret & SIG(tx_done_signal) == 0 ) 
		    nrk_kprintf (PSTR ("TX done signal error\r\n"));
    		nrk_led_clr (BLUE_LED);
	}
    }
  

    // If next_transmit_time is zero, do nothing
    if(next_transmit_time.secs == 0 && next_transmit_time.nano_secs == 0)
	continue;
    
    nrk_time_get(&current_time);
    if(current_time.secs > next_transmit_time.secs)
    {
	// Time to send the updated information that we have gathered		
	nrk_time_set(0, 0);
	next_transmit_time.secs = data_push_rate;
	printf("Next transmit time : %d . %d %d\n", next_transmit_time.secs, next_transmit_time.nano_secs, data_push_rate);
		
        if(current_mode == DATA_MODE)
	{

		// We are in the data mode, so read the sensor requested in the control packet
		fd=nrk_open(FIREFLY_SENSOR_BASIC, READ);
		
		if(fd==NRK_ERROR) nrk_kprintf(PSTR("Driver Failed\r\n"));
		
	    	val=nrk_set_status(fd,SENSOR_SELECT, my_control[0]);
	    	
		val=nrk_read(fd,&buf,1);

	    	nrk_close(fd);
		
		data_pkt.node_specific_data[NODE_NO][0] = 0;
		SET_DATA_VALID(data_pkt.node_specific_data[NODE_NO][0]);
		SET_DATA_NEW(data_pkt.node_specific_data[NODE_NO][0]);

		data_pkt.node_specific_data[NODE_NO][0] = (cnt&0x3F);
	
		data_pkt.node_specific_data[NODE_NO][1] = (buf>>4)&0xFF;

		SET_PACKET_TYPE(data_pkt.pkt_type_cnt, PKT_TYPE_DATA);
		SET_PACKET_COUNT(data_pkt.pkt_type_cnt, cnt);

		memcpy(tx_buf, &data_pkt, sizeof(data_pkt));
    		printf("Type\t= %d\n\r", GET_PACKET_TYPE(data_pkt.pkt_type_cnt));
    		printf("Count\t= %d\n\r", GET_PACKET_COUNT(data_pkt.pkt_type_cnt));
    		printf("Hop #\t= %d\n\r", data_pkt.hop_number);
    		printf("TTF\t= %d\n\r", data_pkt.time_to_flood);
		printf("BCR\t= %d\n\r", data_pkt.bmac_check_rate);
		printf("Nmax\t= %d\n\r", data_pkt.maximum_depth);
		printf("Dlev\t= %d\n\r", data_pkt.delay_at_each_level);
    		printf("Nxt\t= %d\n\r", data_pkt.next_control_time);
		printf("Prate\t= %d\n\r", data_pkt.data_push_rate);		
		for(i=0; i<(MAX_NODES);i++)
		{
			for(j=0; j<(WORDS_PER_NODE); j++)
			{
				printf("%d [%d][%d]", data_pkt.node_specific_data[i][j], i, j);
			}
			printf("\t");
		}
	}
	else
	{

		// We are in the diagnostics mode so simply send the rssi values
		SET_PACKET_TYPE(diag_data_pkt.pkt_type_cnt, PKT_TYPE_DIAG_DATA);
		SET_PACKET_COUNT(diag_data_pkt.pkt_type_cnt, cnt);

		diag_data_pkt.node_specific_data[NODE_NO][0] = 0;
		SET_DATA_VALID(diag_data_pkt.node_specific_data[NODE_NO][0]);
		SET_DATA_NEW(diag_data_pkt.node_specific_data[NODE_NO][0]);

		diag_data_pkt.node_specific_data[NODE_NO][0] = (cnt&0x3F);
	
		diag_data_pkt.node_specific_data[NODE_NO][1] = rssi;	
		
		memcpy(tx_buf, &diag_data_pkt, sizeof(diag_data_pkt));
    		printf("Type\t= %d\n\r", GET_PACKET_TYPE(diag_data_pkt.pkt_type_cnt));
    		printf("Count\t= %d\n\r", GET_PACKET_COUNT(diag_data_pkt.pkt_type_cnt));
    		printf("Hop #\t= %d\n\r", diag_data_pkt.hop_number);
    		printf("TTF\t= %d\n\r", diag_data_pkt.time_to_flood);
		printf("BCR\t= %d\n\r", diag_data_pkt.bmac_check_rate);
		printf("Nmax\t= %d\n\r", diag_data_pkt.maximum_depth);
		printf("Dlev\t= %d\n\r", diag_data_pkt.delay_at_each_level);
    		printf("Nxt\t= %d\n\r", diag_data_pkt.next_control_time);
		printf("Prate\t= %d\n\r", diag_data_pkt.data_push_rate);		
		for(i=0; i<(MAX_NODES);i++)
		{
			for(j=0; j<(WORDS_PER_NODE); j++)
			{
				printf("%d [%d][%d]", diag_data_pkt.node_specific_data[i][j], i, j);
			}
			printf("\n\r");
		}
	    	
	}
	
	// Non-blocking send
    	nrk_led_set (BLUE_LED);
    	val = bmac_tx_pkt_nonblocking (tx_buf, sizeof(struct message_packet));
    	nrk_kprintf (PSTR ("Tx packet enqueued\r\n"));
    	ret = nrk_event_wait (SIG(tx_done_signal));
    	if(ret & SIG(tx_done_signal) == 0 ) 
	    nrk_kprintf (PSTR ("TX done signal error\r\n"));
    	nrk_led_clr (BLUE_LED);
	cnt ++;
    }	 	
  }
}


void nrk_create_taskset ()
{

  // Creates the flash flood task without any reserve

  FLASH_FLOOD_TASK.task = flash_flood_task;
  FLASH_FLOOD_TASK.Ptos = (void *) &flash_flood_task_stack[NRK_APP_STACKSIZE - 1];
  FLASH_FLOOD_TASK.Pbos = (void *) &flash_flood_task_stack[0];
  FLASH_FLOOD_TASK.prio = 2;
  FLASH_FLOOD_TASK.FirstActivation = TRUE;
  FLASH_FLOOD_TASK.Type = BASIC_TASK;
  FLASH_FLOOD_TASK.SchType = PREEMPTIVE;
  FLASH_FLOOD_TASK.period.secs = 0;
  FLASH_FLOOD_TASK.period.nano_secs = 0;
  FLASH_FLOOD_TASK.cpu_reserve.secs = 0;
  FLASH_FLOOD_TASK.cpu_reserve.nano_secs =0 ;
  FLASH_FLOOD_TASK.offset.secs = 0;
  FLASH_FLOOD_TASK.offset.nano_secs = 0;
  nrk_activate_task (&FLASH_FLOOD_TASK);

  nrk_kprintf ( PSTR("Create done\r\n") );
}

void nrk_register_drivers()
{
int8_t val;

// Register the Basic FireFly Sensor device driver
// Make sure to add: 
//     #define NRK_MAX_DRIVER_CNT  
//     in nrk_cfg.h
// Make sure to add: 
//     SRC += $(ROOT_DIR)/src/drivers/platform/$(PLATFORM_TYPE)/source/ff_basic_sensor.c
//     in makefile
val=nrk_register_driver( &dev_manager_ff_sensors,FIREFLY_SENSOR_BASIC);
if(val==NRK_ERROR) nrk_kprintf( PSTR("ADC fail\r\n") );

}
