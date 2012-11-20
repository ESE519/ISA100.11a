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
/*   widom_rf.c                                                               */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#include <include.h>
#include <ulib.h>
#include <nrk.h>
#include <nrk_error.h>
#include <widom.h>
#include <wd_rf.h>
#include <wd_timer.h>

#define SIZE_FCS 2
#define SIZE_SYNC_PKT_PAYLOAD 3

#define SYNC_PKT_TYPE 0xAA
#define BCAST_ADDR 0xFFFF

#define CC2420_DEF_FCF 0x0821 // no ack

int8_t sync_seq_cnt=1; // do not use 0 sequence number 

int8_t txSeqNumber=0;

int8_t cca_state;

bool fifop_ind=false;
uint8_t wait_tx=0;

#define TIME_UNTIL_CCA_VALID_us 192
uint16_t TIME_UNTIL_CCA_VALID = ( TIME_UNTIL_CCA_VALID_us / 1000000.0) / WD_CLOCK_TICK_TIME + 1;

#define TIME_UNTIL_START_TX_us 128
uint16_t TIME_UNTIL_START_TX = ( TIME_UNTIL_START_TX_us / 1000000.0) / WD_CLOCK_TICK_TIME + 1;

/**********************************************************
 * Initializes CC2420 for radio communication via the basic RF library functions.
 * The arguments of this function are passed directly to rf_init(...) from basic_rf.
 * Radio parameters are then adjusted for widom.
 */
void wdrf_init(RF_RX_INFO *pRRI, uint8_t channel) 
{
	rf_init(pRRI, channel, 0x1111, NODE_ADDR);

	// Disable interrupts
	nrk_int_disable();

	// Set registers to some specific widom parameters

	FASTSPI_SETREG(CC2420_RSSI, 0xE580); // CCA THR=-27
	//FASTSPI_SETREG(CC2420_RSSI, 0xD880); // CCA THR=-40
	FASTSPI_SETREG(CC2420_TXCTRL, 0x80FF); // TX TURNAROUND = 128 us
	FASTSPI_SETREG(CC2420_RXCTRL1, 0x0A56); 

	wdrf_flush_rx_fifo();

	// Turn interrupts back on
	nrk_int_enable();

	// disable FIFOP interrupt, just in case
	DISABLE_FIFOP_INT();
}

/**********************************************************
* flush Rx FIFO
*/		 
inline void wdrf_flush_rx_fifo() 
{
	uint8_t tmp;
	// always read 1 byte before flush (data sheet pg 62)
	FASTSPI_READ_FIFO_BYTE(tmp);  
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	fifop_ind=false;
}

/**********************************************************
* flush Rx FIFO
*/		 
inline void wdrf_flush_tx_fifo() 
{
	FASTSPI_STROBE(CC2420_SFLUSHTX);
	FASTSPI_STROBE(CC2420_SFLUSHTX);
}

/**********************************************************
* set the radio into test mode, to send a modulated carrier
* call this before sending carrier pulses
* ASSUMES: running uninterrupted to completion    
*/		 
inline void wdrf_test_mode() 
{
	FASTSPI_STROBE(CC2420_SRFOFF); //stop radio
	// NOTE ON SETTING CC2420_MDMCTRL1
	// 
	// RF studio" uses TX_MODE=3 (CC2420_MDMCTRL1=0x050C)
	// to send an unmodulated carrier; data sheet says TX_MODE 
	// can be 2 or 3. So it should not matter...
	// HOWEVER, using (TX_MODE=3) sometimes causes problems when 
	// going back to "data" mode!
	FASTSPI_SETREG(CC2420_MDMCTRL1, 0x0508); // MDMCTRL1 with TX_MODE=2
	FASTSPI_SETREG(CC2420_DACTST, 0x1800); // send unmodulated carrier
	wdrf_flush_rx_fifo();
}

/**********************************************************
* set the radio into "normal" mode (buffered TXFIFO) 
*
* ASSUMES: running uninterrupted to completion    
*/
inline void wdrf_data_mode() {
	FASTSPI_STROBE(CC2420_SRFOFF); //stop radio
	FASTSPI_SETREG(CC2420_MDMCTRL1, 0x0500); // default MDMCTRL1 value
	FASTSPI_SETREG(CC2420_DACTST, 0); // default value
	wdrf_flush_rx_fifo();
}

/**********************************************************
* start sending a carrier pulse
* assumes wdrf_radio_test_mode() was called before doing this
*/		
inline void wdrf_carrier_on() 
{
	FASTSPI_STROBE(CC2420_STXON); // tell radio to start sending	
}

/**********************************************************
* stop radio; set radio to idle state
*/
inline void wdrf_stp() 
{
  wd_timer_stop_periodic();
  wait_tx = 0;

	FASTSPI_STROBE(CC2420_SRFOFF); // stop radio
}

/**********************************************************
* set radio to receive, to detect carrier pulses
*/
inline void wdrf_set_rcv() 
{
	FASTSPI_STROBE(CC2420_SRXON);	
	//halWait(256); // wait until CCA is valid
  cca_state = -1;
  wd_timer_start_periodic(TIME_UNTIL_CCA_VALID); // start periodic sampling
}

/**********************************************************
* change channel
*/
void wdrf_set_channel( uint8_t channel )
{
	uint16_t f;

  wdrf_stp();

	// Derive frequency programming from the given channel number
	f = (uint16_t) (channel - 11); // Subtract the base channel
	f = f + (f << 2);                // Multiply with 5, which is the channel spacing
	f = f + 357 + 0x4000;            // 357 is 2405-2048, 0x4000 is LOCK_THR = 1

	// Write it to the CC2420
	FASTSPI_SETREG(CC2420_FSCTRL, f);
}

/*************************************************************************
* set rf power
* power = 31 => full power    (0dbm)
*          3 => lowest power  (-25dbm)
*          
* ASSUMES: running uninterrupted to completion
*/
void wdrf_set_power(uint8_t power) 
{
	uint16_t reg_val;
    
  if (power > 31) return;
    
  reg_val=0xA0E0;
  reg_val= reg_val | (power & CC2420_TXCTRL_PAPWR_MASK);  
    
	FASTSPI_SETREG(CC2420_TXCTRL, reg_val);
}

/*************************************************************************
* adjust cca threshold 
*/
void wdrf_set_cca_thr(int8_t cca_thr) 
{	
	uint16_t reg_val = (cca_thr<<CC2420_RSSI_CCA_THRESH) | 0x80;

	FASTSPI_SETREG(CC2420_RSSI, reg_val); 
}

/**********************************************************
* get cca state
* REQUIRES 
*   that is only called 256us after call to wdrf_set_rcv
*   
* RETURN
* 	true if channel is clear
*	  false if chanel is busy
*/		 
inline bool wdrf_cca()
{
	return (CCA_IS_1);
}

/**********************************************************
* get start of frame state
* 
* RETURN
* 	true if start of frame was detected 
*	false if no start of frame was detected 
*/		 
inline bool wdrf_sfd()
{
	return (SFD_IS_1);
}

/**********************************************************
*  BYTE wdrf_tx_sync_packet(RF_TX_INFO *pRTI)
*
*  DESCRIPTION:
*		Tx a synchronization packet	
*
*  RETURN VALUE:
*		uint8_t
*/
uint8_t wdrf_tx_sync_packet() 
{
	uint8_t packetLength, type=SYNC_PKT_TYPE;

	// Wait until the transceiver is idle
	//while (FIFOP_IS_1 || SFD_IS_1);
	FASTSPI_STROBE (CC2420_SRFOFF);

	// Flush the RX/TX FIFOs
	wdrf_flush_rx_fifo();
	wdrf_flush_tx_fifo();

	// Turn off global interrupts to avoid interference on the SPI interface
	nrk_int_disable();

	FASTSPI_STROBE (CC2420_STXON);

	if (sync_seq_cnt <= 0) sync_seq_cnt=1; // overflow; do not send sequence numbers <=0

	// Write the packet to the TX FIFO (assuming AUTOCRC is ENABLED)
	packetLength = SIZE_SYNC_PKT_PAYLOAD + SIZE_FCS; 
	FASTSPI_WRITE_FIFO((uint8_t*) &packetLength, 1);               	// packet length
	FASTSPI_WRITE_FIFO((uint8_t*) &packetLength, 1);               	// packet length
	FASTSPI_WRITE_FIFO((uint8_t*) &type, 1);    			// sync type
	FASTSPI_WRITE_FIFO((uint8_t*) &sync_seq_cnt, 1);    		// sequence number

	// wait for the transmission to begin 
	while (!SFD_IS_1);

	// wait for packet to finish
	while (SFD_IS_1); 
	FASTSPI_STROBE(CC2420_SRFOFF);  // shut off radio

	sync_seq_cnt++;

	nrk_int_enable();
	return 1;
}

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
uint8_t wdrf_tx_packet(RF_TX_INFO *pRTI) {
	uint16_t frameControlField, panId=BCAST_ADDR, myAddr=NODE_ADDR;
	uint8_t packetLength;
	uint8_t checksum,i;

	//nrk_gpio_set(DEBUG_1);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	FASTSPI_STROBE(CC2420_SFLUSHRX);

	// stop transceiver
	FASTSPI_STROBE (CC2420_SRFOFF);

	// Flush the TX FIFO just in case...
	FASTSPI_STROBE(CC2420_SFLUSHTX);
	FASTSPI_STROBE(CC2420_SFLUSHTX);

	checksum=0;
	for(i=0; i<pRTI->length; i++ )
	{
		// lets do our own payload checksum because we don't trust the CRC
		checksum+=pRTI->pPayload[i];
	}
	// Write the packet to the TX FIFO (the FCS is appended automatically when AUTOCRC is enabled)
	// These are only the MAC AGNOSTIC parameters...
	packetLength = pRTI->length + RF_PACKET_OVERHEAD_SIZE + CHECKSUM_OVERHEAD;
	FASTSPI_WRITE_FIFO((uint8_t*)&packetLength, 1);               // Packet length
	frameControlField = CC2420_DEF_FCF;
	frameControlField = RF_FCF_NOACK;
	FASTSPI_WRITE_FIFO((uint8_t*) &frameControlField, 2);         // Frame control field
	FASTSPI_WRITE_FIFO((uint8_t*) &txSeqNumber, 1);    // Sequence number
	FASTSPI_WRITE_FIFO((uint8_t*) &panId, 2);          // Dest. PAN ID
	FASTSPI_WRITE_FIFO((uint8_t*) &pRTI->destAddr, 2);            // Dest. address
	FASTSPI_WRITE_FIFO((uint8_t*) &myAddr, 2);         // Source address
	FASTSPI_WRITE_FIFO((uint8_t*) pRTI->pPayload, pRTI->length);  // Payload
	FASTSPI_WRITE_FIFO((uint8_t*) &checksum, 1);         // Checksum

	FASTSPI_STROBE (CC2420_STXON);
/*
	// Wait for the transmission to begin before exiting 
	do {
		if (i > WD_MAX_PKT_SIZE) {
			return -1;          // timeout as failsafe
		}
		i++;
		halWait(BYTE_TIME_us);
	} while (!SFD_IS_1);

	while (SFD_IS_1); // wait for packet to finish

	wdrf_flush_rx_fifo();

	FASTSPI_STROBE (CC2420_SRFOFF);

	wdrf_set_rcv();
*/

  wait_tx=1;
  wd_timer_start_periodic(TIME_UNTIL_START_TX); // start periodic sampling

	txSeqNumber++; 

	return NRK_OK;
}

/**********************************************************
*  prepare to receive a data packet
*/
void wdrf_polling_rx_on(void)
{
	FASTSPI_STROBE(CC2420_SRXON);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
}

/**********************************************************
*  poll for received packet
*/
int8_t wdrf_polling_rx_packet()
{
  fifop_ind=false;
	// it is safe to call rf_polling_rx_packet from basic_rf...
	// (with RADIO_PRIORITY_CEILING off)
	return rf_polling_rx_packet();
}

/********************************
 * This is used for periodic sampling of the medium
 * and drive the protocol according to the state changes detected 
 */ 
inline void periodic_callback(void)
{
//    nrk_gpio_toggle(NRK_DEBUG_0);

    if (wait_tx == 0) {
      if (CCA_IS_1 != cca_state) {
        cca_state = (bool)CCA_IS_1;
        if (cca_state == true) {
          wd_do_protocol(MSG_TYPE_RADIO_CHANNEL_IDLE);
        } else {
          wd_do_protocol(MSG_TYPE_RADIO_CHANNEL_BUSY);
        }
      }
      if (FIFOP_IS_1 && fifop_ind == false) {
      	if (!(FIFO_IS_1)) { // FIFO overflow
  			 wdrf_flush_rx_fifo(); 
  			 fifop_ind == true;
  		  } else {
  		    fifop_ind=true;
          wd_do_protocol(MSG_TYPE_RADIO_END_RX);
        }
      }
    } else {
      if (wait_tx == 1 && SFD_IS_1) wait_tx = 2;
      if (wait_tx == 2 && !SFD_IS_1) {
        wait_tx = 0;
        wd_do_protocol(MSG_TYPE_RADIO_END_TX);
      }
    }
}
