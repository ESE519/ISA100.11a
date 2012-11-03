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
/*   widom_rf.h                                                               */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#ifndef _WIDOM_RF_H
#define _WIDOM_RF_H

#include <include.h>
#include <widom.h>
#include <nrk.h>
#include <basic_rf.h>

// maximum packet size allowed
#define WD_MAX_PKT_SIZE 128
// maximum sync packet size allowed
#define WD_MAX_SYNC_PKT_SIZE 30
// byte time @250Kbps
#define BYTE_TIME_us 32

// radio channel used
#define WD_CHANNEL 25
// sync channel (the same)
#define WD_SYNC_CHANNEL WD_CHANNEL
// data channel 
#define WD_DATA_CHANNEL WD_CHANNEL

// maximum packet size supported by CC2420
#define WD_MAX_MSG_LEN_us 		4224		// max time to tx our messages (132 bytes (preamble+SFD+length byte+MPDU+crc)@250Kbps)

#define CC2420_TXCTRL_DFT_VAL			0xA0FF
// TXCTRL Register Bit Position
#define CC2420_TXCTRL_PAPWR				0   // 5 bits (0..4): Output PA level
// Mask for the CC2420_TXCTRL_PAPWR register for RF power
#define CC2420_TXCTRL_PAPWR_MASK		0x1F

// RSSI Register Bit Positions
#define CC2420_RSSI_CCA_THRESH			8   // 8 bits (8..15) : 2's compl CCA threshold

////////////////////////////////////////////////////
//
// General functions
//

/**********************************************************
 * Initializes radio(s) communication 
 */
void wdrf_init(RF_RX_INFO *pRRI, uint8_t channel);

/**********************************************************
 * set the radio into test mode, to send a modulated carrier
 * call this before sending carrier pulses
 */		 
void wdrf_test_mode();

/**********************************************************
 * set the radio into "normal" mode (buffered TXFIFO) and go into (data) receive
 */
void wdrf_data_mode();

/**********************************************************
 * start sending a carrier pulse
 * assumes wdrf_radio_test_mode() was called before doing this
 */		
void wdrf_carrier_on();

/**********************************************************
 * stop sending a carrier pulse; set the radio to idle state
 */
void wdrf_carrier_off();

/**********************************************************
 * stop radio; set radio to idle state
 */
void wdrf_stp();

/**********************************************************
 * set radio to receive
 */
void wdrf_set_rcv();

/**********************************************************
 * flush Rx FIFO
 */		 
void wdrf_flush_rx_fifo();

/**********************************************************
 * flush Tx FIFO
 */		 
void wdrf_flush_tx_fifo();

/**********************************************************
 * get cca state
 */		 
inline bool wdrf_cca();

/**********************************************************
 * get start of frame state
 * 
 * RETURN
 * 	true if start of frame was detected 
 *	false if no start of frame was detected 
 */		 
inline bool wdrf_sfd();

/**********************************************************
 * change channel
 */
inline void wdrf_set_channel( uint8_t channel );

/*************************************************************************
* set rf power
* power = 31 => full power    (0dbm)
*          3 => lowest power  (-25dbm)
*/
void wdrf_set_power(uint8_t power); 

/*************************************************************************
* adjust cca threshold 
*/
void wdrf_set_cca_thr(int8_t cca_thr);

/**********************************************************
 *  BYTE wdrf_tx_sync_packet(RF_TX_INFO *pRTI)
 *
 *  DESCRIPTION:
 *		Tx a synchronization packet	
 *
 *  RETURN VALUE:
 *		uint8_t
 */
uint8_t wdrf_tx_sync_packet();

/**********************************************************
 * check reception of synchronization packet
 *
 * ASSUMES DISABLE_GLOBAL_INT() was done before
 *  RETURN VALUE:
 *		uint8_t
 * 		< 0 if error
 *		seq number (>=0) if sync packet ok
 */
int8_t wdrf_poll_synch_packet_rx();

/**********************************************************
 *  BYTE wdrf_tx_packet(RF_TX_INFO *pRTI)
 *
 *  DESCRIPTION:
 *		Tx a packet data packet
 *
 *
 * ASSUMES NO PREEMPTION (DISABLE_GLOBAL_INT() was done before)
 * (function rf_tx_packet from basic_rf cannot be used because it enables interrupts...)
 *
 *  ARGUMENTS:
 *      RF_TX_INFO *pRTI
 *          The transmission structure, which contains all 
 *		relevant info about the packet.
 *
 *  RETURN VALUE:
 * 		<0 if error
 *		0 if ok
 */
uint8_t wdrf_tx_packet(RF_TX_INFO *pRTI);

/**********************************************************
 *  prepare to receive a data packet
 */
inline void wdrf_polling_rx_on(void);

/**********************************************************
 *  poll for received packet
 */
inline int8_t wdrf_polling_rx_packet();
 
////////////////////////////////////////////////////
//
// Functions only related to CC2420 radio module
//

/**********************************************************
 * Initializes CC2420 for radio communication via the basic RF library functions.
 * The arguments of this function are passed directly to rf_init(...) from basic_rf.
 * Radio parameters are then adjusted for widom.
 */
inline void cc2420rf_init(RF_RX_INFO *pRRI, uint8_t channel);

/**********************************************************
 * flush Rx FIFO
 */		 
inline void cc2420rf_flush_rx_fifo();

/**********************************************************
 * flush Tx FIFO
 */		 
inline void cc2420rf_flush_tx_fifo();

/**********************************************************
 * set the radio into test mode, to send a modulated carrier
 * call this before sending carrier pulses
 */		 
inline void cc2420rf_test_mode();

/**********************************************************
 * set the radio into "normal" mode (buffered TXFIFO) and go into (data) receive
 */
//void wdrf_data_mode(uint8_t channel) {
inline void cc2420rf_data_mode();

/**********************************************************
 * start sending a carrier pulse
 * assumes wdrf_radio_test_mode() was called before doing this
 */		
inline void cc2420rf_carrier_on();

/**********************************************************
 * stop sending a carrier pulse; set the radio to idle state
 */
inline void cc2420rf_carrier_off();

/**********************************************************
 * stop radio; set radio to idle state
 */
inline void cc2420rf_stp();

/**********************************************************
 * set radio to receive, to detect carrier pulses
 */
inline void cc2420rf_set_rcv();

/**********************************************************
 * change channel
 */
inline void cc2420rf_set_channel( uint8_t channel );

/*************************************************************************
* set rf power
* power = 31 => full power    (0dbm)
*          3 => lowest power  (-25dbm)
*/
inline void cc2420rf_set_power(uint8_t power); 

/*************************************************************************
* adjust cca threshold 
*/
inline void cc2420rf_set_cca_thr(int8_t cca_thr);

#endif

