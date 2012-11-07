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
/*   basic_timesync_timer.h                                                   */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* ========================================================================== */

#include <include.h>
#include <nrk.h>
#include <basic_timesync_timer.h>

uint16_t timer_high=0;

uint32_t pkt_timestamp, rx_timestamp;

/**********************************************************
 * start high speed timer (atmega 128 Timer/Counter1) 
 * enable overflow interrupt 
 */	
void timesync_timer_start() 
{
	nrk_int_disable();
	timer_high=0;
	TCCR1A = 0; // stop timer
	TCNT1 = 0; // reset timer value
	TIFR&=~BM(OCF1A); // clear OCF flag
	TIMSK|=BM(TOIE1); // enable overflow interrupt
	TCCR1B = TCLK_CPU_DIV; // set timer scale
	nrk_int_enable();
}

/**********************************************************
 * clear timer (atmega 128 Timer/Counter1) overflow interrupt
*/	
void timesync_timer_clr_interrupt( )
{
  nrk_int_disable();
	TCNT1 = 0; // reset timer value
	TIFR&=~BM(OCF1A); // clear OCF flag
	TIMSK&=~BM(TOIE1); // disable compare interrupt
	nrk_int_enable();
}

/**********************************************************
 * get timer value  
 */	
uint32_t timesync_timer_get() 
{
  uint32_t time;
  nrk_int_disable();
  time = ((uint32_t)((uint32_t)timer_high) << 16) + ((uint32_t)(TCNT1));
  nrk_int_enable();
  return time;
}

/**********************************************************
 * set timer value  
 */	
void timesync_timer_set(uint32_t time) 
{
  nrk_int_disable();
  timer_high = (uint16_t)((uint32_t)((uint32_t)(((uint32_t)time) & 0xFFFF0000)) >> 16);
  TCNT1 = (uint16_t) time; 
  nrk_int_enable();
  return time;
}

/**********************************************************
 * set timer value; timestamp in last received packet payload 
 */	
void timesync_timer_last_pkt_timestamp_set(uint32_t time) 
{
  nrk_int_disable();
  pkt_timestamp = time;
  nrk_int_enable();
}

/**********************************************************
 * set timer value; our current time when last received packet started to be received
 */	
void timesync_timer_last_rx_timestamp_set(uint32_t time) 
{
  nrk_int_disable();
  rx_timestamp = time;
  nrk_int_enable();
}

/**********************************************************
 * get timer value; ; timestamp in last received packet payload 
 */	
uint32_t timesync_timer_last_pkt_timestamp_get() 
{
  uint32_t time;
  nrk_int_disable();
  time=pkt_timestamp;
  nrk_int_enable();
  return time;
}

/**********************************************************
 * get timer value; our current time when last received packet started to be received
 */	
uint32_t timesync_timer_last_rx_timestamp_get() 
{
  uint32_t time;
  nrk_int_disable();
  time=rx_timestamp;
  nrk_int_enable();
  return time;
}

/**********************************************************
 * Timer (atmega 128 Timer/Counter1) overflow signal 
 */	
SIGNAL(SIG_OVERFLOW1) // ISR 
{
/*
	if (i == 0) { 
		i=1;
		nrk_gpio_set(DEBUG_0);
	} else {
		i=0;
		nrk_gpio_clr(DEBUG_0);
	}
*/
  timer_high++;	
}

