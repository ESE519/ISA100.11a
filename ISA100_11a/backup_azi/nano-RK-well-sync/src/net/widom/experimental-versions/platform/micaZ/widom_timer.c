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
* time interval given is in clock ticks 
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
