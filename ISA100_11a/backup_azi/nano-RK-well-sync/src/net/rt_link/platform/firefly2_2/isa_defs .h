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


#ifndef _RTL_DEFS_H_
#define _RTL_DEFS_H_

#define RTL_NRK_TICKS_PER_SLOT 9	
// Length of TDMA slot in microseconds 
// Granularity bound by RTL_TIME_TICK_US 
#define RTL_SLOT_TIME_US (US_PER_TICK*(RTL_NRK_TICKS_PER_SLOT)) 


// 8Mhz gives 125 nanos / tick
#define HS_TICKS_PER_SLOT    (NANOS_PER_TICK/125)

// Fix full packet overflow problem
// Why are these both about 100us smaller than they should be?
// 276us = 2208 ticks
//#define SLOT_TO_START	2200
//#define SLOT_TO_START	1400
// 372uS = 2976 ticks
//#define TX_TO_SFD       2200 
// 300uS = 3000 ticks
// must be greater than SLOT_TO_START
#define SLOT_TO_SFD	(TX_GUARD_TIME+TX_TO_SFD)
#define GUARD_LOW	(SLOT_TO_SFD-(HS_TICKS_PER_SLOT-SLOT_TO_START))
#define GUARD_HIGH	SLOT_TO_SFD

// 6 ticks * 977us per tick  / .125uS ticks
#define SLOT_TIME       46896	
#define TX_TO_SFD       2976 
#define TX_GUARD_TIME   4000
#define OS_TICK_TO_TASK_START  800 
//#define SFD_TO_NEXT_SLOT_TIME   (SLOT_TIME-TX_GUARD_TIME-TX_TO_SFD-OS_TICK_TO_TASK_START-OS_TICK_TO_TASK_START)
//#define SFD_TO_NEXT_SLOT_TIME   (41100)
#define SFD_TO_NEXT_SLOT_TIME   (27750)


#endif
