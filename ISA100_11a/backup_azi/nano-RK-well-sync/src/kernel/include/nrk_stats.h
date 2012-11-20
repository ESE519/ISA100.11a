#ifndef NRK_STATS_H
#define NRK_STATS_H
#include <nrk_cfg.h>
#include <nrk_time.h>

#ifdef NRK_STATS_TRACKER
typedef struct task_stat {
	uint32_t total_ticks;
	uint32_t min_exec_ticks;
	uint32_t max_exec_ticks;
	uint32_t last_exec_ticks;
	uint32_t swapped_in;
	uint32_t cur_ticks;
	uint32_t preempted;
	uint8_t violations;
	uint8_t overflow;
} nrk_task_stat_t;

nrk_task_stat_t cur_task_stats[NRK_MAX_TASKS];

nrk_time_t _nrk_stats_sleep_time;

void nrk_stats_reset();
void _nrk_stats_sleep(uint8_t t);
void _nrk_stats_add_violation(uint8_t task_id);
void _nrk_stats_task_start(uint8_t task_id);
void _nrk_stats_task_preempted(uint8_t task_id, uint8_t ticks);
void _nrk_stats_task_suspend(uint8_t task_id, uint8_t ticks);
void nrk_stats_display_all();
void nrk_stats_display_pid(uint8_t pid);
int8_t nrk_stats_get(uint8_t pid, nrk_task_stat_t *t);
void nrk_stats_get_deep_sleep(nrk_time_t *t);






#endif

#endif
