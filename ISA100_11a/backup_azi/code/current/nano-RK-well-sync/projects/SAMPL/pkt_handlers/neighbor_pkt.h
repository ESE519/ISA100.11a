#ifndef _NLIST_PKT_H
#define _NLIST_PKT_H

#ifdef NANORK
#include <nrk.h>
#endif

#include <../include/sampl.h>

#define NLIST_PKT_SIZE	3	

typedef struct nlist_pkt 
{
   uint8_t mac_addr;   
   uint8_t neighbor_mac;
   int8_t rssi;
} NLIST_PKT_T;



int8_t nlist_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out);
int8_t nlist_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt);
uint8_t nlist_pkt_add( NLIST_PKT_T *p, uint8_t *buf, uint8_t index );
void  nlist_pkt_get( NLIST_PKT_T *p, uint8_t *buf, uint8_t index );

#endif
