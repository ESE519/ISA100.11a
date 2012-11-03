#ifndef _CONTROL_PKT_H
#define _CONTROL_PKT_H

#include <../include/sampl.h>


#define CONTROL_PKT_SIZE	7

#define GLOBAL_DEBUG_MASK	0x01	

typedef struct control_pkt 
{
	uint8_t cca_threshold;  // Byte 0
	uint8_t ctrl_flags_1;   // Byte 1
	uint16_t mobile_reserve_cnt;   // Byte 2 and 3
	uint16_t mobile_reserve_seconds;   // Byte 4 and 5
	uint8_t checksum;   // Byte 4 and 5
} CONTROL_PKT_T;



int8_t control_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt);
// This function returns a computed checksum to compare against the normal checksum
uint8_t control_pkt_get( CONTROL_PKT_T *p, uint8_t *buf, uint8_t index );

#endif
