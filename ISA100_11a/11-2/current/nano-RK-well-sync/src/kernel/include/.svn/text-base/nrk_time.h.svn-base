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


#ifndef NRK_TIME_H
#define NRK_TIME_H

#include <include.h>
#include <ulib.h>
#include <nrk_platform_time.h>

#define NANOS_PER_SEC       1000000000
#define US_PER_SEC          1000000
#define NANOS_PER_MS        1000000
#define NANOS_PER_US        1000



//#define NANOS_PER_TICK	    976563	
//#define NANOS_PER_TICK	    976181
//#define US_PER_TICK	    9765	
//#define TICKS_PER_SEC       1024

typedef struct {
   uint32_t secs;
   uint32_t nano_secs;
} nrk_time_t;

void nrk_time_get(nrk_time_t *t);
void nrk_time_set(uint32_t secs, uint32_t nano_secs);
uint16_t _nrk_time_to_ticks(nrk_time_t t);
uint32_t _nrk_time_to_ticks_long(nrk_time_t t);
uint8_t nrk_time_sub(nrk_time_t *result,nrk_time_t high, nrk_time_t low);
uint8_t nrk_time_add(nrk_time_t *result,nrk_time_t a, nrk_time_t b);
inline void nrk_time_compact_nanos(nrk_time_t *t);

#endif
