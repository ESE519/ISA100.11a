#ifndef _TRACE_PKT_H
#define _TRACE_PKT_H

#include <nrk.h>
#include <../include/sampl.h>


//#define EEPROM_STORAGE_PKT_SIZE	7
#define EEPROM_APP_PROTECTION_BOUNDARY  0x100
#define EE_READ		0
#define EE_WRITE 	1
#define EE_REPLY	2
#define EE_ERROR	3

#define EE_PAYLOAD_START_INDEX	5

typedef struct eeprom_pkt 
{
	uint8_t mode;   // Byte 0
	uint16_t addr;      // Byte 1,2
	int8_t data_len;     // Byte 3
	uint8_t mac;	     // Byte 4
	int8_t *eeprom_payload;     // starting Byte 5 
} EEPROM_STORAGE_PKT_T;



int8_t eeprom_storage_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out);
int8_t eeprom_storage_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt);
uint8_t eeprom_storage_pkt_add( EEPROM_STORAGE_PKT_T *p,char *buf);
void  eeprom_storage_pkt_get( EEPROM_STORAGE_PKT_T *p, char *buf);

#endif
