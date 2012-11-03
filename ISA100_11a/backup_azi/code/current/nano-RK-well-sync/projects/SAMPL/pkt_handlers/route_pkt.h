#ifndef _ROUTE_PKT_H
#define _ROUTE_PKT_H

#include <nrk.h>
#include <../include/sampl.h>


#define ROUTE_PKT_SIZE	4

typedef struct route_pkt 
{
	uint8_t mac_addr;   	// Byte 0
	uint8_t dst_mac;   	// Byte 1
	uint8_t next_hop_mac;   // Byte 2
	uint8_t value;   	// Byte 3
} ROUTE_PKT_T;



int8_t route_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt);
void  route_pkt_get( ROUTE_PKT_T *p, uint8_t *buf, uint8_t index );

#endif
