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
#include <avr/interrupt.h>
#include <ulib.h>
#include <nrk_timer.h>
#include <nrk_error.h>
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
  _nrk_prev_timer_val=254;
 
// Timer 0 Setup as Asynchronous timer running from 32Khz Clock
  ASSR = BM(AS2);
  OCR2A = _nrk_prev_timer_val;
  TIFR2 =   BM(OCF2A) | BM(TOV2);       // Clear interrupt flag
  TCCR2A = BM(WGM21);
  TCCR2B = BM(CS21) | BM(CS20); //|      // reset counter on interrupt, set divider to 128
  GTCCR |= BM(PSRASY);              // reset prescaler
  // Clear interrupt flag
  TIFR2 =   BM(OCF2A) | BM(TOV2);       
  // reset counter on interrupt, set divider to 128
  TCCR0A = BM(WGM01) | BM(CS01) | BM(CS00); 
  // reset prescaler
  //GTCCR |= TSM;              
  GTCCR |= BM(PSRASY);              // reset prescaler

// Timer 1 High Precision Timer
// No interrupt, prescaler 1, Normal Operation
  TCCR1A=0;  
  TCCR1B=BM(CS10);  // clk I/O no prescale
  TCNT1=0;  // 16 bit
  GTCCR |= BM(PSRASY);              // reset prescaler
  GTCCR |= BM(PSRSYNC);              // reset prescaler

  _nrk_os_timer_reset();
  _nrk_os_timer_start();
  _nrk_time_trigger=0;
}

void _nrk_high_speed_timer_stop()
{
  TCCR1B=0;  // no clock 
}

void _nrk_high_speed_timer_start()
{
  TCCR1B=BM(CS10);  // clk I/O no prescaler 
}


void _nrk_high_speed_timer_reset()
{
//  nrk_int_disable();
  //SFIOR |= BM(PSR321);              // reset prescaler
  GTCCR |= BM(PSRSYNC);              // reset prescaler
  TCNT1=0;
//  nrk_int_enable();
}

/**
  This function blocks for n ticks of the high speed timer after the
  start number of ticks.  It will handle the overflow that can occur.
  Do not use this for delays longer than 8ms!
*/
void nrk_high_speed_timer_wait( uint16_t start, uint16_t ticks )
{
uint32_t tmp;
if(start>65400) start=0;
tmp=(uint32_t)start+(uint32_t)ticks;
//printf("tmp is %d.\n\r", tmp);
if(tmp>65536) 
	{
//	printf("tmp>65536\n\r");
	tmp-=65536;
	do{}while(_nrk_high_speed_timer_get()>start);
	}

ticks=tmp;
do{}while(_nrk_high_speed_timer_get()<ticks);
}

inline uint16_t _nrk_high_speed_timer_get()
{
volatile uint16_t tmp;
  //nrk_int_disable();
  tmp=TCNT1;
  //nrk_int_enable();
  return tmp;
}

inline void _nrk_os_timer_stop()
{
  TCCR2B=0;  // stop timer 
  TIMSK2 &=  ~BM(OCIE2A) ;
  TIMSK2 &=  ~BM(TOIE2) ;
}

inline void _nrk_os_timer_set(uint8_t v)
{
TCNT2=v;
}
   
inline void _nrk_os_timer_start()
{
  //GTCCR |= BM(PSRASY);              // reset prescaler
  //TCNT2 = 0;                  // reset counter
  TIMSK2 |=   BM(OCIE2A) | BM(TOIE2) ;//| BM(TICIE1);    // Enable interrupt
  TCCR2B = BM(CS21) | BM(CS20); //|      // reset counter on interrupt, set divider to 128
}

inline void _nrk_os_timer_reset()
{

    GTCCR |= BM(PSRASY);              // reset prescaler
    TCNT2 = 0;                  // reset counter
    _nrk_time_trigger=0;
    _nrk_prev_timer_val=0;
}


uint8_t _nrk_get_next_wakeup()
{
	return (uint8_t)(OCR2A+1);
}

void _nrk_set_next_wakeup(uint8_t nw)
{
   OCR2A = nw-1;
   //printf("value of TCNT2 is %d.\n\r",TCNT2);
}

int8_t nrk_timer_int_stop(uint8_t timer )
{
if(timer==NRK_APP_TIMER_0)
	{
	TIMSK3 = 0;
	}
return NRK_ERROR;
}

int8_t nrk_timer_int_reset(uint8_t timer )
{
if(timer==NRK_APP_TIMER_0)
	{
	TCNT3=0;
	return NRK_OK;
	}
return NRK_ERROR;
}

uint16_t nrk_timer_int_read(uint8_t timer )
{
if(timer==NRK_APP_TIMER_0)
	{
	return TCNT3;
	}
return 0;

}

int8_t  nrk_timer_int_start(uint8_t timer)
{
if(timer==NRK_APP_TIMER_0)
	{
		TIMSK3 = BM(OCIE3A);
	return NRK_OK;
	}
return NRK_ERROR;
}

int8_t  nrk_timer_int_configure(uint8_t timer, uint16_t prescaler, uint16_t compare_value, void *callback_func)
{
if(timer==NRK_APP_TIMER_0)
	{
	if(prescaler>0 && prescaler<6 ) app_timer0_prescale=prescaler;
	TCCR3A = 0;  
	TCCR3B = BM(WGM32);  // Automatic restart on compare, count up
  	OCR3AH = (compare_value >> 8) & 0xFF;	
  	OCR3AL = (compare_value & 0xFF );
	app_timer0_callback=callback_func;
	if(app_timer0_prescale==1) TCCR3B |= BM(CS30);  
	// Divide by 1
	else if(app_timer0_prescale==2) TCCR3B |= BM(CS31); 
	// Divide by 8
	else if(app_timer0_prescale==3) TCCR3B |= BM(CS31) | BM(CS30);  
	// Divide by 64
	else if(app_timer0_prescale==4) TCCR3B |= BM(CS32) ;  
	// Divide by 256 
	else if(app_timer0_prescale==5) TCCR3B |= BM(CS32) | BM(CS30);  
	// Divide by 1024
	return NRK_OK;
	}

return NRK_ERROR;
}


inline uint8_t _nrk_os_timer_get()
{
  return (volatile uint8_t)TCNT2;
}

//--------------------------------------------------------------------------------------
//  Default ISR 
//--------------------------------------------------------------------------------------
SIGNAL(__vector_default) {
	nrk_kernel_error_add(NRK_SEG_FAULT,0);
	while(1);
}

void TIMER2_OVF_vect( void ) __attribute__ ( ( signal,naked ));
void TIMER2_OVF_vect(void) {
	#ifdef NRK_KERNEL_TEST
        nrk_kernel_error_add(NRK_TIMER_OVERFLOW,0);
	#endif

	return;  	
} 


// This is the SUSPEND for the OS timer Tick
void TIMER2_COMPA_vect( void ) __attribute__ ( ( signal,naked ));
void TIMER2_COMPA_vect(void) {
asm volatile (
   "push    r0 \n\t" \
   "in      r0, __SREG__  \n\t" \ 
   "push    r0  \n\t" \
   "push    r1 \n\t" \
   "push    r2 \n\t" \
   "push    r3 \n\t" \
   "push    r4 \n\t" \
   "push    r5 \n\t" \
   "push    r6 \n\t" \
   "push    r7 \n\t" \
   "push    r8 \n\t" \
   "push    r9 \n\t" \
   "push    r10 \n\t" \
   "push    r11 \n\t" \
   "push    r12 \n\t" \
   "push    r13 \n\t" \
   "push    r14 \n\t" \
   "push    r15 \n\t" \
   "push    r16 \n\t" \
   "push    r17 \n\t" \
   "push    r18 \n\t" \
   "push    r19 \n\t" \
   "push    r20 \n\t" \
   "push    r21 \n\t" \
   "push    r22 \n\t" \
   "push    r23 \n\t" \
   "push    r24 \n\t" \
   "push    r25 \n\t" \
   "push    r26 \n\t" \
   "push    r27 \n\t" \
   "push    r28 \n\t" \
   "push    r29 \n\t" \
   "push    r30 \n\t" \
   "push    r31 \n\t" \
   "lds r26,nrk_cur_task_TCB \n\t" \
   "lds r27,nrk_cur_task_TCB+1 \n\t" \
   "in r0,__SP_L__ \n\t" \
   "st x+, r0 \n\t" \
   "in r0,__SP_H__ \n\t" \
   "st x+, r0 \n\t" \
   "push r1  \n\t" \
   "lds r26,nrk_kernel_stk_ptr \n\t" \
   "lds r27,nrk_kernel_stk_ptr+1 \n\t" \
   "ld r1,-x \n\t" \
   "out __SP_H__, r27 \n\t" \
   "out __SP_L__, r26 \n\t" \
   "ret\n\t" \
);

} 


SIGNAL(TIMER3_COMPA_vect) {
	if(app_timer0_callback!=NULL) app_timer0_callback();
	else
	nrk_kernel_error_add(NRK_SEG_FAULT,0);
//printf("TIMER3_COMPA_vect\n\r");
	return;  	
}

//--------------------------------------------------------------------------------------
//  TIMER 1 COMPARE ISR
//--------------------------------------------------------------------------------------
SIGNAL(SIG_OUTPUT_COMPARE1A) {

	return;  	
} 

