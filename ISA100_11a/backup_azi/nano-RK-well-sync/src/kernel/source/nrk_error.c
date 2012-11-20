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
 
#include <nrk_error.h>
#include <stdio.h>
#include <nrk.h>
#include <nrk_task.h>
#include <nrk_cfg.h>
#include <nrk_timer.h>

void _nrk_errno_set (NRK_ERRNO error_code) 
{
  nrk_cur_task_TCB->errno = error_code;
} 

uint8_t nrk_errno_get () 
{
  return nrk_cur_task_TCB->errno;
}

void nrk_error_add (uint8_t n) 
{
  error_num = n;
  error_task = nrk_cur_task_TCB->task_ID;
  
#ifdef NRK_REPORT_ERRORS
    nrk_error_print ();
  
#endif  /*  */
} void nrk_kernel_error_add (uint8_t n, uint8_t task) 
{
  error_num = n;
  error_task = task;
  
#ifdef NRK_REPORT_ERRORS
    nrk_error_print ();
  
#endif  /*  */
} 


uint8_t nrk_error_get (uint8_t * task_id, uint8_t * code) 
{
  if (error_num == 0)
    return 0;
  *code = error_num;
  *task_id = error_task;
  return 1;
}

int8_t nrk_error_print () 
{
  int8_t t;
  if (error_num == 0)
    return 0;
 
   #ifdef NRK_HALT_ON_ERROR
     nrk_int_disable ();
       #ifdef NRK_WATCHDOG
         nrk_watchdog_disable();
       #endif
   #endif 

   #ifndef NRK_REBOOT_ON_ERROR
      nrk_int_disable ();
   #endif 


#ifdef NRK_HALT_AND_LOOP_ON_ERROR 
    nrk_int_disable ();
   #ifdef NRK_WATCHDOG
      nrk_watchdog_disable();
   #endif

  while (1)
     {
    
#endif  
    
    nrk_kprintf (PSTR ("*NRK ERROR("));
    printf ("%d", error_task);
    nrk_kprintf (PSTR ("): "));
    if (error_num > NRK_NUM_ERRORS)
      error_num = NRK_UNKOWN;
    switch (error_num)
       {
    case NRK_STACK_TOO_SMALL:
      nrk_kprintf (PSTR ("Stack was not defined as large enough!"));
      break;
    case NRK_STACK_OVERFLOW:
      nrk_kprintf (PSTR ("Task Stack Overflow"));
      break;
    case NRK_INVALID_STACK_POINTER:
      nrk_kprintf (PSTR ("Invalid Stack Pointer"));
      break;
    case NRK_RESERVE_ERROR:
      nrk_kprintf (PSTR ("Reserve Error in Scheduler"));
      break;
    case NRK_RESERVE_VIOLATED:
      nrk_kprintf (PSTR ("Task Reserve Violated"));
      break;
    case NRK_WAKEUP_MISSED:
      nrk_kprintf (PSTR ("Scheduler Missed Wakeup"));
      break;
    case NRK_DUP_TASK_ID:
      nrk_kprintf (PSTR ("Duplicated Task ID"));
      break;
    case NRK_BAD_STARTUP:
      nrk_kprintf (PSTR ("Unexpected Restart"));
      break;
    case NRK_STACK_SMASH:
      nrk_kprintf (PSTR ("Idle or Kernel Stack Overflow"));
      break;
    case NRK_EXTRA_TASK:
      nrk_kprintf (PSTR ("Extra Task started, is nrk_cfg.h ok?"));
      break;
    case NRK_LOW_VOLTAGE:
      nrk_kprintf (PSTR ("Low Voltage"));
      break;
    case NRK_SEG_FAULT:
      nrk_kprintf (PSTR ("Unhandled Interrupt Vector"));
      break;
    case NRK_TIMER_OVERFLOW:
      nrk_kprintf (PSTR ("Timer Overflow"));
      break;
    case NRK_WATCHDOG_ERROR:
      nrk_kprintf (PSTR ("Watchdog Restart"));
      break;
    case NRK_DEVICE_DRIVER:
      nrk_kprintf (PSTR ("Device Driver Error"));
      break;
    case NRK_UNIMPLEMENTED:
      nrk_kprintf (PSTR ("Kernel function not implemented"));
      break;
    case NRK_SIGNAL_CREATE_ERROR:
      nrk_kprintf (PSTR ("Failed to create Signal"));
      break;
    case NRK_SEMAPHORE_CREATE_ERROR:
      nrk_kprintf (PSTR ("Failed to create Semaphore"));
      break;
    default:
      nrk_kprintf (PSTR ("UNKOWN"));
      }
    putchar ('\r');
    putchar ('\n');

#ifdef NRK_REBOOT_ON_ERROR
  // wait for watchdog to kick in
  if(error_num!=NRK_WATCHDOG_ERROR)
  {
  nrk_watchdog_enable();
  nrk_int_disable(); 
  while(1);
  }
#endif

//t=error_num;
#ifdef NRK_HALT_AND_LOOP_ON_ERROR
      nrk_led_set (2);
    nrk_led_clr (3);
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (1000);
    nrk_led_set (3);
    nrk_led_clr (2);
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (1000);
    }
  
#endif  /*  */
    
#ifdef NRK_HALT_ON_ERROR
    while (1)
     {
    nrk_led_set (2);
    nrk_led_clr (3);
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (1000);
    nrk_led_set (3);
    nrk_led_clr (2);
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (1000);
    }
  
#endif  /*  */
    error_num = 0;
  return t;
}


