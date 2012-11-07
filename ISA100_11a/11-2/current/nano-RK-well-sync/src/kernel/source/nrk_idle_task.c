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

#include <nrk.h>
#include <nrk_idle_task.h>
#include <nrk_cpu.h>
#include <nrk_cfg.h>
#include <nrk_stack_check.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_platform_time.h>
#include <nrk_scheduler.h>
#include <stdio.h>

void nrk_idle_task()
{
volatile unsigned char *stkc;
// unsigned int *stk ;  // 2 bytes
while(1)
{

  nrk_stack_check(); 
  
  if(_nrk_get_next_wakeup()<=NRK_SLEEP_WAKEUP_TIME) 
    {
	    _nrk_cpu_state=1;
	    nrk_idle();
    }
    else {
	#ifndef NRK_NO_POWER_DOWN
	    // Allow last UART byte to get out
    	    nrk_spin_wait_us(10);  
	    _nrk_cpu_state=2;
	    nrk_sleep();
	#else
	    nrk_idle();
	#endif
    }
 
#ifdef NRK_STACK_CHECK
   if(nrk_idle_task_stk[0]!=STK_CANARY_VAL) nrk_error_add(NRK_STACK_SMASH);
   #ifdef KERNEL_STK_ARRAY
   	if(nrk_kernel_stk[0]!=STK_CANARY_VAL) nrk_error_add(NRK_STACK_SMASH);
   #else
   	stkc=(unsigned char*)(NRK_KERNEL_STK_TOP-NRK_KERNEL_STACKSIZE);
   	if(*stkc!=STK_CANARY_VAL) nrk_error_add(NRK_STACK_SMASH);
   #endif
#endif



}


}
