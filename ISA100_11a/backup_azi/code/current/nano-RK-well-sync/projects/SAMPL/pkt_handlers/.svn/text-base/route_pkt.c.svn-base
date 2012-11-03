#include <globals.h>
#include <nrk.h>
#include <nrk_error.h>
#include <../include/sampl.h>
#include <route_pkt.h>
#include <ack_pkt.h>
#include <route_table.h>


int8_t route_generate(SAMPL_UPSTREAM_PKT_T *pkt,SAMPL_DOWNSTREAM_PKT_T *ds_pkt )
{
ACK_PKT_T p;
ROUTE_PKT_T r;
uint8_t num_pkts,i,selected;

  selected=0;
  num_pkts=ds_pkt->payload[0];
  for(i=0; i<num_pkts; i++ )
  {
  	route_pkt_get(&r, ds_pkt->payload, i);
	if(r.mac_addr==my_mac)
		{
		selected=1;
		route_table_set(r.dst_mac, r.next_hop_mac, r.value );
		}
  }

  if(selected)
  {
  // build ACK reply packet
  p.mac_addr=my_mac;
  pkt->payload_len = ping_pkt_add( &p, pkt->payload,0);
  pkt->num_msgs=1;
  pkt->pkt_type=ACK_PKT;
  } else
  {
    pkt->pkt_type = EMPTY_PKT;
    pkt->num_msgs = 0;
  }
return NRK_OK;  
}



void route_pkt_get( ROUTE_PKT_T *p, uint8_t *buf, uint8_t index )
{
   // 1 byte offset for number of messages
   p->mac_addr=buf[1+index*ROUTE_PKT_SIZE];
   p->dst_mac=buf[1+index*ROUTE_PKT_SIZE+1];
   p->next_hop_mac=buf[1+index*ROUTE_PKT_SIZE+2];
   p->value=buf[1+index*ROUTE_PKT_SIZE+3];
}


