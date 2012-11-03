#ifndef _STATS_PKT_H
#define _STATS_PKT_H

#ifdef NANORK
#include <nrk.h>
#endif

#include <../include/sampl.h>

#define STATS_PKT_SIZE	21	

typedef struct stats_pkt 
{
   uint8_t mac_addr;   
   uint16_t rx_pkts;
   uint16_t tx_pkts;
   uint16_t tx_retry;
   uint16_t sensor_samples;
   uint32_t uptime;
   uint32_t deep_sleep;
   uint32_t idle_time;
} STATS_PKT_T;



int8_t stats_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out);
int8_t stats_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt);
uint8_t stats_pkt_add( STATS_PKT_T *p, uint8_t *buf, uint8_t index );
void  stats_pkt_get( STATS_PKT_T *p, uint8_t *buf, uint8_t index );

#endif
