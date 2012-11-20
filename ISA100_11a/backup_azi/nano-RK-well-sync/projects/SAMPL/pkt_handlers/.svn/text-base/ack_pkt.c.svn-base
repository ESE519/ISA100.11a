#include <globals.h>
#include <sampl.h>
#include <ack_pkt.h>

#ifdef NANORK 
#include <nrk.h>
#include <nrk_error.h>
#else
#define my_mac 0
#define RF_MAX_PAYLOAD_SIZE  128
#endif



int8_t ack_generate(SAMPL_UPSTREAM_PKT_T *pkt)
{
ACK_PKT_T p;
  p.mac_addr=my_mac;
  pkt->payload_len = ack_pkt_add( &p, pkt->payload,0);
  pkt->num_msgs=1;
  pkt->pkt_type=ACK_PKT;
return NRK_OK;  
}

int8_t ack_p2p_generate(SAMPL_PEER_2_PEER_PKT_T *pkt)
{
ACK_PKT_T p;
  p.mac_addr=my_mac;
  pkt->payload_len = ack_pkt_add( &p, pkt->payload,0);
return NRK_OK;  
}


int8_t ack_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out)
{
uint8_t len,i,j,k,dup;
ACK_PKT_T p1, p2;

for(i=0; i<in->num_msgs; i++ )
{
  dup=0;
  // get next ping packet to compare against current outgoing list
  ack_pkt_get( &p1, in->payload, i );
  for(k=0; k<out->num_msgs; k++ )
	{
	// get packet from outgoing list and compare against incomming packet
  	ack_pkt_get( &p2, out->payload, k );
	if(p1.mac_addr==p2.mac_addr) dup=1;	
	}
   if(dup==0)
	{
		// if packet is unique, add to outgoing packet
		out->payload_len=ack_pkt_add( &p1, out->payload, out->num_msgs );
		out->num_msgs++;
	}
}
return NRK_OK;
}

void ack_pkt_get( ACK_PKT_T *p, uint8_t *buf, uint8_t index )
{
   p->mac_addr=buf[index*ACK_PKT_SIZE];
}


uint8_t ack_pkt_add( ACK_PKT_T *p, uint8_t *buf, uint8_t index )
{
   buf[index*ACK_PKT_SIZE]= p->mac_addr;
   return ((index+1)*ACK_PKT_SIZE);
}

