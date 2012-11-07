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
/*   widom_timer.h                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#ifndef _WIDOM_TIMER_H
#define _WIDOM_TIMER_H

#define WD_TCLK_CPU_DIV TIMER0_PRESCALE_8

#if WD_TCLK_CPU_DIV == TIMER0_PRESCALE_1
// Clock resolution in seconds (125 ns)
#define WD_CLOCK_TICK_TIME	0.000000125
#endif

#if WD_TCLK_CPU_DIV == TIMER0_PRESCALE_8
// Clock resolution in seconds (1 us)
#define WD_CLOCK_TICK_TIME	0.000001
#endif

/***************************************************
 * macro to get current time in clock ticks (x in protocol state machine)
 * assume that _nrk_get_high_speed_timer CAN ONLY OVERFLOW ONCE 
 * between calls to wd_reset_time()
 */
#define x TCNT1

////////////////////////////////////////////////////
//
// functions to setup the high speed timer (atmega 128 Timer/Counter1)
//

/**********************************************************
 * start high speed timer (atmega 128 Timer/Counter1) at the cpu clock frequency
 * (this function could just call _nrk_start_high_speed_timer())
 */	
void wd_start_high_speed_timer();

/**********************************************************
 * set timer (atmega 128 Timer/Counter1) compare interrupt to 
 * fire after time_interval_clktks time 
 * time interval given is in clock ticks and MUST be smaller than 8 ms 
 * (for WD_TCLK_CPU_DIV = TIMER0_PRESCALE_1)
 * must call wd_start_high_speed_timer() before to init timer
 */	
void wd_set_high_speed_timer_interrupt(uint16_t since_clktks, uint16_t time_interval_clktks );

/**********************************************************
 * clear timer (atmega 128 Timer/Counter1) compare interrupt
 *
 * ASSUMES DISABLE_GLOBAL_INT() was done before
*/	
void wd_clr_high_speed_timer_interrupt();

/**********************************************************
 * reset timer (atmega 128 Timer/Counter1) 
 *
 * ASSUMES DISABLE_GLOBAL_INT() was done before
*/	
inline void wd_reset_time();

#endif
