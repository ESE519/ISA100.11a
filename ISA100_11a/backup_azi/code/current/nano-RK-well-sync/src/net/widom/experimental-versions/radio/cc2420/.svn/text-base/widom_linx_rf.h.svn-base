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

////////////////////////////////////////////////////
//
// Functions related to linxrf add-on board 
//
#include "widom.h"

#ifndef _WIDOM_LINX_RF_H
#define _WIDOM_LINX_RF_H

#ifdef WD_USES_RADIO_RXLINX

// pins 
#define TX_PDN  NRK_ADC_INPUT_6
#define TX_DATA NRK_ADC_INPUT_5
#define RX_PDN  NRK_ADC_INPUT_4
#define RX_DATA NRK_ADC_INPUT_3
#define RX_RSSI ADC_INPUT_2

#define SDI NRK_ADC_INPUT_7
#define CLK NRK_ADC_INPUT_6
#define SDO NRK_GPIO28
#define CSN NRK_ADC_INPUT_1

// ports
#define RFLINX_PORT PORTF
#define RFLINX_DDR  DDRF
#define RFLINX_PIN  PINF

// digital potentiometer chanels
#define TX_POWER_CHANNEL 0      // tx power control  
#define THR1_LEVEL_CHANNEL 1   // op-amp 1 control 
#define THR2_LEVEL_CHANNEL 2   // op-amp 2 control 
  
// values for tx power scale control 
#define TX_POWER_MAX 0xFF
#define TX_POWER_MIN 0

// values for rssi thresholds
#define RSSI_THR1 110
#define RSSI_THR2 130

#include <include.h>
#include <nrk.h>

/**********************************************************
 * initializes radio(s) communication 
 */
void linxrf_init();

/**********************************************************
 * Get RSSI reading 
 */
//inline uint16_t linxrf_rssi();

/**********************************************************
 * return true if detects that rssi signal 
 * is above a configured rssi threshold (used to detect if
 * rssi signail is rising )   
 */
inline uint8_t linxrf_rssi_thr1();

/**********************************************************
 * return true if detects that rssi signal 
 * is above a configured rssi threshold (used to detect if
 * rssi signal is falling)   
 */
inline uint8_t linxrf_rssi_thr2();

/**********************************************************
 * return state of the radio
 */
uint8_t linxrf_started();

/**********************************************************
 * activate receive module 
 */
void linxrf_rx_on();

/**********************************************************
 * deactivate receive module 
 */
void linxrf_rx_off();

/**********************************************************
 * activate transmit module 
 */
void linxrf_tx_on();

/**********************************************************
 * deactivate transmit module 
 */
void linxrf_tx_off();

/**********************************************************
 * read receive port 
 */
uint8_t linxrf_rx_read();

/**********************************************************
 * write to transmit port 
 */
void linxrf_tx_write(uint8_t val);

/**********************************************************
 * configure potentiometer chanels 
 */
void linxrf_set_pot(uint8_t channel, uint8_t scale);

/**********************************************************
 * set tx power 
 */
void linxrf_set_tx_power(uint8_t scale);

#endif // ifdef WD_USES_RADIO_RXLINX

#endif
