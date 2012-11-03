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

#include <include.h>
#include <nrk.h>
#include <nrk_stack_check.h>
#include <nrk.h>
#include <nrk_task.h>
#include <nrk_defs.h>
#include <nrk_cfg.h>
#include <nrk_error.h>

#define BUILD_DATE "Date: " __DATE__ "\n"

/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              pdata         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            a 'free' entry on the task stack.  
*                            'ptos' contains the HIGHEST valid address of the stack.  
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            We don't use have any option implemented for this project. You can just
*                            set opt to 0
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 
*********************************************************************************************************
*/


void nrk_battery_save()
{
/*
#ifdef NRK_BATTERY_SAVE
 	_nrk_stop_os_timer();
        _nrk_set_next_wakeup(250);
        nrk_led_clr(0);
        nrk_led_set(1);
        nrk_led_clr(2);
        nrk_led_clr(3);
        SET_VREG_INACTIVE();
        nrk_sleep();
#endif
*/
}

void nrk_sleep()
{
/*
    set_sleep_mode (SLEEP_MODE_PWR_SAVE);
    sleep_mode ();
*/
}

void nrk_idle()
{
/*
    set_sleep_mode( SLEEP_MODE_IDLE);
    sleep_mode ();
*/
}

void nrk_task_set_entry_function( nrk_task_type *task, void *func )
{
	//task->task = (void*)((((uint16_t)func >> 8) & 0xff) | (((uint16_t)func << 8) & 0xff00));
	task->task = (void*)SWAP_BYTES(func);
}

void nrk_task_set_stk( nrk_task_type *task, NRK_STK stk_base[], uint16_t stk_size )
{
	void* addr;
	if(stk_size<32) nrk_error_add(NRK_STACK_TOO_SMALL);

	//TODO: why aren't these swapped?
	addr=&(stk_base[stk_size-2]);
	//task->Ptos = ((addr << 8) & 0xff00) | ((addr >> 8) & 0x00ff);
	task->Ptos = addr;
	addr=&(stk_base[0]);
	//task->Pbos = ((addr << 8) & 0xff00) | ((addr >> 8) & 0x00ff);
	task->Pbos = addr;

	//task->Pbos = (void *) &stk_base[0];
}

//TODO: make sure this works for msp...
void *nrk_task_stk_init (void (*task)(), void *ptos, void *pbos)
{
    uint16_t *stk ;  // 2 bytes
    uint8_t *stkc; // 1 byte

    stk    = (unsigned int *)pbos;          /* Load stack pointer */ 
    stkc = (unsigned char*)stk;
    *stkc = STK_CANARY_VAL;  // Flag for Stack Overflow    
    stk    = (unsigned int *)ptos;          /* Load stack pointer */
    /* build a context for the new task */
    /* Where do these numbers come from? */
   /* *(--stk) = 0x4f50;   // O P 
    *(--stk) = 0x4d4e;   // M N 
    *(--stk) = 0x4b4c;   // K L                      
    *(--stk) = 0x494a;   // I J                      
    *(--stk) = 0x4748;   // G H                      
    *(--stk) = 0x4546;   // E F                    
    *(--stk) = 0x4344;   // C D    	
    *(--stk) = 0x4142;   // A B
*/
/*
    --stk;
    stkc = (unsigned char*)stk;	
    *stkc++ = (unsigned char)((unsigned int)(task)/ 256);
    *stkc = (unsigned char)((unsigned int)(task)%256);
*/
	
    --stk;
		*stk = SWAP_BYTES(task);

    *(--stk) = 0x8; //enable interrupts
    *(--stk) = 0;       
    *(--stk) = 0;                        
    *(--stk) = 0;                        
    *(--stk) = 0;                         
    *(--stk) = 0;                         
    *(--stk) = 0;                        
    *(--stk) = 0; 

    *(--stk) = 0; 
    *(--stk) = 0; 
    *(--stk) = 0; 
    *(--stk) = 0; 
    *(--stk) = 0; 
    //*(--stk) = 0; 
    //*(--stk) = 0; 
    //*(--stk) = 0; 
    //*(--stk) = 0;


    return ((void *)stk);
}

inline void nrk_stack_pointer_init()
{
	uint16_t *stkc;
#ifdef KERNEL_STK_ARRAY
        stkc = (uint16_t*)&nrk_kernel_stk[NRK_KERNEL_STACKSIZE-1];
        nrk_kernel_stk[0]=STK_CANARY_VAL;
        nrk_kernel_stk_ptr = &nrk_kernel_stk[NRK_KERNEL_STACKSIZE-1];
    #else
        stkc = (uint16_t*)(NRK_KERNEL_STK_TOP-NRK_KERNEL_STACKSIZE);
        *stkc = STK_CANARY_VAL;
        stkc = (uint16_t*)NRK_KERNEL_STK_TOP;
        nrk_kernel_stk_ptr = (uint16_t*)NRK_KERNEL_STK_TOP;
    #endif

    //MSP: bytes are swapped
    //TODO: switch these?
    //*stkc = (uint16_t)((uint16_t)_nrk_timer_tick>>8);
    //*stkc++ = (uint16_t)((uint16_t)_nrk_timer_tick&0xFF);
		*stkc = (uint16_t)_nrk_timer_tick;

}


inline void nrk_stack_pointer_restore()
{
//unsigned char *stkc;
uint16_t *stkc;

#ifdef KERNEL_STK_ARRAY
        stkc = (uint16_t*)&nrk_kernel_stk[NRK_KERNEL_STACKSIZE-1];
#else
        stkc = NRK_KERNEL_STK_TOP;
#endif
	//MSP: bytes are swapped
 //       *stkc++ = (uint16_t)((uint16_t)_nrk_timer_tick&0xff);
  //      *stkc = (uint16_t)((uint16_t)_nrk_timer_tick>>8);
	*stkc = _nrk_timer_tick;
}

/* start the target running */
void nrk_target_start(void)
{

  _nrk_setup_timer();
  //nrk_int_enable();  
	
}

