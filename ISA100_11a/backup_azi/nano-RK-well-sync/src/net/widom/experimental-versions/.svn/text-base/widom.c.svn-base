/*****************************************************************************
* Copyright (c) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Carnegie Mellon University nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Contributing Authors:
* Nuno Pereira - Polytechnic Institute of Porto; nap@isep.ipp.pt
*******************************************************************************/

/* ========================================================================== */
/*                                                                            */
/*   widom.c                                                                  */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#include <include.h>
#include <ulib.h>
#include <nrk.h>
#include <nrk_timer.h>
#include <nrk_events.h>
#include <nrk_error.h>
#include <stdio.h>
#include <widom.h>
#include <widom_rf.h>
#include <widom_timer.h>
#include <widom_gpio.h>

#if WD_VERSION == WD_MBD_EXT_SYNC
#include <widom_gpio.h>
#endif

// this defines the interval until the next sync (rounded up to clock ticks)
#if WD_VERSION != WD_MBD_RFLINX && WD_VERSION != WD_MBD_CC2420
const uint16_t  WD_INTER_NEXT_SYNC = ( ((((WD_SYNC_PERIOD_us/1000) + 1)*1000)-WD_MAX_BLOCKING_TIME_us-WD_SYNC_PULSE_RX_SWX_us-WD_SYNC_WITH_MASTER_MAX_ERROR) / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1; 
#endif

//const uint16_t  WD_INTER_NEXT_SYNC = ( (6000) / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1; 

// timeout constants (rounded up to clock ticks)
const uint16_t H 		= ( H_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;
const uint16_t G 		= ( G_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;
#ifndef WD_MBD
const uint16_t ETG 		= ( ETG_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;
#endif 

#ifdef WD_MBD
const uint16_t E 		= ( E_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1; 
const uint16_t F 		= ( F_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1; 
const uint16_t TRX	= ( TRX_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;
const uint16_t TTX	= ( TTX_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;  
const uint16_t TCS	= ( TCS_us / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1; 
const uint16_t H2 		= ( (2*H_us) / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1; // 2*H
const uint16_t H3 		= ( (3*H_us) / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1; // 3*H
const uint16_t G2 		= ( (2*G_us) / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1; // 2*G
const uint16_t CMSG   = ( (CMSG_us) / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;
#endif

const uint16_t C_SYNC		= ( (CSYNCMSG_us) / 1000000.0 ) / WD_CLOCK_TICK_TIME + 1;

// indicate if we have a packet to transmit
volatile uint8_t tx_packet = false; 
// message to transmit priority
volatile uint16_t prio=INVALID_PRIO;
// priority of the winner of last tournament
volatile uint16_t winner_prio=INVALID_PRIO;
// indicate result of last packet transmission
volatile int32_t tx_last_result = -1; 
// indicate result of last packet reception 
volatile int8_t rx_last_result = -1; 
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

#if WD_VERSION == WD_MBD_CC2420 || WD_VERSION == WD_MBD_RFLINX
uint8_t state=1;
uint8_t trnmtCounter=0;
#endif

/**********************************************************
* wait for the synchronization pin to change state
* blocks until pin changes
* ASSUMES NO PREEMPTION (DISABLE_GLOBAL_INT() was done before)
* 
*/	

inline int8_t wd_wait_sync(void)
{
#ifdef WD_SBD
	int8_t n;
	uint16_t cnt;

	//	nrk_gpio_set(DEBUG_1);

	wdrf_data_mode( ); // set the radio back in "normal" data mode and receive

	wdrf_set_rcv();

	cnt=0;
	do {
		if (cnt > (WD_SYNC_MAX_WAIT_TIME_us/BYTE_TIME_us)) {
			wdrf_flush_rx_fifo();
			//nrk_gpio_clr(DEBUG_1);
			return NRK_ERROR; // timeout as failsafe
		}
		halWait(BYTE_TIME_us);
		cnt++;
	} while (wdrf_sfd() == false);
	wd_reset_time();
	cnt=0;
	while ((n = wdrf_poll_synch_packet_rx ()) == 0) { // get rest of the packet and check it
		if (cnt > WD_MAX_SYNC_PKT_SIZE) {
			wdrf_flush_rx_fifo();
			return NRK_ERROR; // timeout as failsafe
		}
		halWait(BYTE_TIME_us);
		cnt++;
	}

	wdrf_flush_rx_fifo();

	//	nrk_gpio_clr(DEBUG_1);

	if (n<0) return NRK_ERROR;
	return NRK_OK;
#endif
#if WD_VERSION == WD_MBD_EXT_SYNC
	int8_t state, new_state;

	state = wd_get_sync_pin();

	do {
		new_state = wd_get_sync_pin();
	} while (new_state == state);

	wd_reset_time();
	/*
	if (SYNC_PIN_VAL) nrk_gpio_set(DEBUG_0);
	else nrk_gpio_clr(DEBUG_0);
	*/
	return NRK_OK;
#endif
}

/**********************************************************
* this function waits for packet reception
*
*  RETURN VALUE:
*  	>=0 if there is a packet received
* 		
*/	
inline int8_t wd_wait_rx_pkt_info(void)
{
	int8_t n;
	uint16_t cnt=0;

  if (rx_packet < 0 ) return NRK_ERROR; // receive buffer not set

	while ((n = wdrf_sfd()) == 0) {
		if (cnt > WD_PKT_MAX_WAIT_TIME_10us) {
			wdrf_flush_rx_fifo();
			return NRK_ERROR;  // timeout as failsafe
		}
		halWait(10);
		cnt++;
	}
	if (n != 0) {
		n = 0;
		// Packet on its way
		cnt=0;
		while ((n = wdrf_polling_rx_packet ()) == 0) {
			if (cnt > WD_MAX_PKT_SIZE) {
				wdrf_flush_rx_fifo();
				break;          // timeout as failsafe
			}
			halWait(BYTE_TIME_us);
			cnt++;
		}
	}

	if (n == 1) return NRK_OK;
	else return NRK_ERROR;
}

/**********************************************************
* just loop until "widom time (x)" reaches the given value
* ASSUMES NO PREEMPTION (DISABLE_GLOBAL_INT() was done before)
* 
*  RETURN VALUE:
*		boolean indicating if a carrier was detected
*/
inline void wd_wait_until( uint16_t when_clktks ) {
	do {
		if (x >= when_clktks) break;
	} while (1); // loop during the carrier sensing interval	
}

/**********************************************************
* start sending a carrier pulse until "widom time (x)" reaches the given value
* assumes wdrf_radio_test_mode() was called before doing this
* ASSUMES DISABLE_GLOBAL_INT() was done before
*/		
inline void wd_carrier_on_until( uint16_t when_clktks ) {
	wdrf_carrier_on();
	wd_wait_until(when_clktks); // loop during the carrier duration
	wdrf_carrier_off();
}

/**********************************************************
* do carrier sense until "widom time (x)" reaches the given value
* ASSUMES NO PREEMPTION (DISABLE_GLOBAL_INT() was done before)
* 
*  RETURN VALUE:
*		boolean indicating if a carrier was detected
*/
inline bool wd_carrier_sense_until( uint16_t when_clktks ) {
	bool carrier=false;
	wdrf_set_rcv();
	halWait(100); // wait until CCA is valid 
	do {
		if (wdrf_cca()==false) carrier = true;
		if (x >= when_clktks) break;
	} while (1); // loop during the carrier sensing interval	

	return carrier;
}

/**********************************************************
* Initializes the protocol
* this calls wdrf_init, that in turn executes rf_init(...) from basic_rf.
*/
int8_t wd_init(uint8_t channel)
{
	wdrf_init(&rx_pkt_info, channel);
#if WD_VERSION != WD_MBD_RFLINX && WD_VERSION != WD_MBD_CC2420
	printf("WD_SYNC_PERIOD=%u ms WD_INTER_NEXT_SYNC=%u\r\n", (WD_SYNC_PERIOD_us/1000+1), (WD_INTER_NEXT_SYNC));
#endif	
	wd_start_high_speed_timer();

	wd_init_sync_pin(); // this macro does nothing, if not using external sync

	nrk_int_disable();

	wdrf_data_mode();

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

#if WD_VERSION != WD_MBD_RFLINX && WD_VERSION != WD_MBD_CC2420
	// block until it finds the first sync packet ...
	nrk_led_set(RED_LED);
	do {
	} while (wd_wait_sync() != NRK_OK);
	nrk_led_clr(RED_LED); 

	wd_set_high_speed_timer_interrupt(0, WD_INTER_NEXT_SYNC); // setup Timer/Counter1
#endif

	nrk_int_enable();

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
* this function performs the tournament (WiDom)
*
* ASSUMES NO PREEMPTION (DISABLE_GLOBAL_INT() was done before)
*
*  RETURN VALUE:
*  		a boolean indicating the result of the tournament
*/	
inline bool wd_do_tournament()
{
	// current priority index
	uint8_t i=0;
	// are we actively participating in the tournament or not
	bool winner=true;
	// current priority bit
	uint8_t pbit_i;
	// carrier sensing result
	bool cs=false;

	wdrf_test_mode(); // go to radio test mode

	do { winner=tx_packet; } while (0);

#ifdef WD_SBD

	wd_wait_until(C_SYNC); // ensure that all nodes start tournament at the same time

	// perform tournament
	for (i = 0; i < NPRIOBITS; i++) {
		pbit_i = wd_prio_bit(prio, i); // get current 
		wd_winner_prio (winner_prio, i, pbit_i);
		if (pbit_i == DOMINANT_BIT && winner == true) {
			wd_carrier_on_until ( H + (H+G)*i + C_SYNC );
		} else {
			cs = wd_carrier_sense_until ( H + (H+G)*i + C_SYNC );
		}
		if (pbit_i == RECESSIVE_BIT && cs == true) {
			winner = false;
			wd_winner_prio (winner_prio, i, DOMINANT_BIT);
		}
		//printf("prio[%d]=%d; winner=%d; cs=%d | ", i, pbit_i, winner, cs);
		if ( i < NPRIOBITS-1 ) {
			wd_wait_until( G + H + (H+G)*i + C_SYNC );  // loop during gap interval
		} else { // last iteration
			wdrf_data_mode( ); // set the radio back in "normal" data mode
			// this guarantees that receivers are listenning when winner
			// transmits data message
			if (winner==false) {
				wdrf_set_rcv();
				wdrf_polling_rx_on ();
			} else {
				wd_wait_until( ETG + H + (H+G)*i + C_SYNC );  
			}
		}
	}
#endif
#ifdef WD_MBD
#if WD_VERSION == WD_MBD_EXT_SYNC
	wd_wait_until(C_SYNC); // ensure that all nodes start tournament at the same time
#endif
	wd_reset_time();
	// perform tournament
	for (i = 0; i < NPRIOBITS; i++) {
		// stage 1
		pbit_i = wd_prio_bit(prio, i); // get current priority bit
		wd_winner_prio (winner_prio, i, pbit_i);
		cs=false;
		if (pbit_i == DOMINANT_BIT && winner == true) {
			wd_carrier_on_until ( H + (H2+G2)*i );
		} else {
			cs = wd_carrier_sense_until ( H + (H2+G2)*i);
		}
		wd_wait_until( G + H + (H2+G2)*i ); // loop during gap interval
		// stage 2
		if (cs == true) {
			wd_carrier_on_until ( H2 + G + (H2+G2)*i );
		} else {
			cs = wd_carrier_sense_until ( H2 + G + (H2+G2)*i );
		}	
		// decision
		wd_winner_prio (winner_prio, i, pbit_i);
		if (pbit_i == RECESSIVE_BIT && cs == true) {
			winner = false;
			wd_winner_prio (winner_prio, i, DOMINANT_BIT);
		}
		//printf("prio[%d]=%d; winner=%d; cs=%d | ", i, pbit_i, winner, cs);
		if ( i < NPRIOBITS-1 ) {
			wd_wait_until( G2 + H2 + (H2+G2)*i );  // loop during gap interval
		} else { // last iteration
			wd_reset_time();
			wdrf_data_mode( ); // set the radio back in "normal" data mode
			// this guarantees that receivers are listenning when winner
			// transmits data message
			if (winner==false) {
				wdrf_set_rcv();
				wdrf_polling_rx_on ();
			} else {
				wd_wait_until( H );  // loop during time interval after tournament (H)
			}
		}
	}
#endif
	tx_last_result=-1;
  rx_last_result = -1;

	// if we are winners, send packet
	if (winner==true) {
		//nrk_led_toggle(GREEN_LED);
		//nrk_led_clr(RED_LED);

		//if ((!tx_packet)==false) printf( "  WON (%d)!\r\n", prio );
		
		if (wdrf_tx_packet (&tx_pkt_info) == 0) { // tx packet
			tx_packet=false;
      tx_last_result=1;
		}
		
	} else { // if we are losers
		//nrk_led_toggle(RED_LED);
		//nrk_led_clr(GREEN_LED);

		//if ((!tx_packet)==false) printf( "  LOST (%d)!\r\n", prio );
		//else printf( "  LISTENNING (%d)!\r\n", prio );
		
		//if (rx_packet==false) { // only try to receive if we have a free packet  ** CHANGE: overwrite previous packet...
			if ( wd_wait_rx_pkt_info() == NRK_OK ) {
				rx_packet = 1;
				rx_last_result = 1;
			}
		//}
	}

#if WD_VERSION == WD_MBD_CC2420 || WD_VERSION == WD_MBD_RFLINX
	wd_wait_until(H+CMSG);
#endif

	return winner; // return result of the tournament
}


/**********************************************************
* called to run the protocol in widom-mbd
* 
*/	
#if WD_VERSION == WD_MBD_CC2420 || WD_VERSION == WD_MBD_RFLINX
inline int8_t wd_mdb_do_protocol()
{
	bool cca, ret, wait_e=false;

	//printf( "state=%d x=%d\r\n", state, TCNT1);

	if (state == 1) {
		// Flush the TX FIFO just in case...
		wdrf_test_mode(); 
		wdrf_set_rcv();
		trnmtCounter=0;
		wd_wait_until (TCS + TRX);
		wd_reset_time();
		state=2;
		return 0;
	}

	if (state == 2) {
		if (x >= F) {
			wd_reset_time();
			state = 3;
			return 0;
		}

		if (wdrf_cca()==false) { // carrier detected
			//nrk_gpio_set(NRK_DEBUG_0);
			wd_reset_time();
			do {
				cca=wdrf_cca();
			} while (x < H3-TCS &&  cca == false);
			if (x >= H3-TCS) {
				wd_wait_until (H3);
				//	nrk_gpio_clr(NRK_DEBUG_0);
				wd_reset_time();
				state = 8; // **** start tournament **** (enter state 8)
			} else wd_reset_time();
			return 0;
		}
	}

	if (state == 3) { 
		if (x >= E && wait_e == false) wait_e = true;

		if (wait_e == true && tx_packet == true) {
			wait_e = false;
			//nrk_gpio_set(NRK_DEBUG_0);
			wdrf_carrier_on();
			wd_reset_time();
			wd_wait_until(TTX);
			wd_reset_time();
			wd_wait_until(H3);
			wdrf_carrier_off();
			wd_reset_time();
			//nrk_gpio_clr(NRK_DEBUG_0);
			state = 8; // **** start tournament **** (enter state 8)
			return 0;
		}

		if (wdrf_cca()==false) { // carrier detected
			wait_e = false;
			wd_reset_time();
			wd_carrier_on_until( H3-TTX );
			wd_reset_time();
			state = 8; // **** start tournament **** (enter state 8)
			return 0;                  
		}
	}

	if (state == 8) {
		wd_wait_until (G);
		//nrk_gpio_set(NRK_DEBUG_0);
		ret = wd_do_tournament();
		//nrk_gpio_clr(NRK_DEBUG_0);
		wd_reset_time();
		trnmtCounter++; 
		state = 1;
		/*
		if (trnmtCounter == MAX_TC) {
		state = 1;
		} else {
		wd_wait_until(TRX+TCS);
		wd_reset_time();
		state = 17;
		}
		*/
		return 0;
	}  

	if (state == 17) {        
		if (x>=E+TCS) {
			wd_reset_time();
			state = 3;
			return 0;
		}

		if (wdrf_cca()==false) { // carrier detected
			wd_reset_time();
			wd_carrier_on_until( H3 );
			wd_reset_time();
			state = 8; // **** start tournament **** (enter state 8)
			return 0;                  
		}
	}
}
#endif

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
  if(wd_rx_pkt_ready()==NRK_OK) 
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
	bool ret;
	nrk_int_disable();
	ret=rx_packet;
	nrk_int_enable();
	if (ret==1) return NRK_OK;
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
  event=nrk_event_wait (SIG(wd_tx_pkt_info_signal));

  // Check if it was a time out instead of packet RX signal
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
  event=nrk_event_wait (SIG(wd_rx_pkt_info_signal));

  // Check if it was a time out instead of packet RX signal
  if((event & SIG(wd_rx_pkt_info_signal)) == 0 ) return NRK_ERROR;

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
}

/**********************************************************
* set rf power
* power = 31 => full power    (0dbm)
*          3 => lowest power  (-25dbm)
*/
int8_t wd_set_rf_power(uint8_t power)
{
  wdrf_set_power ( power );
}

/**********************************************************
* set cca threshold  
*/	
int8_t wd_set_cca_thresh(int8_t cca_thresh)
{
  wdrf_set_cca_thr ( cca_thresh );
}

/**********************************************************
* called every timer interrupt
* this function causes the node to start executing the protocol 
*/	
inline void wd_int_timer_handler() 
{
	bool winner;

	//nrk_gpio_toggle(NRK_DEBUG_0);

	nrk_int_disable();

	// Flush the TX FIFO just in case...
	wdrf_flush_rx_fifo();

	//nrk_led_clr(BLUE_LED);

	if (wd_wait_sync() == NRK_OK) {
		//nrk_led_set(BLUE_LED);
		// check result of tournament and send signal accordingly
		winner = wd_do_tournament(); 
		if (winner == true) {
      if (tx_last_result == 1) {
        nrk_led_toggle(RED_LED);
        nrk_event_signal ( wd_tx_pkt_info_signal );	// signal packet transmission
      }
		} else {
			if (rx_last_result == 1) {
        nrk_event_signal ( wd_rx_pkt_info_signal );	// signal packet reception
			}
		}
	} 

	nrk_int_enable();
}
