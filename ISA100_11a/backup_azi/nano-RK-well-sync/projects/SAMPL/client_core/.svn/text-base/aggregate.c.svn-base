#include "aggregate.h"
#include <nrk.h>
#include <stdio.h>
#include <sampl.h>
#include <globals.h>
#include <sampl.h>
#include <ping_pkt.h>
#include <eeprom_data.h>
#include <ack_pkt.h>
#include <control_pkt.h>
#include <trace.h>
#include <stats_pkt.h>
#include <transducer_pkt.h>


void aggregate_upstream_data ( SAMPL_UPSTREAM_PKT_T * us_pkt_in,
                              SAMPL_UPSTREAM_PKT_T * us_pkt)
{
  uint8_t i,j,k,dup,len;
// This function takes replies that are sent up the tree
// and packages them into the main reply packet
#ifdef DEBUG_TXT
  nrk_kprintf (PSTR ("Aggregating Incomming Data: "));
#endif


    // We concatinate the packets 
    // If it is an empty packet, replace with aggregated packet(s)
    us_pkt->error_code =us_pkt_in->error_code;
    us_pkt_in->payload=&(us_pkt_in->buf[US_PAYLOAD_START]);
    us_pkt_in->payload_len=us_pkt_in->buf_len-US_PAYLOAD_START;

if(admin_debug_flag==1 && (us_pkt->ctrl_flags & DEBUG_FLAG) !=0 )
	{
	printf( "0x%x%x: ",my_subnet_mac[0],my_mac);
	nrk_kprintf( PSTR(" us_pkt aggregating\r\n") );
	}


  switch (us_pkt->pkt_type) {
  case ACK_PKT:
	ack_aggregate(us_pkt_in, us_pkt );	
	break;
  case PING_PKT:
	ping_aggregate(us_pkt_in, us_pkt );	
	break;

  case TRACEROUTE_PKT:
	trace_aggregate(us_pkt_in, us_pkt );	
	break;

  case SUBNET_NEIGHBOR_LIST_PKT:
	nlist_aggregate(us_pkt_in, us_pkt );	
	break;

  case STATS_PKT:
	stats_aggregate(us_pkt_in, us_pkt );	
	break;

  case TRANSDUCER_REPLY_PKT:
	transducer_aggregate(us_pkt_in, us_pkt );	
	break;


  // No DATA STORAGE AGGREGATE, so just treat as an empty pkt
  case DATA_STORAGE_PKT:

  case EMPTY_PKT:
	for(i=0; i<us_pkt_in->buf_len; i++ ) us_pkt->buf[i]=us_pkt_in->buf[i];
	us_pkt->buf_len=us_pkt_in->buf_len;
	us_pkt->payload_start=us_pkt_in->payload_start;
	us_pkt->payload_len=us_pkt_in->payload_len;
	us_pkt->payload = &(us_pkt->buf[US_PAYLOAD_START]);
  	us_pkt->pkt_type = us_pkt_in->pkt_type;
    	us_pkt->ctrl_flags = us_pkt_in->ctrl_flags;
    	if((us_pkt_in->ctrl_flags & LINK_ACK) !=0 ) us_pkt->ctrl_flags |= LINK_ACK;
    	if((us_pkt_in->ctrl_flags & ENCRYPT) !=0 ) us_pkt->ctrl_flags |= ENCRYPT;
    	us_pkt->ack_retry= us_pkt_in->ack_retry;
    	//us_pkt->subnet_mac[0] = us_pkt_in->subnet_mac[0];
    	//us_pkt->subnet_mac[1] = us_pkt_in->subnet_mac[1];
    	//us_pkt->subnet_mac[2] = us_pkt_in->subnet_mac[2];
    	us_pkt->subnet_mac[0] = my_subnet_mac[0];
    	us_pkt->subnet_mac[1] = my_subnet_mac[1];
    	us_pkt->subnet_mac[2] = my_subnet_mac[2];
    	us_pkt->priority = us_pkt_in->priority;
    	us_pkt->error_code = us_pkt_in->error_code;
    	us_pkt->num_msgs = us_pkt_in->num_msgs;
  break;
  default:
    nrk_kprintf (PSTR ("ERROR\r\n"));
    break;

  }

}
