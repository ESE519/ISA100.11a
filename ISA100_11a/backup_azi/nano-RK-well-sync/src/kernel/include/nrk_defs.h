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


#ifndef NRK_DEFS_H
#define NRK_DEFS_H

#include <nrk.h>

#define SWAP_BYTES(x)\
	((((uint16_t)x)<<8)|((((uint16_t)x)>>8)&0xff))

#ifndef NRK_MAX_RESOURCE_CNT
#define NRK_MAX_RESOURCE_CNT 0
#endif

#ifdef NRK_KERNEL_TEST
uint8_t nrk_max_sleep_wakeup_time;
#endif

#ifndef NRK_IDLE_TASK_ID
#define NRK_IDLE_TASK_ID	0
#endif

NRK_TCB	nrk_task_TCB[NRK_MAX_TASKS];   /* Table of TCBs */

nrk_queue	_nrk_readyQ[NRK_MAX_TASKS+1];
nrk_queue	*_free_node,*_head_node;


nrk_sig_t nrk_wakeup_signal;

/*--------- Running task context --------*/

uint8_t  nrk_cur_task_prio ;
NRK_TCB	*nrk_cur_task_TCB;

uint8_t	nrk_high_ready_prio;
NRK_TCB	*nrk_high_ready_TCB;



//Resource Tracking Globals;
uint8_t _nrk_resource_cnt;
//int8_t	  nrk_resource_count[NRK_MAX_RESOURCE_CNT];
//nrk_sem_t nrk_resource_value[NRK_MAX_RESOURCE_CNT];
//int8_t	  nrk_resource_ceiling[NRK_MAX_RESOURCE_CNT];
nrk_sem_t nrk_sem_list[NRK_MAX_RESOURCE_CNT];

nrk_time_t nrk_system_time;

extern NRK_STK	nrk_idle_task_stk[NRK_TASK_IDLE_STK_SIZE];  /* Idle task stack   */
#ifdef KERNEL_STK_ARRAY
	extern NRK_STK  nrk_kernel_stk[NRK_KERNEL_STACKSIZE];
#endif
extern NRK_STK *nrk_kernel_stk_ptr;


#endif
