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
#include <nrk_error.h>
#include "../../include/sampl.h"
#include "../../include/pkt_packer.h"
#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <ff_basic_sensor.h>

#define REPLY_WAIT_SECS	1
#define ADXL_THRESH	9 
#define my_subnet_mac_2	  1	
#define my_subnet_mac_1	  0	
#define my_subnet_mac_0	  0	
#define my_mac		  0x68

#define TXT_DEBUG

#define NLIST_SIZE	5
#define MAX_NEIGHBORS	16

void build_extended_neighbor_list_pkt(SAMPL_PEER_2_PEER_PKT_T *p2p_pkt,uint8_t *nlist,uint8_t nlist_size);
void build_ping_pkt(SAMPL_PEER_2_PEER_PKT_T *p2p_pkt);
void nrk_register_drivers();

nrk_task_type MOBILE_TASK;
NRK_STK mobile_task_stack[NRK_APP_STACKSIZE];
void tx_task (void);

void nrk_create_taskset ();


uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t cnt;

uint8_t my_nlist[MAX_NEIGHBORS*NLIST_SIZE];
uint8_t my_nlist_elements;

SAMPL_PEER_2_PEER_PKT_T p2p_pkt;

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
  nrk_register_drivers();

  bmac_task_config ();

  nrk_create_taskset ();
  nrk_start ();

  return 0;
}

void tx_task ()
{
  uint8_t j, i,  error,unique;
  uint8_t samples,moved,fd;
  int8_t len;
  int8_t rssi, val;
  uint8_t *local_rx_buf;
  uint16_t adxlx,last_adxlx;
  uint16_t adxly,last_adxly;
  uint16_t adxlz,last_adxlz;

  nrk_sig_t tx_done_signal;
  nrk_sig_t rx_signal;
  nrk_sig_mask_t ret;
  nrk_time_t check_period;
  nrk_time_t timeout, start, current;
  nrk_sig_mask_t my_sigs;

  printf ("tx_task PID=%d\r\n", nrk_get_pid ());


  // Wait until the tx_task starts up bmac
  // This should be called by all tasks using bmac that
  // do not call bmac_init()...
  bmac_init (26);

  bmac_rx_pkt_set_buffer (rx_buf, RF_MAX_PAYLOAD_SIZE);

  val=bmac_addr_decode_set_my_mac(((uint16_t)my_subnet_mac_0<<8)|my_mac);
  val=bmac_addr_decode_dest_mac(0xffff);  // broadcast by default
  bmac_addr_decode_enable();

  nrk_kprintf (PSTR ("bmac_started()\r\n"));
  bmac_set_cca_thresh (-45);


  check_period.secs = 0;
  check_period.nano_secs = 100 * NANOS_PER_MS;
  val = bmac_set_rx_check_rate (check_period);

  // Get and register the tx_done_signal if you want to
  // do non-blocking transmits
  tx_done_signal = bmac_get_tx_done_signal ();
  nrk_signal_register (tx_done_signal);

  rx_signal = bmac_get_rx_pkt_signal ();
  nrk_signal_register (rx_signal);

  cnt = 0;

  check_period.secs = 0;
  check_period.nano_secs = DEFAULT_CHECK_RATE * NANOS_PER_MS;
  val = bmac_set_rx_check_rate (check_period);
  adxlx=0; last_adxlx=0;
  adxly=0; last_adxly=0;
  adxlz=0; last_adxlz=0;
  while (1) {

	nrk_led_clr(ORANGE_LED);
	do {

/*
	// This section checks the accelerometer for motion
	fd=nrk_open(FIREFLY_SENSOR_BASIC,READ);
	if(fd==NRK_ERROR) nrk_kprintf(PSTR("Failed to open sensor driver\r\n"));

	moved=0;
	val=nrk_set_status(fd,SENSOR_SELECT,ACC_X);
        val=nrk_read(fd,&adxlx,2);
	if(adxlx>last_adxlx) { if(adxlx-last_adxlx>ADXL_THRESH) moved=1;	} 
	else{ if(last_adxlx-adxlx>ADXL_THRESH) moved=1;}

	nrk_close(fd);
	last_adxlx=adxlx;
	last_adxly=adxly;
	last_adxlz=adxlz;
	if(moved==0) nrk_wait_until_next_period();
	} while(moved==0);
	nrk_led_set(ORANGE_LED);
*/
	  my_nlist_elements=0;

	  for(samples=0; samples<3; samples++ )
	  {
		  nrk_led_set (GREEN_LED);
		  check_period.secs = 0;
		  check_period.nano_secs = DEFAULT_CHECK_RATE * NANOS_PER_MS;
		  val = bmac_set_rx_check_rate (check_period);
		  build_ping_pkt( &p2p_pkt );
		  // Pack data structure values in buffer before transmit
		  pack_peer_2_peer_packet(&p2p_pkt);
		  // For blocking transmits, use the following function call.
		  val = bmac_tx_pkt (p2p_pkt.buf, p2p_pkt.buf_len);

		  check_period.secs = 0;
		  check_period.nano_secs = p2p_pkt.check_rate * NANOS_PER_MS;
		  val = bmac_set_rx_check_rate (check_period);
#ifdef TXT_DEBUG
		  nrk_kprintf (PSTR ("Pinging...\r\n"));
#endif
		  nrk_led_clr (GREEN_LED);



		  // Wait for packets or timeout
		  nrk_time_get (&start);
		  while (1) {

      timeout.secs = REPLY_WAIT_SECS;
      timeout.nano_secs = 0;

      // Wait until an RX packet is received
      //val = bmac_wait_until_rx_pkt ();
      nrk_set_next_wakeup (timeout);
      my_sigs = nrk_event_wait (SIG (rx_signal) | SIG (nrk_wakeup_signal));


      if (my_sigs == 0)
        nrk_kprintf (PSTR ("Error calling nrk_event_wait()\r\n"));
      if (my_sigs & SIG (rx_signal)) {
	
        // Get the RX packet 
        local_rx_buf = bmac_rx_pkt_get (&len, &rssi);
	// Check the packet type from raw buffer before unpacking
        if ((local_rx_buf[CTRL_FLAGS] & (DS_MASK | US_MASK)) == 0) {

	// Set the buffer
	p2p_pkt.buf=local_rx_buf;
	p2p_pkt.buf_len=len;
	p2p_pkt.rssi=rssi;
	unpack_peer_2_peer_packet(&p2p_pkt);
#ifdef TXT_DEBUG
            if (p2p_pkt.dst_mac == my_mac || p2p_pkt.dst_mac == BROADCAST && p2p_pkt.pkt_type==PING_PKT) {
		// Packet arrived and is good to go
              	printf( "src: %d ",p2p_pkt.src_mac);  
              	printf( "rssi: %d ",p2p_pkt.rssi);      
              	printf( "subnet: %d %d %d ",p2p_pkt.subnet_mac[0], p2p_pkt.subnet_mac[1], p2p_pkt.subnet_mac[2]);  
              	printf( "type: %d ",p2p_pkt.pkt_type);      
              	nrk_kprintf (PSTR ("payload: ["));
              	for (i = 0; i < p2p_pkt.payload_len; i++)
                	printf ("%d ", p2p_pkt.payload[i]); 
              	nrk_kprintf (PSTR ("]\r\n"));
	
		unique=1;	
		// Check if the MAC is unique
		for(i=0; i<my_nlist_elements; i++ )
		{

		if(my_nlist[i*NLIST_SIZE]==p2p_pkt.subnet_mac[2] &&
			my_nlist[i*NLIST_SIZE+1]==p2p_pkt.subnet_mac[1] &&
			my_nlist[i*NLIST_SIZE+2]==p2p_pkt.subnet_mac[0] &&
			my_nlist[i*NLIST_SIZE+3]==p2p_pkt.src_mac)
			{
				unique=0;
				break;
			}
		}

		// If MAC is unique, add it
		if(unique)
		{
			my_nlist[my_nlist_elements*NLIST_SIZE]=p2p_pkt.subnet_mac[2];
			my_nlist[my_nlist_elements*NLIST_SIZE+1]=p2p_pkt.subnet_mac[1];
			my_nlist[my_nlist_elements*NLIST_SIZE+2]=p2p_pkt.subnet_mac[0];
			my_nlist[my_nlist_elements*NLIST_SIZE+3]=p2p_pkt.src_mac;
			my_nlist[my_nlist_elements*NLIST_SIZE+4]=p2p_pkt.rssi;
			my_nlist_elements++;
		}
            }
#endif

        }
        // Release the RX buffer so future packets can arrive 
        bmac_rx_pkt_release ();
      }

      nrk_time_get (&current);
      if (start.secs + REPLY_WAIT_SECS < current.secs)
        break;
    }
    cnt++; 
}
    check_period.secs = 0;
    check_period.nano_secs = DEFAULT_CHECK_RATE * NANOS_PER_MS;
    val = bmac_set_rx_check_rate (check_period);
    
    nrk_kprintf (PSTR ("Done Waiting for response...\r\n"));

  if(my_nlist_elements>0)
  {
    build_extended_neighbor_list_pkt(&p2p_pkt,&my_nlist,my_nlist_elements);
    pack_peer_2_peer_packet(&p2p_pkt);
    nrk_led_set (BLUE_LED);
    // For blocking transmits, use the following function call.
    val = bmac_tx_pkt (p2p_pkt.buf, p2p_pkt.buf_len);
    printf( "size of pkt: %d\r\n",p2p_pkt.buf_len );
    nrk_kprintf( PSTR("sent neighbor list packet\r\n") );
    nrk_led_clr (BLUE_LED);
  } else 
  {
	nrk_led_set(RED_LED);
	nrk_spin_wait_us(1000);
	nrk_led_clr(RED_LED);
  }
 
    for(i=0; i<10; i++ ) nrk_wait_until_next_period ();
        bmac_rx_pkt_release ();
    
  }

}

void build_extended_neighbor_list_pkt(SAMPL_PEER_2_PEER_PKT_T *p2p_pkt,uint8_t *nlist,uint8_t nlist_size)
{
uint8_t i;

    // Build a TX packet by hand...
    p2p_pkt->pkt_type = EXTENDED_NEIGHBOR_LIST_PKT;
    // set as p2p packet (no US_MASK or DS_MASK)
    p2p_pkt->ctrl_flags = MOBILE_MASK; // | DEBUG_FLAG ;  
    p2p_pkt->ack_retry= 0x00;
    p2p_pkt->ttl = 5;
    // All flooding or routed packets must be default checkrate
    // only 1-hop reply messages can be less
    p2p_pkt->check_rate = DEFAULT_CHECK_RATE;
    p2p_pkt->subnet_mac[0] = my_subnet_mac_0;
    p2p_pkt->subnet_mac[1] = my_subnet_mac_1;
    p2p_pkt->subnet_mac[2] = my_subnet_mac_2;
    p2p_pkt->src_mac = my_mac;
    p2p_pkt->last_hop_mac = my_mac;
    p2p_pkt->dst_mac = 0;
    p2p_pkt->buf=tx_buf;
    p2p_pkt->buf_len = P2P_PAYLOAD_START;
    p2p_pkt->payload= &tx_buf[P2P_PAYLOAD_START];
    p2p_pkt->seq_num = cnt;
    p2p_pkt->priority = 0;

    p2p_pkt->payload[0]=nlist_size;
    for(i=0; i<nlist_size*NLIST_SIZE; i++ )
	p2p_pkt->payload[i+1]=nlist[i];

    p2p_pkt->payload_len=nlist_size*NLIST_SIZE+1;

}

void build_ping_pkt(SAMPL_PEER_2_PEER_PKT_T *p2p_pkt)
{

    // Build a TX packet by hand...
    p2p_pkt->pkt_type = PING_PKT;
    // set as p2p packet (no US_MASK or DS_MASK)
    p2p_pkt->ctrl_flags = MOBILE_MASK; // | DEBUG_FLAG ;  
    p2p_pkt->ack_retry= 0x00;
    p2p_pkt->ttl = 1;
    p2p_pkt->check_rate = 50;
    p2p_pkt->subnet_mac[0] = my_subnet_mac_0;
    p2p_pkt->subnet_mac[1] = my_subnet_mac_1;
    p2p_pkt->subnet_mac[2] = my_subnet_mac_2;
    p2p_pkt->src_mac = my_mac;
    p2p_pkt->last_hop_mac = my_mac;
    p2p_pkt->dst_mac = BROADCAST;
    p2p_pkt->buf=tx_buf;
    p2p_pkt->buf_len = P2P_PAYLOAD_START;
    p2p_pkt->seq_num = cnt;
    p2p_pkt->priority = 0;
    p2p_pkt->payload= &tx_buf[P2P_PAYLOAD_START];
    p2p_pkt->payload[0]=my_mac;
    p2p_pkt->payload_len=1;
}

void nrk_create_taskset ()
{

  MOBILE_TASK.task = tx_task;
  nrk_task_set_stk (&MOBILE_TASK, mobile_task_stack, NRK_APP_STACKSIZE);
  MOBILE_TASK.prio = 2;
  MOBILE_TASK.FirstActivation = TRUE;
  MOBILE_TASK.Type = BASIC_TASK;
  MOBILE_TASK.SchType = PREEMPTIVE;
  MOBILE_TASK.period.secs = 0;
  MOBILE_TASK.period.nano_secs = 500*NANOS_PER_MS;
  MOBILE_TASK.cpu_reserve.secs = 1;
  MOBILE_TASK.cpu_reserve.nano_secs = 500 * NANOS_PER_MS;
  MOBILE_TASK.offset.secs = 0;
  MOBILE_TASK.offset.nano_secs = 0;
  nrk_activate_task (&MOBILE_TASK);



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
if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to load my ADC driver\r\n") );

}

