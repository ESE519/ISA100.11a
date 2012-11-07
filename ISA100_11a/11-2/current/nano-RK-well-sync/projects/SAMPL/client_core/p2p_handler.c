#include "p2p_handler.h"
#include <globals.h>
#include <nrk.h>
#include <nrk_eeprom.h>
#include <ping_pkt.h>
#include <../include/sampl.h>

#ifdef PHOENIX
#include "./phoenix/phoenix.h"
#endif



PING_PKT_T p;

// Input:
// p2p_pkt_in 

// Output:
// p2p_pkt_out 

uint8_t handle_peer_2_peer_pkt(SAMPL_PEER_2_PEER_PKT_T *p2p_pkt_in, SAMPL_PEER_2_PEER_PKT_T *p2p_pkt_out)
{
uint8_t i, mode, len;
uint16_t addr;
// This function is responsible for creating the
// data that is sent back up the tree.
#ifdef DEBUG_TXT
  nrk_kprintf (PSTR ("Create P2P reply: "));
#endif


    // Copy header information for reply packet
    p2p_pkt_out->pkt_type = p2p_pkt_in->pkt_type;
    p2p_pkt_out->ctrl_flags = p2p_pkt_in->ctrl_flags; 
    p2p_pkt_out->seq_num = p2p_pkt_in->seq_num;
    p2p_pkt_out->ttl = p2p_pkt_in->ttl-1;
    p2p_pkt_out->priority = p2p_pkt_in->priority;
    p2p_pkt_out->ack_retry= p2p_pkt_in->ack_retry;
    p2p_pkt_out->check_rate= p2p_pkt_in->check_rate;


    p2p_pkt_out->subnet_mac[0] = p2p_pkt_in->subnet_mac[0];
    p2p_pkt_out->subnet_mac[1] = p2p_pkt_in->subnet_mac[1];
    p2p_pkt_out->subnet_mac[2] = p2p_pkt_in->subnet_mac[2];
    p2p_pkt_out->src_mac = p2p_pkt_in->src_mac;
    p2p_pkt_out->dst_mac = p2p_pkt_in->dst_mac;
    p2p_pkt_out->last_hop_mac = my_mac;
    p2p_pkt_out->next_hop_mac = (route_table_get(p2p_pkt_in->dst_mac) & 0xff);
    


if(admin_debug_flag==1 && (p2p_pkt_in->ctrl_flags & DEBUG_FLAG) !=0 )
	{
	nrk_kprintf( PSTR( "Route Option: " ));
	if(p2p_pkt_out->next_hop_mac==0xff) nrk_kprintf( PSTR("Flooding\r\n") );
	nrk_kprintf( PSTR("Set Route\r\n" ));
	printf( "0x%x%x: ",my_subnet_mac[0],my_mac);
	nrk_kprintf( PSTR(" p2p_pkt ") );
	}

// If the packet is a broadcast or direct to my node
// Packets that are directed towards a node without routing data set
// next_hop_mac to 255.
if(p2p_pkt_in->dst_mac==255 || p2p_pkt_in->dst_mac==my_mac )
{

if(admin_debug_flag==1 && (p2p_pkt_in->ctrl_flags & DEBUG_FLAG) !=0 )
	nrk_kprintf( PSTR("for me\r\n") );

  switch (p2p_pkt_in->pkt_type) {

 case PING_PKT:
    	// setup 1-hop reply (change below for packet specific mods)
        p2p_pkt_out->ctrl_flags = 0;
        p2p_pkt_out->ttl = 1;
        p2p_pkt_out->subnet_mac[0] = my_subnet_mac[0];
        p2p_pkt_out->subnet_mac[1] = my_subnet_mac[1];
        p2p_pkt_out->subnet_mac[2] = my_subnet_mac[2];
    	p2p_pkt_out->src_mac = my_mac;
    	p2p_pkt_out->dst_mac = p2p_pkt_in->src_mac;
   	p2p_pkt_out->next_hop_mac = p2p_pkt_in->src_mac;
   	ping_p2p_generate(p2p_pkt_out); 
   break;


  case DATA_STORAGE_PKT:
    	// setup 1-hop reply (change below for packet specific mods)
        p2p_pkt_out->ctrl_flags = 0;
        p2p_pkt_out->ttl = 1;
        p2p_pkt_out->subnet_mac[0] = my_subnet_mac[0];
        p2p_pkt_out->subnet_mac[1] = my_subnet_mac[1];
        p2p_pkt_out->subnet_mac[2] = my_subnet_mac[2];
    	p2p_pkt_out->src_mac = my_mac;
    	p2p_pkt_out->dst_mac = p2p_pkt_in->src_mac;
   	p2p_pkt_out->next_hop_mac = p2p_pkt_in->src_mac;
	eeprom_storage_p2p_generate(p2p_pkt_in, p2p_pkt_out );
	break;

#ifdef PHOENIX
  case WIRELESS_UPDATE_PKT:
#ifdef DEBUG_TXT
    nrk_kprintf (PSTR ("Wireless Update Packet\r\n"));
#endif
    // Don't reply if packet is not encrypted
    if((p2p_pkt_in->ctrl_flags & ENCRYPT) == 0 ) return;
    phoenix_wireless_update();
    // This function reboots, it never returns...
    break;
#endif

   // Just copy the packet and route as needed
  default:

#ifdef DEBUG_TXT
	nrk_kprintf( PSTR( "Got UNKOWN pkt, forwarding\r\n" ));
#endif
    // No clue what this packet is, so just forward it 
    for(i=0; i<p2p_pkt_in->payload_len; i++ )
	    p2p_pkt_out->payload[i]=p2p_pkt_in->payload[i];
    p2p_pkt_out->payload_len=p2p_pkt_in->payload_len;
  }
}else if(p2p_pkt_out->ttl>0)
{
// This packet should be forwarded
if(admin_debug_flag==1 && (p2p_pkt_in->ctrl_flags & DEBUG_FLAG) !=0 )
	nrk_kprintf( PSTR("forward\r\n") );
    // If the packet isn't for my handler, just copy it for forwarding
    // Routing should have been correctly set above
    for(i=0; i<p2p_pkt_in->payload_len; i++ )
	    p2p_pkt_out->payload[i]=p2p_pkt_in->payload[i];
    p2p_pkt_out->payload_len=p2p_pkt_in->payload_len;

} else
{
    // This packet is junk, don't reply at all
    p2p_pkt_out->pkt_type = EMPTY_PKT;
    return 0;
}

return 1;
}
