#include <globals.h>
#include <../include/sampl.h>
#include <neighbor_pkt.h>

#ifdef NANORK 
#include <nrk.h>
#include <nrk_error.h>
#include <debug.h>
#include <neighbor_list.h>
#else
#define my_mac 0
#define RF_MAX_PAYLOAD_SIZE  128
#endif



#ifdef NANORK
int8_t nlist_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt)
{
NLIST_PKT_T p;
int8_t i,cnt;

  neighbor_list_ttl_update();
  cnt=0;
  i=0;

  do 
  {
  p.mac_addr=my_mac;
  i=neighbor_list_get_next_valid_index(i);
  if(i==NRK_ERROR) break;
  p.neighbor_mac=neighbor_list_get_mac(i);
  p.rssi=neighbor_list_get_rssi(i);
  i++;
  pkt->payload_len = nlist_pkt_add( &p, pkt->payload,cnt);
  cnt++;
  pkt->num_msgs=cnt;
  } while(i!=NRK_ERROR);

  if(cnt==0) 
	  {
		// This should never happen, but if it does...
    		pkt->pkt_type = EMPTY_PKT;
    		pkt->num_msgs = 0;
	  }
return NRK_OK;  
}

int8_t nlist_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out)
{
uint8_t len,i,j,k,dup;
NLIST_PKT_T p1, p2;

//if(in->next_hop_dst_mac!=my_mac ) nrk_kprintf( PSTR( "aggregating bad packet!\r\n" ));
for(i=0; i<in->num_msgs; i++ )
{
  dup=0;
  // get next ping packet to compare against current outgoing list
  nlist_pkt_get( &p1, in->payload, i );
  for(k=0; k<out->num_msgs; k++ )
	{
	// get packet from outgoing list and compare against incomming packet
  	nlist_pkt_get( &p2, out->payload, k );
	if(p1.mac_addr==p2.mac_addr ) dup=1;	
	}
   if(dup==0)
	{
		// if packet is unique, add to outgoing packet
		out->payload_len=nlist_pkt_add( &p1, out->payload, out->num_msgs );
		out->num_msgs++;
	}
}
return NRK_OK;
}
#endif

void nlist_pkt_get( NLIST_PKT_T *p, uint8_t *buf, uint8_t index )
{
   p->mac_addr=buf[index*NLIST_PKT_SIZE];
   p->neighbor_mac=buf[index*NLIST_PKT_SIZE+1];
   p->rssi=buf[index*NLIST_PKT_SIZE+2];
}


uint8_t nlist_pkt_add( NLIST_PKT_T *p, uint8_t *buf, uint8_t index )
{
   if((index+1)*NLIST_PKT_SIZE>MAX_PKT_PAYLOAD ) return (index*NLIST_PKT_SIZE);
   buf[index*NLIST_PKT_SIZE]= p->mac_addr;
   buf[index*NLIST_PKT_SIZE+1]= p->neighbor_mac;
   buf[index*NLIST_PKT_SIZE+2]= p->rssi;
   return ((index+1)*NLIST_PKT_SIZE);
}

