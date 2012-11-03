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




#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <nrk.h>
#include <nrk_events.h>
#include <basic_timesync_rf.h>
#include <basic_timesync_timer.h>

#define SIZE_FCS 2
#define SIZE_SYNC_PKT_PAYLOAD 9

#define SYNC_PKT_TYPE 0xAA
#define BCAST_ADDR 0xFFFF

#define CC2420_DEF_FCF 0x0821 // no ack

const uint16_t BYTE_TIME_clktks = ( BYTE_TIME_us / 1000000.0 ) / CLOCK_TICK_TIME + 1;
const uint16_t HALF_BYTE_TIME_clktks = ( BYTE_TIME_us / 2000000.0 ) / CLOCK_TICK_TIME + 1;

uint8_t sync_seq_cnt=1; // do not use 0 sequence number 

int16_t myAddr=0;

uint8_t i=0;

/**********************************************************
 * Initializes CC2420 for radio communication.
 */
inline void btrf_init(uint8_t channel, uint16_t panId, uint16_t mAddr) 
{
	uint8_t n;

	myAddr = mAddr;

	// Make sure that the voltage regulator is on, and that the reset pin is inactive
	SET_VREG_ACTIVE();
	halWait(1000);
	SET_RESET_ACTIVE();
	halWait(1);
	SET_RESET_INACTIVE();
	halWait(5);

	// Turn off all interrupts while we're accessing the CC2420 registers
	DISABLE_GLOBAL_INT();

	FASTSPI_STROBE(CC2420_SXOSCON);

    	// Register modifications
	FASTSPI_SETREG(CC2420_MDMCTRL0, 0x02E2);  // Std Preamble, no auto ack, no hw addr decoding; CCA mode = 3
	//FASTSPI_SETREG(CC2420_MDMCTRL0, 0x0AF2);  // Turn on automatic packet acknowledgment
						// Also could disable auto hardware address decoding
						// Also could disable auto CRC
	FASTSPI_SETREG(CC2420_MDMCTRL1, 0x0500); // Set the correlation threshold = 20
	FASTSPI_SETREG(CC2420_IOCFG0, 0x007F);   // Set the FIFOP threshold to maximum
	FASTSPI_SETREG(CC2420_SECCTRL0, 0x01C4); // Turn off "Security enable"
	FASTSPI_SETREG(CC2420_RXCTRL1, 0x1A56); // All default except
					    // reference bias current to RX
					    // bandpass filter is set to 3uA 

    	// Set the RF channel
	halRfSetChannel(channel);

	btrf_flush_rx_fifo();

	// Turn interrupts back on
	ENABLE_GLOBAL_INT();

	// Wait for the crystal oscillator to become stable
	halRfWaitForCrystalOscillator();

	// Write the short address and the PAN ID to the CC2420 RAM (requires that the XOSC is on and stable)
	DISABLE_GLOBAL_INT();
	FASTSPI_WRITE_RAM_LE(&myAddr, CC2420RAM_SHORTADDR, 2, n);
	FASTSPI_WRITE_RAM_LE(&panId, CC2420RAM_PANID, 2, n);
	ENABLE_GLOBAL_INT();
	
	// disable FIFOP interrupt, just in case
	DISABLE_FIFOP_INT();
}

/**********************************************************
 * flush Rx FIFO
 */		 
inline void btrf_flush_rx_fifo() 
{
	uint8_t tmp;
	// always read 1 byte before flush (data sheet pg 62)
	FASTSPI_READ_FIFO_BYTE(tmp);  
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
}

/**********************************************************
 * flush Tx FIFO
 */		 
inline void btrf_flush_tx_fifo() 
{
	FASTSPI_STROBE(CC2420_SFLUSHTX);
	FASTSPI_STROBE(CC2420_SFLUSHTX);
}

/**********************************************************
 * stop radio; set radio to idle state
 */
inline void btrf_stp() 
{
	FASTSPI_STROBE(CC2420_SRFOFF); // stop radio
}

/**********************************************************
 * set radio to receive
 */
inline void btrf_set_rcv() 
{
	FASTSPI_STROBE(CC2420_SRXON);
}

/**********************************************************
 * change channel
 */
inline void btrf_set_channel( uint8_t channel )
{
    halRfSetChannel(channel);
}

/**********************************************************
 *  BYTE tx_sync_packet(RF_TX_INFO *pRTI)
 *
 *  DESCRIPTION:
 *		Tx a synchronization packet	
 *
 *  ARGUMENTS:
 *      RF_TX_INFO *pRTI
 *          The transmission structure, which contains all relevant info 
 *		about the packet.
 *
 *  RETURN VALUE:
 *		uint8_t
 */
uint8_t tx_sync_packet(uint32_t offset, uint16_t id) 
{
	uint8_t packetLength, type=SYNC_PKT_TYPE;
	uint32_t timestamp;

	// Set transceiver to idle
	btrf_stp();

	// Flush the RX/TX FIFOs
	btrf_flush_rx_fifo();
	btrf_flush_tx_fifo();

	// Turn off global interrupts to avoid interference on the SPI interface
	DISABLE_GLOBAL_INT();

	//FASTSPI_STROBE (CC2420_STXONCCA);
	//btrf_flush_tx_fifo();

	FASTSPI_STROBE (CC2420_STXON);

	if (sync_seq_cnt == 0) sync_seq_cnt=1; // overflow; do not send sequence numbers <=0

	// Write the packet to the TX FIFO (assuming AUTOCRC is ENABLED)
	packetLength = SIZE_SYNC_PKT_PAYLOAD + SIZE_FCS; 
	FASTSPI_WRITE_FIFO((uint8_t*) &packetLength, 1);               	// packet length
	FASTSPI_WRITE_FIFO((uint8_t*) &packetLength, 1);               	// packet length
	FASTSPI_WRITE_FIFO((uint8_t*) &type, 1);    			// sync type
	FASTSPI_WRITE_FIFO((uint8_t*) &id, 2);    			// root id
	FASTSPI_WRITE_FIFO((uint8_t*) &sync_seq_cnt, 1);    		// sequence number

	// wait for the transmission to begin 
	while (!SFD_IS_1);

	// send timestamp	
	timestamp = ((uint32_t) timesync_timer_get()) + HALF_BYTE_TIME_clktks + offset;
	FASTSPI_WRITE_FIFO((uint8_t*) &timestamp, 4);    		// timestamp
	
	// wait for packet to finish
	while (SFD_IS_1); 
	FASTSPI_STROBE(CC2420_SRFOFF);  // shut off radio

	sync_seq_cnt++;

	//CLEAR_SFD_CAPTURE_INT();
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
 *		==0 if sync packet ok
 */
inline int8_t poll_synch_packet_rx(uint16_t *id, uint8_t *seq)
{
    uint32_t timestamp;
    uint8_t tmp;
    uint8_t type;
    uint8_t length;
    uint8_t pFooter[2];

    if (FIFOP_IS_1) {

	// Clean up and exit in case of FIFO overflow, indicated by FIFOP = 1 and FIFO = 0
	if((FIFOP_IS_1) && (!(FIFO_IS_1))) {
	    btrf_flush_rx_fifo();
	    return -1; // FIFO overflow
	}

	// Payload length
	FASTSPI_READ_FIFO_BYTE(length);
	FASTSPI_READ_FIFO_BYTE(length);
	length &= RF_LENGTH_MASK; // Ignore MSB

	// Ignore the packet if the length is invalid
	if (length != SIZE_SYNC_PKT_PAYLOAD+SIZE_FCS) {
	    	btrf_flush_rx_fifo();
		return -2; // invalid len
	}

	// read type
	FASTSPI_READ_FIFO_BYTE(type);	
	if (type == SYNC_PKT_TYPE) {

		// Read the packet source id
		FASTSPI_READ_FIFO_NO_WAIT((uint8_t*) id, 2);

		// Read the packet sequence
		FASTSPI_READ_FIFO_BYTE((*seq));

		// Read the packet timestamp
		FASTSPI_READ_FIFO_NO_WAIT((uint8_t*) &timestamp, 4);
		//printf("%lu\r\n", timestamp);
		timesync_timer_last_pkt_timestamp_set(timestamp);

		// Read the footer to get the RSSI value
		FASTSPI_READ_FIFO_NO_WAIT((uint8_t*) pFooter, 2);
		rfSettings.pRxInfo->rssi = pFooter[0];

	    	btrf_flush_rx_fifo();

		// check atomatically generated crc
		if (pFooter[1] & RF_CRC_OK_BM) {
			return 0; // sync pkt ok; 0 indicates success
		} else {
			return -3; // crc failed
		}

	} else {
	    	btrf_flush_rx_fifo();
		return -4; // invalid sync packet type
	}
    }

    //nrk_event_signal ( SIG(SYNC_PKT_RX_EVENT) ); // signal packet transmission

    return -5;
}

inline int8_t wait_rx_sync_pkt_until(uint32_t until_clkTks, uint16_t *id, uint8_t *seq)
{
	int8_t n;
	uint32_t cnt=0, timestamp;

	btrf_stp();
	btrf_set_channel(TIMESYNC_CHANNEL);
	btrf_set_rcv();

	do {
		cnt++;
		n = SFD_IS_1;
		timestamp=timesync_timer_get();
		//printf("%ld < %ld\r\n", timestamp,until_clkTks);
	} while (n == 0 && timestamp < until_clkTks);

	if (i == 0) { 
		//nrk_gpio_set(DEBUG_0);
		i=1;
	} else {
		//nrk_gpio_clr(DEBUG_0);
		i=0;
	}

	if (n == 1 && cnt>1) {
      		timesync_timer_last_rx_timestamp_set(timestamp);
        	n = 0;
        	// Packet on its way
    		cnt=0;
        	while ((n = poll_synch_packet_rx (id, seq)) != 0) {
			if (cnt > MAX_PKT_SIZE) {
				btrf_flush_rx_fifo();
				break;          // timeout as failsafe
			}
			halWait(BYTE_TIME_us);
			cnt++;
		}
	} else n=1;
	
	return n;
}

/**********************************************************
 * Timer input capture signal 
 *
 */	
/*
SIGNAL(SIG_INPUT_CAPTURE1) // ISR
{

	if (i == 0) { 
		i=1;
		nrk_gpio_set(DEBUG_0);
		nrk_set_led(1);
	} else {
		i=0;
		nrk_gpio_clr(DEBUG_0);
		nrk_clr_led(1);
	}

	TIFR|=BM(TOV1);
        CLEAR_SFD_CAPTURE_INT();

	FASTSPI_STROBE(CC2420_SFLUSHRX);
	FASTSPI_STROBE(CC2420_SFLUSHRX);
	FASTSPI_STROBE(CC2420_SRXON);

      // set rx timestamp
      //timesync_timer_last_rx_timestamp_set(timesync_timer_get());
}
*/
