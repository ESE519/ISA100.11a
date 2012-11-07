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
/*   widom_timer.c                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#include <include.h>
#include <nrk.h>
#include <widom_timer.h>
#include <widom.h>

/**********************************************************
* start high speed timer (atmega 128 Timer/Counter1) at the cpu clock frequency
* (this function could just call _nrk_start_high_speed_timer())
*/	
void wd_start_high_speed_timer() 
{
	// Timer1
	TCCR1A = 0; // stop timer
	TCNT1 = (uint16_t)0; // reset timer value
	OCR1A = 0; // compare value (interval)
	TCCR1B = WD_TCLK_CPU_DIV; // set timer scale;
}

/**********************************************************
* set timer (atmega 128 Timer/Counter1) compare interrupt to 
* fire after time_interval_clktks time counting from since_clktks
* time interval given is in clock ticks and MUST be smaller than 8 ms
* must call wd_start_high_speed_timer() before to init timer
* 
* ASSUMES DISABLE_GLOBAL_INT() was done before
*/	
void wd_set_high_speed_timer_interrupt(uint16_t since_clktks, uint16_t time_interval_clktks )
{
	TIFR&=~BM(OCF1A); // clear OCF flag
	OCR1A = (uint16_t)(since_clktks+time_interval_clktks); // compare value (interval)
	TIMSK|=BM(OCIE1A); // enable compare interrupt
}

/**********************************************************
* clear timer (atmega 128 Timer/Counter1) compare interrupt
*
* ASSUMES DISABLE_GLOBAL_INT() was done before
*/	
void wd_clr_high_speed_timer_interrupt( )
{
	TCNT1 = 0; // reset timer value
	OCR1AL = 0; // compare value (interval)
	TIFR&=~BM(OCF1A); // clear OCF flag
	TIMSK&=~BM(OCIE1A); // disable compare interrupt
}

/**********************************************************
* reset timer (atmega 128 Timer/Counter1) 
*
* ASSUMES DISABLE_GLOBAL_INT() was done before
*/	
inline void wd_reset_time()
{
	TCNT1 = (uint16_t)0; // reset timer value
}

/**********************************************************
* Timer (atmega 128 Timer/Counter1) compare signal 
*/	
SIGNAL(SIG_OUTPUT_COMPARE1A) // ISR 
{
	wd_int_timer_handler();
}
