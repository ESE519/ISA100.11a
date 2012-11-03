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
*  Anthony Rowe
*******************************************************************************/

#include <include.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <ulib.h>
#include <nrk_timer.h>
#include<nrk_driver.h>
#include<nrk_error.h>
#include <nrk_cfg.h>

void nrk_spin_wait_us(uint16_t timeout)
{

    // This sequence uses exactly 8 clock cycle for each round
    do {
        NOP();
        NOP();
        NOP();
        NOP();
    } while (--timeout);

}


void _nrk_setup_timer() {
  //printf( "Setting timer\n" );
  /*ASSR = BM(AS0);
  _nrk_prev_timer_val=254;
  OCR0 = _nrk_prev_timer_val;
  TIFR =   BM(OCF0) | BM(TOV0);       // Clear interrupt flag
  TIMSK =  BM(OCIE0) | BM(TOIE0)|0x02 ;//| BM(TICIE1);    // Enable interrupt
  //TCCR0 = BM(WGM01) | BM(CS02) | BM(CS00); //|      // reset counter on interrupt, set divider to 128
  TCCR0 = BM(WGM01) | BM(CS01) | BM(CS00); //|      // reset counter on interrupt, set divider to 128
  */
    TIFR = BM (OCF1A) | BM (ICF1);      // Clear interrupt flag
    TIMSK = BM (OCIE1A); // | BM(OCIE0);        //| BM(TICIE1);    // Enable interrupt
    TCCR1A =  TCCR1B = BM (WGM12) | BM (CS12); // | BM(CS10);       	
    SFIOR |= PSR10;              // reset prescaler
    TCNT1 = 0;                  // reset counter
   _nrk_time_trigger=0;
/*
#ifdef NRK_DRIVER_FREQ
TCCR0=BM(WGM01) | BM(CS01);  //0x0C;
TCNT0=0x00;
OCR0=NRK_DRIVER_FREQ;
#endif
*/
}

void _nrk_stop_os_timer()
{
  TIMSK &=  ~BM(OCIE1A) ;

}

void _nrk_start_os_timer()
{

  TIMSK |=  BM(OCIE1A) ;

}

uint8_t _nrk_get_next_wakeup()
{
	return (OCR1A/33)+1;  // modify scale to match 32768 clock
}

void _nrk_set_next_wakeup(uint8_t nw)
{
   OCR1A = (nw*33)-1;
}

void _nrk_reset_os_timer()
{
    SFIOR |= PSR2;              // reset prescaler
    SFIOR |= PSR10;              // reset prescaler
    TCNT1 = 0;                  // reset counter
    _nrk_time_trigger=0;
    _nrk_prev_timer_val=0;
}

uint8_t _nrk_get_os_timer()
{
  return (((uint16_t)TCNT1)/(uint16_t)33);
}

void _nrk_stop_high_speed_timer()
{
}

void _nrk_start_high_speed_timer()
{
}


void _nrk_reset_high_speed_timer()
{
}

uint16_t _nrk_get_high_speed_timer()
{
  return 0;
}

//-------------------------------------------------------------------------------------------------------
//  Default ISR 
//-------------------------------------------------------------------------------------------------------
SIGNAL(__vector_default) {
	//printf("Seg Fault");
	nrk_kernel_error_add(NRK_SEG_FAULT,0);
	while(1);
}

#ifdef NRK_MAX_DRIVER_CNT
SIGNAL(SIG_OUTPUT_COMPARE0)
{
	nrk_high_freq_driver_scheduler();
return;

}
#endif


SIGNAL(SIG_OVERFLOW0) {

nrk_kernel_error_add(NRK_TIMER_OVERFLOW,0);
//	printf( "\r\n*Timer overflow!!!\r\n" );
//	SET_LED_0();
//	SET_LED_1();
//	SET_LED_2();
	while(1);
	return;  	
} 

SIGNAL(SIG_OUTPUT_COMPARE1A) {
	//static uint8_t blink;
	//_nrk_time_trigger++;
	//printf( "Timer triggered!\n" );
	//while(1);
/*	if(i==0 ) {
	   ff_set_led(0);
	   i=1;
	}else
	{
	   ff_clr_led(0);
	   i=0;
	}
*/
 	//if(blink) {blink=0; nrk_set_led(1); } else {blink=1; nrk_clr_led(1); }
	_nrk_timer_suspend_task();	
	return;  	
} 

/*
//-------------------------------------------------------------------------------------------------------
//  TIMER 1 COMPARE ISR
//-------------------------------------------------------------------------------------------------------
SIGNAL(SIG_OUTPUT_COMPARE1A) {
	static uint8_t i=0;

	CLR_LED_1();
	if(i==0 ) {

	   SET_LED_0();
		i=1;
	}else
	{
	   CLR_LED_0();
	   i=0;

	}

	//SuspendTask2();	
	return;  	
} 
*/
