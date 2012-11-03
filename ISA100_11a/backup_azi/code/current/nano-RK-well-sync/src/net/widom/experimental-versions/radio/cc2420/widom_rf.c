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
/*   widom_rf.c                                                               */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#include <include.h>
#include <widom.h>
#include <widom_rf.h>
#include <widom_linx_rf.h>
#include <ulib.h>
#include <nrk.h>

#define SIZE_FCS 2
#define SIZE_SYNC_PKT_PAYLOAD 3

#define SYNC_PKT_TYPE 0xAA
#define BCAST_ADDR 0xFFFF

#define CC2420_DEF_FCF 0x0821 // no ack

int8_t sync_seq_cnt=1; // do not use 0 sequence number 

int8_t txSeqNumber=0;

/**********************************************************
* Initializes radio(s) communication 
*/
void wdrf_init(RF_RX_INFO *pRRI, uint8_t channel) 
{

#ifdef WD_USES_RADIO_RXLINX
	// init linxrfs radio and turn receive and transmit modules on
	linxrf_init();
#endif

	cc2420rf_init(pRRI, channel);

	// disable FIFOP interrupt, just in case
	DISABLE_FIFOP_INT();
}

/**********************************************************
* set the radio into test mode, to send a modulated carrier
* call this before sending carrier pulses
*/		 
void wdrf_test_mode() 
{
#ifndef WD_USES_RADIO_RXLINX
	cc2420rf_test_mode();
#endif
}

/**********************************************************
* set the radio into "normal" mode (buffered TXFIFO) and go into (data) receive
*/
void wdrf_data_mode() {
#ifndef WD_USES_RADIO_RXLINX
	cc2420rf_data_mode();
#endif
}

/**********************************************************
* start sending a carrier pulse
* assumes wdrf_radio_test_mode() was called before doing this
*/		
void wdrf_carrier_on() 
{
#ifndef WD_USES_RADIO_RXLINX
	cc2420rf_carrier_on();
#else // WD_USES_RADIO_RXLINX defined
	linxrf_tx_write(1);
#endif
}

/**********************************************************
* stop sending a carrier pulse; set the radio to idle state
*/
void wdrf_carrier_off() 
{
#ifndef WD_USES_RADIO_RXLINX
	cc2420rf_carrier_off();
#else // WD_USES_RADIO_RXLINX defined
	linxrf_tx_write(0);
#endif
}

/**********************************************************
* stop radio; set radio to idle state
*/
void wdrf_stp() 
{
#ifndef WD_USES_RADIO_RXLINX
	cc2420rf_stp();
#else // WD_USES_RADIO_RXLINX defined
	linxrf_tx_write(0);
#endif
}

/**********************************************************
* set radio to receive
*/
void wdrf_set_rcv() 
{
#ifndef WD_USES_RADIO_RXLINX
	cc2420rf_set_rcv();
	halWait(256); // wait until CCA is valid 
#else // WD_USES_RADIO_RXLINX defined
	cc2420rf_set_rcv(); // should still set CC2420 to receive
	// do nothing... radio does not change between rx/tx
#endif
}

/**********************************************************
* flush Rx FIFO
*/		 
void wdrf_flush_rx_fifo() 
{
	cc2420rf_flush_rx_fifo();
}

/**********************************************************
* flush Tx FIFO
*/		 
inline void wdrf_flush_tx_fifo() 
{
	cc2420rf_flush_tx_fifo();
}

/**********************************************************
* get cca state
* 
* RETURN
* 	true if channel is clear
*	false if chanel is busy
*/		 
inline bool wdrf_cca()
{
#ifndef WD_USES_RADIO_RXLINX
	return (CCA_IS_1);
#else // WD_USES_RADIO_RXLINX defined
	return !(linxrf_rssi_thr1());
#endif
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

inline void wdrf_set_channel( uint8_t channel )
{
#ifndef WD_USES_RADIO_RXLINX
	cc2420rf_set_channel( channel );
#endif
}

/*************************************************************************
* set rf power
* power = 31 => full power    (0dbm)
*          3 => lowest power  (-25dbm)
*/
void wdrf_set_power(uint8_t power) 
{
#ifndef WD_USES_RADIO_RXLINX
  cc2420rf_set_power(power);
#endif
}

/*************************************************************************
* adjust cca threshold 
*/
void wdrf_set_cca_thr(int8_t cca_thr) 
{
#ifndef WD_USES_RADIO_RXLINX
  cc2420rf_set_cca_thr(cca_thr);
#endif
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
	DISABLE_GLOBAL_INT();

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

	ENABLE_GLOBAL_INT();
	return 1;
}

/**********************************************************
* check reception of synchronization packet
*
* ASSUMES DISABLE_GLOBAL_INT() was done before
*  RETURN VALUE:
*		uint8_t
* 		< 0 if error
*		seq number (>=0) if sync packet ok
*/
int8_t wdrf_poll_synch_packet_rx()
{
	uint8_t type, seq;
	uint8_t length;
	uint8_t pFooter[2];

	if (FIFOP_IS_1) {

		// Clean up and exit in case of FIFO overflow, indicated by FIFOP = 1 and FIFO = 0
		if((FIFOP_IS_1) && (!(FIFO_IS_1))) {
			wdrf_flush_rx_fifo();
			return -1; // FIFO overflow
		}

		// Payload length
		FASTSPI_READ_FIFO_BYTE(length);
		FASTSPI_READ_FIFO_BYTE(length);
		length &= RF_LENGTH_MASK; // Ignore MSB

		// Ignore the packet if the length is invalid
		if (length != SIZE_SYNC_PKT_PAYLOAD+SIZE_FCS) {
			wdrf_flush_rx_fifo();
			return -2; // invalid len
		}

		// read type
		FASTSPI_READ_FIFO_BYTE(type);	
		if (type == SYNC_PKT_TYPE) {

			// Read the packet sequence
			FASTSPI_READ_FIFO_BYTE(seq);

			// Read the footer to get the RSSI value
			FASTSPI_READ_FIFO_NO_WAIT((uint8_t*) pFooter, 2);
			rfSettings.pRxInfo->rssi = pFooter[0];

			// check atomatically generated crc
			if (pFooter[1] & RF_CRC_OK_BM) {
				return seq; // sync pkt ok; return sequence number (this should never be <=0)
			} else {
				wdrf_flush_rx_fifo();
				return -3; // crc failed
			}
		} else {
			// always read 1 byte before flush (data sheet pg 62)
			wdrf_flush_rx_fifo();
			return -4; // invalid sync packet type
		}
	}
	return 0;
}

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
	// Slots for example are at a slighly higher later since they assume TDMA
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

	txSeqNumber++; 

	//nrk_gpio_clr(DEBUG_1);

	return 0;
}

/**********************************************************
*  prepare to receive a data packet
*/
inline void wdrf_polling_rx_on(void)
{
	FASTSPI_STROBE(CC2420_SRXON);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
}

/**********************************************************
*  poll for received packet
*/
inline int8_t wdrf_polling_rx_packet()
{
	// it is safe to call rf_polling_rx_packet from basic_rf...
	return rf_polling_rx_packet();
}

////////////////////////////////////////////////////
//
// Functions only related to CC2420 radio module
//

/**********************************************************
* Initializes CC2420 for radio communication via the basic RF library functions.
* The arguments of this function are passed directly to rf_init(...) from basic_rf.
* Radio parameters are then adjusted for widom.
*/
inline void cc2420rf_init(RF_RX_INFO *pRRI, uint8_t channel) 
{
	rf_init(pRRI, channel, 0x1111, NODE_ADDR);

	// Disable interrupts
	DISABLE_GLOBAL_INT(); 

	// Set registers to some specific widom parameters

	FASTSPI_SETREG(CC2420_RSSI, 0xE580); // CCA THR=-27
	//FASTSPI_SETREG(CC2420_RSSI, 0xD880); // CCA THR=-40
	FASTSPI_SETREG(CC2420_TXCTRL, 0x80FF); // TX TURNAROUND = 128 us
	FASTSPI_SETREG(CC2420_RXCTRL1, 0x0A56); 

	wdrf_flush_rx_fifo();

	// Turn interrupts back on
	ENABLE_GLOBAL_INT();
}

/**********************************************************
* flush Rx FIFO
*/		 
inline void cc2420rf_flush_rx_fifo() 
{
	uint8_t tmp;
	// always read 1 byte before flush (data sheet pg 62)
	FASTSPI_READ_FIFO_BYTE(tmp);  
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
}

/**********************************************************
* flush Rx FIFO
*/		 
inline void cc2420rf_flush_tx_fifo() 
{
	FASTSPI_STROBE(CC2420_SFLUSHTX);
	FASTSPI_STROBE(CC2420_SFLUSHTX);
}

/**********************************************************
* set the radio into test mode, to send a modulated carrier
* call this before sending carrier pulses
*/		 
inline void cc2420rf_test_mode() 
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
	cc2420rf_flush_rx_fifo();
}

/**********************************************************
* set the radio into "normal" mode (buffered TXFIFO) and go into (data) receive
*/
inline void cc2420rf_data_mode() {
	FASTSPI_STROBE(CC2420_SRFOFF); //stop radio
	FASTSPI_SETREG(CC2420_MDMCTRL1, 0x0500); // default MDMCTRL1 value
	FASTSPI_SETREG(CC2420_DACTST, 0); // default value
	cc2420rf_flush_rx_fifo();
}

/**********************************************************
* start sending a carrier pulse
* assumes wdrf_radio_test_mode() was called before doing this
*/		
inline void cc2420rf_carrier_on() 
{
	FASTSPI_STROBE(CC2420_STXON); // tell radio to start sending	
}

/**********************************************************
* stop sending a carrier pulse; set the radio to idle state
*/
inline void cc2420rf_carrier_off() 
{
	FASTSPI_STROBE(CC2420_SRFOFF); // stop radio
}

/**********************************************************
* stop radio; set radio to idle state
*/
inline void cc2420rf_stp() 
{
	FASTSPI_STROBE(CC2420_SRFOFF); // stop radio
}

/**********************************************************
* set radio to receive, to detect carrier pulses
*/
inline void cc2420rf_set_rcv() 
{
	FASTSPI_STROBE(CC2420_SRXON);
}

/**********************************************************
* change channel
*
* ASSUMES NO PREEMPTION (DISABLE_GLOBAL_INT() was done before)
*/
inline void cc2420rf_set_channel( uint8_t channel )
{
	uint16_t f;

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
*/
inline void cc2420rf_set_power(uint8_t power) 
{
	uint8_t reg_val;
    
  if (power > 31) return;
    
//	reg_val = (CC2420_TXCTRL_DFT_VAL & (~CC2420_TXCTRL_PAPWR_MASK)) | (power << CC2420_TXCTRL_PAPWR);
  reg_val=0xA0E0;
  reg_val= reg_val | (power & CC2420_TXCTRL_PAPWR_MASK);  
    
	FASTSPI_SETREG(CC2420_TXCTRL, reg_val);
}

/*************************************************************************
* adjust cca threshold 
*/
inline void cc2420rf_set_cca_thr(int8_t cca_thr) 
{	
	uint16_t reg_val = (cca_thr<<CC2420_RSSI_CCA_THRESH) | 0x80;

	FASTSPI_SETREG(CC2420_RSSI, reg_val); 
}
