#include <nrk.h>
#include <nrk_stats.h>
#include <nrk_time.h>
#include <nrk_defs.h>
#include <nrk_error.h>
#include <stdio.h>

#ifdef NRK_STATS_TRACKER
void nrk_stats_reset()
{
uint8_t i;

_nrk_stats_sleep_time.secs=0;
_nrk_stats_sleep_time.nano_secs=0;
for(i=0; i<NRK_MAX_TASKS; i++ )
	{
	cur_task_stats[i].total_ticks=0;
	cur_task_stats[i].max_exec_ticks=0;
	cur_task_stats[i].min_exec_ticks=0;
	cur_task_stats[i].last_exec_ticks=0;
	cur_task_stats[i].swapped_in=0;
	cur_task_stats[i].preempted=0;
	cur_task_stats[i].violations=0;
	cur_task_stats[i].overflow=0;
	}

}


void _nrk_stats_sleep(uint8_t t)
{
//_nrk_stats_sleep_time+=t;
_nrk_stats_sleep_time.nano_secs+=t*NANOS_PER_TICK;
nrk_time_compact_nanos(&_nrk_stats_sleep_time);
}

void nrk_stats_get_deep_sleep(nrk_time_t *t)
{
t->secs=_nrk_stats_sleep_time.secs;
t->nano_secs=_nrk_stats_sleep_time.nano_secs;
}

void _nrk_stats_add_violation(uint8_t task_id)
{
//if( cur_task_stats[task_id].overflow==1) return;
cur_task_stats[task_id].violations++;
if(cur_task_stats[task_id].violations==255) cur_task_stats[task_id].overflow=1;
}


// task_id is the PID of the task in question
void _nrk_stats_task_start(uint8_t task_id)
{
// if( cur_task_stats[task_id].overflow==1) return;
cur_task_stats[task_id].cur_ticks=0;
cur_task_stats[task_id].swapped_in++;
if(cur_task_stats[task_id].swapped_in==255) cur_task_stats[task_id].overflow=1;
}


void _nrk_stats_task_preempted(uint8_t task_id, uint8_t ticks)
{
// if( cur_task_stats[task_id].overflow==1) return;
cur_task_stats[task_id].preempted++;
cur_task_stats[task_id].cur_ticks+=ticks;
cur_task_stats[task_id].total_ticks+=ticks;
}

void _nrk_stats_task_suspend(uint8_t task_id, uint8_t ticks)
{
if( cur_task_stats[task_id].overflow==1) return;
cur_task_stats[task_id].last_exec_ticks = cur_task_stats[task_id].cur_ticks+ticks;
cur_task_stats[task_id].total_ticks+=ticks;

if(cur_task_stats[task_id].min_exec_ticks==0 || cur_task_stats[task_id].last_exec_ticks<cur_task_stats[task_id].min_exec_ticks) 
	cur_task_stats[task_id].min_exec_ticks=cur_task_stats[task_id].last_exec_ticks;

if(cur_task_stats[task_id].last_exec_ticks>cur_task_stats[task_id].max_exec_ticks)
	cur_task_stats[task_id].max_exec_ticks=cur_task_stats[task_id].last_exec_ticks;

}



void nrk_stats_display_pid(uint8_t pid)
{
nrk_time_t t;

	nrk_kprintf( PSTR( " Task ID: "));
	printf( "%d",pid );
if(pid==NRK_IDLE_TASK_ID)
   {
	nrk_kprintf( PSTR( "\r\n   Total System Uptime: "));
	nrk_time_get(&t);	
	printf( "%lu secs %lu ms", t.secs, t.nano_secs/NANOS_PER_MS );
	nrk_kprintf( PSTR( "\r\n   Idle Task Deep Sleep Time: "));
	//t=_nrk_ticks_to_time(_nrk_stats_sleep_time);
	//printf( "%lu secs %lu ms", t.secs, t.nano_secs/NANOS_PER_MS );
	printf( "%lu secs %lu ms", _nrk_stats_sleep_time.secs, _nrk_stats_sleep_time.nano_secs/NANOS_PER_MS);
   } 
	nrk_kprintf( PSTR( "\r\n   Total CPU: "));
	t=_nrk_ticks_to_time(cur_task_stats[pid].total_ticks);
	printf( "%lu secs %lu ms", t.secs, t.nano_secs/NANOS_PER_MS );
	nrk_kprintf( PSTR( "\r\n   Time [Min,Last,Max]: "));
	t=_nrk_ticks_to_time(cur_task_stats[pid].min_exec_ticks);
	printf( "%lu secs %lu ms, ", t.secs, t.nano_secs/NANOS_PER_MS );
	t=_nrk_ticks_to_time(cur_task_stats[pid].last_exec_ticks);
	printf( "%lu secs %lu ms, ", t.secs, t.nano_secs/NANOS_PER_MS );
	t=_nrk_ticks_to_time(cur_task_stats[pid].max_exec_ticks);
	printf( "%lu secs %lu ms", t.secs, t.nano_secs/NANOS_PER_MS );
	nrk_kprintf( PSTR( "\r\n   Swap-ins: "));
	printf( "%lu",cur_task_stats[pid].swapped_in );
	nrk_kprintf( PSTR( "\r\n   Preemptions: "));
	printf( "%lu",cur_task_stats[pid].preempted);
	nrk_kprintf( PSTR( "\r\n   Kernel Violations: "));
	printf( "%u",cur_task_stats[pid].violations);
	nrk_kprintf( PSTR( "\r\n   Overflow Error Status: "));
	printf( "%u",cur_task_stats[pid].overflow);
	nrk_kprintf( PSTR("\r\n") );

}


void nrk_stats_display_all()
{
uint8_t i;
nrk_kprintf( PSTR( "\r\nNano-RK Task Statistics:\r\n" ));

for(i=0; i<NRK_MAX_TASKS; i++ )
	nrk_stats_display_pid(i);
}


int8_t nrk_stats_get(uint8_t pid, nrk_task_stat_t *t)
{
if(pid>=NRK_MAX_TASKS) return NRK_ERROR;

t->total_ticks=cur_task_stats[pid].total_ticks;
t->min_exec_ticks=cur_task_stats[pid].min_exec_ticks;
t->max_exec_ticks=cur_task_stats[pid].max_exec_ticks;
t->last_exec_ticks=cur_task_stats[pid].last_exec_ticks;
t->swapped_in=cur_task_stats[pid].swapped_in;
t->cur_ticks=cur_task_stats[pid].cur_ticks;
t->preempted=cur_task_stats[pid].preempted;
t->violations=cur_task_stats[pid].violations;
t->overflow=cur_task_stats[pid].overflow;

return NRK_OK;
}


#endif
