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
*  Zane Starr
*  Anthony Rowe
*******************************************************************************/

#ifndef NRK_RESERVE_H
#define NRK_RESERVE_H

#include <nrk_cfg.h>

#ifdef NRK_MAX_RESERVES

void _nrk_reserve_init();

int8_t nrk_reserve_create();
int8_t nrk_reserve_delete(uint8_t reserve_id);
// Get counts remaining
uint8_t nrk_reserve_get(uint8_t reserve_id);
// Set period and counts in reserve
int8_t nrk_reserve_set(uint8_t id, nrk_time_t *period,int16_t access_count,void *errhandler);
// Consume reserve
int8_t nrk_reserve_consume(uint8_t reserve_id);

void _nrk_reserve_update(uint8_t reserve_id);


typedef struct nrk_reserve
{
uint32_t period_ticks; //period in ticks
uint32_t set_time;
uint32_t cur_time;//current time in ticks
int16_t set_access;//reserve on number of calls etc..
int16_t cur_access;//current count of accesses
int8_t  active;
void (*error)();
}nrk_reserve;

#endif
#endif
