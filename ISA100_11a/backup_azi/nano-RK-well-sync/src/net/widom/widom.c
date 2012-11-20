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

/* ========================================================================== */
/*                                                                            */
/*   widom.c                                                                  */
/*                                                                            */
/* This implementation has no internal transmit or receive buffers.           */
/* Applications or higher level network layers must provide their own         */
/* transmit and receive buffers.                                              */
/* WiDOM will pass transmit buffers directly to the basic_rf transmit         */
/* functions where additional information like a CRC checksum is added.  When */
/* data is received, wiDOM will store this data in the last set receive buffer*/
/* until the wd_rx_pkt_release() function is called by a higher layer or      */
/* application.  The reception of a packet will generate a signal notifying   */
/* any waiting tasks that the packet is ready.  Received packets can be       */
/* checked in a polling fashion using the wd_rx_pkt_ready() function, or a    */
/* task can suspend until a packet arrives using the wd_wait_until_rx_packet()*/
/* function.  Timeouts on packet reception can be achieved using the wait     */
/* until next wakeup configuration commands provided by Nano-RK.  If a        */
/* timeout occurs, wd_wait_until_rx_packet() will return an error code.       */
/* To allow efficient network layer development, the receive buffer can be    */
/* changed using the wd_rx_pkt_set_buffer() function. This should only be     */
/* done after a packet is received or at startup.  If a new receive buffer    */
/* pointer has been set, it is then safe to call wd_rx_pkt_release()          */
/* indicating that the wiDOM task is allowed to buffer new packets at that    */
/* memory location.                                                           */
/*                                                                            */
/*   Author: Nuno Pereira   wd_rx_pkt_set_buffer                                                  */
/* ========================================================================== */

#include <include.h>
#include <ulib.h>
#include <nrk.h>
#include <nrk_timer.h>
#include <nrk_events.h>
#include <nrk_error.h>
#include <stdio.h>
#include <widom.h>
#include <wd_rf.h>
#include <wd_timer.h>

// MAC protocol current state
uint8_t     state = 0;
// priobits index counter
uint8_t     i = 0;

// indicate if we have a packet to transmit
uint8_t tx_packet = false;
// message to transmit priority
uint16_t prio=INVALID_PRIO;
// priority bit at index i
uint8_t prio_i = 0;
// indicates if the node won the tournament
uint8_t winner = 0;
// indicates if the node participates in the tournament
uint8_t listen = 0;
// priority of the winner of last tournament
uint16_t winner_prio=INVALID_PRIO;
// transmit buffer info
RF_TX_INFO tx_pkt_info;

// indicate if we have a received packet
volatile int8_t rx_packet = -1;
// receive buffer info
RF_RX_INFO rx_pkt_info;

// signal to indicate a packet rx
nrk_sig_t wd_rx_pkt_info_signal;
// signal to indicate a packet tx
nrk_sig_t wd_tx_pkt_info_signal;
  
// timeouts constants (rounded up to clock ticks)
const uint32_t E        = ( E_us / 1000000.0) / WD_CLOCK_TICK_TIME + 1;	
const uint32_t F        = ( F_us / 1000000.0) / WD_CLOCK_TICK_TIME + 1 ;
const uint32_t H        = ( H_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;
const uint32_t G        = ( G_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;
const uint32_t TFCS     = ( TFCS_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;
const uint32_t SWX      = ( SWX_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;
const uint32_t CMSG     = ( CMSG_us  / 1000000.0) / WD_CLOCK_TICK_TIME + 1;
const uint32_t ETG      = ( ETG_us  / 1000000.0) / WD_CLOCK_TICK_TIME + 1;

/**********************************************************
* Initializes the protocol
* this calls wdrf_init, that in turn executes rf_init(...) from basic_rf.
*/
int8_t wd_init(uint8_t channel)
{
  
  nrk_int_disable();
  
  wdrf_init(&rx_pkt_info, channel);
  
  rx_packet=0;

  wd_rx_pkt_info_signal=nrk_signal_create();
  if(wd_rx_pkt_info_signal==NRK_ERROR) {
	 nrk_kprintf(PSTR("WD ERROR: creating rx signal failed\r\n"));
	 return NRK_ERROR;
	}
  wd_tx_pkt_info_signal=nrk_signal_create();
  if(wd_tx_pkt_info_signal==NRK_ERROR)
	{
	 nrk_kprintf(PSTR("WD ERROR: creating tx signal failed\r\n"));
	 return NRK_ERROR;
	}
	
  wd_set_active(); // interrupts are enabled here
  
	return NRK_OK;
}

/**********************************************************
* State of protocol initialization
* 
*  RETURN VALUE:
*    1 if protocol is initialized
*    0 if protocol is NOT initialized
*/
int8_t wd_started(void)
{
  if (wd_tx_pkt_info_signal != NRK_ERROR) return 1;
  return 0;
}

/**********************************************************
* This can be used to put the protocol and radio to idle and
* save energy.
* The node will perform no radio activity while idle.  
*
* This can only be called after the protocol is initialized.
*/
int8_t wd_set_idle()
{
  if (wd_started() == false) return NRK_ERROR;

  nrk_int_disable();

 	wdrf_stp();	
 	wd_timer_stop();

  nrk_int_enable();

	return NRK_OK;
}

/**********************************************************
* Will set the protocol active again, after a previous
* call to  wd_set_idle()  
* 
* This can only be called after the protocol is initialized.
*/
int8_t wd_set_active()
{
  nrk_int_disable();
  
  if (wd_started() == false) return NRK_ERROR;
  
 	wd_timer_start();
  state=0;

  nrk_int_enable(); 
  
  wd_do_protocol(MSG_TYPE_MAC_STATE_ENTER);
	return NRK_OK;
}

void print_mt(uint8_t messageType) 
{
switch (messageType) {
case	MSG_TYPE_DATA_MSG:
	printf("MSG_TYPE_DATA_MSG");
	break;
case	MSG_TYPE_MAC_QUEUE:
	printf("MAC_QUEUE");
	break;
case	MSG_TYPE_MAC_WAIT_TIME_E:
	printf("MAC_WAIT_TIME_E");
	break;
case	MSG_TYPE_MAC_WAIT_TIME_F:
	printf("MAC_WAIT_TIME_F");
	break;
case	MSG_TYPE_MAC_WAIT_TIME_G:
	printf("MAC_WAIT_TIME_G");
	break;
case	MSG_TYPE_MAC_WAIT_TIME_H:
	printf("MAC_WAIT_TIME_H");
	break;
case	MSG_TYPE_MAC_WAIT_TIME_R:
	printf("MAC_WAIT_TIME_R");
	break;
case	MSG_TYPE_MAC_WAIT_TIME_SWX:
	printf("MAC_WAIT_TIME_SWX");
	break;
case	MSG_TYPE_MAC_WAIT_TIME_TFCS:
	printf("MAC_WAIT_TIME_TFCS");
	break;
case	MSG_TYPE_MAC_WAIT_TIME_ETG:
	printf("MAC_WAIT_TIME_ETG");
	break;
case	MSG_TYPE_MAC_WAIT_TIME_ENDTXRX:
	printf("MAC_WAIT_TIME_ENDTXRX");
	break;
case	MSG_TYPE_MAC_STATE_ENTER:
	printf("MAC_STATE_ENTER");
	break;
	
// Radio messages
case	MSG_TYPE_RADIO_CHANNEL_IDLE: 
	printf("RADIO_CHANNEL_IDLE");
	break;
case	MSG_TYPE_RADIO_CHANNEL_BUSY:
	printf("RADIO_CHANNEL_BUSY");
	break;
case	MSG_TYPE_RADIO_END_RX: 
	printf("RADIO_END_RX");
	break;
case	MSG_TYPE_RADIO_END_TX:
	printf("RADIO_END_TX");
	break;
	
case  MSG_TYPE_MAC_DO_ASSERT:
	printf("MAC_DO_ASSERT");
	break;
}
}

/**********************************************************
* this function performs the protocol (WiDom)
*
* ASSUMES: running uninterrupted to completion (it runs inside an ISR)    
*
*  RETURN VALUE:
*  		a boolean indicating the result of the tournament
*/	
inline void wd_do_protocol(uint8_t messageType)
{

//printf("S=%u;(%d)\r\n", state, messageType);
//printf("(%u)", state); print_mt(messageType); printf("\r\n");

	switch (state) {
		case MAC_STATE_0:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:	  // enter from (call _toState()) MAC_STATE_10 or MAC_STATE_11
				  wd_timer_cancel_alarm(); // cancel any pending timeout
					wdrf_test_mode();			  // prepare radio for carrier tx
					wdrf_set_rcv();				  // set radio to receive, to detect a carrier
          toState(1);
					break;
			}
			break;
		case MAC_STATE_1:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:	  // enter from (call toState()) MAC_STATE_0
					wd_timer_logical_time_reset();		
					break;
				case MSG_TYPE_RADIO_END_RX:		  // this is a special case, we had (probably?) previously received a channel busy indication, now channel should be clear
					wd_timer_cancel_alarm();
				  if (wdrf_polling_rx_packet() == 1) {
            rx_packet=1;           
            nrk_event_signal( wd_rx_pkt_info_signal );	// signal packet reception
          }					
				case MSG_TYPE_RADIO_CHANNEL_IDLE:  // channel idle detected
					wd_timer_logical_time_reset();												  
					wd_timer_set_alarm(TFCS, MSG_TYPE_MAC_WAIT_TIME_TFCS); // next state transition when x=TFCS
					break;
				case MSG_TYPE_RADIO_CHANNEL_BUSY:  // channel busy detected
					wd_timer_cancel_alarm();
					wd_timer_logical_time_reset();
					break;
				case MSG_TYPE_MAC_WAIT_TIME_TFCS:  // ended waiting for TFCS time units
					wd_timer_logical_time_reset();
					toState(2);
					break;
			}
			break;
		case MAC_STATE_2:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:	  // enter from (call toState()) MAC_STATE_1							  
					wd_timer_set_alarm(F, MSG_TYPE_MAC_WAIT_TIME_F);// continue to sense the medium for a "long time interval"
					break;
				case MSG_TYPE_RADIO_END_RX:
					wd_timer_cancel_alarm();
				  if (wdrf_polling_rx_packet() == 1) {
            rx_packet=1;           
            nrk_event_signal( wd_rx_pkt_info_signal );	// signal packet reception
          }
					wd_timer_logical_time_reset();
					wd_timer_set_alarm(TFCS, MSG_TYPE_MAC_WAIT_TIME_TFCS);// next state transition when x=TFCS
					toState(1);
					break;
        case MSG_TYPE_RADIO_CHANNEL_BUSY: // channel busy detected (while waiting for a a "long time interval")
					wd_timer_logical_time_reset();
          wd_timer_cancel_alarm();
          toState(1); // go back to state 1 (2->1)
          break;
				case MSG_TYPE_MAC_WAIT_TIME_F:	  // ended waiting for a "long time interval" (F)
					wd_timer_logical_time_reset();
					toState(3);
					break;
			}
			break;
		case MAC_STATE_3:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:	  // enter from (call toState()) MAC_STATE_2
					wd_timer_set_alarm(E, MSG_TYPE_MAC_WAIT_TIME_E);
					break;
				case MSG_TYPE_RADIO_CHANNEL_BUSY:  // carrier detected when waiting for E time units, or waiting for someone to start a tournament
					wd_timer_cancel_alarm();
					wd_timer_logical_time_reset();
					toState(5);
					break;
				case MSG_TYPE_RADIO_END_RX:
					wd_timer_cancel_alarm();
				  if (wdrf_polling_rx_packet() == 1) {
            rx_packet=1;           
            nrk_event_signal( wd_rx_pkt_info_signal );	// signal packet reception
          }
					wd_timer_logical_time_reset();
					wd_timer_set_alarm(TFCS, MSG_TYPE_MAC_WAIT_TIME_TFCS); // next state transition when x=TFCS
					toState(1);
					break;
				case MSG_TYPE_MAC_QUEUE:		  // application queued a message
				case MSG_TYPE_MAC_WAIT_TIME_E:	  // signaled at the end of E time units (3->4)
				  wd_timer_logical_time_reset();
					if (tx_packet==true) {
						wdrf_stp();				  // stop carrier sensing
						wdrf_carrier_on();		  // CarrierOn
						toState(4);
					}
					break;
			}
			break;
		case MAC_STATE_4:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:	  // enter from (call toState()) MAC_STATE_3							  
					wd_timer_set_alarm(SWX, MSG_TYPE_MAC_WAIT_TIME_SWX); // next state transition when x=SWX (+SWX time units)
					break;
				case MSG_TYPE_MAC_WAIT_TIME_SWX:  // signaled at the end of E time units (4->5)
					wd_timer_logical_time_reset();
					toState(5);					  // (4->5)
					break;
			}
			break;
		case MAC_STATE_5:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:	  // enter from (call toState()) MAC_STATE_4	
					wd_timer_set_alarm(H, MSG_TYPE_MAC_WAIT_TIME_H); // next state transition when x=H
					break;
				case MSG_TYPE_MAC_WAIT_TIME_H:	  // signaled at the end of H time units
					wdrf_stp();
					i=0;
					if (tx_packet==true) {		  // we have something to send, so we init for the tournament
						winner=true;
						listen=false;
					} // we have nothing to send
					else {
						prio = INVALID_PRIO;
						winner=false;
						listen=true;
					}
					toState(6);
					break;
			}
			break;
		case MAC_STATE_6:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:	  // enter from (call toState()) MAC_STATE_5 or MAC_STATE_8
					wd_timer_set_alarm(H+G+(G+H)*i, MSG_TYPE_MAC_WAIT_TIME_G);
					break;
				case MSG_TYPE_MAC_WAIT_TIME_G:
					prio_i = wd_prio_bit(prio, i);
					wd_winner_prio(winner_prio, i, prio_i);
					if (prio_i == DOMINANT_BIT && winner == true) {
						wdrf_carrier_on();
					}
					else {
						wdrf_set_rcv();
					}
					toState(7);
					break;
			}
			break;
		case MAC_STATE_7:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:												  
					wd_timer_set_alarm(2*H+G+(G+H)*i, MSG_TYPE_MAC_WAIT_TIME_H); // next state transition when x=2*H+G+(G+H)*i (+H time units)
					break;
				case MSG_TYPE_RADIO_CHANNEL_BUSY:  // heard a carrier during the tournament; lost the tournament
					//assert((prio_i==1 || winner == false), messageType);
					wd_winner_prio(winner_prio, i, DOMINANT_BIT);
					winner=false;
					break;
				case MSG_TYPE_MAC_WAIT_TIME_H:	  // ended waiting H time units; check results of this tournament round
					toState(8);
					break;
			}
			break;
		case MAC_STATE_8:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:
					wdrf_stp();					  // stop radio; either from carrier sensing or carrier tx
					if (i<NPRIOBITS-1) {		  // go to the next tournament round
						i++;
						toState(6);
					} else if (i==NPRIOBITS-1) { // this was the last tournament round
//						nrk_led_toggle(BLUE_LED);
					  wdrf_data_mode();	  // set radio to "normal" mode
						if (winner == true) {	  // we won the tournament
							toState(9);
						} else {						  // if (winner == false)						
							wdrf_set_rcv();
							wd_timer_set_alarm(2*H+G+(G+H)*(NPRIOBITS-1)+ ETG + CMSG, MSG_TYPE_MAC_WAIT_TIME_ENDTXRX); // next state transition
							toState(11);		  // (8->11)
						}
					}
					break;
			}
			break;
		case MAC_STATE_9:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:
          //printf("WON!\r\n");
					nrk_led_clr(RED_LED);
					nrk_led_set(GREEN_LED);
					wd_timer_set_alarm(2*H+G+(G+H)*(NPRIOBITS-1) + ETG, MSG_TYPE_MAC_WAIT_TIME_ETG); // next state transition
					break;
				case MSG_TYPE_MAC_WAIT_TIME_ETG:  // ended waiting after winning the tournament
					wd_timer_set_alarm(2*H+G+(G+H)*(NPRIOBITS-1) + ETG + CMSG, MSG_TYPE_MAC_WAIT_TIME_ENDTXRX); // next state transition
      		wdrf_tx_packet (&tx_pkt_info); // tx packet
          toState(10);
					break;
			}
			break;
		case MAC_STATE_10:
			switch (messageType) {		
				case MSG_TYPE_MAC_STATE_ENTER:
          break;
        case MSG_TYPE_RADIO_END_TX:
          wd_timer_cancel_alarm(); // cancel MAC_WAIT_TIME_ENDTXRX timeout        
          nrk_event_signal ( wd_tx_pkt_info_signal );	// signal packet transmission
				case MSG_TYPE_MAC_WAIT_TIME_ENDTXRX:// ended waiting message tx
#ifndef CONTINUOUS_SEND      		
        		tx_packet=false;
#endif
          wdrf_stp();				  // stop 					
					toState(0);				  // go back to state 0
					break;
      }
		  break;
    case MAC_STATE_11:
			switch (messageType) {
				case MSG_TYPE_MAC_STATE_ENTER:
          if (listen == false) {
            //printf("LOST!\r\n");
            nrk_led_set(RED_LED);
            nrk_led_clr(GREEN_LED);
					}					
					break;
				case MSG_TYPE_MAC_WAIT_TIME_ENDTXRX:// ended waiting message rx
          wdrf_stp();				  // stop radio		
					toState(0);
					break;
				case MSG_TYPE_RADIO_END_RX:		  // ended receiving message
          wd_timer_cancel_alarm(); // cancel MAC_WAIT_TIME_ENDTXRX timeout
          wdrf_stp();				  // stop radio		
				  if (wdrf_polling_rx_packet() == 1) {
            rx_packet=1;           
            nrk_event_signal( wd_rx_pkt_info_signal );	// signal packet reception
          }
					toState(0);					  // go back to state 0
					break;					
			}
			break;
	}
}

/**********************************************************
* this function returns the tx done signal reference
*/
nrk_sig_t wd_get_tx_signal()
{
  return wd_tx_pkt_info_signal;
}

/**********************************************************
* this function passes a packet to be sent
* and waits until the end transmission
* 
*  RETURN VALUE:
*  	NRK_ERROR if packet was not sent successfully
*  	NRK_OK if it was transmitted
*/	
int8_t wd_tx_packet(uint8_t *buf, uint8_t len, uint16_t priority)
{
	if (wd_tx_packet_enqueue(buf, len, priority)==NRK_OK) return wd_wait_until_tx_packet();
	return NRK_ERROR;
}

/**********************************************************
* this function passes a packet to be sent 
*
*  RETURN VALUE:
*  	NRK_ERROR if packet was not accepted
*  	NRK_OK if it was accepted
*/	
int8_t wd_tx_packet_enqueue(uint8_t *buf, uint8_t len, uint16_t priority) 
{
  int8_t ret=NRK_ERROR;

	nrk_int_disable();

	if (tx_packet==false) {
    tx_pkt_info.pPayload=buf;
    tx_pkt_info.length=len;	
		tx_packet = true;
		prio = priority;
		ret=NRK_OK;
		if (state == MAC_STATE_3) wd_do_protocol(MSG_TYPE_MAC_QUEUE);
	}

	nrk_int_enable();

	return ret;
}

/**********************************************************
* this function sets the receive buffer 
*
*  RETURN VALUE:
*  	NRK_ERROR 
*  	NRK_OK 
*/
int8_t wd_rx_pkt_set_buffer(uint8_t *buf, uint8_t size)
{
  if(buf==NULL) return NRK_ERROR;

  rx_pkt_info.pPayload = buf;
  rx_pkt_info.max_length = size;
  rx_packet=0;
  return NRK_OK;
}

/**********************************************************
* this function returns the receive buffer 
*
*  RETURN VALUE:
*  	NRK_ERROR 
*  	NRK_OK 
*/
uint8_t *wd_rx_pkt_get(uint8_t *len, uint8_t *rssi)
{
  if(wd_rx_pkt_ready()!=NRK_OK) 
	{	
	 *len=0;
	 *rssi=0;
	 return NULL;
	}
  
  *len=rx_pkt_info.length;
  *rssi=rx_pkt_info.rssi;
  return rx_pkt_info.pPayload;
}

/**********************************************************
* this function checks if a packet was received completely
*
*  RETURN VALUE:
*  	NRK_ERROR no packet 
*  	NRK_OK packet completely
*/	
int8_t wd_rx_pkt_ready()
{
  uint8_t res=0;
	nrk_int_disable();
	res=rx_packet;
	nrk_int_enable();
	if (res==1) {return NRK_OK;}
	return NRK_ERROR;
}

/**********************************************************
* this function releases the reception buffer 
*
*  RETURN VALUE:
*  	NRK_OK 	
*/	
int8_t wd_rx_pkt_release()
{
	nrk_int_disable();
	rx_packet=0;
	nrk_int_enable();
	return NRK_OK;
}

/**********************************************************
* wait until end of next packet transmission
* 
* RETURN VALUE:
*  	NRK_ERROR timeout
*  	NRK_OK 
*/	
int8_t wd_wait_until_tx_packet()
{
	nrk_sig_mask_t event;

  nrk_signal_register(wd_tx_pkt_info_signal);   
  event=nrk_event_wait (SIG(wd_tx_pkt_info_signal) | SIG(nrk_wakeup_signal));

  // Check if packet TX signal was OK
  if((event & SIG(wd_tx_pkt_info_signal)) == 0 ) return NRK_ERROR;

	return NRK_OK;
}

/**********************************************************
* wait until end of next packet reception
* 
* RETURN VALUE:
*  	NRK_ERROR timeout
*  	NRK_OK 
*/	
int8_t wd_wait_until_rx_packet()
{
  nrk_sig_mask_t event;

  nrk_signal_register(wd_rx_pkt_info_signal); 
  event=nrk_event_wait (SIG(wd_rx_pkt_info_signal) | SIG(nrk_wakeup_signal));

  // Check if packet RX signal was OK
  if( (event & SIG(wd_rx_pkt_info_signal)) == 0 ) return NRK_ERROR;

	return NRK_OK;
}

/**********************************************************
* return the priority of the winner of the last tournament 
* 
* RETURN VALUE:
*   uint16_t the priority 
*/	
uint16_t wd_get_winner()
{
  uint16_t wprio;
	nrk_int_disable();
  wprio = winner_prio;
	nrk_int_enable();
  return wprio;   
}

/**********************************************************
* change channel  
*/	
int8_t wd_set_channel(uint8_t channel )
{
  wdrf_set_channel( channel );

  // must restart protocol to state 0
  state=0;
  wd_do_protocol(MSG_TYPE_MAC_STATE_ENTER);
  
  return NRK_OK;
}

/**********************************************************
* set rf power
* power = 31 => full power    (0dbm)
*          3 => lowest power  (-25dbm)
*/
int8_t wd_set_rf_power(uint8_t power)
{
  wdrf_set_power ( power );
  return NRK_OK;
}

/**********************************************************
* set cca threshold  
*/	
int8_t wd_set_cca_thresh(int8_t cca_thresh)
{
  wdrf_set_cca_thr ( cca_thresh );
  return NRK_OK;
}

inline void alarm_callback(uint8_t alarm_type)
{
	wd_do_protocol(alarm_type);
}
