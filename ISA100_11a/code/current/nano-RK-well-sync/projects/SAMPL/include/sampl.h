/******************************************************************************
*  Nano-RK, a real-time operating system for sensor networks.
*  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
*  All rights reserved.
*
*  This is the Open Source Version of Nano-RK included as part of a Dual
*  Licensing Model. If you are unsure which license to use please refer to:
*  http://www.nanork.org/nano-RK/wiki/Licensing
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, version 2.0 of the License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Contributing Authors (specific to this file):
*  Anthony Rowe 
*  Karthik Lakshmanan 
*******************************************************************************/


#ifndef SAMPL_H 
#define SAMPL_H 

#include <stdint.h>

#ifndef NRK_ERROR
  #define NRK_ERROR (-1)
#endif

#ifndef NRK_OK
  #define NRK_OK    1
#endif

// #define DEBUG_TXT
#define MAX_PKT_PAYLOAD		(120-US_PAYLOAD_START)
#define GATEWAY_MAC	0

/* NOTE: THE SIZE OF CONTROL PACKET SHOULD BE LESS THAN RF_MAX_PAYLOAD_SIZE	*/

#define MAX_NODES	 	30	
#define WORDS_PER_NODE		2

#define SAMPL_ID		1
#define SAMPL_VERSION		1

#define FAST_CHECK_RATE		100	

#define BROADCAST		255

// limits
#define MAX_NAV			240	
#define MAX_DELAY_PER_LEVEL	5
#define MAX_HOPS		32


#define DEFAULT_CHECK_RATE	100

// error flags for packets
#define HOP_ERROR_MASK   		0x01
#define NAV_ERROR_MASK			0x02
#define DELAY_PER_LEVEL_ERROR_MASK	0x04
#define MAX_HOPS_ERROR_MASK		0x08


// ctrl_flag MASKS 
#define DS_MASK     0x01
#define US_MASK	    0x02
#define TREE_FILTER 0x04
#define LED_FLOOD   0x08
#define LINK_ACK    0x10
#define ENCRYPT     0x20
#define MOBILE_MASK 0x40
#define DEBUG_FLAG  0x80

// PKT Types
#define  EMPTY_PKT			0x00
#define  PING_PKT  			0x01
#define  WIRELESS_UPDATE_PKT		0x02
#define  ACK_PKT 			0x03
#define  ERROR_PKT			0x04
#define  ROUTE_PKT			0x05
#define  FF_SENSOR_LONG_PKT		0x06
#define  FF_SENSOR_SHORT_PKT		0x07
#define  TRACEROUTE_PKT			0x08
#define  CONTROL_PKT			0x09
#define  LED_CONTROL_PKT		0x0a
#define  DATA_STORAGE_PKT		0x0b
#define  XMPP_PKT			0x0c
#define  STATS_PKT			0x0d
#define  SUBNET_NEIGHBOR_LIST_PKT	0x0e
#define  EXTENDED_NEIGHBOR_LIST_PKT	0x0f
#define  TRANSDUCER_REPLY_PKT		0x10
#define  TRANSDUCER_CMD_PKT		0x11
#define  UNKNOWN_PKT			0xff
// UNKNOWN_PKT used by phoenix etc, do not remove 

// Common to all packets
#define PROTOCOL_ID		0  
#define PROTOCOL_VERSION	1  
#define CTRL_FLAGS		2  
#define PKT_TYPE		3
#define SEQ_NUM			4	
#define PRIORITY	 	5	
#define ACK_RETRY		5	
#define SUBNET_MAC		8  // when operating in 8 bit mode
#define SUBNET_MAC_2		6
#define SUBNET_MAC_1		7
#define SUBNET_MAC_0		8


// Common to downstream packets
#define DS_LAST_HOP_MAC		9	
#define DS_HOP_CNT		10	
#define DS_HOP_MAX		11
#define DS_DELAY_PER_LEVEL 	12	
#define DS_NAV			13	
#define DS_MAC_CHECK_RATE	14	
#define DS_RSSI_THRESHOLD	15	
#define DS_AES_CTR_3		16	
#define DS_AES_CTR_2		17	
#define DS_AES_CTR_1		18	
#define DS_AES_CTR_0		19	
#define DS_MAC_FILTER_LIST_SIZE 20	
#define DS_PAYLOAD_START	21	

// Common to upstream reply packets
#define US_LAST_HOP_SRC_MAC	9	
#define US_NEXT_HOP_DST_MAC     10	
#define US_ERROR_CODE          	11 
#define US_NUM_MSGS		12	
#define US_PAYLOAD_START	13	

// Common to mobile packets 
#define P2P_SRC_MAC		9
#define P2P_DST_MAC		10	
#define P2P_LAST_HOP_MAC	11	
#define P2P_NEXT_HOP_MAC	12	
#define P2P_TTL			13	
#define P2P_CHECK_RATE		14
#define P2P_PAYLOAD_START	15

// Common to gateway packets
#define GW_LAST_HOP_MAC		9	
#define GW_RSSI			10	
#define GW_SRC_MAC		11	
#define GW_DST_MAC		12	
#define GW_ERROR_CODE          	13 
#define GW_NUM_MSGS		14	
#define GW_PAYLOAD_START	15	


typedef struct sampl_downstream_pkt 
{
// Common Header
	uint8_t protocol_id;
	uint8_t protocol_version;
	uint8_t pkt_type;
	uint8_t ctrl_flags;  
	uint8_t seq_num;
	uint8_t priority;
	uint8_t ack_retry;
	uint8_t subnet_mac[3];

// DS Specific Payload	
	uint8_t hop_cnt;
	uint8_t hop_max;
	uint8_t delay_per_level;
	uint8_t nav;
	uint8_t mac_check_rate;
	int8_t rssi_threshold;
	uint8_t last_hop_mac;
	uint8_t mac_filter_num;
	uint8_t aes_ctr[4];

// Buffer Management 
	uint8_t payload_start;
	uint8_t *buf;
	uint8_t buf_len;
	uint8_t *payload;
	uint8_t payload_len;

// Special Flags
	uint8_t is_mac_selected;
	int8_t rssi;

} SAMPL_DOWNSTREAM_PKT_T;

typedef struct sampl_gateway_pkt
{
// Common Header
	uint8_t protocol_id;
	uint8_t protocol_version;
	uint8_t pkt_type;
	uint8_t ctrl_flags;  
	uint8_t subnet_mac[3];
	uint8_t ack_retry;
	uint8_t priority;  
	uint8_t seq_num;  


// GW Specific Values
	uint8_t error_code;  
	uint8_t num_msgs;
	uint8_t src_mac;    
	uint8_t dst_mac;   // 255 is a broadcast
	uint8_t last_hop_mac;   
	int8_t rssi;



// Buffer Management 
	uint8_t payload_start;
	uint8_t *buf;
	uint8_t buf_len;
	uint8_t *payload;
	uint8_t payload_len;

}SAMPL_GATEWAY_PKT_T;



typedef struct sampl_upstream_pkt
{
// Common Header
	uint8_t protocol_id;
	uint8_t protocol_version;
	uint8_t pkt_type;
	uint8_t ctrl_flags;  
	uint8_t subnet_mac[3];
	uint8_t ack_retry;
	uint8_t priority;  
	uint8_t seq_num;  


// US Specific Values
	uint8_t error_code;  // Allow errors to be sent up
	uint8_t next_hop_dst_mac;
	uint8_t last_hop_src_mac;
	uint8_t num_msgs;

// Buffer Management 
	uint8_t payload_start;
	uint8_t *buf;
	uint8_t buf_len;
	uint8_t *payload;
	uint8_t payload_len;

// Special Flags
	int8_t rssi;
}SAMPL_UPSTREAM_PKT_T;

typedef struct sampl_peer_2_peer_pkt
{
// Common Header
	uint8_t protocol_id;
	uint8_t protocol_version;
	uint8_t pkt_type;
	uint8_t ctrl_flags;  
	uint8_t subnet_mac[3];
	uint8_t ack_retry;
	uint8_t priority;    
	uint8_t seq_num;   

// General Purpose Packet Specific 
	uint8_t ttl;   // Time to live   
	uint8_t src_mac;    
	uint8_t dst_mac;   // 255 is a broadcast
	uint8_t last_hop_mac;   
	uint8_t next_hop_mac;   
	uint8_t check_rate;     


// Buffer Management 
	uint8_t payload_start;
	uint8_t *buf;
	uint8_t buf_len;
	uint8_t *payload;
	uint8_t payload_len;

// Special Flags
	int8_t rssi;
}SAMPL_PEER_2_PEER_PKT_T;




#endif
