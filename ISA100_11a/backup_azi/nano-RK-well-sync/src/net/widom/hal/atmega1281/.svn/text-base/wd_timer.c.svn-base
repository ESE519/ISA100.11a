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
/*   wd_timer.c                                                               */
/*                                                                            */
/*   IMPORTANT NOTE:                                                          */
/*     This code assumes that interrupt disabled                              */
/*     periods are small (concrete values depend on protocol                  */ 
/*     parameters; see widom.h)                                               */
/*                                                                            */
/*   USE_TIMER_COUNTER in wd_timer_cpu_clk.h defines which timer/counter      */
/*   from the atmega128 is used                                               */
/*                                                                            */
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#include <include.h>
#include <nrk.h>
#include <wd_timer.h>
#include <wd_timer_cpu_clk.h>

uint16_t logical_time = 0;
uint16_t periodic_time_offset = 0;
bool is_periodic_enabled=false;

uint16_t alarm_time = 0;
bool is_alarm_set = false;
uint8_t alarm_type = 0;

uint16_t WD_CLOCK_TICKS = WD_CLOCK_TICK_TIME/CPU_CLOCK_TICK_TIME;

/**********************************************************
 * start timer
 * (start atmega 128 Timer/Counter) 
 */	
void wd_timer_start()
{
	nrk_int_disable();

  periodic_time_offset = 0;
  logical_time = 0;
  alarm_time = 0;
  alarm_type = 0;
  is_alarm_set = false;
  is_periodic_enabled = false;

#if USE_TIMER_COUNTER == 1
	TCCR1A = 0; // Compare Output Mode : Normal 
	TCNT1 = 0; // reset timer value
	TIFR1&=~BM(OCF1A); // clear OCF flag
	TIMSK1|=BM(OCIE1A); // enable compare interrupt
	OCR1A = (uint16_t)(WD_CLOCK_TICKS); // set compare value
	TCCR1B = TIMER_CLK_DIV1; // set timer scale  
	TCCR1B |= BM(WGM12); // Waveform Generation Mode: Clear Timer on Compare
#elif USE_TIMER_COUNTER == 3
	TCCR3A = 0; // Compare Output Mode : Normal 
	TCNT3 = 0; // reset timer value
	TIFR3&=~BM(OCF3A); // clear OCF flag
	TIMSK3|=BM(OCIE3A); // enable compare interrupt
	OCR3A = (uint16_t)(WD_CLOCK_TICKS); // set compare value
	TCCR3B = TCLK_CPU_DIV; // set timer scale  
	TCCR3B |= BM(WGM32); // Waveform Generation Mode: Clear Timer on Compare
#else
  #error "Must define USE_TIMER_COUNTER in wd_timer.h as 1 or 3."
#endif

	nrk_int_enable();
}

/**********************************************************
 * stop timer 
 * (stop atmega 128 Timer/Counter overflow and compare interrupt)
 *  
 * ASSUMES: running uninterrupted to completion    
*/	
void wd_timer_stop()
{
#if USE_TIMER_COUNTER == 1
	TIFR1&=~BM(OCF1A); // clear OCF flag
	TIMSK1&=~BM(TOIE1); // disable overflow interrupt
	TIMSK1&=~BM(OCIE1A); // disable compare interrupt	
	TCNT1 = 0; // reset timer value
#elif USE_TIMER_COUNTER == 3
	TIFR3&=~BM(OCF3A); // clear OCF flag
	TIMSK3&=~BM(TOIE3); // disable overflow interrupt
	TIMSK3&=~BM(OCIE3A); // disable compare interrupt
	TCNT3 = 0; // reset timer value
#else
  #error "Must define USE_TIMER_COUNTER in wd_timer.h as 1 or 3."
#endif	
}

/**********************************************************
 * start periodic callback
 *
 * this is a periodic callback that is used to perform 
 * periodic sampling of the radio medium   
 *  
 * REQUIRES:
 *  user application defines callback function:
 *    periodic_callback(void);      
 *  
 * ASSUMES: running uninterrupted to completion    
*/	
void wd_timer_start_periodic(uint16_t offset_clktks)
{
  periodic_time_offset = (uint16_t)(offset_clktks);
  is_periodic_enabled = true; 
}

/**********************************************************
 * stop periodic callback
 *  
 * ASSUMES: running uninterrupted to completion    
*/	
void wd_timer_stop_periodic()
{
  is_periodic_enabled = false;
}

/**********************************************************
 * get logical time value
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
uint16_t wd_timer_logical_time_get() 
{
  return logical_time;
}

/**********************************************************
 * set logical time value
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
void wd_timer_logical_time_set(uint16_t time_clkTks) 
{
  logical_time = time_clkTks;
}

/**********************************************************
 * reset logical time value
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
void wd_timer_logical_time_reset() 
{
  logical_time=0;
}

/**********************************************************
 * set alarm 
 * (set alarm based on atmega 128 Timer/Counter comparator)
 *  
 * REQUIRES:
 *  definition of callback function:
 *     void alarm_callback(uint8_t alarm_type);   
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
void wd_timer_set_alarm(uint32_t when_clkTks, uint8_t type) 
{
  alarm_time = when_clkTks;
	alarm_type = type;
	is_alarm_set = true;	
}

/**********************************************************
 * cancel alarm 
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
void wd_timer_cancel_alarm() 
{
	is_alarm_set = false;
} 

/**********************************************************
 * check if an alarm is set 
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
bool wd_timer_is_alarm_set()
{
	return is_alarm_set;
}

/**********************************************************
* Timer (atmega 128 Timer/Counter) compare signal 
*/
#if USE_TIMER_COUNTER == 1	
SIGNAL(SIG_OUTPUT_COMPARE1A) // Timer/Counter1 ISR 
#elif USE_TIMER_COUNTER == 3
SIGNAL(SIG_OUTPUT_COMPARE3A) // Timer/Counter ISR 
#else
  #error "Must define USE_TIMER_COUNTER in wd_timer.h as 1 or 3."
#endif
{ 

//  nrk_gpio_toggle(NRK_DEBUG_0);  

  logical_time++;

  if (is_alarm_set == true) {
    if (logical_time >= alarm_time-1) {
      is_alarm_set = false;
      alarm_callback(alarm_type);
    }
  }
 
  if (is_periodic_enabled == true) {
    if (periodic_time_offset > 0) periodic_time_offset--;
    else periodic_callback();
  }
  
}
