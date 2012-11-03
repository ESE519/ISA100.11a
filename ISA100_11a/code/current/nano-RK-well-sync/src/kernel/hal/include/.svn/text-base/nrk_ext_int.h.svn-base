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


#ifndef NRK_EXT_INT_H
#define NRK_EXT_INT_H

#define NRK_EXT_INT_0		0
#define NRK_EXT_INT_1		1
#define NRK_EXT_INT_2		2

#define NRK_PC_INT_0		3		
#define NRK_PC_INT_1		4		
#define NRK_PC_INT_2		5		
#define NRK_PC_INT_3		6		
#define NRK_PC_INT_4		7		
#define NRK_PC_INT_5		8		
#define NRK_PC_INT_6		9		
#define NRK_PC_INT_7		10		

#define NRK_LOW_TRIGGER		0
#define NRK_LEVEL_TRIGGER	1
#define NRK_FALLING_EDGE	2
#define NRK_RISING_EDGE		3

void (*ext_int0_callback)(void);
void (*ext_int1_callback)(void);
void (*ext_int2_callback)(void);
void (*pc_int0_callback)(void);


int8_t  nrk_ext_int_enable(uint8_t pin );
int8_t  nrk_ext_int_disable(uint8_t pin );
int8_t  nrk_ext_int_configure(uint8_t pin, uint8_t mode, void *callback_func);


#endif
