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
*  Karthik Lakshmanan 
*******************************************************************************/


#ifndef TREE_FLOODING_H
#define TREE_FLOODING_H


/* NOTE: THE SIZE OF CONTROL PACKET SHOULD BE LESS THAN RF_MAX_PAYLOAD_SIZE	*/

#define MAX_NODES		30
#define WORDS_PER_NODE		2

#define GET_PACKET_TYPE(x)		((x&0xC0)>>6)
#define GET_PACKET_COUNT(x)		((x)&0x3F)	
#define IS_DATA_VALID(x)		((x>>7)&0x01)
#define IS_DATA_NEW(x)			((x>>6)&0x01)
#define SET_PACKET_TYPE(x,y)		(x = (((uint8_t)(x<<2))>>2)|((y&0x03)<<6))
#define SET_PACKET_COUNT(x,y)		(x = ((x>>6)<<6)|(y&0x3F))	
#define SET_DATA_VALID(x)		(x = (x|0x80))
#define SET_DATA_NEW(x)			(x = (x|0x40))
#define CLEAR_DATA_VALID(x)		(x = (x&0x7F))
#define CLEAR_DATA_NEW(x)		(x = (x&0xBF))


#define PKT_TYPE_DIAG_CONTROL	0
#define PKT_TYPE_DIAG_DATA	1
#define PKT_TYPE_CONTROL	2
#define	PKT_TYPE_DATA		3


struct message_packet
{
	uint8_t pkt_type_cnt;
	uint8_t hop_number;
	uint8_t time_to_flood;
	uint8_t bmac_check_rate;
	uint8_t maximum_depth;
	uint8_t delay_at_each_level;
	uint8_t next_control_time;
	uint8_t data_push_rate;
	uint8_t node_specific_data[MAX_NODES][WORDS_PER_NODE];

};



#endif
