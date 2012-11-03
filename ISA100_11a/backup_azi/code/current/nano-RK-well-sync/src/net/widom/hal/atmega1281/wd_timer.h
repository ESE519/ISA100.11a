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
/*   wd_timer.h                                                               */
/*                                                                            */
/*   USE_TIMER_COUNTER in wd_timer_cpu_clk.h defines which timer/counter      */
/*   from the atmega128 is used                                               */
/*                                                                            */
/*   Author: Nuno Pereira                                                     */
/* ========================================================================== */

#ifndef _WD_TIMER_H
#define _WD_TIMER_H

#include <hal.h>
#include <wd_timer_cpu_clk.h>

// widom tick resolution in seconds (32 us)
// this also defines what is the frequency for 
// the periodic sampling of the radio medium 
#define WD_CLOCK_TICK_TIME 0.000032

// Callback functions
inline void alarm_callback(uint8_t alarm_type);
inline void periodic_callback(void);

////////////////////////////////////////////////////
//
// functions to setup the timer 
// timer is run by atmega 128 Timer/Counter
//

/**********************************************************
 * start timer
 * (start atmega 128 Timer/Counter) 
 */	
void wd_timer_start();

/**********************************************************
 * stop timer 
 * (stop atmega 128 Timer/Counter overflow and compare interrupt)
 *  
 * ASSUMES: running uninterrupted to completion    
*/	
void wd_timer_stop();

/**********************************************************
 * start periodic callback
 *  
 * ASSUMES: running uninterrupted to completion    
*/	
void wd_timer_start_periodic(uint16_t offset_clktks);

/**********************************************************
 * stop periodic callback
 *  
 * ASSUMES: running uninterrupted to completion    
*/	
void wd_timer_stop_periodic();

/**********************************************************
 * get logical time value
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
uint16_t wd_timer_logical_time_get(); 

/**********************************************************
 * set logical time value
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
void wd_timer_logical_time_set(uint16_t time_clkTks); 

/**********************************************************
 * reset logical time value
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
void wd_timer_logical_time_reset();

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
void wd_timer_set_alarm(uint32_t when_clkTks, uint8_t type); 

/**********************************************************
 * cancel alarm 
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
void wd_timer_cancel_alarm();

/**********************************************************
 * check if an alarm is set 
 *  
 * ASSUMES: running uninterrupted to completion    
 */	
bool wd_timer_is_alarm_set();

#endif
