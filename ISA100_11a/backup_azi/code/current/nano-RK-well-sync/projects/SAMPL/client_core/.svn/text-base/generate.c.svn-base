#include "generate.h"
#include <globals.h>
#include <nrk.h>
#include <bmac.h>
#include <sampl.h>
#include <ping_pkt.h>
#include <eeprom_data.h>
#include <ack_pkt.h>
#include <control_pkt.h>
#include <trace.h>
#include <stats_pkt.h>
#include <transducer_pkt.h>

/*
  uint8_t upstream_reply_mac
	This is the address of the node higher in the tree that sent
	the packet.  Typically this should be used as the destination
	reply address.
*/
void create_upstream_data_packet (SAMPL_DOWNSTREAM_PKT_T * ds_pkt,
				  SAMPL_UPSTREAM_PKT_T * us_pkt,
                                  uint8_t upstream_reply_mac)
{
uint8_t len,i,mode,num;
uint16_t addr;
// This function is responsible for creating the
// data that is sent back up the tree.
#ifdef DEBUG_TXT
  nrk_kprintf (PSTR ("Composing reply\r\n"));
#endif

    us_pkt->seq_num = ds_pkt->seq_num;
    us_pkt->next_hop_dst_mac = upstream_reply_mac;
    us_pkt->buf_len=US_PAYLOAD_START;
    us_pkt->payload=&(us_pkt->buf[US_PAYLOAD_START]);
    us_pkt->payload_len=0;
    us_pkt->payload_start=US_PAYLOAD_START;
    us_pkt->ctrl_flags = US_MASK;
    if((ds_pkt->ctrl_flags & LINK_ACK) !=0 ) us_pkt->ctrl_flags |= LINK_ACK;
    if((ds_pkt->ctrl_flags & ENCRYPT) !=0 ) us_pkt->ctrl_flags |= ENCRYPT;
    us_pkt->ack_retry= ds_pkt->ack_retry;
    us_pkt->subnet_mac[0] = my_subnet_mac[0];
    us_pkt->subnet_mac[1] = my_subnet_mac[1];
    us_pkt->subnet_mac[2] = my_subnet_mac[2];
    us_pkt->priority = ds_pkt->priority;
    us_pkt->error_code = 0;
    us_pkt->num_msgs = 1;

if(admin_debug_flag==1 && (ds_pkt->ctrl_flags & DEBUG_FLAG) !=0 )
	{
	printf( "0x%x%x: ",my_subnet_mac[0],my_mac);
	nrk_kprintf( PSTR(" ds_pkt ") );
	if(ds_pkt->is_mac_selected==1) 
		nrk_kprintf( PSTR("selected\r\n") );
	else 
		nrk_kprintf( PSTR("not selected\r\n") );
	}


if(ds_pkt->is_mac_selected==1)
{
    us_pkt->pkt_type = ds_pkt->pkt_type;

switch (ds_pkt->pkt_type) {

  case PING_PKT:
	ping_generate(us_pkt);
    	break;

  case XMPP_PKT:
	// just forward and send acks back
	// mobile nodes will interperate the packets
	xmpp_generate(us_pkt,ds_pkt);
    	break;

  case CONTROL_PKT:
	// Don't reply if packet is not encrypted
 	if((ds_pkt->ctrl_flags & ENCRYPT) == 0 ) return;
  	control_generate(us_pkt,ds_pkt);
    	break;


  case STATS_PKT:
	stats_generate(us_pkt,ds_pkt);
    	break;

  case SUBNET_NEIGHBOR_LIST_PKT:
	nlist_generate(us_pkt,ds_pkt);
    	break;

  case TRACEROUTE_PKT:
	trace_generate(us_pkt,ds_pkt);
    	break;

  case ROUTE_PKT:
	// Don't reply if packet is not encrypted
 	if((ds_pkt->ctrl_flags & ENCRYPT) == 0 ) return;
	route_generate(us_pkt,ds_pkt);
    	break;

  case DATA_STORAGE_PKT:
	eeprom_storage_generate(us_pkt, ds_pkt);
	break;

  case TRANSDUCER_CMD_PKT:
	transducer_generate(us_pkt, ds_pkt);
	break;

  default:
    if(admin_debug_flag==1 && (ds_pkt->ctrl_flags & DEBUG_FLAG) !=0 )
      {
	      printf ("Unknown %d, %d: ", ds_pkt->pkt_type, ds_pkt->payload_len);
	      for(i=0; i<ds_pkt->payload_len; i++ )
		printf( "%x ",ds_pkt->payload[i] );
	      printf( "\r\n" );
      }

    us_pkt->pkt_type = EMPTY_PKT;
    us_pkt->num_msgs = 0;
  }
} 
else
   {
    // Fill in blank reply
    us_pkt->pkt_type = EMPTY_PKT;
    us_pkt->num_msgs = 0;
   }
}
