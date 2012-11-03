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
*  Anthony Rowe
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
#include <wd_rf.h>

// maximum packet size allowed
#define WD_MAX_PKT_SIZE 128
// byte time @250Kbps
#define BYTE_TIME_us 32

// radio channel used
#define WD_CHANNEL 0x0F
// sync channel (the same)
#define WD_SYNC_CHANNEL WD_CHANNEL
// data channel 
#define WD_DATA_CHANNEL WD_CHANNEL

// maximum packet size supported by CC2420
#define WD_MAX_MSG_LEN_us 		4224		// max time to tx our messages (132 bytes (preamble+SFD+length byte+MPDU+crc)@250Kbps)

// TXCTRL Register Bit Position
#define CC2420_TXCTRL_PAPWR				0   // 5 bits (0..4): Output PA level
// Mask for the CC2420_TXCTRL_PAPWR register for RF power
#define CC2420_TXCTRL_PAPWR_MASK		0x1F

// RSSI Register Bit Positions
#define CC2420_RSSI_CCA_THRESH			8   // 8 bits (8..15) : 2's compl CCA threshold

// sampling period (us)
#define WDRF_SAMPLING_PERIOD_us 32 

/**********************************************************
 * Initializes CC2420 for radio communication via the basic RF library functions.
 * The arguments of this function are passed directly to rf_init(...) from basic_rf.
 * Radio parameters are then adjusted for widom.
 */
void wdrf_init(RF_RX_INFO *pRRI, uint8_t channel); 

/**********************************************************
* flush Rx FIFO
*/		 
inline void wdrf_flush_rx_fifo();

/**********************************************************
* flush Rx FIFO
*/		 
inline void wdrf_flush_tx_fifo();

/**********************************************************
* set the radio into test mode, to send a modulated carrier
* call this before sending carrier pulses
* ASSUMES: running uninterrupted to completion    
*/		 
inline void wdrf_test_mode();

/**********************************************************
* set the radio into "normal" mode (buffered TXFIFO) and go into (data) receive
*
* ASSUMES: running uninterrupted to completion    
*/
inline void wdrf_data_mode();

/**********************************************************
* start sending a carrier pulse
* assumes wdrf_radio_test_mode() was called before doing this
*/		
inline void wdrf_carrier_on();

/**********************************************************
* stop radio; set radio to idle state
*/
inline void wdrf_stp();

/**********************************************************
* set radio to receive, to detect carrier pulses
*/
inline void wdrf_set_rcv();

/**********************************************************
* change channel
*/
void wdrf_set_channel( uint8_t channel );

/*************************************************************************
* set rf power
* power = 31 => full power    (0dbm)
*          3 => lowest power  (-25dbm)
*          
* ASSUMES: running uninterrupted to completion
*/
void wdrf_set_power(uint8_t power);

/*************************************************************************
* adjust cca threshold 
*/
void wdrf_set_cca_thr(int8_t cca_thr);

/**********************************************************
* get cca state
* REQUIRES 
*   that is only called 256us after call to wdrf_set_rcv
*   
* RETURN
* 	true if channel is clear
*	  false if chanel is busy
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
* check reception of synchronization packet
*
* ASSUMES: running uninterrupted to completion
* 
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
* ASSUMES: running uninterrupted to completion
* 
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
void wdrf_polling_rx_on(void);

/**********************************************************
*  poll for received packet
*/
int8_t wdrf_polling_rx_packet();

/********************************
 * This is used for periodic sampling of the medium
 * and drive the protocol according to the state changes detected 
 */ 
inline void periodic_callback(void);

#endif
