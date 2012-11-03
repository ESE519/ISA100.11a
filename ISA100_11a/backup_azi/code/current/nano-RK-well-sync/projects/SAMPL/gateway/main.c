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
#include <slip.h>
#include "../include/pkt_packer.h"
#include "../include/sampl.h"

#define MAX_SLIP_BUF 128 
#define gw_mac		0

//#define TXT_DEBUG
//#define AUTO_PING
//#define NO_SLIP

uint8_t aes_key[16]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e, 0x0f};

uint8_t mac_check_rate;
uint32_t mac_address;
uint8_t my_mac;
uint8_t my_subnet_mac[3];

nrk_task_type RX_TASK;
NRK_STK rx_task_stack[NRK_APP_STACKSIZE];
void rx_task (void);


nrk_task_type TX_TASK;
NRK_STK tx_task_stack[NRK_APP_STACKSIZE];
void tx_task (void);

void nrk_create_taskset ();

uint8_t slip_rx_buf[MAX_SLIP_BUF];

uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t gw_buf[RF_MAX_PAYLOAD_SIZE];


SAMPL_GATEWAY_PKT_T gw_pkt;
SAMPL_UPSTREAM_PKT_T us_pkt;
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

  val=read_eeprom_mac_address(&mac_address);
  if(val==NRK_OK)
	  {
		my_mac=(mac_address & 0xff);
		my_subnet_mac[0] = ((mac_address>>8)&0xff);
		my_subnet_mac[1] = ((mac_address>>16)&0xff);
		my_subnet_mac[2] = ((mac_address>>24)&0xff);
	  } else
	  {
		  while(1) {
		 	nrk_kprintf( PSTR( "* ERROR reading MAC address from EEPROM run eeprom-config utility\r\n" ));
		        nrk_wait_until_next_period();
			}
	  }

  val=read_eeprom_channel(&i);
  val=read_eeprom_aes_key(&aes_key);
  printf ("Netmask = 0x%x %x %x\r\n", my_subnet_mac[2], my_subnet_mac[1], my_subnet_mac[0]);
  printf ("MAC = 0x%x\r\n", my_mac);
  printf ("Channel = %d\r\n", i);

  // init bmac on channel 26 
  bmac_init (i);

  bmac_encryption_set_key(aes_key,16);
  bmac_encryption_enable();

  if(my_mac!=gw_mac)
  {

	while(1) {
	nrk_kprintf( PSTR( "* ERROR set gateway mac to 0\r\n" ));
	nrk_wait_until_next_period();
	}

  }

  val=bmac_addr_decode_set_my_mac(((uint16_t)my_subnet_mac<<8)|my_mac);
  val=bmac_addr_decode_dest_mac(0xffff);  // broadcast by default
  val=bmac_auto_ack_enable();

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
    nrk_led_set (RED_LED);
    local_rx_buf = bmac_rx_pkt_get (&len, &rssi);
//    printf ("Got RX packet len=%d RSSI=%d [", len, rssi);
//    for (i = 0; i < len; i++) printf ("%d ", rx_buf[i]); printf ("]\r\n");

    if((local_rx_buf[CTRL_FLAGS] & DS_MASK)!=0)
	{
	#ifdef TXT_DEBUG
		nrk_kprintf( PSTR("Flood pkt coming back...\r\n"));
	#endif
	}
    else 
	{
	// Pack an P2P packet into the gateway packet
	if((local_rx_buf[CTRL_FLAGS] & (DS_MASK | US_MASK ))==0 )
	{
	#ifdef TXT_DEBUG
	nrk_kprintf( PSTR( "P2P->Gateway\r\n" ));
        #endif
	p2p_pkt.buf=local_rx_buf;
	p2p_pkt.buf_len=len;
	unpack_peer_2_peer_packet(&p2p_pkt);
	// pack gateway packet
	gw_pkt.buf=gw_buf;
	gw_pkt.payload=&(gw_buf[GW_PAYLOAD_START]);
	gw_pkt.protocol_id=p2p_pkt.protocol_id;
	gw_pkt.protocol_version=p2p_pkt.protocol_version;
	gw_pkt.pkt_type=p2p_pkt.pkt_type;
	gw_pkt.ctrl_flags=p2p_pkt.ctrl_flags;
	gw_pkt.subnet_mac[0]=p2p_pkt.subnet_mac[0];
	gw_pkt.subnet_mac[1]=p2p_pkt.subnet_mac[1];
	gw_pkt.subnet_mac[2]=p2p_pkt.subnet_mac[2];
	gw_pkt.priority=p2p_pkt.priority;
	gw_pkt.ack_retry=p2p_pkt.ack_retry;
	gw_pkt.seq_num=p2p_pkt.seq_num;
	gw_pkt.error_code=0;
	gw_pkt.rssi=rssi;
	gw_pkt.last_hop_mac=p2p_pkt.last_hop_mac;
	gw_pkt.src_mac=p2p_pkt.src_mac;
	gw_pkt.dst_mac=p2p_pkt.dst_mac;
	gw_pkt.num_msgs=1;
	for(i=0; i<p2p_pkt.payload_len; i++ )
		gw_pkt.payload[i]=p2p_pkt.payload[i];
	gw_pkt.payload_len=p2p_pkt.payload_len;
	}


	// Pack an Upstream packet into the gateway packet
	if((local_rx_buf[CTRL_FLAGS] & DS_MASK)==0 && (local_rx_buf[CTRL_FLAGS] & US_MASK)!=0)
	{
	#ifdef TXT_DEBUG
	nrk_kprintf( PSTR( "Upstream->Gateway\r\n" ));
        #endif
	us_pkt.buf=local_rx_buf;
	us_pkt.buf_len=len;
	unpack_upstream_packet(&us_pkt);
	gw_pkt.buf=gw_buf;
	gw_pkt.payload=&(gw_buf[GW_PAYLOAD_START]);
	gw_pkt.protocol_id=us_pkt.protocol_id;
	gw_pkt.protocol_version=us_pkt.protocol_version;
	gw_pkt.pkt_type=us_pkt.pkt_type;
	gw_pkt.ctrl_flags=us_pkt.ctrl_flags;
	gw_pkt.subnet_mac[0]=us_pkt.subnet_mac[0];
	gw_pkt.subnet_mac[1]=us_pkt.subnet_mac[1];
	gw_pkt.subnet_mac[2]=us_pkt.subnet_mac[2];
	gw_pkt.priority=us_pkt.priority;
	gw_pkt.ack_retry=us_pkt.ack_retry;
	gw_pkt.seq_num=us_pkt.seq_num;
	gw_pkt.error_code=us_pkt.error_code;
	gw_pkt.rssi=rssi;
	gw_pkt.last_hop_mac=us_pkt.last_hop_src_mac;
	gw_pkt.src_mac=gw_mac;
	gw_pkt.dst_mac=gw_mac;
	gw_pkt.num_msgs=us_pkt.num_msgs;
	#ifdef TXT_DEBUG
	printf( "us_payload=%d\r\n",us_pkt.payload_len );
	#endif
	for(i=0; i<us_pkt.payload_len; i++ )
		gw_pkt.payload[i]=us_pkt.payload[i];
	gw_pkt.payload_len=us_pkt.payload_len;
	}


	pack_gateway_packet(&gw_pkt);
#ifndef NO_SLIP
	slip_tx (gw_pkt.buf, gw_pkt.buf_len);
#endif

	}
    nrk_led_clr (RED_LED);
    // Release the RX buffer so future packets can arrive 
    bmac_rx_pkt_release ();
  }

}


void tx_task ()
{
  uint8_t j, i, val, cnt,error,CTR_buf[4];
  int8_t len;
  nrk_sig_t tx_done_signal;
  nrk_sig_mask_t ret;
  nrk_time_t check_period;
  SAMPL_DOWNSTREAM_PKT_T ds_pkt;

  printf ("tx_task PID=%d\r\n", nrk_get_pid ());


  slip_init (stdin, stdout, 0, 0);
  nrk_kprintf( PSTR("slip_init()\r\n" )); 
  // Wait until the tx_task starts up bmac
  // This should be called by all tasks using bmac that
  // do not call bmac_init()...
  while (!bmac_started ())
    nrk_wait_until_next_period ();

  nrk_kprintf( PSTR("bmac_started()\r\n" )); 
  check_period.secs=0;
  check_period.nano_secs=100*NANOS_PER_MS;
  val=bmac_set_rx_check_rate(check_period);
  
  // Get and register the tx_done_signal if you want to
  // do non-blocking transmits
  tx_done_signal = bmac_get_tx_done_signal ();
  nrk_signal_register (tx_done_signal);
  
  cnt = 0;
  nrk_kprintf( PSTR("Waiting for SLIP data...\r\n") ); 
  while (1) {

#ifdef AUTO_PING
     nrk_kprintf( PSTR("Running in automatic PING mode...\r\n") ); 
     nrk_wait_until_next_period();
     len=0;
#else
     len = slip_rx (tx_buf, RF_MAX_PAYLOAD_SIZE);
     nrk_led_set(GREEN_LED);
     nrk_kprintf( PSTR("Got Request: ") ); 
     printf( "%d",tx_buf[SEQ_NUM] );
     nrk_kprintf( PSTR("\r\n") ); 
#endif

#ifdef TXT_DEBUG
    if (len > 0) {
      nrk_kprintf (PSTR ("SLIP input data: "));
      for (i = 0; i < len; i++)
        printf ("%d ", tx_buf[i]);
      printf ("\r\n");
    }
#endif

#ifndef AUTO_PING
    // Do some sanity checks on the packet about to go out...
    error=0;
    if(tx_buf[DS_HOP_CNT]!=0) 
	error=HOP_ERROR_MASK; 
    if(tx_buf[DS_NAV]>MAX_NAV) 
	error|=NAV_ERROR_MASK; 
    if(tx_buf[DS_DELAY_PER_LEVEL]>MAX_DELAY_PER_LEVEL) 
	error|=DELAY_PER_LEVEL_ERROR_MASK; 
    if(tx_buf[DS_HOP_MAX]>MAX_HOPS) 
	error|=MAX_HOPS_ERROR_MASK; 
#endif

#ifdef AUTO_PING
    tx_buf[PROTOCOL_ID]=SAMPL_VERSION;
 
    // Build a TX packet by hand...
  //  tx_buf[PKT_TYPE]=TRACEROUTE_PKT;
   // tx_buf[PKT_TYPE]=DATA_STORAGE_PKT;
   // tx_buf[PKT_TYPE]=SENSOR_LONG_PKT;
    tx_buf[PKT_TYPE]=PING_PKT;
  //  tx_buf[PKT_TYPE]=SENSOR_LONG_PKT;
   // tx_buf[CTRL_FLAGS]=   TREE_FILTER | LINK_ACK |  ENCRYPT |  DS_MASK  |  LED_FLOOD;
     tx_buf[CTRL_FLAGS]=   LINK_ACK |  ENCRYPT |  DS_MASK  |  LED_FLOOD;
   // tx_buf[CTRL_FLAGS]=     ENCRYPT |  DS_MASK  |  LED_FLOOD;
    //tx_buf[CTRL_FLAGS]=    TREE_FILTER |  ENCRYPT |  DS_MASK  |  LED_FLOOD;
    //tx_buf[CTRL_FLAGS]=   LINK_ACK | MAC_FILTER | TREE_FILTER | ENCRYPT |  DS_MASK  |  LED_FLOOD;
//    tx_buf[CTRL_FLAGS]=  ENCRYPT | LINK_ACK | DS_MASK  |  LED_FLOOD;
    tx_buf[SEQ_NUM]=cnt;
    tx_buf[PRIORITY]=0;
    tx_buf[ACK_RETRY]|=0x30;
    tx_buf[DS_HOP_CNT]=0;
    tx_buf[DS_HOP_MAX]=6;
    tx_buf[DS_DELAY_PER_LEVEL]=1;
    tx_buf[DS_NAV]=15;
    tx_buf[DS_MAC_CHECK_RATE]=25;
    mac_check_rate = tx_buf[DS_MAC_CHECK_RATE];
//    tx_buf[DS_RSSI_THRESHOLD]=-50;
    tx_buf[DS_RSSI_THRESHOLD]=-38;
    tx_buf[DS_LAST_HOP_MAC]=gw_mac;
    len=DS_PAYLOAD_START;
    cnt++;
    error=0;
#endif

   if(error==0)
	{
    		nrk_led_set (BLUE_LED);

    		// Send initial flood at 100ms checkrate
    		check_period.secs=0;
    		check_period.nano_secs=100*NANOS_PER_MS;
    		val=bmac_set_rx_check_rate(check_period);

		if((tx_buf[CTRL_FLAGS] & ENCRYPT)!= 0 )
		{
			CTR_buf[0]=tx_buf[DS_AES_CTR_0];
			CTR_buf[1]=tx_buf[DS_AES_CTR_1];
			CTR_buf[2]=tx_buf[DS_AES_CTR_2];
			CTR_buf[3]=tx_buf[DS_AES_CTR_3];
		bmac_encryption_set_ctr_counter(&CTR_buf,4);
		}
	
		// If the gateway sends 0 as the sequence number, then automatically increase it locally
		if(tx_buf[SEQ_NUM]==0) 
			{
				tx_buf[SEQ_NUM]=cnt;
				cnt++;
			}

    		// For blocking transmits, use the following function call.
    		// For this there is no need to register  
    		val=bmac_tx_pkt(tx_buf, len);

    		// Look for reply at different checkrate
    		check_period.secs=0;
    		check_period.nano_secs=mac_check_rate*NANOS_PER_MS;
    		val=bmac_set_rx_check_rate(check_period);
    
    		#ifdef TXT_DEBUG
    		nrk_kprintf (PSTR ("Sent packet!\r\n"));
    		#endif
    		nrk_led_clr (BLUE_LED);
	} 
	#ifdef TXT_DEBUG
	else printf( "Incomming Pkt error code = 0x%x\r\n",error );
	#endif
	
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
  TX_TASK.prio = 2;
  TX_TASK.FirstActivation = TRUE;
  TX_TASK.Type = BASIC_TASK;
  TX_TASK.SchType = PREEMPTIVE;
  TX_TASK.period.secs = 40;
  TX_TASK.period.nano_secs = 0;
  TX_TASK.cpu_reserve.secs = 1;
  TX_TASK.cpu_reserve.nano_secs = 500 * NANOS_PER_MS;
  TX_TASK.offset.secs = 0;
  TX_TASK.offset.nano_secs = 0;
  nrk_activate_task (&TX_TASK);



}
