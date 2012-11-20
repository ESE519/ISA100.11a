#include "sampl.h"
#include "pkt_packer.h"



uint8_t unpack_downstream_packet( SAMPL_DOWNSTREAM_PKT_T *ds_pkt, uint8_t my_node_mac)
{
uint8_t i;

      ds_pkt->protocol_id= ds_pkt->buf[PROTOCOL_ID];
      ds_pkt->protocol_version= ds_pkt->buf[PROTOCOL_VERSION];
      ds_pkt->pkt_type = ds_pkt->buf[PKT_TYPE];
      ds_pkt->ctrl_flags = ds_pkt->buf[CTRL_FLAGS];
      ds_pkt->seq_num= ds_pkt->buf[SEQ_NUM];
      ds_pkt->priority = ds_pkt->buf[PRIORITY]&0x0F;
      ds_pkt->ack_retry = (ds_pkt->buf[ACK_RETRY]&0xF0)>>4;
      ds_pkt->subnet_mac[0]= ds_pkt->buf[SUBNET_MAC_0];
      ds_pkt->subnet_mac[1]= ds_pkt->buf[SUBNET_MAC_1];
      ds_pkt->subnet_mac[2]= ds_pkt->buf[SUBNET_MAC_2];

      ds_pkt->hop_cnt = ds_pkt->buf[DS_HOP_CNT];
      ds_pkt->hop_max = ds_pkt->buf[DS_HOP_MAX];
      ds_pkt->delay_per_level = ds_pkt->buf[DS_DELAY_PER_LEVEL];
      ds_pkt->nav = ds_pkt->buf[DS_NAV];
      ds_pkt->mac_check_rate = ds_pkt->buf[DS_MAC_CHECK_RATE];
      ds_pkt->rssi_threshold = (int8_t) ds_pkt->buf[DS_RSSI_THRESHOLD];
      ds_pkt->last_hop_mac = ds_pkt->buf[DS_LAST_HOP_MAC];
      ds_pkt->aes_ctr[0] = ds_pkt->buf[DS_AES_CTR_0];
      ds_pkt->aes_ctr[1] = ds_pkt->buf[DS_AES_CTR_1];
      ds_pkt->aes_ctr[2] = ds_pkt->buf[DS_AES_CTR_2];
      ds_pkt->aes_ctr[3] = ds_pkt->buf[DS_AES_CTR_3];
      ds_pkt->mac_filter_num = ds_pkt->buf[DS_MAC_FILTER_LIST_SIZE];

      // Set the start of payload index (include variable params)
      ds_pkt->payload_start=DS_PAYLOAD_START;

      ds_pkt->is_mac_selected=1;
      if(ds_pkt->mac_filter_num!=0)
	  {
	      ds_pkt->payload_start+=ds_pkt->mac_filter_num+1;
	      ds_pkt->is_mac_selected=0;
	      for(i=0; i<ds_pkt->mac_filter_num; i++ )
		      {
			if(ds_pkt->buf[i+DS_PAYLOAD_START]==my_node_mac)
				ds_pkt->is_mac_selected=1;
		      }
	  }

	ds_pkt->payload=&(ds_pkt->buf[ds_pkt->payload_start]);
	ds_pkt->payload_len=ds_pkt->buf_len-ds_pkt->payload_start;
return 1; 
}

uint8_t downstream_packet_add_mac_filter( SAMPL_DOWNSTREAM_PKT_T *ds_pkt, uint8_t mac)
{
// FIXME: Add correct max number of messages 
if(ds_pkt->mac_filter_num>32) return 0;
ds_pkt->buf[DS_PAYLOAD_START+ds_pkt->mac_filter_num]=mac;
ds_pkt->mac_filter_num++;
ds_pkt->buf[DS_MAC_FILTER_LIST_SIZE]++;
ds_pkt->buf_len=DS_PAYLOAD_START+ds_pkt->mac_filter_num;
ds_pkt->payload_start=DS_PAYLOAD_START+ds_pkt->mac_filter_num+1;
ds_pkt->payload=ds_pkt->buf[ds_pkt->payload_start];

return 1;
}

uint8_t pack_downstream_packet( SAMPL_DOWNSTREAM_PKT_T *ds_pkt)
{
	ds_pkt->buf_len=DS_PAYLOAD_START+ds_pkt->payload_len;
  
	ds_pkt->buf[PROTOCOL_ID]=SAMPL_ID;
	ds_pkt->buf[PROTOCOL_VERSION]=SAMPL_VERSION;
	ds_pkt->buf[PKT_TYPE]=ds_pkt->pkt_type;
	ds_pkt->buf[CTRL_FLAGS]=ds_pkt->ctrl_flags;
        ds_pkt->buf[SUBNET_MAC_0]= ds_pkt->subnet_mac[0];
        ds_pkt->buf[SUBNET_MAC_1]= ds_pkt->subnet_mac[1];
        ds_pkt->buf[SUBNET_MAC_2]= ds_pkt->subnet_mac[2];
        ds_pkt->buf[PRIORITY]= ds_pkt->priority&0x0F;
        ds_pkt->buf[ACK_RETRY] |= (ds_pkt->ack_retry&0x0F)<<4;
	ds_pkt->buf[SEQ_NUM]=ds_pkt->seq_num;

	ds_pkt->buf[DS_LAST_HOP_MAC]=ds_pkt->last_hop_mac;
	ds_pkt->buf[DS_HOP_CNT]=ds_pkt->hop_cnt;
	ds_pkt->buf[DS_HOP_MAX]=ds_pkt->hop_max;
	ds_pkt->buf[DS_DELAY_PER_LEVEL]=ds_pkt->delay_per_level;
	ds_pkt->buf[DS_MAC_CHECK_RATE]=ds_pkt->mac_check_rate;
	ds_pkt->buf[DS_RSSI_THRESHOLD]=ds_pkt->rssi_threshold;
	ds_pkt->buf[DS_AES_CTR_3]=ds_pkt->aes_ctr[3];
	ds_pkt->buf[DS_AES_CTR_2]=ds_pkt->aes_ctr[2];
	ds_pkt->buf[DS_AES_CTR_1]=ds_pkt->aes_ctr[1];
	ds_pkt->buf[DS_AES_CTR_0]=ds_pkt->aes_ctr[0];
	ds_pkt->buf[DS_MAC_FILTER_LIST_SIZE]=ds_pkt->mac_filter_num;

return 1; 
}




uint8_t pack_upstream_packet( SAMPL_UPSTREAM_PKT_T *us_pkt)
{
	us_pkt->buf_len=US_PAYLOAD_START+us_pkt->payload_len;
  
	us_pkt->buf[PROTOCOL_ID]=SAMPL_ID;
	us_pkt->buf[PROTOCOL_VERSION]=SAMPL_VERSION;
	us_pkt->buf[PKT_TYPE]=us_pkt->pkt_type;
	us_pkt->buf[CTRL_FLAGS]=us_pkt->ctrl_flags;
	us_pkt->buf[SUBNET_MAC_0]=us_pkt->subnet_mac[0];
	us_pkt->buf[SUBNET_MAC_1]=us_pkt->subnet_mac[1];
	us_pkt->buf[SUBNET_MAC_2]=us_pkt->subnet_mac[2];
        us_pkt->buf[PRIORITY]= us_pkt->priority&0x0F;
        us_pkt->buf[ACK_RETRY] |= (us_pkt->ack_retry&0x0F)<<4;
	us_pkt->buf[SEQ_NUM]=us_pkt->seq_num;
	us_pkt->buf[US_ERROR_CODE]=us_pkt->error_code;
	us_pkt->buf[US_NEXT_HOP_DST_MAC]=us_pkt->next_hop_dst_mac;
	us_pkt->buf[US_LAST_HOP_SRC_MAC]=us_pkt->last_hop_src_mac;
	us_pkt->buf[US_NUM_MSGS]=us_pkt->num_msgs;

return 1; 
}

uint8_t pack_gateway_packet( SAMPL_GATEWAY_PKT_T *gw_pkt)
{
	gw_pkt->buf_len=GW_PAYLOAD_START+gw_pkt->payload_len;
  
	gw_pkt->buf[PROTOCOL_ID]=SAMPL_ID;
	gw_pkt->buf[PROTOCOL_VERSION]=SAMPL_VERSION;
	gw_pkt->buf[PKT_TYPE]=gw_pkt->pkt_type;
	gw_pkt->buf[CTRL_FLAGS]=gw_pkt->ctrl_flags;
	gw_pkt->buf[SUBNET_MAC_0]=gw_pkt->subnet_mac[0];
	gw_pkt->buf[SUBNET_MAC_1]=gw_pkt->subnet_mac[1];
	gw_pkt->buf[SUBNET_MAC_2]=gw_pkt->subnet_mac[2];
        gw_pkt->buf[PRIORITY]= gw_pkt->priority&0x0F;
        gw_pkt->buf[ACK_RETRY] |= (gw_pkt->ack_retry&0x0F)<<4;
	gw_pkt->buf[SEQ_NUM]=gw_pkt->seq_num;
	gw_pkt->buf[GW_ERROR_CODE]=gw_pkt->error_code;
	gw_pkt->buf[GW_RSSI]=gw_pkt->rssi;
	gw_pkt->buf[GW_LAST_HOP_MAC]=gw_pkt->last_hop_mac;
	gw_pkt->buf[GW_SRC_MAC]=gw_pkt->src_mac;
	gw_pkt->buf[GW_DST_MAC]=gw_pkt->dst_mac;
	gw_pkt->buf[GW_NUM_MSGS]=gw_pkt->num_msgs;

return 1; 
}

uint8_t unpack_gateway_packet( SAMPL_GATEWAY_PKT_T *gw_pkt)
{
      gw_pkt->protocol_id= gw_pkt->buf[PROTOCOL_ID];
      gw_pkt->protocol_version= gw_pkt->buf[PROTOCOL_VERSION];
      gw_pkt->pkt_type = gw_pkt->buf[PKT_TYPE];
      gw_pkt->ctrl_flags = gw_pkt->buf[CTRL_FLAGS];
      gw_pkt->subnet_mac[0] = gw_pkt->buf[SUBNET_MAC_0];
      gw_pkt->subnet_mac[1] = gw_pkt->buf[SUBNET_MAC_1];
      gw_pkt->subnet_mac[2] = gw_pkt->buf[SUBNET_MAC_2];
      gw_pkt->priority = gw_pkt->buf[PRIORITY]&0x0F;
      gw_pkt->ack_retry = (gw_pkt->buf[ACK_RETRY]&0xF0)>>4;
      gw_pkt->seq_num= gw_pkt->buf[SEQ_NUM];

      gw_pkt->rssi = gw_pkt->buf[GW_RSSI];
      gw_pkt->last_hop_mac = gw_pkt->buf[GW_LAST_HOP_MAC];
      gw_pkt->src_mac = gw_pkt->buf[GW_SRC_MAC];
      gw_pkt->dst_mac = gw_pkt->buf[GW_DST_MAC];
      gw_pkt->error_code = gw_pkt->buf[GW_ERROR_CODE];
      gw_pkt->num_msgs= gw_pkt->buf[GW_NUM_MSGS];

      gw_pkt->payload_start=GW_PAYLOAD_START;
      gw_pkt->payload = &(gw_pkt->buf[GW_PAYLOAD_START]);
      gw_pkt->payload_len = gw_pkt->buf_len - GW_PAYLOAD_START; 

return 1; 
}


uint8_t unpack_upstream_packet( SAMPL_UPSTREAM_PKT_T *us_pkt)
{
      us_pkt->protocol_id= us_pkt->buf[PROTOCOL_ID];
      us_pkt->protocol_version= us_pkt->buf[PROTOCOL_VERSION];
      us_pkt->pkt_type = us_pkt->buf[PKT_TYPE];
      us_pkt->ctrl_flags = us_pkt->buf[CTRL_FLAGS];
      us_pkt->subnet_mac[0] = us_pkt->buf[SUBNET_MAC_0];
      us_pkt->subnet_mac[1] = us_pkt->buf[SUBNET_MAC_1];
      us_pkt->subnet_mac[2] = us_pkt->buf[SUBNET_MAC_2];
      us_pkt->priority = us_pkt->buf[PRIORITY]&0x0F;
      us_pkt->ack_retry = (us_pkt->buf[ACK_RETRY]&0xF0)>>4;
      us_pkt->seq_num= us_pkt->buf[SEQ_NUM];

      us_pkt->next_hop_dst_mac = us_pkt->buf[US_NEXT_HOP_DST_MAC];
      us_pkt->last_hop_src_mac = us_pkt->buf[US_LAST_HOP_SRC_MAC];
      us_pkt->error_code = us_pkt->buf[US_ERROR_CODE];
      us_pkt->num_msgs= us_pkt->buf[US_NUM_MSGS];

      //us_pkt->buf=us_pkt->buf;
      //us_pkt->buf_len=us_pkt->buf_len;
      us_pkt->payload_start=US_PAYLOAD_START;
      us_pkt->payload = &(us_pkt->buf[US_PAYLOAD_START]);
      //us_pkt->payload = us_pkt->buf+US_PAYLOAD_START;
      us_pkt->payload_len = us_pkt->buf_len - US_PAYLOAD_START; 
return 1; 
}


uint8_t pack_peer_2_peer_packet( SAMPL_PEER_2_PEER_PKT_T *p2p_pkt)
{
	p2p_pkt->buf_len=P2P_PAYLOAD_START+p2p_pkt->payload_len;

	p2p_pkt->buf[PROTOCOL_ID]=SAMPL_ID;
	p2p_pkt->buf[PROTOCOL_VERSION]=SAMPL_VERSION;
	p2p_pkt->buf[PKT_TYPE]=p2p_pkt->pkt_type;
	p2p_pkt->buf[CTRL_FLAGS]=p2p_pkt->ctrl_flags;
	p2p_pkt->buf[SEQ_NUM]=p2p_pkt->seq_num;
        p2p_pkt->buf[PRIORITY]= p2p_pkt->priority&0x0F;
        p2p_pkt->buf[ACK_RETRY] |= (p2p_pkt->ack_retry&0x0F)<<4;
	p2p_pkt->buf[SUBNET_MAC_0]=p2p_pkt->subnet_mac[0];
	p2p_pkt->buf[SUBNET_MAC_1]=p2p_pkt->subnet_mac[1];
	p2p_pkt->buf[SUBNET_MAC_2]=p2p_pkt->subnet_mac[2];
	p2p_pkt->buf[P2P_SRC_MAC]=p2p_pkt->src_mac;
	p2p_pkt->buf[P2P_DST_MAC]=p2p_pkt->dst_mac;
	p2p_pkt->buf[P2P_LAST_HOP_MAC]=p2p_pkt->last_hop_mac;
	p2p_pkt->buf[P2P_NEXT_HOP_MAC]=p2p_pkt->next_hop_mac;
	p2p_pkt->buf[P2P_TTL]=p2p_pkt->ttl;
	p2p_pkt->buf[P2P_CHECK_RATE]=p2p_pkt->check_rate;
return 1;
}


uint8_t unpack_peer_2_peer_packet( SAMPL_PEER_2_PEER_PKT_T *p2p_pkt)
{
      p2p_pkt->protocol_id= p2p_pkt->buf[PROTOCOL_ID];
      p2p_pkt->protocol_version= p2p_pkt->buf[PROTOCOL_VERSION];
      p2p_pkt->pkt_type= p2p_pkt->buf[PKT_TYPE];
      p2p_pkt->ctrl_flags = p2p_pkt->buf[CTRL_FLAGS];
      p2p_pkt->subnet_mac[0]= p2p_pkt->buf[SUBNET_MAC_0];
      p2p_pkt->subnet_mac[1]= p2p_pkt->buf[SUBNET_MAC_1];
      p2p_pkt->subnet_mac[2]= p2p_pkt->buf[SUBNET_MAC_2];
      p2p_pkt->priority = p2p_pkt->buf[PRIORITY]&0x0F;
      p2p_pkt->ack_retry = (p2p_pkt->buf[ACK_RETRY]&0xF0)>>4;
      p2p_pkt->seq_num = p2p_pkt->buf[SEQ_NUM];

      p2p_pkt->ttl = p2p_pkt->buf[P2P_TTL];
      p2p_pkt->check_rate = p2p_pkt->buf[P2P_CHECK_RATE];
      p2p_pkt->src_mac= p2p_pkt->buf[P2P_SRC_MAC];
      p2p_pkt->dst_mac= p2p_pkt->buf[P2P_DST_MAC];
      p2p_pkt->last_hop_mac= p2p_pkt->buf[P2P_LAST_HOP_MAC];
      p2p_pkt->next_hop_mac= p2p_pkt->buf[P2P_NEXT_HOP_MAC];

      p2p_pkt->payload_start= P2P_PAYLOAD_START;
      p2p_pkt->payload=&(p2p_pkt->buf[P2P_PAYLOAD_START]);
      p2p_pkt->payload_len= p2p_pkt->buf_len - P2P_PAYLOAD_START;
return 1;
}


