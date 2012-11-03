#include <debug.h>
#include <nrk.h>
#include <nrk_time.h>
#include <nrk_stats.h>


nrk_task_stat_t t_stat;
nrk_time_t t;

void debug_reset()
{
debug_stats.rx_pkts=0;
debug_stats.tx_pkts=0;
debug_stats.tx_retry=0;
debug_stats.sensor_samples=0;
}


void debug_update()
{

nrk_time_get(&t);
debug_stats.uptime.secs=t.secs;
debug_stats.uptime.nano_secs=t.nano_secs;

nrk_stats_get_deep_sleep(&t);
debug_stats.deep_sleep.secs=t.secs;
debug_stats.deep_sleep.nano_secs=t.nano_secs;

nrk_stats_get(0, &t_stat);
t=_nrk_ticks_to_time(t_stat.total_ticks);
debug_stats.idle_time.secs=t.secs;
debug_stats.idle_time.nano_secs=t.nano_secs;
}

