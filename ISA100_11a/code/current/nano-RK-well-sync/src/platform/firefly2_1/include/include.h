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
*  Chipcon Development Team 
*******************************************************************************/


#ifndef INCLUDE_H
#define INCLUDE_H

//-------------------------------------------------------------------------------------------------------
// Common data types
//typedef uint8_t		bool;
/*typedef unsigned char		bool;

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned long		DWORD;
typedef unsigned long long	QWORD;

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned long		uint32_t;
typedef unsigned long long	UINT64;

typedef signed char			int8_t;
typedef signed short		INT16;
typedef signed long			int32_t;
typedef signed long long	INT64;
*/
// Common values
#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef NULL
	#define NULL 0
#endif

// Useful stuff
#define BM(n) (1 << (n))
#define BF(x,b,s) (((x) & (b)) >> (s))
#define MIN(n,m) (((n) < (m)) ? (n) : (m))
#define MAX(n,m) (((n) < (m)) ? (m) : (n))
#define ABS(n) ((n < 0) ? -(n) : (n))

// Dynamic function pointer
typedef void (*VFPTR)(void);
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
// Standard GCC include files for AVR
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/signal.h>
#include <string.h>

#ifdef STK501
    //#include "hal/atmega128/hal_stk501.h"
#endif

#ifdef CC2420DB
    //#include "hal/atmega128/hal_cc2420db.h"
#endif

// HAL include files
#include <hal.h>
#include <hal_firefly2.h>
#include <hal_cc2420.h>

   #include <basic_rf.h>
//-----------------------------------------------------------------------------------------------

#endif
