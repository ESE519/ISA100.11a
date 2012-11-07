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


#ifndef _RT_DEBUG_H
#define _RT_DEBUG_H

#include <nrk.h>
#include <include.h>

#define MAX_PKT_LOG  64 
#define MAX_TSYNC_LOG 8

uint16_t prev_offset;
uint16_t rtl_drift_rate;

uint8_t _tsync_index;
uint8_t _dbg_index;
uint8_t pkt_log[MAX_PKT_LOG/8];
uint8_t tsync_log[MAX_TSYNC_LOG];
uint8_t tsync_delay[MAX_TSYNC_LOG];

void rtl_debug_init();
int16_t rtl_debug_time_get_drift();
void rtl_debug_time_update(uint16_t offset);
uint8_t rtl_debug_get_pkt_loss();
uint8_t rtl_debug_get_tsync_loss();
uint8_t rtl_debug_get_tsync_delay(uint8_t index);
void rtl_debug_rx_pkt(); 
void rtl_debug_dropped_pkt(); 

void rtl_debug_rx_tsync(); 
void rtl_debug_dropped_tsync(uint8_t delay); 
#endif
