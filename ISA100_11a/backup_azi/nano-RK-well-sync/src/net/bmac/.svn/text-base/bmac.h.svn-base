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


#ifndef _BMAC_H
#define _BMAC_H
#include <include.h>
#include <basic_rf.h>
#include <nrk.h>

/************************************************************************

BMAC is a low-power listen CSMA (lpl-csma) protocol.  This implementation 
has no internal transmit or receive buffers.  Applications or higher 
level network layers must provide their own transmit and receive buffers.  
BMAC will pass transmit buffers directly to the basic_rf transmit 
functions where additional information like a CRC checksum is added.  When 
data is received, BMAC will store this data in the last set receive buffer 
until the bmac_rx_release() function is called by a higher layer or 
application.  The reception of a packet will generate a signal notifying 
any waiting tasks that the packet is ready.  Received packets can be 
checked in a polling fashion using the bmac_rx_status() function, or a 
task can suspend until a packet arrives using the bmac_rx_packet_get() 
function.  Timeouts on packet reception can be achieved using the wait 
until next wakeup configuration commands provided by Nano-RK.  If a 
timeout occurs, bmac_wait_until_rx_pkt() will return an error code.  
To allow efficient network layer development, the receive buffer can be 
changed using the bmac_rx_pkt_set_buffer() function. This should only be 
done after a packet is received or at startup.  If a new receive buffer 
pointer has been set, it is then safe to call bmac_rx_pkt_release() 
indicating that the BMAC task is allowed to buffer new packets at that 
memory location. 

************************************************************************/



#define BMAC_STACK_SIZE 	   	128
#define BMAC_MIN_CHECK_RATE_MS  	20 
#define BMAC_DEFAULT_CHECK_RATE_MS 	100 
#define BMAC_TASK_PRIORITY		20 


// Use hardware AES encryption
// Provide a key and length which must be 16 bytes.
// When encryption is enabled, you will still receive
// unencrypted packets.
int8_t bmac_encryption_set_key(uint8_t *key, uint8_t len);
int8_t bmac_encryption_enable();
int8_t bmac_encryption_set_ctr_counter(uint8_t *counter, uint8_t len);
int8_t bmac_encryption_disable();
int8_t bmac_rx_pkt_is_encrypted();

int8_t  bmac_auto_ack_disable();
int8_t  bmac_auto_ack_enable();
int8_t  bmac_addr_decode_disable();
int8_t  bmac_addr_decode_enable();
int8_t bmac_addr_decode_set_my_mac(uint16_t my_mac);
int8_t  bmac_addr_decode_dest_mac(uint16_t dest); 

int8_t tx_reserve;
int8_t bmac_tx_reserve_set( nrk_time_t *period, uint16_t pkts );
uint16_t bmac_tx_reserve_get();

nrk_task_type bmac_task;
NRK_STK bmac_task_stack[BMAC_STACK_SIZE];
uint8_t cca_active;

nrk_sig_t bmac_rx_pkt_signal;
nrk_sig_t bmac_tx_pkt_done_signal;
nrk_sig_t bmac_enable_signal;

RF_RX_INFO bmac_rfRxInfo;
RF_TX_INFO bmac_rfTxInfo; 

void bmac_enable();
void bmac_disable();

int8_t bmac_set_rx_check_rate(nrk_time_t period);
void bmac_task_config ();
int8_t bmac_set_channel(uint8_t chan);
int8_t bmac_set_rf_power(uint8_t power);
int8_t bmac_tx_pkt(uint8_t *buf, uint8_t len);
uint8_t _b_pow(uint8_t in);
nrk_sig_t bmac_get_tx_done_signal();
nrk_sig_t bmac_get_rx_pkt_signal();
int8_t bmac_tx_pkt_nonblocking(uint8_t *buf, uint8_t len);

void bmac_set_cca_active(uint8_t active);
int8_t bmac_set_cca_thresh(int8_t thresh);
uint8_t *bmac_rx_pkt_get(uint8_t *len, int8_t *rssi);
int8_t bmac_rx_pkt_ready(void);
int8_t bmac_rx_pkt_release(void);
int8_t bmac_wait_until_rx_pkt();

int8_t bmac_started();
int8_t bmac_init(uint8_t chan);

int8_t _bmac_channel_check();
int8_t _bmac_rx();
int8_t _bmac_tx();
int8_t bmac_rx_pkt_set_buffer(uint8_t *buf, uint8_t size);

#endif
