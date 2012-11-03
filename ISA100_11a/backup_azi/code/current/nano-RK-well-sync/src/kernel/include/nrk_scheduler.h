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
*  Anand Eswaren
*  Zane Starr
*******************************************************************************/

#ifndef _NRK_SCHEDULER_h	/* Only include stuff once */
#define _NRK_SCHEDULER_h


#include <nrk_cpu.h> /* New add on */
#include <nrk_cfg.h>
#include <nrk.h>
#include <nrk_time.h>

#define MAX_SCHED_WAKEUP_TIME         250

uint8_t _nrk_cpu_state;

void _nrk_scheduler(void);

uint16_t next_next_wakeup;
uint16_t _nrk_get_next_next_wakeup();

// defined in hardware specific assembly file
void nrk_start_high_ready_task();

#endif

