#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdint.h>
#include <nrk_time.h>


typedef struct debug_data
{
   uint16_t rx_pkts;
   uint16_t tx_pkts;
   uint16_t tx_retry;
   uint16_t sensor_samples;
   nrk_time_t uptime;
   nrk_time_t deep_sleep;
   nrk_time_t idle_time;
} DEBUG_DATA_T;

DEBUG_DATA_T debug_stats;
void debug_reset();
void debug_update();


#endif
