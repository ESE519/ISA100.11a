#include <globals.h>
#include <../include/sampl.h>
#include <stats_pkt.h>

#ifdef NANORK 
#include <nrk.h>
#include <nrk_error.h>
#include <debug.h>
#else
#define my_mac 0
#define RF_MAX_PAYLOAD_SIZE  128
#endif



#ifdef NANORK
int8_t stats_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt)
{
STATS_PKT_T p;
  debug_update(); 
  p.mac_addr=my_mac;
  p.mac_addr=my_mac;
  p.rx_pkts=debug_stats.rx_pkts;
  p.tx_pkts=debug_stats.tx_pkts;
  p.tx_retry=debug_stats.tx_retry;
  p.sensor_samples=debug_stats.sensor_samples;
  p.uptime=debug_stats.uptime.secs;
  p.deep_sleep=debug_stats.deep_sleep.secs;
  p.idle_time=debug_stats.idle_time.secs;
  pkt->payload_len = stats_pkt_add( &p, pkt->payload,0);
  pkt->num_msgs=1;
return NRK_OK;  
}

int8_t stats_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out)
{
uint8_t len,i,j,k,dup;
STATS_PKT_T p1, p2;

//if(in->next_hop_dst_mac!=my_mac ) nrk_kprintf( PSTR( "aggregating bad packet!\r\n" ));
for(i=0; i<in->num_msgs; i++ )
{
  dup=0;
  // get next ping packet to compare against current outgoing list
  stats_pkt_get( &p1, in->payload, i );
  for(k=0; k<out->num_msgs; k++ )
	{
	// get packet from outgoing list and compare against incomming packet
  	stats_pkt_get( &p2, out->payload, k );
	if(p1.mac_addr==p2.mac_addr ) dup=1;	
	}
   if(dup==0)
	{
		// if packet is unique, add to outgoing packet
		out->payload_len=stats_pkt_add( &p1, out->payload, out->num_msgs );
		out->num_msgs++;
	}
}
return NRK_OK;
}
#endif

void stats_pkt_get( STATS_PKT_T *p, uint8_t *buf, uint8_t index )
{
   p->mac_addr=buf[index*STATS_PKT_SIZE];
   p->rx_pkts=((uint16_t)buf[index*STATS_PKT_SIZE+1]<<8)|buf[index*STATS_PKT_SIZE+2];
   p->tx_pkts=((uint16_t)buf[index*STATS_PKT_SIZE+3]<<8)|buf[index*STATS_PKT_SIZE+4];
   p->tx_retry=((uint16_t)buf[index*STATS_PKT_SIZE+5]<<8)|buf[index*STATS_PKT_SIZE+6];
   p->sensor_samples=((uint16_t)buf[index*STATS_PKT_SIZE+7]<<8)|buf[index*STATS_PKT_SIZE+8];


   p->uptime=((uint32_t)buf[index*STATS_PKT_SIZE+9]<<24)|
   	     ((uint32_t)buf[index*STATS_PKT_SIZE+10]<<16)| 
   	     ((uint32_t)buf[index*STATS_PKT_SIZE+11]<<8)| 
   	     ((uint32_t)buf[index*STATS_PKT_SIZE+12]);

   p->deep_sleep=((uint32_t)buf[index*STATS_PKT_SIZE+13]<<24)|
   	     ((uint32_t)buf[index*STATS_PKT_SIZE+14]<<16)| 
   	     ((uint32_t)buf[index*STATS_PKT_SIZE+15]<<8)| 
   	     ((uint32_t)buf[index*STATS_PKT_SIZE+16]);

   p->idle_time=((uint32_t)buf[index*STATS_PKT_SIZE+17]<<24)|
   	     ((uint32_t)buf[index*STATS_PKT_SIZE+18]<<16)| 
   	     ((uint32_t)buf[index*STATS_PKT_SIZE+19]<<8)| 
   	     ((uint32_t)buf[index*STATS_PKT_SIZE+20]);
}


uint8_t stats_pkt_add( STATS_PKT_T *p, uint8_t *buf, uint8_t index )
{
   if((index+1)*STATS_PKT_SIZE>MAX_PKT_PAYLOAD ) return (index*STATS_PKT_SIZE);
   buf[index*STATS_PKT_SIZE]= p->mac_addr;
   buf[index*STATS_PKT_SIZE+1]= (p->rx_pkts>>8)&0xff;
   buf[index*STATS_PKT_SIZE+2]= (p->rx_pkts)&0xff;
   buf[index*STATS_PKT_SIZE+3]= (p->tx_pkts>>8)&0xff;
   buf[index*STATS_PKT_SIZE+4]= (p->tx_pkts)&0xff;
   buf[index*STATS_PKT_SIZE+5]= (p->tx_retry>>8)&0xff;
   buf[index*STATS_PKT_SIZE+6]= (p->tx_retry)&0xff;
   buf[index*STATS_PKT_SIZE+7]= (p->sensor_samples>>8)&0xff;
   buf[index*STATS_PKT_SIZE+8]= (p->sensor_samples)&0xff;
   buf[index*STATS_PKT_SIZE+9]= (p->uptime>>24)&0xff;
   buf[index*STATS_PKT_SIZE+10]= (p->uptime>>16)&0xff;
   buf[index*STATS_PKT_SIZE+11]= (p->uptime>>8)&0xff;
   buf[index*STATS_PKT_SIZE+12]= (p->uptime)&0xff;
   buf[index*STATS_PKT_SIZE+13]= (p->deep_sleep>>24)&0xff;
   buf[index*STATS_PKT_SIZE+14]= (p->deep_sleep>>16)&0xff;
   buf[index*STATS_PKT_SIZE+15]= (p->deep_sleep>>8)&0xff;
   buf[index*STATS_PKT_SIZE+16]= (p->deep_sleep)&0xff;
   buf[index*STATS_PKT_SIZE+17]= (p->idle_time>>24)&0xff;
   buf[index*STATS_PKT_SIZE+18]= (p->idle_time>>16)&0xff;
   buf[index*STATS_PKT_SIZE+19]= (p->idle_time>>8)&0xff;
   buf[index*STATS_PKT_SIZE+20]= (p->idle_time)&0xff;
   return ((index+1)*STATS_PKT_SIZE);
}

