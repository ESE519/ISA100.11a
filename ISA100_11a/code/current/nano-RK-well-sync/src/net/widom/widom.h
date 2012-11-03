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
/*   widom.h                                                                  */
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
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#ifndef _WIDOM_H
#define _WIDOM_H

#include <wd_rf.h>

////////////////////////////////////////////////////
//
// Protocol parameters
//

// This flag is for testing purposes only; If activated the node will try
// to access the medium continuously, acting like it is always backlogged. 
// 
// Application must call wd_tx_packet_enqueue to enquen the message once.
// Flag NRK_WATCHDOG in nrk_cfg.h must be disabled.
//#define CONTINUOUS_SEND

#define DOMINANT_BIT 0
#define RECESSIVE_BIT 1

// num of priorities
#define NPRIOBITS		3
#define MAX_NUM_PRIO_BITS	16
#define INVALID_PRIO_INDEX	MAX_NUM_PRIO_BITS
#define INVALID_PRIO 		0xFFFF

// timeouts interval constants (in us)

// timeout parameters (using cc2420 to transmit priority bits)
// these timeouts should be able to cope with interrupt disabled periods of ~100us
// timeouts constants (in us)
#define E_us 		 190 	   // timeout to improve reliability 
#define F_us 		 5301	   // initial idle period
#define H_us 		 961 	   // duration of a pulse of a carrier
#define G_us 		 440 	   // "guarding" time interval 
#define ETG_us 	 500		 // end Tournament GAP
#define TFCS_us  256		 // time to detect a carrier (value from experiments)
#define SWX_us 	 192		 // RX/TX switch time  

#define CMSG_us 		(WD_MAX_MSG_LEN_us+100) // max time to tx a message + some extra time 

////////////////////////////////////////////////////
//
// functions used by aplications/upper layers 
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
* This can be used to put the protocol and radio to idle and
* save energy.
* The node will perform no radio activity while idle.  
*
* This can only be called after the protocol is initialized.
*/
int8_t wd_set_idle();

/**********************************************************
* Will set the protocol active again, after a previous
* call to  wd_set_idle()  
* 
* This can only be called after the protocol is initialized.
*/
int8_t wd_set_active();

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
int8_t wd_wait_until_tx_packet();

/**********************************************************
* wait until end of next packet reception
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
int8_t wd_set_cca_thresh(int8_t cca_thresh);

////////////////////////////////////////////////////
//
// functions used by the protocol 
//

/**********************************************************
* this function performs the protocol (WiDom)
*
* ASSUMES: running uninterrupted to completion (it runs inside an ISR)    
*
*  RETURN VALUE:
*  		a boolean indicating the result of the tournament
*/	
inline void wd_do_protocol(uint8_t messageType);


////////////////////////////////////////////////////
//
// MAC protocol states, according to Fig. 1 from:  
// N. Pereira, B. Andersson and E. Tovar, "Implementation of a Dominance Protocol for Wireless Medium Access".
// 
enum MAC_STATES {
	MAC_STATE_0,			
	MAC_STATE_1,		
	MAC_STATE_2,
	MAC_STATE_3,
	MAC_STATE_4,
	MAC_STATE_5,
	MAC_STATE_6,
	MAC_STATE_7,
	MAC_STATE_8,
	MAC_STATE_9,
	MAC_STATE_10,
	MAC_STATE_11
};

////////////////////////////////////////////////////
//
// Message Types
//
enum MSG_TYPES {
	MSG_TYPE_DATA_MSG,						// (0)
	
// MAC messages
	MSG_TYPE_MAC_QUEUE,						// (1) A new message was queued
	MSG_TYPE_MAC_WAIT_TIME_E,				// (2) MAC ended waiting for E
	MSG_TYPE_MAC_WAIT_TIME_F,				// (3) MAC ended waiting initial idle period (F)
	MSG_TYPE_MAC_WAIT_TIME_G,				// (4) MAC ended waiting for guard time (G)
	MSG_TYPE_MAC_WAIT_TIME_H,				// (5) MAC ended waiting carrier sense (H)
	MSG_TYPE_MAC_WAIT_TIME_R,				// (6) MAC ended waiting period for increase of synchronization reliability (R)
	MSG_TYPE_MAC_WAIT_TIME_SWX,				// (7) MAC ended waiting for SWX
	MSG_TYPE_MAC_WAIT_TIME_TFCS,			// (8) MAC ended waiting for initial carrier sensing time
	MSG_TYPE_MAC_WAIT_TIME_ETG,				// (9) End of tournament gap
	MSG_TYPE_MAC_WAIT_TIME_ENDTXRX,		// (10) Timeout for tx/rx a message
	MSG_TYPE_MAC_STATE_ENTER,				// (11) used when the MAC protocol enters a state, after a state transition with _toState()
	
// Radio messages
	MSG_TYPE_RADIO_CHANNEL_IDLE,			// (12) Channel is idle 
	MSG_TYPE_RADIO_CHANNEL_BUSY,			// (13) Channel is busy (a carrier or a transmission detected)
	MSG_TYPE_RADIO_END_RX,					// (14) end receiving a * data message * 
	MSG_TYPE_RADIO_END_TX,					// (15) end sending a * data message *
	
	MSG_TYPE_MAC_DO_ASSERT
};

////////////////////////////////////////////////////
//
// Some utility macros 
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

/***************************************************
* macro to transition imediatelly to a state
*/
#define toState( _next_state ) \
{ \
		state = MAC_STATE_##_next_state; \
		wd_do_protocol(MSG_TYPE_MAC_STATE_ENTER); \
}

#endif
