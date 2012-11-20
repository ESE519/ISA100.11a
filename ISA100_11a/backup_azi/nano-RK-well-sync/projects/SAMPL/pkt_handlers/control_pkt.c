#include <globals.h>
#include <nrk.h>
#include <nrk_error.h>
#include <../include/sampl.h>
#include <control_pkt.h>
#include <ack_pkt.h>


int8_t control_generate(SAMPL_UPSTREAM_PKT_T *pkt,SAMPL_DOWNSTREAM_PKT_T *ds_pkt )
{
ACK_PKT_T p;
CONTROL_PKT_T r;
uint8_t num_pkts,i,selected,checksum;
nrk_time_t t;

  checksum=control_pkt_get(&r, ds_pkt->payload, 0);

  if(checksum==r.checksum && r.mobile_reserve_seconds!=0)
  {
  bmac_set_cca_thresh (r.cca_threshold);
  if((r.ctrl_flags_1 & GLOBAL_DEBUG_MASK )!=0) admin_debug_flag=1;
  else admin_debug_flag=0;
  t.secs=r.mobile_reserve_seconds;
  t.nano_secs=0;
  i=nrk_reserve_set(mobile_reserve, &t,r.mobile_reserve_cnt, NULL);
  // add route persistence here...

  nrk_kprintf( PSTR("Control Pkt:\r\n"));
  nrk_kprintf( PSTR("  CCA:"));
  printf( "%d",(int8_t)r.cca_threshold );
  nrk_kprintf( PSTR("\r\n  Debug:"));
  printf( "%d",(r.ctrl_flags_1 & GLOBAL_DEBUG_MASK));
  nrk_kprintf( PSTR("\r\n  Mobile Reserve time:"));
  printf( "%d",r.mobile_reserve_seconds);
  nrk_kprintf( PSTR(" sec\r\n  Mobile Reserve count:"));
  printf( "%d",r.mobile_reserve_cnt);
  nrk_kprintf( PSTR("\r\n"));


  // build ACK reply packet
  p.mac_addr=my_mac;
  pkt->payload_len = ack_pkt_add( &p, pkt->payload,0);
  pkt->num_msgs=1;
  pkt->pkt_type=ACK_PKT;
  pkt->error_code=0;  // set error type for NCK
  }
  else
  {
  nrk_kprintf( PSTR( "Control packet failed checksum\r\n"));
  nrk_kprintf( PSTR("  pkt: " ));
  printf( "%d",r.checksum );
  nrk_kprintf( PSTR("  calc: " ));
  printf( "%d\r\n",checksum );
  // build NCK reply packet
  p.mac_addr=my_mac;
  pkt->payload_len = ack_pkt_add( &p, pkt->payload,0);
  pkt->num_msgs=1;
  pkt->pkt_type=ACK_PKT;
  pkt->error_code=1;  // set error type for NCK
  }

return NRK_OK;  
}



uint8_t control_pkt_get( CONTROL_PKT_T *p, uint8_t *buf, uint8_t index )
{
uint8_t i,c;
   // 1 byte offset for number of messages
   p->cca_threshold=buf[index*CONTROL_PKT_SIZE];
   p->ctrl_flags_1=buf[index*CONTROL_PKT_SIZE+1];
   p->mobile_reserve_cnt=(((uint16_t)buf[index*CONTROL_PKT_SIZE+2])<<8)|buf[index*CONTROL_PKT_SIZE+3];
   p->mobile_reserve_seconds=(((uint16_t)buf[index*CONTROL_PKT_SIZE+4])<<8)|buf[index*CONTROL_PKT_SIZE+5];
   p->checksum=buf[index*CONTROL_PKT_SIZE+6];
   c=0;
for(i=0; i<CONTROL_PKT_SIZE-1; i++ )   
{
	c+=buf[index*CONTROL_PKT_SIZE+i];
printf( "c=%d\r\n",c );
}
  return c;
}


