#ifndef _ACK_PKT_H
#define _ACK_PKT_H

#include <../include/sampl.h>


#define ACK_PKT_SIZE	1

typedef struct ack_pkt 
{
	uint8_t mac_addr;   // Byte 0
} ACK_PKT_T;



int8_t ack_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out);
int8_t ack_generate(SAMPL_UPSTREAM_PKT_T *pkt);
uint8_t ack_pkt_add( ACK_PKT_T *p, uint8_t *buf, uint8_t index );
void  ack_pkt_get( ACK_PKT_T *p, uint8_t *buf, uint8_t index );

#endif
