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
/*   Author: Nuno Pereira                                                                         */
/* ========================================================================== */

#ifndef _TIMESYNC_TIMER_H
#define _TIMESYNC_TIMER_H

// for setup of Timer/Counter1
#define TCLK_CPU_DIV1 1	
#define TCLK_CPU_DIV8 2

#define TCLK_CPU_DIV TCLK_CPU_DIV8

#if TCLK_CPU_DIV == TCLK_CPU_DIV1
// Clock resolution in seconds (125 ns)
#define CLOCK_TICK_TIME	0.000000125
// macro to convert us to clock cycles
#define US2CYCLES( _time_us_ )  (_time_us_ * 8)
#endif

#if TCLK_CPU_DIV == TCLK_CPU_DIV8
// Clock resolution in seconds (1 us)
#define CLOCK_TICK_TIME	0.000001
// macro to convert us to clock cycles
#define US2CYCLES( _time_us_ )  (_time_us_)
#endif

////////////////////////////////////////////////////
//
// functions to setup the timer 
// it timer is run by atmega 128 Timer/Counter1
//

/**********************************************************
 * start high speed timer (atmega 128 Timer/Counter1) at the cpu clock frequency
 * (this function could just call _nrk_start_high_speed_timer())
 */	
void timesync_timer_start();

/**********************************************************
 * clear timer (atmega 128 Timer/Counter1) overflow interrupt
*/	
void timesync_timer_clr_interrupt();

/**********************************************************
 * get timer value  
 */	
uint32_t timesync_timer_get(); 

/**********************************************************
 * set timer value  
 */	
void timesync_timer_set(uint32_t time);

/**********************************************************
 * set timer value; timestamp in last received packet payload 
 */	
void timesync_timer_last_pkt_timestamp_set(uint32_t time);

/**********************************************************
 * set timer value; our current time when last received packet started to be received
 */	
void timesync_timer_last_rx_timestamp_set(uint32_t time);

/**********************************************************
 * get timer value; ; timestamp in last received packet payload 
 */	
uint32_t timesync_timer_last_pkt_timestamp_get();

/**********************************************************
 * get timer value; our current time when last received packet started to be received
 */	
uint32_t timesync_timer_last_rx_timestamp_get();

#endif
