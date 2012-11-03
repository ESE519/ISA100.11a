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




#ifndef _TIMESYNC_RF_H
#define _TIMESYNC_RF_H

#define TIMESYNC_CHANNEL 20

#define MAX_PKT_SIZE 4000
#define BYTE_TIME_us 32

/**********************************************************
 * Initializes CC2420 for radio communication.
 */
inline void btrf_init(uint8_t channel, uint16_t panId, uint16_t mAddr);

/**********************************************************
 * flush Rx FIFO
 */		 
inline void btrf_flush_rx_fifo();

/**********************************************************
 * flush Tx FIFO
 */		 
inline void btrf_flush_tx_fifo();

/**********************************************************
 * stop radio; set radio to idle state
 */
inline void btrf_stp();

/**********************************************************
 * set radio to receive
 */
inline void btrf_set_rcv();

/**********************************************************
 * change channel
 */
inline void btrf_set_channel( uint8_t channel );

/**********************************************************
 *  BYTE tx_sync_packet(RF_TX_INFO *pRTI)
 *
 *  DESCRIPTION:
 *		Tx a synchronization packet	
 *
 *  RETURN VALUE:
 *		uint8_t
 */
uint8_t tx_sync_packet(uint32_t offset, uint16_t id);

/**********************************************************
 * check reception of synchronization packet
 *
 * ASSUMES DISABLE_GLOBAL_INT() was done before
 *  RETURN VALUE:
 *		uint8_t
 * 		< 0 if error
 *		seq number (>=0) if sync packet ok
 */
inline int8_t poll_synch_packet_rx(uint16_t *id, uint8_t *seq);

/**********************************************************
 * block until a packet is received or clock is past given time
 *
 *  RETURN VALUE:
 *		uint8_t
 * 		< 0 if error
 *		0 if ok
 */
inline int8_t wait_rx_sync_pkt_until(uint32_t until_clkTks, uint16_t *id, uint8_t *seq);

#endif
