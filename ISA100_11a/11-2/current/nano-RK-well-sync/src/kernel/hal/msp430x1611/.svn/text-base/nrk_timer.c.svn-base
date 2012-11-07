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
* Anthony Rowe
*******************************************************************************/

#include <include.h>
//#include <avr/interrupt.h>
#include <signal.h>
#include <ulib.h>
#include <nrk_timer.h>
#include <nrk_error.h>


//TODO: fill in all of these for msp!!!
void nrk_spin_wait_us(uint16_t timeout)
{
    // This sequence uses exactly 8 clock cycle for each round
}


void _nrk_setup_timer()
{
	_nrk_prev_timer_val = 254;
	BCSCTL1 |= (DIVA1 | DIVA0); // ACLK = LFXT1CLK / 8
	TACTL |= (TASSEL0); // Timer_A clock source = ACLK
	TACTL |= (ID1); // Input clock = ACLK / 4
	//TACTL |= (MC0); // Up mode - timer counts to TACCR0
	//TACTL |= (TAIE); // Timer_A overflow interrupt enable FIXME: add handler

	TACCR0 = _nrk_prev_timer_val;
	TACCTL0 |= (CCIE); // Timer_A CCR0 compare interrupt enable
	
	TACTL &= (~TAIFG); // Clear overflow interrupt flag
	TACCTL0 &= (~CCIFG); // Clear compare interrupt flag
	TACTL |= (TACLR); // Reset TAR, clock divider, and count direction FIXME: do this?

	_nrk_reset_os_timer();
	_nrk_start_os_timer();
	_nrk_time_trigger = 0;
	//eint(); //enable interrupts FIXME: should probably be doing this somewhere else
}

void _nrk_high_speed_timer_stop()
{
	// TODO: implement
}

void _nrk_high_speed_timer_start()
{
	// TODO: implement
}


void _nrk_high_speed_timer_reset()
{
	// TODO: implement
}

/**
  This function blocks for n ticks of the high speed timer after the
  start number of ticks.  It will handle the overflow that can occur.
  Do not use this for delays longer than 8ms!
*/
void nrk_high_speed_timer_wait( uint16_t start, uint16_t ticks )
{
	// TODO: implement
}

inline uint16_t _nrk_high_speed_timer_get()
{
	// TODO: implement
	return 0;
}

void _nrk_stop_os_timer()
{
	TACTL &= (~(MC1 | MC0)); // Timer_A stop mode
}
                                 //must also include timer3 
void _nrk_start_os_timer()
{
	TAR = 0; // reset counter
	TACTL |= (MC0); // Up mode - timer counts to TACCR0
}

inline void _nrk_reset_os_timer()
{
	TAR = 0;
	_nrk_time_trigger = 0;
	_nrk_prev_timer_val = 0;
}


uint8_t _nrk_get_next_wakeup()
{
	return TACCR0+1;
}

void _nrk_set_next_wakeup(uint8_t nw)
{
	TACCR0 = nw - 1;
}



inline uint8_t _nrk_os_timer_get()
{
	return TACCR0;
}

interrupt(TIMERA0_VECTOR) timer (void) {
/*
	if(P6OUT & 0x40)
		P6OUT &= 0xbf;
	else
		P6OUT |= 0x40;
*/
	//TACTL &= ~TAIFG;
	//nrk_led_toggle(BLUE_LED);
	//TAR = 0;
	_nrk_timer_suspend_task();
}

//TODO: fix isrs for msp
/*
//-------------------------------------------------------------------------------------------------------
//  Default ISR 
//-------------------------------------------------------------------------------------------------------
interrupt(__vector_default) {
}                                  

#ifdef NRK_MAX_DRIVER_CNT
interrupt(SIG_OUTPUT_COMPARE3A)
{
}
#endif  
interrupt(TIMER2_OVF_vect) {
} 

interrupt(TIMER2_COMPA_vect) {
} 


//-------------------------------------------------------------------------------------------------------
//  TIMER 1 COMPARE ISR
//-------------------------------------------------------------------------------------------------------
interrupt(SIG_OUTPUT_COMPARE1A) {
} 
*/
