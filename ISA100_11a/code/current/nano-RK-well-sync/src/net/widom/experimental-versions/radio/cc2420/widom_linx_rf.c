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
// Functions related to linx rf add-on board 
//
#include <widom.h>
#include "widom_linx_rf.h"

#ifdef WD_USES_RADIO_RXLINX

uint8_t linxrf_state;


#define BV( _var, _bit_index ) (!(!(_var & BM(_bit_index))))
#define CB(_var,_bit_index) ((_var) &= (~BM(_bit_index)))
#define SB(_var,_bit_index) ((_var) |= (~BM(_bit_index)))

/**********************************************************
 * Initializes radio(s) communication 
 */
void linxrf_init()
{
  printf("11");
  
  // init pins
  
  nrk_gpio_direction (RX_DATA, NRK_PIN_INPUT);
  //nrk_gpio_direction (RX_RSSI, NRK_PIN_INPUT);
  
  nrk_gpio_direction (TX_PDN, NRK_PIN_OUTPUT);
  nrk_gpio_direction (TX_DATA, NRK_PIN_OUTPUT);
  nrk_gpio_direction (RX_PDN, NRK_PIN_OUTPUT);

  SFIOR |= BM(PUD);

/*
  // init adc
  ADC_INIT ();
  ADC_ENABLE ();
  ADC_SET_CHANNEL (RX_RSSI);
  ADC_SAMPLE_SINGLE();
*/
  // setup potentiometer pins
  nrk_gpio_direction (SDI, NRK_PIN_OUTPUT);
  nrk_gpio_direction (CLK, NRK_PIN_OUTPUT);
  nrk_gpio_direction (CSN, NRK_PIN_OUTPUT);

  // setup rssi digital pins
  nrk_gpio_raw_direction ( DDRB, 6,  NRK_PIN_INPUT );
  nrk_gpio_raw_direction ( DDRB, 7,  NRK_PIN_INPUT );

  // setup antenna switch pin
  nrk_gpio_direction (NRK_GPIO34, NRK_PIN_OUTPUT);
  
  // turn rx module on
  nrk_gpio_set(RX_PDN);
   
  // turn tx module on
  nrk_gpio_set(TX_PDN);

  // set tx power
  linxrf_set_tx_power(TX_POWER_MAX);

  // turn antenna switch
  nrk_gpio_clr(NRK_GPIO34); // clear, means antenna on

  // set potentiometer channel for op-amp compare value; rssi threshold 1    
  linxrf_set_pot(THR1_LEVEL_CHANNEL, RSSI_THR1);

  // set potentiometer channel for op-amp compare value; rssi threshold 2    
  linxrf_set_pot(THR2_LEVEL_CHANNEL, RSSI_THR2);
  
  linxrf_state=1;
}

/**********************************************************
 * get RSSI reading 
 */
 /*
inline uint16_t linxrf_rssi()
{
  uint16_t adc_val;
  ADC_GET_SAMPLE_10(adc_val);
  return adc_val;
}
*/
/**********************************************************
 * return true if detects that rssi signal 
 * is above a configured rssi threshold 
 */
inline uint8_t linxrf_rssi_thr1() { 
  return !(!nrk_gpio_raw_get(PINB, 7)); 
}

/**********************************************************
 * return true if detects that rssi signal 
 * is above a configured rssi threshold 
 */
inline uint8_t linxrf_rssi_thr2() {
  return !(!nrk_gpio_raw_get(PINB, 7)); 
}
  

/**********************************************************
 * return state of the radio
 */
uint8_t linxrf_started() { return linxrf_state; }

/**********************************************************
 * activate receive module 
 */
void linxrf_rx_on() { nrk_gpio_set(RX_PDN); }

/**********************************************************
 * deactivate receive module 
 */
void linxrf_rx_off() { nrk_gpio_clr(RX_PDN); }

/**********************************************************
 * activate transmit module 
 */
void linxrf_tx_on() { nrk_gpio_set(TX_PDN); }

/**********************************************************
 * deactivate transmit module 
 */
void linxrf_tx_off() { nrk_gpio_clr(TX_PDN);}

/**********************************************************
 * read receive port 
 */
uint8_t linxrf_rx_read() { return nrk_gpio_get(RX_DATA); }

/**********************************************************
 * write to transmit port 
 */
void linxrf_tx_write(uint8_t val)
{
	if(val==1) nrk_gpio_set(TX_DATA);
	else nrk_gpio_clr(TX_DATA); 
}

/**********************************************************
 * configure potentiometer chanels 
 */
void linxrf_set_pot(uint8_t channel, uint8_t scale)
{
  int8_t i;
  uint16_t pot_conf = ((channel) << 8) + scale;

  nrk_gpio_clr(CSN); 
  nrk_gpio_clr(CLK);
  for (i=10; i>=0; i--) {
	 halWait(50);
	 if ( BV(pot_conf, i) == 0) nrk_gpio_clr(SDI);
	 else nrk_gpio_set(SDI);
	 halWait(50);
   nrk_gpio_set(CLK); 
	 halWait(100);
	 nrk_gpio_clr(CLK);
  }
  nrk_gpio_set(CSN); 
  nrk_gpio_set(CLK); 
}

/**********************************************************
 * set tx power 
 */
void linxrf_set_tx_power(uint8_t scale)
{
	linxrf_set_pot(TX_POWER_CHANNEL, scale);
}

#endif // ifdef WD_USES_RADIO_RXLINX
