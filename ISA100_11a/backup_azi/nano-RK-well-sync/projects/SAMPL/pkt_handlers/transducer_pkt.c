#include <globals.h>
#include <../include/sampl.h>
#include <control_pkt.h>
#include <ack_pkt.h>
#include <transducer_pkt.h>

#ifdef NANORK 
#include <transducer_handler.h>
#include <nrk.h>
#include <nrk_error.h>


int8_t transducer_generate(SAMPL_UPSTREAM_PKT_T *pkt,SAMPL_DOWNSTREAM_PKT_T *ds_pkt )
{
TRANSDUCER_REPLY_PKT_T r;
TRANSDUCER_CMD_PKT_T p;
ACK_PKT_T a;
uint8_t num_pkts,i,selected,checksum;
uint8_t status;
uint8_t pkt_generated;

pkt_generated=0;
pkt->payload_len=0;
pkt->num_msgs=0;
status=0;
  checksum=transducer_cmd_pkt_get(&p, ds_pkt->payload);

  if(checksum==p.checksum )
  {
  for(i=0; i<p.num_msgs; i++ )
	{
	if(p.msg[i].mac_addr==my_mac || p.msg[i].mac_addr==255 )
		{
			pkt_generated=1;

			// Setup a blank default reply pkt
			r.mac_addr=my_mac;
			r.len=0;
			r.type=0;
			r.payload=&(pkt->payload[pkt->payload_len+TRANSDUCER_REPLY_HEADER_SIZE]);	

			// Call application transducer handler
			status=transducer_handler( p.msg[i].key, p.msg[i].value, &r);
  			if( status==0) pkt->error_code=1;  

			// Copy header elements into packet
			pkt->payload[pkt->payload_len]=r.mac_addr;
			pkt->payload[pkt->payload_len+1]=r.type;
			pkt->payload[pkt->payload_len+2]=r.len;
			
			// Update new length of packet
			pkt->payload_len+=r.len+TRANSDUCER_REPLY_HEADER_SIZE;
  			pkt->num_msgs++;
  			pkt->pkt_type=TRANSDUCER_REPLY_PKT;
		}
	}
  }
  else
  {
  pkt_generated=1;
  nrk_kprintf( PSTR( "Actuator packet failed checksum\r\n"));
  nrk_kprintf( PSTR("  pkt: " ));
  printf( "%d",p.checksum );
  nrk_kprintf( PSTR("  calc: " ));
  printf( "%d\r\n",checksum );
  // build NCK reply packet
  a.mac_addr=my_mac;
  pkt->payload_len = ack_pkt_add( &a, pkt->payload,0);
  pkt->num_msgs=1;
  pkt->pkt_type=ACK_PKT;
  pkt->error_code=1;  // set error type for NCK
  }

if(pkt_generated==0)
   {
	pkt->pkt_type=EMPTY_PKT;	
	pkt->num_msgs=0;
	pkt->payload_len=0;
   }


return NRK_OK;  
}

int8_t transducer_p2p_generate(SAMPL_PEER_2_PEER_PKT_T *pkt)
{
ACK_PKT_T p;
  p.mac_addr=my_mac;
  pkt->payload_len = ack_pkt_add( &p, pkt->payload,0);
return NRK_OK;  
}

int8_t transducer_aggregate(SAMPL_UPSTREAM_PKT_T *in, SAMPL_UPSTREAM_PKT_T *out)
{
uint8_t len,i,j,k,dup;
TRANSDUCER_REPLY_PKT_T p1, p2;

for(i=0; i<in->num_msgs; i++ )
{
  dup=0;
  // get next ping packet to compare against current outgoing list
  transducer_reply_pkt_get( &p1, in->payload, i );
  for(k=0; k<out->num_msgs; k++ )
	{
	// get packet from outgoing list and compare against incomming packet
  	transducer_reply_pkt_get( &p2, out->payload, k );
	if(p1.mac_addr==p2.mac_addr && p1.type==p2.type) dup=1;	
	}
   if(dup==0)
	{
		// if packet is unique, add to outgoing packet
		//out->payload_len=transducer_reply_pkt_add( &p1, out->payload, out->num_msgs );
		len=transducer_reply_pkt_add( &p1, out->payload, out->num_msgs );
		if(len>0)
			{
			  out->payload_len=len;
			  out->num_msgs++;
			}
			else
			{
			// Set overflow error code
			out->error_code=1;
			}
	}
}
return NRK_OK;
}






#else
#define my_mac 0
#define RF_MAX_PAYLOAD_SIZE  128


#endif


uint8_t transducer_cmd_pkt_get( TRANSDUCER_CMD_PKT_T *p, uint8_t *buf)
{
uint8_t i,c;
   // 1 byte offset for number of messages
   p->checksum=buf[0];
   p->num_msgs=buf[1];
   c=buf[1];
   p->msg=(TRANSDUCER_MSG_T *) &buf[2];
   for(i=0; i<p->num_msgs*sizeof(TRANSDUCER_MSG_T); i++ )
   	c+=buf[2+i]; 
   return c;
}

uint8_t transducer_cmd_pkt_checksum( TRANSDUCER_CMD_PKT_T *p, uint8_t *buf)
{
uint8_t i,c;
c=buf[1];
for(i=0; i<p->num_msgs*3; i++ )
		c+=buf[2+i];
p->checksum=c;
buf[0]=c;
}

uint8_t transducer_cmd_pkt_add( TRANSDUCER_CMD_PKT_T *p, uint8_t *buf)
{
uint8_t i;
   
   // pack the number of messages into the payload 
   buf[1]=p->num_msgs;
   // Copy the data to the payload
   for(i=0; i<p->num_msgs; i++ )
	{
		buf[2+(i*3)]=p->msg[i].mac_addr;
		buf[2+(i*3)+1]=p->msg[i].key;
		buf[2+(i*3)+2]=p->msg[i].value;
	}
   // return the size of the packet (number of messages + 2 header bytes)
   return (p->num_msgs*sizeof(TRANSDUCER_MSG_T)+2);
}


uint8_t transducer_reply_pkt_add( TRANSDUCER_REPLY_PKT_T *p, uint8_t *buf, uint8_t index)
{
uint8_t i,j,next_pkt;

next_pkt=0;
for(i=0; i<index; i++ )
{
  // index of next packet plus the length of the packet and header
  next_pkt+=buf[next_pkt+2]+TRANSDUCER_REPLY_HEADER_SIZE;
}
if(next_pkt+p->len > MAX_PKT_PAYLOAD  ) return 0;
buf[next_pkt]=p->mac_addr;
buf[next_pkt+1]=p->type;
buf[next_pkt+2]=p->len;
next_pkt+=TRANSDUCER_REPLY_HEADER_SIZE;
for(i=0; i<p->len; i++ )
	buf[next_pkt+i]=p->payload[i];
return (next_pkt+p->len);
}


uint8_t transducer_reply_pkt_get( TRANSDUCER_REPLY_PKT_T *p, uint8_t *buf, uint8_t index )
{
uint8_t i,next_pkt;

next_pkt=0;
for(i=0; i<index; i++ )
{
    next_pkt+=buf[next_pkt+2]+TRANSDUCER_REPLY_HEADER_SIZE;
}

p->mac_addr=buf[next_pkt];
p->type=buf[next_pkt+1];
p->len=buf[next_pkt+2];
p->payload=&(buf[next_pkt+3]);
}

