#ifndef _PING_PKT_H
#define _PING_PKT_H

#include <sampl.h>


#define PING_PKT_SIZE	1

typedef struct ping_pkt 
{
	uint8_t mac_addr;   // Byte 0
} PING_PKT_T;



int8_t ping_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out);
int8_t ping_generate(SAMPL_UPSTREAM_PKT_T *pkt);
uint8_t ping_pkt_add( PING_PKT_T *p, uint8_t *buf, uint8_t index );
void  ping_pkt_get( PING_PKT_T *p, uint8_t *buf, uint8_t index );
int8_t ping_p2p_generate(SAMPL_PEER_2_PEER_PKT_T *pkt);

#endif
