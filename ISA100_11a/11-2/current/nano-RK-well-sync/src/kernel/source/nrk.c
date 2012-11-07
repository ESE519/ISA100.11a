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

#include <stdio.h>
#include <nrk_includes.h> 
#include <ulib.h>
#include <nrk.h>
#include <nrk_task.h>
#include <nrk_idle_task.h>
#include <nrk_defs.h>
#include <nrk_cpu.h>
#include <nrk_scheduler.h>
#include <nrk_error.h>
#include <nrk_events.h>
#include <nrk_stack_check.h>
#include <nrk_status.h>
#include <nrk_watchdog.h>
#include <nrk_reserve.h>
#include <nrk_cfg.h>
#include <nrk_stats.h>

inline void nrk_int_disable(void) {
  DISABLE_GLOBAL_INT();
};

inline void nrk_int_enable(void) {
  ENABLE_GLOBAL_INT();
};

uint8_t nrk_task_init_cnt;


void nrk_halt()
{
nrk_int_disable();
while(1);
}

/**
 *  nrk_init();
 * *  - Init TCBlist - linked list of empty TCBs
 *  - Init global variables
 *  - Init event list
 *  - Create idle task
 */
void nrk_init()
{
	
    uint8_t i;	
//    unsigned char *stkc;
	
   nrk_task_type IdleTask;
   nrk_wakeup_signal = nrk_signal_create();
   if(nrk_wakeup_signal==NRK_ERROR) nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,0);
	
   //if((volatile)TCCR1B!=0) nrk_kernel_error_add(NRK_STACK_OVERFLOW,0); 
   if(_nrk_startup_ok()==0) nrk_kernel_error_add(NRK_BAD_STARTUP,0); 
   #ifdef NRK_STARTUP_VOLTAGE_CHECK
   	if(nrk_voltage_status()==0) nrk_kernel_error_add(NRK_LOW_VOLTAGE,0);
   #endif

   #ifdef NRK_REBOOT_ON_ERROR
   #ifndef NRK_WATCHDOG
   while(1)
	   {
		nrk_kprintf( PSTR("KERNEL CONFIG CONFLICT:  NRK_REBOOT_ON_ERROR needs watchdog!\r\n") );
    		for (i = 0; i < 100; i++)
      			nrk_spin_wait_us (1000);
	   }
   #endif
   #endif

    #ifdef NRK_WATCHDOG
    if(nrk_watchdog_check()==NRK_ERROR) 
	{
    	nrk_watchdog_disable();
	nrk_kernel_error_add(NRK_WATCHDOG_ERROR,0);
	}
    nrk_watchdog_enable();
    #endif
  
  // nrk_stack_pointer_init(); 
/* 
    #ifdef KERNEL_STK_ARRAY
	stkc = (uint16_t*)&nrk_kernel_stk[NRK_KERNEL_STACKSIZE-1];
	nrk_kernel_stk[0]=STK_CANARY_VAL;
    	nrk_kernel_stk_ptr = &nrk_kernel_stk[NRK_KERNEL_STACKSIZE-1];
    #else
    	stkc = NRK_KERNEL_STK_TOP-NRK_KERNEL_STACKSIZE;
    	*stkc = STK_CANARY_VAL;
    	stkc = NRK_KERNEL_STK_TOP;
	nrk_kernel_stk_ptr = NRK_KERNEL_STK_TOP;
    #endif
    *stkc++ = (uint16_t)((uint16_t)_nrk_timer_tick>>8);
    *stkc = (uint16_t)((uint16_t)_nrk_timer_tick&0xFF); 
*/	
 
   // printf( "Init kernel_entry= %d %d\n",kernel_entry[1], kernel_entry[0] );

    
    nrk_cur_task_prio = 0;
    nrk_cur_task_TCB = NULL;
    
    nrk_high_ready_TCB = NULL;
    nrk_high_ready_prio = 0; 

   #ifdef NRK_STATS_TRACKER
	nrk_stats_reset();
   #endif

    #ifdef NRK_MAX_RESERVES 
    // Setup the reserve structures
    _nrk_reserve_init();
    #endif

    _nrk_resource_cnt=0; //NRK_MAX_RESOURCE_CNT;

for(i=0;i<NRK_MAX_RESOURCE_CNT;i++)
{
    nrk_sem_list[i].count=-1;
    nrk_sem_list[i].value=-1;
    nrk_sem_list[i].resource_ceiling=-1;
    //nrk_resource_count[i]=-1;
    //nrk_resource_value[i]=-1;
    //nrk_resource_ceiling[i]=-1;
    
}        
    for (i= 0; i<NRK_MAX_TASKS; i++)
	{
        nrk_task_TCB[i].task_prio = TCB_EMPTY_PRIO;
        nrk_task_TCB[i].task_ID = -1; 
        }
  
       
    // Setup a double linked list of Ready Tasks 
    for (i=0;i<NRK_MAX_TASKS;i++)
	{
		_nrk_readyQ[i].Next	=	&_nrk_readyQ[i+1];
		_nrk_readyQ[i+1].Prev	=	&_nrk_readyQ[i];
	}
	
	_nrk_readyQ[0].Prev	=	NULL;
	_nrk_readyQ[NRK_MAX_TASKS].Next	=	NULL;
	_head_node = NULL;
	_free_node = &_nrk_readyQ[0];
	
	
	

	nrk_task_set_entry_function( &IdleTask, nrk_idle_task);
	nrk_task_set_stk( &IdleTask, nrk_idle_task_stk, NRK_TASK_IDLE_STK_SIZE);
	nrk_idle_task_stk[0]=STK_CANARY_VAL;	
	//IdleTask.task_ID = NRK_IDLE_TASK_ID;
	IdleTask.prio = 0;
	IdleTask.offset.secs = 0;
	IdleTask.offset.nano_secs = 0;
	IdleTask.FirstActivation = TRUE;
	IdleTask.Type = IDLE_TASK;
	IdleTask.SchType = PREEMPTIVE;
	nrk_activate_task(&IdleTask);
	
}








void nrk_start (void)
{
	int8_t task_ID;
	uint8_t i,j;
//	NRK_STK *x;
//	unsigned char *stkc;

	/*
		- Get highest priority task from rdy list
		- set cur prio and start the task 
	*/
    // Check to make sure all tasks unique
    for(i=0; i<NRK_MAX_TASKS; i++ )
    {
	task_ID = nrk_task_TCB[i].task_ID;
	// only check activated tasks
	if(task_ID!=-1)
	{
    		for(j=0; j<NRK_MAX_TASKS; j++ )
		{
			if(i!=j && task_ID==nrk_task_TCB[j].task_ID)
			{
			nrk_kernel_error_add(NRK_DUP_TASK_ID,task_ID);

			}
		}
	}

    }

    task_ID = nrk_get_high_ready_task_ID();	
    nrk_high_ready_prio = nrk_task_TCB[task_ID].task_prio;
    nrk_high_ready_TCB = nrk_cur_task_TCB = &nrk_task_TCB[task_ID];           
    nrk_cur_task_prio = nrk_high_ready_prio;
		
//    nrk_stack_pointer_restore();

    /*
    #ifdef KERNEL_STK_ARRAY
     	stkc = (uint16_t*)&nrk_kernel_stk[NRK_KERNEL_STACKSIZE-1];
    #else
    	stkc = NRK_KERNEL_STK_TOP;
    #endif
    *stkc++ = (uint16_t)((uint16_t)_nrk_timer_tick>>8);
    *stkc = (uint16_t)((uint16_t)_nrk_timer_tick&0xFF); 
    
    //TODO: this way on msp
    // *stkc++ = (uint16_t)((uint16_t)_nrk_timer_tick&0xFF);
    // *stkc = (uint16_t)((uint16_t)_nrk_timer_tick>>8); 
*/
    nrk_target_start();
    nrk_stack_pointer_init(); 
    nrk_start_high_ready_task();	

    // you should never get here    
    while(1);
}


int8_t nrk_TCB_init (nrk_task_type *Task, NRK_STK *ptos, NRK_STK *pbos, uint16_t stk_size, void *pext, uint16_t opt)
{
	
    //  Already in critical section so no needenter critical section
    if(Task->Type!=IDLE_TASK)
    	Task->task_ID=nrk_task_init_cnt;
    else Task->task_ID=NRK_IDLE_TASK_ID;

    if(nrk_task_init_cnt>=NRK_MAX_TASKS) nrk_kernel_error_add(NRK_EXTRA_TASK,0);
    if(Task->Type!=IDLE_TASK) nrk_task_init_cnt++; 
    if(nrk_task_init_cnt==NRK_IDLE_TASK_ID) nrk_task_init_cnt++;
    //initialize member of TCB structure
    nrk_task_TCB[Task->task_ID].OSTaskStkPtr = ptos;
    nrk_task_TCB[Task->task_ID].task_prio = Task->prio;
    nrk_task_TCB[Task->task_ID].task_state = SUSPENDED;
    
    nrk_task_TCB[Task->task_ID].task_ID = Task->task_ID;
    nrk_task_TCB[Task->task_ID].suspend_flag = 0;
    nrk_task_TCB[Task->task_ID].period= _nrk_time_to_ticks( Task->period );
    nrk_task_TCB[Task->task_ID].next_wakeup= _nrk_time_to_ticks( Task->offset);
    nrk_task_TCB[Task->task_ID].next_period= nrk_task_TCB[Task->task_ID].period+nrk_task_TCB[Task->task_ID].next_wakeup;
    nrk_task_TCB[Task->task_ID].cpu_reserve= _nrk_time_to_ticks(Task->cpu_reserve);
    nrk_task_TCB[Task->task_ID].cpu_remaining = nrk_task_TCB[Task->task_ID].cpu_reserve;
    nrk_task_TCB[Task->task_ID].num_periods = 1;
    nrk_task_TCB[Task->task_ID].OSTCBStkBottom = pbos;
    nrk_task_TCB[Task->task_ID].errno= NRK_OK;
 
	
	         

			
    return NRK_OK;

}


/*
 * _nrk_timer_tick() 
 *
 * This function is called by the interrupt timer0.
 * It calls the scheduler.
 */
void _nrk_timer_tick(void)
{
	// want to do something before the scheduler gets called? 
	// Go ahead and put it here...

	_nrk_scheduler();

  	return;
}


uint16_t nrk_version (void)
{
    return (NRK_VERSION);
}


