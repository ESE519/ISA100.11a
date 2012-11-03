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


#ifndef _SLIP_H_
#define _SLIP_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <nrk.h>

#define ESC	219
#define END	192	
#define START	193	

int8_t slip_started();
int8_t slip_init( FILE *device_in, FILE *device_out, bool echo, uint8_t delay );
int8_t slip_tx(uint8_t *buf, uint8_t size); 
int8_t slip_rx(uint8_t *buf, uint8_t max_len); 
void put_byte(uint8_t c);
uint8_t get_byte(void);

#endif
