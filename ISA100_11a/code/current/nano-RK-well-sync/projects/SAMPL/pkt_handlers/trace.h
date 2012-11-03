#ifndef _TRACE_PKT_H
#define _TRACE_PKT_H

#include <nrk.h>
#include <../include/sampl.h>


#define TRACE_PKT_SIZE	4

typedef struct trace_pkt 
{
	uint8_t mac_addr;   // Byte 0
	int8_t parent_mac;      // Byte 1
	int8_t ds_rssi;     // Byte 2
	int8_t us_rssi;     // Byte 4
} TRACE_PKT_T;



int8_t trace_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out);
int8_t trace_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt);
uint8_t trace_pkt_add( TRACE_PKT_T *p, uint8_t *buf, uint8_t index );
void  trace_pkt_get( TRACE_PKT_T *p, uint8_t *buf, uint8_t index );

#endif
