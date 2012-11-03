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


#ifndef _RTL_SCHEDULER_H_
#define _RTL_SCHEDULER_H_


uint8_t rtl_sched[16];            // only one since you can TX and RX on the same slot
uint8_t rtl_sched_cache[32];
uint16_t rtl_abs_wakeup[MAX_ABS_WAKEUP];  // MSB is the repeat flag


void _rtl_clr_abs_all_wakeup ();
uint16_t _rtl_get_next_abs_wakeup (uint16_t global_slot);
uint8_t _rtl_match_abs_wakeup (uint16_t global_slot);

uint8_t _rtl_pow (uint8_t x, uint8_t y);
void _rtl_clear_sched_cache ();

#endif
