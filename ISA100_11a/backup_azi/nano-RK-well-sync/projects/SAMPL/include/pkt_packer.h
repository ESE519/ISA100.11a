#ifndef PKT_PACKER_H 
#define PKT_PACKER_H 

#include "sampl.h"

uint8_t pack_peer_2_peer_packet( SAMPL_PEER_2_PEER_PKT_T *p2p_pkt);
uint8_t unpack_peer_2_peer_packet( SAMPL_PEER_2_PEER_PKT_T *p2p_pkt);

uint8_t unpack_downstream_packet( SAMPL_DOWNSTREAM_PKT_T *ds_pkt,uint8_t my_node_mac);
uint8_t pack_downstream_packet( SAMPL_DOWNSTREAM_PKT_T *ds_pkt);

uint8_t pack_upstream_packet( SAMPL_UPSTREAM_PKT_T *us_pkt);
uint8_t unpack_upstream_packet( SAMPL_UPSTREAM_PKT_T *us_pkt);

#endif
