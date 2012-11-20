#include <globals.h>
#include <sampl.h>
#include "ping_pkt.h"

#ifdef NANORK 
#include <nrk.h>
#include <nrk_error.h>
#else
#define my_mac 0
#define RF_MAX_PAYLOAD_SIZE  128
#endif

#define MAX_MSGS	50

int8_t ping_generate(SAMPL_UPSTREAM_PKT_T *pkt)
{
PING_PKT_T p;
  p.mac_addr=my_mac;
  pkt->payload_len = ping_pkt_add( &p, pkt->payload,0);
  pkt->num_msgs=1;
return NRK_OK;  
}

int8_t ping_p2p_generate(SAMPL_PEER_2_PEER_PKT_T *pkt)
{
PING_PKT_T p;
  p.mac_addr=my_mac;
  pkt->payload_len = ping_pkt_add( &p, pkt->payload,0);
return NRK_OK;  
}


int8_t ping_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out)
{
uint8_t len,i,j,k,dup;
PING_PKT_T p1, p2;

if(in->num_msgs>MAX_MSGS || out->num_msgs>MAX_MSGS )
{
#ifdef NANORK 
	nrk_kprintf( PSTR("MAX messages exceeded in aggregate!\r\n"));
#endif
	return;
}
for(i=0; i<in->num_msgs; i++ )
{
  dup=0;
  // get next ping packet to compare against current outgoing list
  ping_pkt_get( &p1, in->payload, i );
  for(k=0; k<out->num_msgs; k++ )
	{
	// get packet from outgoing list and compare against incomming packet
  	ping_pkt_get( &p2, out->payload, k );
	if(p1.mac_addr==p2.mac_addr) dup=1;	
	}
   if(dup==0)
	{
		// if packet is unique, add to outgoing packet
		out->payload_len=ping_pkt_add( &p1, out->payload, out->num_msgs );
		out->num_msgs++;
	}
}
return NRK_OK;
}

void ping_pkt_get( PING_PKT_T *p, uint8_t *buf, uint8_t index )
{
   p->mac_addr=buf[index*PING_PKT_SIZE];
}


uint8_t ping_pkt_add( PING_PKT_T *p, uint8_t *buf, uint8_t index )
{
   if(index*PING_PKT_SIZE>RF_MAX_PAYLOAD_SIZE) return (index*PING_PKT_SIZE);
   buf[index*PING_PKT_SIZE]= p->mac_addr;
   return ((index+1)*PING_PKT_SIZE);
}

