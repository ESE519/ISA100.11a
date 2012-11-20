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

#include <nrk.h>
#include <nrk_events.h>
#include <nrk_task.h>
#include <nrk_error.h>
#include <nrk_scheduler.h>
#include <include.h>
#include <ulib.h>
#include <nrk_timer.h>
#include <nrk_time.h>
#include <nrk_cfg.h>
#include <nrk_cpu.h>
#include <nrk_defs.h>

int8_t nrk_signal_create()
{
	uint8_t i=0;
	for(i=0;i<32;i++)   
	{                         
		if( !(_nrk_signal_list & SIG(i)))
		{    
			_nrk_signal_list|=SIG(i);
			return i;
		}
	}
	return NRK_ERROR;


}

uint32_t nrk_signal_get_registered_mask()
{
        return nrk_cur_task_TCB->registered_signal_mask;
}

//return the number removed from signal set
int8_t nrk_signal_delete(nrk_sig_t sig_id)
{
	uint8_t task_ID;
	uint32_t sig_mask;

	sig_mask=SIG(sig_id);

	if( (sig_mask & _nrk_signal_list)==0) return NRK_ERROR; 

	nrk_int_disable();
	for (task_ID=0; task_ID < NRK_MAX_TASKS; task_ID++){
		if(nrk_task_TCB[task_ID].task_ID==-1) continue;
		// Check for tasks waiting on the signal
		// If there is a task that is waiting on just this signal
		// then we need to change it to the normal SUSPEND state
		if(nrk_task_TCB[task_ID].registered_signal_mask==sig_mask) //check to make sure its only signal its waiting on 
		{
		      //  printf("delete t(%i) signal(%li)\r\n",task_ID,nrk_task_TCB[task_ID].registered_signal_mask);
			nrk_task_TCB[task_ID].active_signal_mask=0;
			nrk_task_TCB[task_ID].event_suspend=0;
			nrk_task_TCB[task_ID].task_state=SUSPENDED;
		}
		nrk_task_TCB[task_ID].registered_signal_mask&=~sig_mask; //cheaper to remove than do a check
		nrk_task_TCB[task_ID].active_signal_mask&=~sig_mask; //cheaper to remove than do a check

	}
	
	_nrk_signal_list&=~SIG(sig_id);
	nrk_int_enable();

	return NRK_OK;
}


int8_t nrk_signal_unregister(int8_t sig_id)
{
uint32_t sig_mask;

sig_mask=SIG(sig_id);

	if(nrk_cur_task_TCB->registered_signal_mask & sig_mask)
	{
		nrk_cur_task_TCB->registered_signal_mask&=~(sig_mask); 	
		nrk_cur_task_TCB->active_signal_mask&=~(sig_mask); 	
	}
	else
		return NRK_ERROR;
return NRK_OK;
}

int8_t nrk_signal_register(int8_t sig_id)
{

	// Make sure the signal was created...
	if(SIG(sig_id) & _nrk_signal_list )
	{
		nrk_cur_task_TCB->registered_signal_mask|=SIG(sig_id); 	
		return NRK_OK;
	}
            
	return NRK_ERROR;
}

int8_t nrk_event_signal(int8_t sig_id)
{

	uint8_t task_ID;
	uint8_t event_occured=0;
	uint32_t sig_mask;

	sig_mask=SIG(sig_id);
	// Check if signal was created
	// Signal was not created
	if((sig_mask & _nrk_signal_list)==0 ) { _nrk_errno_set(1); return NRK_ERROR;}
	
	//needs to be atomic otherwise run the risk of multiple tasks being scheduled late and not in order of priority.  
	nrk_int_disable();
	for (task_ID=0; task_ID < NRK_MAX_TASKS; task_ID++){


	//	if (nrk_task_TCB[task_ID].task_state == EVENT_SUSPENDED)   
	//	{
	//	printf( "task %d is event suspended\r\n",task_ID );
			if(nrk_task_TCB[task_ID].event_suspend==SIG_EVENT_SUSPENDED)
				if((nrk_task_TCB[task_ID].active_signal_mask & sig_mask))
				{
					nrk_task_TCB[task_ID].task_state=SUSPENDED;
					nrk_task_TCB[task_ID].next_wakeup=0;
					nrk_task_TCB[task_ID].event_suspend=0;
					// Add the event trigger here so it is returned
					// from nrk_event_wait()
					nrk_task_TCB[task_ID].active_signal_mask=sig_mask;
					event_occured=1;
				}

			if(nrk_task_TCB[task_ID].event_suspend==RSRC_EVENT_SUSPENDED)
				if((nrk_task_TCB[task_ID].active_signal_mask == sig_mask))
				{
					nrk_task_TCB[task_ID].task_state=SUSPENDED;
					nrk_task_TCB[task_ID].next_wakeup=0;
					nrk_task_TCB[task_ID].event_suspend=0;
					// Add the event trigger here so it is returned
					// from nrk_event_wait()
					nrk_task_TCB[task_ID].active_signal_mask=0;
					event_occured=1;
				}   

	//	}
	}
	nrk_int_enable();
	if(event_occured)
	{
		return NRK_OK;
	} 
	// No task was waiting on the signal
	_nrk_errno_set(2);
	return NRK_ERROR;
}


uint32_t nrk_event_wait(uint32_t event_mask)
{

	// FIXME: Should go through list and check that all masks are registered, not just 1
	if(event_mask &  nrk_cur_task_TCB->registered_signal_mask)
	  {
	   nrk_cur_task_TCB->active_signal_mask=event_mask; 
	   nrk_cur_task_TCB->event_suspend=SIG_EVENT_SUSPENDED; 
	  }
	else
	  {
	   return 0;
	  }

	if(event_mask & SIG(nrk_wakeup_signal))
		nrk_wait_until_nw();
	else
		nrk_wait_until_ticks(0);
	//unmask the signal when its return so it has logical value like 1 to or whatever was user defined
	return ( (nrk_cur_task_TCB->active_signal_mask));
}

int8_t nrk_sem_query(nrk_sem_t *rsrc )
{
	int8_t id;
	id=nrk_get_resource_index(rsrc);  
	if(id==-1) { _nrk_errno_set(1); return NRK_ERROR;}
	if(id==NRK_MAX_RESOURCE_CNT) { _nrk_errno_set(2); return NRK_ERROR; }
	
	return(nrk_sem_list[id].value);
}



int8_t nrk_sem_pend(nrk_sem_t *rsrc )
{
	int8_t id;
	id=nrk_get_resource_index(rsrc);  
	if(id==-1) { _nrk_errno_set(1); return NRK_ERROR;}
	if(id==NRK_MAX_RESOURCE_CNT) { _nrk_errno_set(2); return NRK_ERROR; }
	
	nrk_int_disable();
	if(nrk_sem_list[id].value==0)
	{
		nrk_cur_task_TCB->event_suspend|=RSRC_EVENT_SUSPENDED;
		nrk_cur_task_TCB->active_signal_mask=id;
		// Wait on suspend event
		nrk_int_enable();
		nrk_wait_until_ticks(0);
	}

	nrk_sem_list[id].value--;	
	nrk_cur_task_TCB->task_prio_ceil=nrk_sem_list[id].resource_ceiling;
	nrk_cur_task_TCB->elevated_prio_flag=1;
	nrk_int_enable();

	return NRK_OK;
}



int8_t nrk_sem_post(nrk_sem_t *rsrc)
{
	int8_t id=nrk_get_resource_index(rsrc);	
	int8_t task_ID;
	if(id==-1) { _nrk_errno_set(1); return NRK_ERROR;}
	if(id==NRK_MAX_RESOURCE_CNT) { _nrk_errno_set(2); return NRK_ERROR; }

	if(nrk_sem_list[id].value<nrk_sem_list[id].count)
	{
		// Signal RSRC Event		
		nrk_int_disable();

		nrk_sem_list[id].value++;
		nrk_cur_task_TCB->elevated_prio_flag=0;

		for (task_ID=0; task_ID < NRK_MAX_TASKS; task_ID++){
			if(nrk_task_TCB[task_ID].event_suspend==RSRC_EVENT_SUSPENDED)
				if((nrk_task_TCB[task_ID].active_signal_mask == id))
				{
					nrk_task_TCB[task_ID].task_state=SUSPENDED;
					nrk_task_TCB[task_ID].next_wakeup=0;
					nrk_task_TCB[task_ID].event_suspend=0;
					nrk_task_TCB[task_ID].active_signal_mask=0;
				}   

		}
		nrk_int_enable();
	}
		
return NRK_OK;
}

int8_t  nrk_sem_delete(nrk_sem_t *rsrc)
{
int8_t id=nrk_get_resource_index(rsrc);	
	int8_t task_ID;
	if(id==-1) { _nrk_errno_set(1); return NRK_ERROR;}
	if(id==NRK_MAX_RESOURCE_CNT) { _nrk_errno_set(2); return NRK_ERROR; }

	nrk_sem_list[id].count=-1;
	nrk_sem_list[id].value=-1;
	nrk_sem_list[id].resource_ceiling=-1;
	_nrk_resource_cnt--;
return NRK_OK;
}

nrk_sem_t* nrk_sem_create(uint8_t count,uint8_t ceiling_prio)
{
uint8_t i;
	if(_nrk_resource_cnt>=(NRK_MAX_RESOURCE_CNT-1))
		return NULL;  
	for(i=0; i<NRK_MAX_RESOURCE_CNT; i++ )
		{
		   if(nrk_sem_list[i].count==-1) break;
		}
	                                              
	nrk_sem_list[i].value=count;
	nrk_sem_list[i].count=count;
	nrk_sem_list[i].resource_ceiling=ceiling_prio;
	_nrk_resource_cnt++;
	return	&nrk_sem_list[i];
}

int8_t nrk_get_resource_index(nrk_sem_t *resrc)
{
	int8_t id;
		for(id=0;id<NRK_MAX_RESOURCE_CNT;id++)
			if((nrk_sem_t *)(&nrk_sem_list[id])==(nrk_sem_t*)resrc)
				return id;
	return NRK_ERROR;
}




