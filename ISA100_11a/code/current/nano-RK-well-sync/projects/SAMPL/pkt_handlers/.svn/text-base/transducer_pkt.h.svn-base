#ifndef _TRANSDUCER_PKT_H
#define _TRANSDUCER_PKT_H

#include <../include/sampl.h>


#define TRANSDUCER_REPLY_HEADER_SIZE	3
#define TRANSDUCER_ELEMENT_SIZE		3

#define GLOBAL_DEBUG_MASK	0x01	

typedef struct transducer_msg
{
	uint8_t mac_addr;   
	uint8_t key;   
	uint8_t value; 
} TRANSDUCER_MSG_T;

typedef struct transducer_cmd_pkt 
{
	uint8_t checksum;   // Byte 0 
	uint8_t num_msgs;  // Byte 1
	TRANSDUCER_MSG_T *msg;   
} TRANSDUCER_CMD_PKT_T;


typedef struct transducer_reply_pkt 
{
	uint8_t mac_addr;  
	uint8_t type;  
	uint8_t len;  
	uint8_t *payload;   
} TRANSDUCER_REPLY_PKT_T;



int8_t transducer_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out);
int8_t transducer_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt);
// This function returns a computed checksum to compare against the normal checksum
uint8_t transducer_cmd_pkt_get( TRANSDUCER_CMD_PKT_T *p, uint8_t *buf);
uint8_t transducer_cmd_pkt_add( TRANSDUCER_CMD_PKT_T *p, uint8_t *buf);
uint8_t transducer_reply_pkt_add( TRANSDUCER_REPLY_PKT_T *p, uint8_t *buf, uint8_t index );
uint8_t transducer_reply_pkt_get( TRANSDUCER_REPLY_PKT_T *p, uint8_t *buf, uint8_t index );
uint8_t transducer_cmd_pkt_checksum( TRANSDUCER_CMD_PKT_T *p, uint8_t *buf);

#endif
