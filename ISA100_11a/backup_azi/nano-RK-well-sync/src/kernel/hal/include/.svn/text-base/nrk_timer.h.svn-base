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


#ifndef NRK_TIMER_H
#define NRK_TIMER_H

#define NRK_APP_TIMER_0		0

typedef struct {
   	VFPTR pFunc;
	int32_t timeout;
    uint8_t nextTimer;
    uint8_t prevTimer;
    bool active;
} MAC_TIMER_INFO;

uint8_t _nrk_prev_timer_val;
uint8_t _nrk_time_trigger;

void (*app_timer0_callback)(void);
uint8_t app_timer0_prescale;


int8_t nrk_timer_int_reset(uint8_t timer );
uint16_t nrk_timer_int_read(uint8_t timer );
int8_t nrk_timer_int_stop(uint8_t timer );
int8_t nrk_timer_int_start(uint8_t timer );
int8_t nrk_timer_int_configure(uint8_t timer, uint16_t prescaler, uint16_t compare_value, void *callback_func);

inline void nrk_high_speed_timer_wait( uint16_t start, uint16_t ticks );
inline void _nrk_high_speed_timer_stop();
inline void _nrk_high_speed_timer_start();
inline void _nrk_high_speed_timer_reset();
inline uint16_t _nrk_high_speed_timer_get();

inline void _nrk_os_timer_reset();
inline void _nrk_os_timer_set(uint8_t v);
inline void _nrk_os_timer_stop();
inline void _nrk_os_timer_start();
inline uint8_t _nrk_os_timer_get();

// implemented in hardware sepcific assembly file
void _nrk_timer_suspend_task();

void nrk_spin_wait_us(uint16_t timeout);

// Only used for scheduling
void _nrk_setup_timer();
void _nrk_set_next_wakeup(uint8_t nw);
uint8_t _nrk_get_next_wakeup();

void _nrk_start_os_timer();
inline void _nrk_reset_os_timer();

#endif
