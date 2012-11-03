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
*  Zane Starr
*  Anand Eswaren
*******************************************************************************/

#ifndef __NRK_CPU_H	/* Only include stuff once */
#define __NRK_CPU_H

//#include <stdint.h>
#include <nrk_cfg.h>



//typedef uint8_t   NRK_STK;                   // Each stack entry is 8-bit wide 
// moved to hal.h in platforms directory

extern void nrk_target_start(void);


extern void nrk_int_disable(void);
extern void nrk_int_enable(void);

void nrk_watchdog_disable();
void nrk_battery_save();

void nrk_sleep(void);
void nrk_idle(void);
inline void nrk_stack_pointer_restore(void);
inline void nrk_stack_pointer_init(void);


#endif
