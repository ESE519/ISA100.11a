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
/*   widom.h                                                                  */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#ifndef _WIDOM_H
#define _WIDOM_H

#include <widom_rf.h>

////////////////////////////////////////////////////
//
// Protocol modes/versions
//

// single broadcast domain (SBD)
#define WD_SBD_CC2420	1 // the CC2420 is used to send priority bits; needs a master synchronization node (firefly; micaZ platforms)
#define WD_SBD_RFLINX	2 // uses the RXLINX radio addon board to send priority bits; needs a master synchronization node (only with firefly platform)

// multiple broadcast domain (MBD) [*Still Experimental*]
#define WD_MBD_RFLINX	3   // uses the RXLINX radio addon board to send priority bits (only with firefly platform)
#define WD_MBD_CC2420	5   // uses the CC2420 radio to send priority bits (firefly; micaZ platforms)
#define WD_MBD_EXT_SYNC	4 // uses an external sync device connected (SYNC_PIN (pin 28) signals a new period; only with firefly platform)

// define mode/version of the protocol (WD_SBD_CC2420, WD_SBD_RFLINX, WD_MBD_RFLINX, WD_MBD_CC2420, WD_MBD_EXT_SYNC)
#define WD_VERSION WD_SBD_CC2420

// do defines accordingly...
#if WD_VERSION == WD_SBD_CC2420 || WD_VERSION == WD_SBD_RFLINX
#define WD_SBD
#if WD_VERSION == WD_SBD_RFLINX
#define WD_USES_RADIO_RXLINX
#endif
#endif
#if WD_VERSION == WD_MBD_EXT_SYNC || WD_VERSION == WD_MBD_RFLINX || WD_VERSION == WD_MBD_CC2420
#define WD_MBD
#if WD_VERSION == WD_MBD_RFLINX
#define WD_USES_RADIO_RXLINX
#endif
#endif

////////////////////////////////////////////////////
//
// Protocol parameters
//

#define DOMINANT_BIT 0
#define RECESSIVE_BIT 1

// these three parameters define how long we wake up before the synchronization
#define WD_MAX_BLOCKING_TIME_us 150		// maximum blocking widom may experience
#define WD_SYNC_PULSE_RX_SWX_us 256		// time to switch to rx and lock on to the preamble

#ifdef WD_SBD
#define WD_SYNC_WITH_MASTER_MAX_ERROR 400	// maximum difference with synch master
#endif
#ifdef WD_MBD
#define WD_SYNC_WITH_MASTER_MAX_ERROR 50	// maximum difference synch error
#endif

// num of priorities
#define NPRIOBITS		3
#define MAX_NUM_PRIO_BITS	16
#define INVALID_PRIO_INDEX	MAX_NUM_PRIO_BITS
#define INVALID_PRIO 		0xFFFF

// timeouts interval constants (in us)

#if WD_VERSION == WD_SBD_CC2420 || WD_VERSION == WD_MBD_EXT_SYNC

// timeout parameters when using cc2420 to transmit priority bits
#define H_us 			320 		// duration of a pulse of a carrier
#define G_us 			60 		  // "guarding" time interval 
#define ETG_us 		400 		// "guarding" time interval after the tournament

#endif // #if WD_VERSION == WD_SBD_CC2420
#if WD_VERSION == WD_SBD_RFLINX

// timeout parameters when using rxlinx radio addon board to transmit priority bits
#define H_us 			100 		// duration of a pulse of a carrier
#define G_us 			100 		// "guarding" time interval 
#define ETG_us 		400 		// "guarding" time interval after the tournament

#endif // #if WD_VERSION == WD_SBD_RFLINX
#if WD_VERSION == WD_MBD_RFLINX

// timeout parameters when using rxlinx radio addon board to transmit priority bits
#define H_us 			166 		// duration of a pulse of a carrier
#define G_us 			111 		// "guarding" time interval 
#define E_us 		  56 		// time to cope with sync imperfections
#define F_us 		  9321 	// initial long period of silence 

#define TRX_us     0        // time to switch to rx
#define TTX_us     0        // time to switch to rx
#define TCS_us     50      // time for carrier sensing

#endif // #if WD_VERSION == WD_MBD_RFLINX

#if WD_VERSION == WD_MBD_CC2420

// timeout parameters when using cc2420 onboard radio to transmit priority bits
#define H_us 			2119 		// duration of a pulse of a carrier
#define G_us 			921 		// "guarding" time interval 
#define E_us 		  496 		// time to cope with sync imperfections
#define F_us 		  39016 	// initial long period of silence 

#define TRX_us     0        // time to switch to rx
#define TTX_us     0        // time to switch to rx
#define TCS_us     486      // time for carrier sensing

#define MAX_TC     100

#endif // #if WD_VERSION == WD_MBD_RFLINX

#define CMSG_us 		WD_MAX_MSG_LEN_us// max time to tx our messages 
#define CSYNCMSG_us		352		// time for transmission of sync message

// define the synchronization period 
#ifdef WD_SBD

// tournament duration+maximum packet+blocking+switch to rx 
// NOTE: Changing any of the factors involved in the following 
//        expression means reprogramming the sync master again
#define WD_SYNC_PERIOD_us (((NPRIOBITS-1) * (H_us+G_us)+H_us+ETG_us)+WD_MAX_MSG_LEN_us+WD_MAX_BLOCKING_TIME_us+WD_SYNC_PULSE_RX_SWX_us+WD_SYNC_WITH_MASTER_MAX_ERROR)

// timeout when waiting for sync packet (us)
#define WD_SYNC_MAX_WAIT_TIME_us 1800

#endif // #ifdef WD_SBD

#if WD_VERSION == WD_MBD_EXT_SYNC

// in widom MBD using external sync device depends on 
// the sync pin period defined by the external sync device
#define WD_SYNC_PERIOD_us 10000

#endif // WD_VERSION == WD_MBD_EXT_SYNC

// timeout when waiting for data packet (10 us units)
#define WD_PKT_MAX_WAIT_TIME_10us 100


////////////////////////////////////////////////////
//
// Macros to handle priority bits
//

/***************************************************
* macro that returns the value of the priority bit at the index given
*/
#define wd_prio_bit( _priority, _index ) (!(!(_priority & (1 << ((NPRIOBITS - 1) - _index)))))

/***************************************************
* macro that saves the winner priority bit at the given index
*/
#define wd_winner_prio( _var, _bit_index, _bit_val ) { \
	if (_bit_val) ((_var) |= ( BM(_bit_index))); \
		else ((_var) &= (~BM(_bit_index))); \
}

////////////////////////////////////////////////////
//
// functions used by aplications 
//

/**********************************************************
* Initializes the protocol
* this calls wdrf_init, that in turn executes rf_init(...) from basic_rf.
*/
int8_t wd_init(uint8_t channel); 

/**********************************************************
* State of protocol initialization
* 
*  RETURN VALUE:
*    1 if protocol is initialized
*    0 if protocol is NOT initialized
*/
int8_t wd_started(void); 

/**********************************************************
* this function returns the tx done signal reference
*/
nrk_sig_t wd_get_tx_signal(); 

/**********************************************************
* this function passes a packet to be sent
* and waits until the end transmission
* 
*  RETURN VALUE:
*  	NRK_ERROR if packet was not sent successfully
*  	NRK_OK if it was transmitted
*/
int8_t wd_tx_packet(uint8_t *buf, uint8_t len, uint16_t priority);	

/**********************************************************
* this function passes a packet to be sent 
*
*  RETURN VALUE:
*  	NRK_ERROR if packet was not accepted
*  	NRK_OK if it was accepted
*/	
int8_t wd_tx_packet_enqueue(uint8_t *buf, uint8_t len, uint16_t priority);

/**********************************************************
* wait until end of next packet transmission
* 
* RETURN VALUE:
*  	NRK_ERROR timeout
*  	NRK_OK 
*/	
int8_t wd_wait_until_tx_packet();

/**********************************************************
* this function sets the receive buffer 
*
*  RETURN VALUE:
*  	NRK_ERROR 
*  	NRK_OK 
*/
int8_t wd_rx_pkt_set_buffer(uint8_t *buf, uint8_t size);

/**********************************************************
* this function returns the receive buffer 
*
*  RETURN VALUE:
*  	NRK_ERROR 
*  	NRK_OK 
*/
uint8_t *wd_rx_pkt_get(uint8_t *len, uint8_t *rssi);

/**********************************************************
* this function checks if a packet was received completely
*
*  RETURN VALUE:
*  	NRK_ERROR no packet 
*  	NRK_OK packet completely
*/	
int8_t wd_rx_pkt_ready();

/**********************************************************
* this function releases the reception buffer 
*
*  RETURN VALUE:
*  	NRK_OK 	
*/	
int8_t wd_rx_pkt_release();

/**********************************************************
* wait until end of next packet transmission
* 
* RETURN VALUE:
*  	NRK_ERROR timeout
*  	NRK_OK 
*/	
int8_t wd_wait_until_rx_packet();

/**********************************************************
* return the priority of the winner of the last tournament 
* 
* RETURN VALUE:
*   uint16_t the priority 
*/	
uint16_t wd_get_winner();

/**********************************************************
* change channel  
*/	
int8_t wd_set_channel(uint8_t channel);

/**********************************************************
* set rf power
* power = 31 => full power    (0dbm)
*          3 => lowest power  (-25dbm)
*/
int8_t wd_set_rf_power(uint8_t power);

/**********************************************************
* set cca threshold  
*/	
int8_t wd_set_cca_thresh(int8_t thresh);

////////////////////////////////////////////////////
//
// functions used by the protocol 
//

/**********************************************************
* wait for the synchronization packet
* blocks until the reception of a valid sync pkt
* ASSUMES NO PREEMPTION (DISABLE_GLOBAL_INT() was done before)
* 
* sets up timer (atmega 128 Timer/Counter1) compare interrupt to
* fire again in order to wait for the next synchronization packet
*/	
inline int8_t wd_wait_sync(void);

/**********************************************************
* this function waits for packet reception
*
*  RETURN VALUE:
*  	>=0 if there is a packet received
* 		
*/	
inline int8_t wd_wait_rx_pkt(void);

/**********************************************************
* just loop until "widom time (x)" reaches the given value
* ASSUMES NO PREEMPTION (DISABLE_GLOBAL_INT() was done before)
* 
*  RETURN VALUE:
*		boolean indicating if a carrier was detected
*/
inline void wd_wait_until( uint16_t when_clktks );

/**********************************************************
* start sending a carrier pulse until "widom time (x)" reaches the given value
* assumes wdrf_radio_test_mode() was called before doing this
* ASSUMES NO PREEMPTION (DISABLE_GLOBAL_INT() was done before)
*/		
inline void wd_carrier_on_until( uint16_t when_clktks );

/**********************************************************
* do carrier sense until "widom time (x)" reaches the given value
* ASSUMES NO PREEMPTION (DISABLE_GLOBAL_INT() was done before)
* 
*  RETURN VALUE:
*		boolean indicating if a carrier was detected
*/
inline bool wd_carrier_sense_until( uint16_t when_clktks );


////////////////////////////////////////////////////
//
// the protocol 
//

/**********************************************************
* this function performs the tournament
*
*  RETURN VALUE:
*  		a boolean indicating the result of the tournament
*/	
inline bool wd_do_tournament();

/**********************************************************
* called to run the protocol in widom-mbd
* 
*/	
inline uint8_t wd_mdb_do_protocol();

/**********************************************************
* called every timer interrupt
* this function causes the node to start executing the protocol 
*/	
inline void wd_int_timer_handler();

#endif
