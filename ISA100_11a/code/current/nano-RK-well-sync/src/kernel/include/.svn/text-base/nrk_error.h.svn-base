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
*******************************************************************************/

#ifndef NRK_ERROR_H
#define NRK_ERROR_H

#include <include.h>
#include <ulib.h>

#define NRK_OK		  1
#define NRK_ERROR	(-1)

#define NRK_UNKOWN		        0
#define NRK_STACK_OVERFLOW		1
#define NRK_RESERVE_ERROR		2
#define NRK_RESERVE_VIOLATED    	3
#define NRK_WAKEUP_MISSED		4
#define NRK_DUP_TASK_ID			5
#define NRK_BAD_STARTUP			6
#define NRK_EXTRA_TASK      		7
#define NRK_STACK_SMASH			8
#define NRK_LOW_VOLTAGE			9
#define NRK_SEG_FAULT		        10	
#define NRK_TIMER_OVERFLOW		11	
#define NRK_DEVICE_DRIVER		12	
#define NRK_UNIMPLEMENTED		13	
#define NRK_SIGNAL_CREATE_ERROR		14
#define NRK_SEMAPHORE_CREATE_ERROR	15
#define NRK_WATCHDOG_ERROR		16
#define NRK_STACK_TOO_SMALL		17
#define NRK_INVALID_STACK_POINTER	18
#define NRK_NUM_ERRORS			19
typedef uint8_t NRK_ERRNO;

uint8_t error_task;
uint8_t error_num;

uint8_t nrk_errno_get();
void _nrk_errno_set(NRK_ERRNO error_code);
void nrk_kernel_error_add(uint8_t n,uint8_t task);
void nrk_error_add(uint8_t error_num);
int8_t nrk_error_print();
uint8_t nrk_error_get(uint8_t *task_id, uint8_t *code);

#endif
