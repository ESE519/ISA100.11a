/* This file contains the necessary functions for marshalling and unmarshalling of network stack 
   data structures for the Firefly node
   
   Authors:
   Aditya Bhave
*/

/******************************** Include files *******************************************/
#include "Pack.h"
#include "NWErrorCodes.h"
#include <nrk_error.h>

/******************************** Data structures *****************************************/
static int8_t endian;		// stores the endianess of this node

/******************************** Functions ***********************************************/
int8_t endianness()		// CHECKED
{
	uint16_t n = 0x0102;
	uint8_t *ptr = (uint8_t*)(&n);
	
	if(ptr[0] == 2 && ptr[1] == 1)
	{
		endian = LITTLE_ENDIAN;
		return LITTLE_ENDIAN;
	}		
	if(ptr[0] == 1 && ptr[1] == 2)
	{
		endian = BIG_ENDIAN;	
		return BIG_ENDIAN;
	}		
	
	return ERROR_ENDIAN;
}

/****************************** PACKING FUNCTIONS ****************************************/
void pack_Neighbor(uint8_t *dest, Neighbor *n)		// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(n -> addr)) + 1 );
		dest[1] = *( (uint8_t*)(&(n -> addr)) );
		
		//dest[2] = *( (uint8_t*)(&(n -> lastSeqNo)) + 1 );
		//dest[3] = *( (uint8_t*)(&(n -> lastSeqNo)) );
		
	}	
	else //  BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(n -> addr)) );
		dest[1] = *( (uint8_t*)(&(n -> addr)) + 1 );
		
		//dest[2] = *( (uint8_t*)(&(n -> lastSeqNo)) );
		//dest[3] = *( (uint8_t*)(&(n -> lastSeqNo)) + 1 );
	}
	
	dest[2] = n -> rssi;
	dest[3] = n -> lastReport;
	dest[4] = n -> isNew;
	
	return;
}

void pack_Msg_Hello(uint8_t *dest, Msg_Hello *m)	// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(m -> dg)) + 1 );
		dest[1] = *( (uint8_t*)(&(m -> dg)) );
		dest[2] = *( (uint8_t*)(&(m -> addr)) + 1 );
		dest[3] = *( (uint8_t*)(&(m -> addr)) );
	}
	else //  BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(m -> dg)) );
		dest[1] = *( (uint8_t*)(&(m -> dg)) + 1 );
		dest[2] = *( (uint8_t*)(&(m -> addr)) );
		dest[3] = *( (uint8_t*)(&(m -> addr)) + 1 );
	}
	return;
}
/*****************************************************************************************/
void pack_NeighborList(uint8_t *dest, NeighborList *n)		// CHECKED
{
	uint8_t i,j;		// loop indices 	
	
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(n -> addr)) + 1 );
		dest[1] = *( (uint8_t*)(&(n -> addr)) );
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(n -> addr)) );
		dest[1] = *( (uint8_t*)(&(n -> addr)) + 1 );
	}
	
	// pack MAX_NGBS neighbors into the destination buffer	
	for(i = 0, j = 2; i < MAX_NGBS; i++, j += SIZE_NEIGHBOR)
		pack_Neighbor( dest + j, &(n -> ngbs[i]) );
		
	dest[j] = n -> count;
	
	return;
}

void pack_Msg_NgbList(uint8_t *dest, Msg_NgbList *m)		// CHECKED
{
	pack_NeighborList(dest, &(m -> nl));
	return;
}
/******************************************************************************************/
void pack_RoutingTableEntry(uint8_t *dest, RoutingTableEntry *rte)	// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(rte -> dest)) + 1 );
		dest[1] = *( (uint8_t*)(&(rte -> dest)) );
		
		dest[2] = *( (uint8_t*)(&(rte -> nextHop)) + 1 );
		dest[3] = *( (uint8_t*)(&(rte -> nextHop)) );
	}	
	else //  BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(rte -> dest)) );
		dest[1] = *( (uint8_t*)(&(rte -> dest)) + 1 );
		
		dest[2] = *( (uint8_t*)(&(rte -> nextHop)) );
		dest[3] = *( (uint8_t*)(&(rte -> nextHop)) + 1 );
	}
	dest[4] = rte -> cost;
	return;
}

void pack_RoutingTable(uint8_t *dest, RoutingTable *rt)				// CHECKED
{
	int8_t i,j;
	
	for(i = 0, j = 0; i < NUM_ROUTING_TABLE_ENTRIES; i++, j += SIZE_ROUTING_TABLE_ENTRY)
	{
		pack_RoutingTableEntry(dest + j, &(rt -> rte[i]));
	}
	return;
}
void pack_Msg_RoutingTable(uint8_t *dest, Msg_RoutingTable *m)		// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(m -> dg)) + 1 );
		dest[1] = *( (uint8_t*)(&(m -> dg)) );
			
		dest[2] = *( (uint8_t*)(&(m -> addr)) + 1 );
		dest[3] = *( (uint8_t*)(&(m -> addr)) );
			
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(m -> dg)) );
		dest[1] = *( (uint8_t*)(&(m -> dg)) + 1 );
		
		dest[2] = *( (uint8_t*)(&(m -> addr)) );
		dest[3] = *( (uint8_t*)(&(m -> addr)) + 1 );
	}
	
	pack_RoutingTable(dest + 4, &(m -> rt));
	return;
}
/*******************************************************************************************/
void pack_RouteReplyElement(uint8_t *dest, RouteReplyElement *rre)	// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(rre -> src)) + 1 );
		dest[1] = *( (uint8_t*)(&(rre -> src)) );
			
		dest[2] = *( (uint8_t*)(&(rre -> nextHop)) + 1 );
		dest[3] = *( (uint8_t*)(&(rre -> nextHop)) );
			
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(rre -> src)) );
		dest[1] = *( (uint8_t*)(&(rre -> src)) + 1 );
		
		dest[2] = *( (uint8_t*)(&(rre -> nextHop)) );
		dest[3] = *( (uint8_t*)(&(rre -> nextHop)) + 1 );
	}
	
	dest[4] = rre -> cost;
	
	return;
}

void pack_Msg_RouteReply(uint8_t *dest, Msg_RouteReply *m)			// CHECKED
{
	int8_t i, j;	// loop indices
	
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(m -> dg)) + 1 );
		dest[1] = *( (uint8_t*)(&(m -> dg)) );
		
		dest[2] = *( (uint8_t*)(&(m -> dest)) + 1 );
		dest[3] = *( (uint8_t*)(&(m -> dest)) );
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(m -> dg)) );
		dest[1] = *( (uint8_t*)(&(m -> dg)) + 1 );
		
		dest[2] = *( (uint8_t*)(&(m -> dest)) );
		dest[3] = *( (uint8_t*)(&(m -> dest)) + 1 );
	}	
	
	dest[4] = m -> seq_no;	 
	
	for(i = 0, j = 5; i < NUM_ROUTE_REPLY_ELEMENTS; i++, j += SIZE_ROUTE_REPLY_ELEMENT)
		pack_RouteReplyElement(dest + j, &(m -> rre[i]));
		
	return;
}

/*******************************************************************************************/
void pack_NW_Packet_header(uint8_t *dest, NW_Packet *pkt)			// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(pkt -> src)) + 1 );
		dest[1] = *( (uint8_t*)(&(pkt -> src)) );
				
		dest[2] = *( (uint8_t*)(&(pkt -> dest)) + 1 );
		dest[3] = *( (uint8_t*)(&(pkt -> dest)) );
		
		dest[4] = *( (uint8_t*)(&(pkt -> nextHop)) + 1 );
		dest[5] = *( (uint8_t*)(&(pkt -> nextHop)) );
		
		dest[6] = *( (uint8_t*)(&(pkt -> prevHop)) + 1 );
		dest[7] = *( (uint8_t*)(&(pkt -> prevHop)) );
		
		dest[8] = *( (uint8_t*)(&(pkt -> prevprevHop)) + 1 );
		dest[9] = *( (uint8_t*)(&(pkt -> prevprevHop)) );
		
		dest[10] = *( (uint8_t*)(&(pkt -> seqNo)) + 1 );
		dest[11] = *( (uint8_t*)(&(pkt -> seqNo)) );
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(pkt -> src)) );
		dest[1] = *( (uint8_t*)(&(pkt -> src)) + 1 );
				
		dest[2] = *( (uint8_t*)(&(pkt -> dest)) );
		dest[3] = *( (uint8_t*)(&(pkt -> dest)) + 1 );
		
		dest[4] = *( (uint8_t*)(&(pkt -> nextHop)) );
		dest[5] = *( (uint8_t*)(&(pkt -> nextHop)) + 1 );
		
		dest[6] = *( (uint8_t*)(&(pkt -> prevHop)) );
		dest[7] = *( (uint8_t*)(&(pkt -> prevHop)) + 1 );
		
		dest[8] = *( (uint8_t*)(&(pkt -> prevprevHop)) );
		dest[9] = *( (uint8_t*)(&(pkt -> prevprevHop)) + 1 );
		
		dest[10] = *( (uint8_t*)(&(pkt -> seqNo)) );
		dest[11] = *( (uint8_t*)(&(pkt -> seqNo)) + 1 );
	}
	
	dest[12] = pkt -> ttl;
	dest[13] = pkt -> type;
	dest[14] = pkt -> length;
	dest[15] = pkt -> prio;
	
	return;
}
/*******************************************************************************************/
void pack_NodeToGatewaySerial_Packet_header(uint8_t *dest, NodeToGatewaySerial_Packet *pkt) //CHECKED
{
	dest[0] = pkt -> type;
	dest[1] = pkt -> length;
		
	return;
}
/******************************************************************************************/
void pack_GatewayToNodeSerial_Packet_header(uint8_t *dest, GatewayToNodeSerial_Packet *pkt) // CHECKED
{
	dest[0] = pkt -> type;
	dest[1] = pkt -> length;
		
	return;
}
/******************************************************************************************/
void pack_TL_UDP_header(uint8_t *dest, Transport_Segment_UDP* seg)	// CHECKED
{
	dest[0] = seg -> srcPort;
	dest[1] = seg -> destPort;
	dest[2] = seg -> length;
	
	return;
}
/*******************************************************************************************/
void pack_nrk_time_t(uint8_t *dest, nrk_time_t *time)	// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(time -> secs)) + 3 );
		dest[1] = *( (uint8_t*)(&(time -> secs)) + 2 );
		dest[2] = *( (uint8_t*)(&(time -> secs)) + 1 );
		dest[3] = *( (uint8_t*)(&(time -> secs)) );
		 
		dest[4] = *( (uint8_t*)(&(time -> nano_secs)) + 3 );
		dest[5] = *( (uint8_t*)(&(time -> nano_secs)) + 2 );
		dest[6] = *( (uint8_t*)(&(time -> nano_secs)) + 1 );
		dest[7] = *( (uint8_t*)(&(time -> nano_secs)) );
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(time -> secs)) );
		dest[1] = *( (uint8_t*)(&(time -> secs)) + 1 );
		dest[2] = *( (uint8_t*)(&(time -> secs)) + 2 );
		dest[3] = *( (uint8_t*)(&(time -> secs)) + 3 );
				
		dest[4] = *( (uint8_t*)(&(time -> nano_secs)) );
		dest[5] = *( (uint8_t*)(&(time -> nano_secs)) + 1 );
		dest[6] = *( (uint8_t*)(&(time -> nano_secs)) + 2 );
		dest[7] = *( (uint8_t*)(&(time -> nano_secs)) + 3 );
	}
	
	return;
}
/*********************************************************************************************/
void pack_NodeState(uint8_t *dest, NodeState *n)		// CHECKED 
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(n -> battery)) + 1 );
		dest[1] = *( (uint8_t*)(&(n -> battery)) );
		
		dest[2] = *( (uint8_t*)(&(n -> timestamp)) + 3 );
		dest[3] = *( (uint8_t*)(&(n -> timestamp)) + 2 );
		dest[4] = *( (uint8_t*)(&(n -> timestamp)) + 1 );
		dest[5] = *( (uint8_t*)(&(n -> timestamp)) );
		
		dest[6] = *( (uint8_t*)(&(n -> active_time)) + 3 );
		dest[7] = *( (uint8_t*)(&(n -> active_time)) + 2 );
		dest[8] = *( (uint8_t*)(&(n -> active_time)) + 1 );
		dest[9] = *( (uint8_t*)(&(n -> active_time)) );
		
		dest[10] = *( (uint8_t*)(&(n -> bytesSent)) + 1 );
		dest[11] = *( (uint8_t*)(&(n -> bytesSent)) );
		
		dest[12] = *( (uint8_t*)(&(n -> bytesReceived)) + 1 );
		dest[13] = *( (uint8_t*)(&(n -> bytesReceived)) );
		
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(n -> battery)) );
		dest[1] = *( (uint8_t*)(&(n -> battery)) + 1 );
		
		dest[2] = *( (uint8_t*)(&(n -> timestamp)) );
		dest[3] = *( (uint8_t*)(&(n -> timestamp)) + 1 );
		dest[4] = *( (uint8_t*)(&(n -> timestamp)) + 2 );
		dest[5] = *( (uint8_t*)(&(n -> timestamp)) + 3 );
		
		dest[6] = *( (uint8_t*)(&(n -> active_time)) );
		dest[7] = *( (uint8_t*)(&(n -> active_time)) + 1 );
		dest[8] = *( (uint8_t*)(&(n -> active_time)) + 2 );
		dest[9] = *( (uint8_t*)(&(n -> active_time)) + 3 );
		
		
		dest[10] = *( (uint8_t*)(&(n -> bytesSent)) );
		dest[11] = *( (uint8_t*)(&(n -> bytesSent)) + 1 );
		
		dest[12] = *( (uint8_t*)(&(n -> bytesReceived)) );
		dest[13] = *( (uint8_t*)(&(n -> bytesReceived)) + 1 );
	}	
	
	dest[14] = n -> total_pkts_inserted;
	dest[15] = n -> total_wait_time;
	dest[16] = n -> total_ack_time;
	dest[17] = n -> tx_queue_size;
	dest[18] = n -> rx_queue_size;
	
	return;
}
/******************************************************************************************/
void pack_Msg_RouteRequest(uint8_t *dest, Msg_RouteRequest *mrrq)	// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(mrrq -> src)) + 1 );
		dest[1] = *( (uint8_t*)(&(mrrq -> src)) );
		
		dest[2] = *( (uint8_t*)(&(mrrq -> dest)) + 1 );
		dest[3] = *( (uint8_t*)(&(mrrq -> dest)) );
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(mrrq -> src)) );
		dest[1] = *( (uint8_t*)(&(mrrq -> src)) + 1 );
		
		dest[2] = *( (uint8_t*)(&(mrrq -> dest)) );
		dest[3] = *( (uint8_t*)(&(mrrq -> dest)) + 1 );
		
	}	
	return;
}
/*****************************************************************************************/
void pack_Msg_NwInfoAcquired(uint8_t *dest, Msg_NwInfoAcquired *mnwia)	// CHECKED
{
	int8_t i, j;
	
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(mnwia -> dg)) + 1 );
		dest[1] = *( (uint8_t*)(&(mnwia -> dg)) );
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			dest[j] = *( (uint8_t*)(&(mnwia -> addrs[i])) + 1 );
			dest[j+1] = *( (uint8_t*)(&(mnwia -> addrs[i])) );
		}
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(mnwia -> dg)) );
		dest[1] = *( (uint8_t*)(&(mnwia -> dg)) + 1 );
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			dest[j] = *( (uint8_t*)(&(mnwia -> addrs[i])) );
			dest[j+1] = *( (uint8_t*)(&(mnwia -> addrs[i])) + 1 );
		}
	}	
	
	dest[j] = mnwia -> seq_no;
	return;
}
/******************************************************************************************/
void pack_Msg_NodeInfo(uint8_t *dest, Msg_NodeInfo *mni)
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(mni -> addr)) + 1 );
		dest[1] = *( (uint8_t*)(&(mni -> addr)) );
		
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(mni-> addr)) );
		dest[1] = *( (uint8_t*)(&(mni -> addr)) + 1 );
	}
	pack_NodeStateQueue(dest + 2, &(mni -> nsq));
	
	return;	
}
/******************************************************************************************/
void pack_NodeStateQueue(uint8_t *dest, NodeStateQueue *nsq)
{
	int8_t i,j;
	
	dest[0] = nsq -> front;
	dest[1] = nsq -> rear;
	
	for(i = 0, j = 2; i < MAX_STATE_VALUES; i++, j += SIZE_NODE_STATE)
		pack_NodeState(dest + j, &(nsq -> ns[i]));
		
	return;
}
/******************************************************************************************/
void pack_Msg_SendNwInfo(uint8_t *dest, Msg_SendNwInfo *msnwi)			// CHECKED
{
	int8_t i, j;
	
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(msnwi -> dg)) + 1 );
		dest[1] = *( (uint8_t*)(&(msnwi -> dg)) );
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			dest[j] = *( (uint8_t*)(&(msnwi -> addrs[i])) + 1 );
			dest[j+1] = *( (uint8_t*)(&(msnwi -> addrs[i])) );
		}
			
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(msnwi -> dg)) );
		dest[1] = *( (uint8_t*)(&(msnwi -> dg)) + 1 );
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			dest[j] = *( (uint8_t*)(&(msnwi -> addrs[i])) );
			dest[j+1] = *( (uint8_t*)(&(msnwi -> addrs[i])) + 1 );
		}
	}	
	dest[j] = msnwi -> seq_no;
	
	return;
}
/***********************************************************************************************/
void pack_Msg_SendNodeInfo(uint8_t *dest, Msg_SendNodeInfo *msni)		// CHECKED
{
	int8_t i, j;
	
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(msni -> dg)) + 1 );
		dest[1] = *( (uint8_t*)(&(msni -> dg)) );
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			dest[j] = *( (uint8_t*)(&(msni -> addrs[i])) + 1 );
			dest[j+1] = *( (uint8_t*)(&(msni -> addrs[i])) );
		}
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(msni -> dg)) );
		dest[1] = *( (uint8_t*)(&(msni -> dg)) + 1 );
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			dest[j] = *( (uint8_t*)(&(msni -> addrs[i])) );
			dest[j+1] = *( (uint8_t*)(&(msni -> addrs[i])) + 1 );
		}
	}
	
	dest[j] = msni -> seq_no;	
	return;
}
/************************************************************************************************/	
void pack_Msg_NodeInfoAcquired(uint8_t *dest, Msg_NodeInfoAcquired *mnia) // CHECKED
{
	int8_t i, j;
	
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(mnia -> dg)) + 1 );
		dest[1] = *( (uint8_t*)(&(mnia -> dg)) );
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			dest[j] = *( (uint8_t*)(&(mnia -> addrs[i])) + 1 );
			dest[j+1] = *( (uint8_t*)(&(mnia -> addrs[i])) );
		}
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(mnia -> dg)) );
		dest[1] = *( (uint8_t*)(&(mnia -> dg)) + 1 );
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			dest[j] = *( (uint8_t*)(&(mnia -> addrs[i])) );
			dest[j+1] = *( (uint8_t*)(&(mnia -> addrs[i])) + 1 );
		}
		
	}
	
	dest[j] = mnia -> seq_no;	
	return;
}

/***************************** UNPACKING FUNCTIONS ***************************************/
void unpack_TL_UDP_header(Transport_Segment_UDP* seg, uint8_t *src)		// CHECKED
{
	seg -> srcPort = src[0];
	seg -> destPort = src[1];
	seg -> length = src[2];
	
	return;
}
/*****************************************************************************************/	
void unpack_Neighbor(Neighbor *n, uint8_t* src)			// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(n -> addr)) ) = src[1];
		*( (uint8_t*)(&(n -> addr)) + 1 ) = src[0];
		
		//*( (uint8_t*)(&(n -> lastSeqNo)) ) = src[3];
		//*( (uint8_t*)(&(n -> lastSeqNo)) + 1 ) = src[2];
	}
	else // BIG ENDIAN 
	{
		*( (uint8_t*)(&(n -> addr)) ) = src[0];
		*( (uint8_t*)(&(n -> addr)) + 1 ) = src[1];
		
		//*( (uint8_t*)(&(n -> lastSeqNo)) ) = src[2];
		//*( (uint8_t*)(&(n -> lastSeqNo)) + 1 ) = src[3];
	}
	
	n -> rssi = src[2];
	n -> lastReport = src[3];
	n -> isNew = src[4];
	
	return;
}

void unpack_Msg_Hello(Msg_Hello *m, uint8_t* src)		// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(m -> dg)) ) = src[1];
		*( (uint8_t*)(&(m -> dg)) + 1 ) = src[0];
		*( (uint8_t*)(&(m -> addr)) ) = src[3];
		*( (uint8_t*)(&(m -> addr)) + 1 ) = src[2];
	}
	else // BIG ENDIAN 
	{
		*( (uint8_t*)(&(m -> dg)) ) = src[0];
		*( (uint8_t*)(&(m -> dg)) + 1 ) = src[1];
		*( (uint8_t*)(&(m -> addr)) ) = src[2];
		*( (uint8_t*)(&(m -> addr)) + 1 ) = src[3];
	}
	return;
}
/*****************************************************************************************/
void unpack_NeighborList(NeighborList *nlist, uint8_t *src)	// CHECKED
{
	Neighbor n;
	uint8_t i,j; 		// loop indices 
	
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(nlist -> addr)) ) = src[1];
		*( (uint8_t*)(&(nlist -> addr)) + 1 ) = src[0];
	}
	else
	{
		*( (uint8_t*)(&(nlist -> addr)) ) = src[0];
		*( (uint8_t*)(&(nlist -> addr)) + 1 ) = src[1];
	}
	
	for(i = 0,j = 2; i < MAX_NGBS; i++, j += SIZE_NEIGHBOR)
	{
		unpack_Neighbor(&n, src + j);
		nlist -> ngbs[i] = n;
	}
	nlist -> count = src[j];
		
	return;
}

void unpack_Msg_NgbList(Msg_NgbList *m, uint8_t *src)		// CHECKED
{
	unpack_NeighborList(&(m -> nl), src);
	return;
}
/********************************************************************************************/
void unpack_RoutingTableEntry(RoutingTableEntry *rte, uint8_t *src)	// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(rte -> dest)) ) = src[1];
		*( (uint8_t*)(&(rte -> dest)) + 1 ) = src[0];
		
		*( (uint8_t*)(&(rte -> nextHop)) ) = src[3];
		*( (uint8_t*)(&(rte -> nextHop)) + 1 ) = src[2];
				
	}
	else // BIG ENDIAN 
	{
		*( (uint8_t*)(&(rte -> dest)) ) = src[0];
		*( (uint8_t*)(&(rte -> dest)) + 1 ) = src[1];
		
		*( (uint8_t*)(&(rte -> nextHop)) ) = src[2];
		*( (uint8_t*)(&(rte -> nextHop)) + 1 ) = src[3];
	}

	rte -> cost = src[4];	
	return;
}
void unpack_RoutingTable(RoutingTable *rt, uint8_t *src)			// CHECKED
{
	int8_t i, j;
	
	for(i = 0, j = 0; i < NUM_ROUTING_TABLE_ENTRIES; i++, j += SIZE_ROUTING_TABLE_ENTRY)
	{	
		unpack_RoutingTableEntry( &(rt -> rte[i]), src + j);
	}
	return;
}
void unpack_Msg_RoutingTable(Msg_RoutingTable *m, uint8_t *src)		// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(m -> dg)) ) = src[1];
		*( (uint8_t*)(&(m -> dg)) + 1 ) = src[0];
			
		*( (uint8_t*)(&(m -> addr)) ) = src[3];
		*( (uint8_t*)(&(m -> addr)) + 1 ) = src[2];
	}
	else
	{
		*( (uint8_t*)(&(m -> dg)) ) = src[0];
		*( (uint8_t*)(&(m -> dg)) + 1 ) = src[1];
			
		*( (uint8_t*)(&(m -> addr)) ) = src[2];
		*( (uint8_t*)(&(m -> addr)) + 1 ) = src[3];
	}
	
	unpack_RoutingTable(&(m -> rt), src + 4);
	return;
}

/*******************************************************************************************************/
void unpack_RouteReplyElement(RouteReplyElement *rre, uint8_t *src)	// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(rre -> src)) ) = src[1];
		*( (uint8_t*)(&(rre -> src)) + 1 ) = src[0];
			
		*( (uint8_t*)(&(rre -> nextHop)) ) = src[3];
		*( (uint8_t*)(&(rre -> nextHop)) + 1 ) = src[2];
	}
	else
	{
		*( (uint8_t*)(&(rre -> src)) ) = src[0];
		*( (uint8_t*)(&(rre -> src)) + 1 ) = src[1];
			
		*( (uint8_t*)(&(rre -> nextHop)) ) = src[2];
		*( (uint8_t*)(&(rre -> nextHop)) + 1 ) = src[3];
	}
	
	rre -> cost = src[4];
	
	return;
}

void unpack_Msg_RouteReply(Msg_RouteReply *m, uint8_t *src)			// CHECKED
{
	uint8_t i, j;
	
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(m -> dg)) ) = src[1];
		*( (uint8_t*)(&(m -> dg)) + 1 ) = src[0];
		
		*( (uint8_t*)(&(m -> dest)) ) = src[3];
		*( (uint8_t*)(&(m -> dest)) + 1 ) = src[2];
	}
	else
	{
		*( (uint8_t*)(&(m -> dg)) ) = src[0];
		*( (uint8_t*)(&(m -> dg)) + 1 ) = src[1];
		
		*( (uint8_t*)(&(m -> dest)) ) = src[2];
		*( (uint8_t*)(&(m -> dest)) + 1 ) = src[3];
	}
	
	m -> seq_no = src[4];
	
	for(i = 0, j = 5; i < NUM_ROUTE_REPLY_ELEMENTS; i++, j += SIZE_ROUTE_REPLY_ELEMENT)
		unpack_RouteReplyElement( &(m -> rre[i]), src + j);
		
	return;
} 	
/********************************************************************************************/
void unpack_nrk_time_t(nrk_time_t *time, uint8_t *src)		// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(time -> secs)) ) = src[3];
		*( (uint8_t*)(&(time -> secs)) + 1 ) = src[2];
		*( (uint8_t*)(&(time -> secs)) + 2 ) = src[1];
		*( (uint8_t*)(&(time -> secs)) + 3 ) = src[0];
		
		*( (uint8_t*)(&(time -> nano_secs)) ) = src[7];
		*( (uint8_t*)(&(time -> nano_secs)) + 1 ) = src[6];
		*( (uint8_t*)(&(time -> nano_secs)) + 2 ) = src[5];
		*( (uint8_t*)(&(time -> nano_secs)) + 3 ) = src[4];
		
	}
	else
	{
		*( (uint8_t*)(&(time -> secs)) ) = src[0];
		*( (uint8_t*)(&(time -> secs)) + 1 ) = src[1];
		*( (uint8_t*)(&(time -> secs)) + 2) = src[2];
		*( (uint8_t*)(&(time -> secs)) + 3 ) = src[3];
		
		*( (uint8_t*)(&(time -> nano_secs)) ) = src[4];
		*( (uint8_t*)(&(time -> nano_secs)) + 1 ) = src[5];
		*( (uint8_t*)(&(time -> nano_secs)) + 2) = src[6];
		*( (uint8_t*)(&(time -> nano_secs)) + 3 ) = src[7];
	}
	
	return;
}	
/********************************************************************************************/
void unpack_NodeState(NodeState *n, uint8_t *src)			// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(n -> battery)) ) = src[1];
		*( (uint8_t*)(&(n -> battery)) + 1 ) = src[0];
		
		*( (uint8_t*)(&(n -> timestamp)) ) = src[5];
		*( (uint8_t*)(&(n -> timestamp)) + 1 ) = src[4];
		*( (uint8_t*)(&(n -> timestamp)) + 2 ) = src[3];
		*( (uint8_t*)(&(n -> timestamp)) + 3 ) = src[2];
		
		*( (uint8_t*)(&(n -> active_time)) ) = src[9];
		*( (uint8_t*)(&(n -> active_time)) + 1 ) = src[8];
		*( (uint8_t*)(&(n -> active_time)) + 2 ) = src[7];
		*( (uint8_t*)(&(n -> active_time)) + 3 ) = src[6];
		
		*( (uint8_t*)(&(n -> bytesSent)) ) = src[11];
		*( (uint8_t*)(&(n -> bytesSent)) + 1 ) = src[10];
		
		*( (uint8_t*)(&(n -> bytesReceived)) ) = src[13];
		*( (uint8_t*)(&(n -> bytesReceived)) + 1 ) = src[12];
		
	}
	else  // BIG_ENDIAN
	{
		*( (uint8_t*)(&(n -> battery)) ) = src[0];
		*( (uint8_t*)(&(n -> battery)) + 1 ) = src[1];
		
		*( (uint8_t*)(&(n -> timestamp)) ) = src[2];
		*( (uint8_t*)(&(n -> timestamp)) + 1 ) = src[3];
		*( (uint8_t*)(&(n -> timestamp)) + 2) = src[4];
		*( (uint8_t*)(&(n -> timestamp)) + 3 ) = src[5];
		
		*( (uint8_t*)(&(n -> active_time)) ) = src[6];
		*( (uint8_t*)(&(n -> active_time)) + 1 ) = src[7];
		*( (uint8_t*)(&(n -> active_time)) + 2) = src[8];
		*( (uint8_t*)(&(n -> active_time)) + 3 ) = src[9];
				
		*( (uint8_t*)(&(n -> bytesSent)) ) = src[10];
		*( (uint8_t*)(&(n -> bytesSent)) + 1 ) = src[11];
		
		*( (uint8_t*)(&(n -> bytesReceived)) ) = src[12];
		*( (uint8_t*)(&(n -> bytesReceived)) + 1 ) = src[13];
		
	}
	
	n -> total_pkts_inserted = src[14];
	n -> total_wait_time = src[15];
	n -> total_ack_time = src[16];
	n -> tx_queue_size = src[17];
	n -> rx_queue_size = src[18];
		
	return;
}
/*********************************************************************************************/
void unpack_Msg_RouteRequest(Msg_RouteRequest *mrrq, uint8_t *src)	// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(mrrq -> src)) ) = src[1];
		*( (uint8_t*)(&(mrrq -> src)) + 1 ) = src[0];
		
		*( (uint8_t*)(&(mrrq -> dest)) ) = src[3];
		*( (uint8_t*)(&(mrrq -> dest)) + 1 ) = src[2];
		
	}
	else  // BIG_ENDIAN
	{
		*( (uint8_t*)(&(mrrq -> src)) ) = src[0];
		*( (uint8_t*)(&(mrrq -> src)) + 1 ) = src[1];
		
		*( (uint8_t*)(&(mrrq -> dest)) ) = src[2];
		*( (uint8_t*)(&(mrrq -> dest)) + 1 ) = src[3];
		
	}
	return;
}
/********************************************************************************************/
void unpack_NW_Packet_header(NW_Packet *pkt, uint8_t* src)			// CHECKED
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(pkt -> src)) ) = src[1];
		*( (uint8_t*)(&(pkt -> src)) + 1 ) = src[0];
		
		*( (uint8_t*)(&(pkt -> dest)) ) = src[3];
		*( (uint8_t*)(&(pkt -> dest)) + 1 ) = src[2];
		
		*( (uint8_t*)(&(pkt -> nextHop)) ) = src[5];
		*( (uint8_t*)(&(pkt -> nextHop)) + 1 ) = src[4];
		
		*( (uint8_t*)(&(pkt -> prevHop)) ) = src[7];
		*( (uint8_t*)(&(pkt -> prevHop)) + 1 ) = src[6];
		
		*( (uint8_t*)(&(pkt -> prevprevHop)) ) = src[9];
		*( (uint8_t*)(&(pkt -> prevprevHop)) + 1 ) = src[8];
		
		*( (uint8_t*)(&(pkt -> seqNo)) ) = src[11];
		*( (uint8_t*)(&(pkt -> seqNo)) + 1 ) = src[10];
		
	}
	else  // BIG_ENDIAN
	{
		*( (uint8_t*)(&(pkt -> src)) ) = src[0];
		*( (uint8_t*)(&(pkt -> src)) + 1 ) = src[1];
		
		*( (uint8_t*)(&(pkt -> dest)) ) = src[2];
		*( (uint8_t*)(&(pkt -> dest)) + 1 ) = src[3];
		
		*( (uint8_t*)(&(pkt -> nextHop)) ) = src[4];
		*( (uint8_t*)(&(pkt -> nextHop)) + 1 ) = src[5];
		
		*( (uint8_t*)(&(pkt -> prevHop)) ) = src[6];
		*( (uint8_t*)(&(pkt -> prevHop)) + 1 ) = src[7];
		
		*( (uint8_t*)(&(pkt -> prevprevHop)) ) = src[8];
		*( (uint8_t*)(&(pkt -> prevprevHop)) + 1 ) = src[9];
		
		*( (uint8_t*)(&(pkt -> seqNo)) ) = src[10];
		*( (uint8_t*)(&(pkt -> seqNo)) + 1 ) = src[11];
			
	}
	
	pkt -> ttl = src[12];
	pkt -> type = src[13];
	pkt -> length = src[14];
	pkt -> prio = src[15];
	
	return;
}
/******************************************************************************************/
void unpack_NodeToGatewaySerial_Packet_header(NodeToGatewaySerial_Packet *pkt, uint8_t *src) // CHECKED
{
	pkt -> type = src[0];
	pkt -> length = src[1];
	
	return;
}
/********************************************************************************************/
void unpack_GatewayToNodeSerial_Packet_header(GatewayToNodeSerial_Packet *pkt, uint8_t *src)  // CHECKED
{
	pkt -> type = src[0];
	pkt -> length = src[1];
	
	return;
}
/*********************************************************************************************/
void unpack_Msg_NwInfoAcquired(Msg_NwInfoAcquired *mnwia, uint8_t *src)	// CHECKED
{
	int8_t i, j;
	
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(mnwia -> dg)) ) = src[1];
		*( (uint8_t*)(&(mnwia -> dg)) + 1 ) = src[0];
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			*( (uint8_t*)(&(mnwia -> addrs[i])) ) = src[j+1];
			*( (uint8_t*)(&(mnwia -> addrs[i])) + 1 ) = src[j];
		}
		
	}
	else // BIG ENDIAN 
	{
		*( (uint8_t*)(&(mnwia -> dg)) ) = src[0];
		*( (uint8_t*)(&(mnwia -> dg)) + 1 ) = src[1];
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			*( (uint8_t*)(&(mnwia -> addrs[i])) ) = src[j];
			*( (uint8_t*)(&(mnwia -> addrs[i])) + 1 ) = src[j+1];
		}
	}
	mnwia -> seq_no = src[j];
	
	return;
}
/*********************************************************************************************/
void unpack_Msg_NodeInfo(Msg_NodeInfo *m, uint8_t *src)
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(m -> addr)) ) = src[1];
		*( (uint8_t*)(&(m -> addr)) + 1 ) = src[0];
	}
	else  // BIG_ENDIAN
	{
		*( (uint8_t*)(&(m -> addr)) ) = src[0];
		*( (uint8_t*)(&(m -> addr)) + 1 ) = src[1];
	}
	
	unpack_NodeStateQueue(&(m -> nsq), src + 2);
	return;
}
/*********************************************************************************************/
void unpack_NodeStateQueue(NodeStateQueue *nsq, uint8_t *src)
{
	int8_t i, j;
	
	nsq -> front = src[0];
	nsq -> rear = src[1];
	
	for(i = 0, j = 2; i < MAX_STATE_VALUES; i++, j += SIZE_NODE_STATE)
		unpack_NodeState(&(nsq -> ns[i]), src + j);
		
	return;
}	
/*********************************************************************************************/
void unpack_Msg_SendNwInfo(Msg_SendNwInfo *msnwi, uint8_t *src)			// CHECKED
{
	int8_t i, j;
	
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(msnwi -> dg)) ) = src[1];
		*( (uint8_t*)(&(msnwi -> dg)) + 1 ) = src[0];
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			*( (uint8_t*)(&(msnwi -> addrs[i])) ) = src[j+1];
			*( (uint8_t*)(&(msnwi -> addrs[i])) + 1 ) = src[j];
		}
	}
	else // BIG ENDIAN 
	{
		*( (uint8_t*)(&(msnwi -> dg)) ) = src[0];
		*( (uint8_t*)(&(msnwi -> dg)) + 1 ) = src[1];
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			*( (uint8_t*)(&(msnwi -> addrs[i])) ) = src[j];
			*( (uint8_t*)(&(msnwi -> addrs[i])) + 1 ) = src[j+1];
		}
		
	}
	msnwi -> seq_no = src[j];
	
	return;
}

/*************************************************************************************************/
void unpack_Msg_SendNodeInfo(Msg_SendNodeInfo *msni, uint8_t *src)		// CHECKED
{
	int8_t i, j;
	
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(msni -> dg)) ) = src[1];
		*( (uint8_t*)(&(msni -> dg)) + 1 ) = src[0];
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			*( (uint8_t*)(&(msni -> addrs[i])) ) = src[j+1];
			*( (uint8_t*)(&(msni -> addrs[i])) + 1 ) = src[j];
		}
	}
	else // BIG ENDIAN 
	{
		*( (uint8_t*)(&(msni -> dg)) ) = src[0];
		*( (uint8_t*)(&(msni -> dg)) + 1 ) = src[1];
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			*( (uint8_t*)(&(msni -> addrs[i])) ) = src[j];
			*( (uint8_t*)(&(msni -> addrs[i])) + 1 ) = src[j+1];
		}
	}
	msni -> seq_no = src[j];
	
	return;
}
/***************************************************************************************************/
void unpack_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *mnia, uint8_t *src)	// CHECKED
{
	int8_t i, j;
	
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(mnia -> dg)) ) = src[1];
		*( (uint8_t*)(&(mnia -> dg)) + 1 ) = src[0];
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			*( (uint8_t*)(&(mnia -> addrs[i])) ) = src[j+1];
			*( (uint8_t*)(&(mnia -> addrs[i])) + 1 ) = src[j];
		}
	}
	else // BIG ENDIAN 
	{
		*( (uint8_t*)(&(mnia -> dg)) ) = src[0];
		*( (uint8_t*)(&(mnia -> dg)) + 1 ) = src[1];
		
		for(i = 0, j = 2; i < MAX_SUBNET_SIZE; i++, j += SIZE_NODE_ADDR)
		{
			*( (uint8_t*)(&(mnia -> addrs[i])) ) = src[j];
			*( (uint8_t*)(&(mnia -> addrs[i])) + 1 ) = src[j+1];
		}
		
	}
	mnia -> seq_no = src[j];
	
	return;
}	
/*********************************************************************************************/

uint16_t hton(uint16_t host)			// CHECKED
{
	uint16_t nw;
	uint8_t *src;
	uint8_t *dest;
	
	if(endian == BIG_ENDIAN)
		return host;
		
	src =  (uint8_t*)(&host);
	dest = (uint8_t*)(&nw);
		
	dest[0] = src[1];
	dest[1] = src[0];
	
	return nw;
}

/**********************************************************************************************/

uint16_t ntoh(uint16_t nw)				// CHECKED
{
	uint16_t host;
	uint8_t *src;
	uint8_t *dest;
	
	if(endian == BIG_ENDIAN)
		return nw;
		
	src = (uint8_t*)(&nw);
	dest = (uint8_t*)(&host);
	
	dest[0] = src[1];
	dest[1] = src[0];
	
	return host;
}

/***********************************************************************************************/
	
	
		
	
