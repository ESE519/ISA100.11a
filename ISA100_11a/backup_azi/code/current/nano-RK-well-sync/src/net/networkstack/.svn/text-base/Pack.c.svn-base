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
*  Aditya Bhave
*******************************************************************************/


//#include "NetworkLayer.h"
#include "Pack.h"
#include "NWErrorCodes.h"
//#include "TransportLayerUDP.h"
//#include "Serial.h"

static int8_t endian;

/******************************* CHECK ENDIANNESS ****************************************/\
int8_t endianness()
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
void pack_Neighbor(uint8_t *dest, Neighbor *n)
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(n -> addr)) + 1 );
		dest[1] = *( (uint8_t*)(&(n -> addr)) );
	}	
	else //  BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(n -> addr)) );
		dest[1] = *( (uint8_t*)(&(n -> addr)) + 1 );
	}
	
	dest[2] = n -> rssi;
	dest[3] = n -> lastReport;
	dest[4] = n -> isNew;

	return;
}

void pack_Msg_Hello(uint8_t *dest, Msg_Hello *m)
{
	pack_Neighbor(dest, &(m -> n));
	return;
}
/*****************************************************************************************/
void pack_NeighborList(uint8_t *dest, NeighborList *n)
{
	uint8_t i,j;		// loop indices 	
	
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(n -> my_addr)) + 1 );
		dest[1] = *( (uint8_t*)(&(n -> my_addr)) );
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(n -> my_addr)) );
		dest[1] = *( (uint8_t*)(&(n -> my_addr)) + 1 );
	}
	
	// pack MAX_NGBS neighbors into the destination buffer	
	for(i = 0, j = 2; i < MAX_NGBS; i++, j += SIZE_NEIGHBOR)
		pack_Neighbor( dest + j, &(n -> ngbs[i]) );
		
	dest[27] = n -> count;
	
	return;
}

void pack_Msg_NgbList(uint8_t *dest, Msg_NgbList *m)
{
	pack_NeighborList(dest, &(m -> nl));
	return;
}
/******************************************************************************************/
void pack_RoutingTableEntry(uint8_t *dest, RoutingTable *rt)
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(rt -> dest)) + 1 );
		dest[1] = *( (uint8_t*)(&(rt -> dest)) );
		
		dest[2] = *( (uint8_t*)(&(rt -> nextHop)) + 1 );
		dest[3] = *( (uint8_t*)(&(rt -> nextHop)) );
	}	
	else //  BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(rt -> dest)) );
		dest[1] = *( (uint8_t*)(&(rt -> dest)) + 1 );
		
		dest[2] = *( (uint8_t*)(&(rt -> nextHop)) );
		dest[3] = *( (uint8_t*)(&(rt -> nextHop)) + 1 );
	}
	dest[4] = rt -> cost;
	return;
}

void pack_RoutingTable(uint8_t *dest, RoutingTable rt[])
{
	int8_t i,j;
	
	for(i = 0, j = 0; i < MAX_NODES; i++, j += SIZE_ROUTING_TABLE_ENTRY)
	{
		pack_RoutingTableEntry(dest + j, &rt[i]);
	}
	return;
}
void pack_Msg_RoutingTable(uint8_t *dest, Msg_RoutingTable *m)
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(m -> dg)) + 1 );
		dest[1] = *( (uint8_t*)(&(m -> dg)) );
			
		dest[2] = *( (uint8_t*)(&(m -> node)) + 1 );
		dest[3] = *( (uint8_t*)(&(m -> node)) );
			
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(m -> dg)) );
		dest[1] = *( (uint8_t*)(&(m -> dg)) + 1 );
		
		dest[2] = *( (uint8_t*)(&(m -> node)) );
		dest[3] = *( (uint8_t*)(&(m -> node)) + 1 );
	}
	
	pack_RoutingTable(dest + 4, m -> rt);
	return;
}
/*******************************************************************************************/
void pack_NW_Packet_header(uint8_t *dest, NW_Packet *pkt)
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(pkt -> src)) + 1 );
		dest[1] = *( (uint8_t*)(&(pkt -> src)) );
				
		dest[2] = *( (uint8_t*)(&(pkt -> dest)) + 1 );
		dest[3] = *( (uint8_t*)(&(pkt -> dest)) );
		
		dest[4] = *( (uint8_t*)(&(pkt -> prevHop)) + 1 );
		dest[5] = *( (uint8_t*)(&(pkt -> prevHop)) );
		
		dest[6] = *( (uint8_t*)(&(pkt -> nextHop)) + 1 );
		dest[7] = *( (uint8_t*)(&(pkt -> nextHop)) );
		
			
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(pkt -> src)) );
		dest[1] = *( (uint8_t*)(&(pkt -> src)) + 1 );
				
		dest[2] = *( (uint8_t*)(&(pkt -> dest)) );
		dest[3] = *( (uint8_t*)(&(pkt -> dest)) + 1 );
		
		dest[4] = *( (uint8_t*)(&(pkt -> prevHop)) );
		dest[5] = *( (uint8_t*)(&(pkt -> prevHop)) + 1 );
		
		dest[6] = *( (uint8_t*)(&(pkt -> nextHop)) );
		dest[7] = *( (uint8_t*)(&(pkt -> nextHop)) + 1 );
		
				
	}
	dest[8] = pkt -> ttl;
	dest[9] = pkt -> type;
	dest[10] = pkt -> length;
	dest[11] = pkt -> prio;
	
	return;
}
/*******************************************************************************************/
void pack_NodeToGatewaySerial_Packet_header(uint8_t *dest, NodeToGatewaySerial_Packet *pkt)
{
	dest[0] = pkt -> type;
	dest[1] = pkt -> length;
		
	return;
}
/******************************************************************************************/
void pack_GatewayToNodeSerial_Packet_header(uint8_t *dest, GatewayToNodeSerial_Packet *pkt)
{
	dest[0] = pkt -> type;
	dest[1] = pkt -> length;
		
	return;
}
/******************************************************************************************/
void pack_TL_UDP_header(uint8_t *dest, Transport_Segment_UDP* seg)
{
	dest[0] = seg -> srcPort;
	dest[1] = seg -> destPort;
	dest[2] = seg -> length;
	
	return;
}

/*****************************************************************************************/

/***************************** UNPACKING FUNCTIONS ***************************************/
void unpack_TL_UDP_header(Transport_Segment_UDP* seg, uint8_t *src)
{
	seg -> srcPort = src[0];
	seg -> destPort = src[1];
	seg -> length = src[2];
	
	return;
}
/*****************************************************************************************/	
void unpack_Neighbor(Neighbor *n, uint8_t* src)
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(n -> addr)) ) = src[1];
		*( (uint8_t*)(&(n -> addr)) + 1 ) = src[0];
	}
	else // BIG ENDIAN 
	{
		*( (uint8_t*)(&(n -> addr)) ) = src[0];
		*( (uint8_t*)(&(n -> addr)) + 1 ) = src[1];
	}
	
	n -> rssi = src[2];
	n -> lastReport = src[3];
	n -> isNew = src[4];
	
	return;
}

void unpack_Msg_Hello(Msg_Hello *m, uint8_t* src)
{
	unpack_Neighbor(&(m -> n), src);
	return;
}
/*****************************************************************************************/
void unpack_NeighborList(NeighborList *nlist, uint8_t *src)
{
	Neighbor n;
	uint8_t i,j; 		// loop indices 
	
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(nlist -> my_addr)) ) = src[1];
		*( (uint8_t*)(&(nlist -> my_addr)) + 1 ) = src[0];
	}
	else
	{
		*( (uint8_t*)(&(nlist -> my_addr)) ) = src[0];
		*( (uint8_t*)(&(nlist -> my_addr)) + 1 ) = src[1];
	}
	
	for(i = 0,j = 2; i < MAX_NGBS; i++, j += SIZE_NEIGHBOR)
	{
		unpack_Neighbor(&n, src + j);
		nlist -> ngbs[i] = n;
	}
	nlist -> count = src[2 + MAX_NGBS * SIZE_NEIGHBOR];
		
	return;
}

void unpack_Msg_NgbList(Msg_NgbList *m, uint8_t *src)
{
	unpack_NeighborList(&(m -> nl), src);
	return;
}
/********************************************************************************************/
void unpack_RoutingTableEntry(RoutingTable *rt, uint8_t *src)
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(rt -> dest)) ) = src[1];
		*( (uint8_t*)(&(rt -> dest)) + 1 ) = src[0];
		
		*( (uint8_t*)(&(rt -> nextHop)) ) = src[3];
		*( (uint8_t*)(&(rt -> nextHop)) + 1 ) = src[2];
				
	}
	else // BIG ENDIAN 
	{
		*( (uint8_t*)(&(rt -> dest)) ) = src[0];
		*( (uint8_t*)(&(rt -> dest)) + 1 ) = src[1];
		
		*( (uint8_t*)(&(rt -> nextHop)) ) = src[2];
		*( (uint8_t*)(&(rt -> nextHop)) + 1 ) = src[3];
	}

	rt -> cost = src[4];	
	return;
}
void unpack_RoutingTable(RoutingTable rt[], uint8_t *src)
{
	int8_t i, j;
	
	for(i = 0, j = 0; i < MAX_NODES; i++, j += SIZE_ROUTING_TABLE_ENTRY)
	{	
		unpack_RoutingTableEntry(&rt[i], src + j);
	}
	return;
}
void unpack_Msg_RoutingTable(Msg_RoutingTable *m, uint8_t *src)
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(m -> dg)) ) = src[1];
		*( (uint8_t*)(&(m -> dg)) + 1 ) = src[0];
			
		*( (uint8_t*)(&(m -> node)) ) = src[3];
		*( (uint8_t*)(&(m -> node)) + 1 ) = src[2];
	}
	else
	{
		*( (uint8_t*)(&(m -> dg)) ) = src[0];
		*( (uint8_t*)(&(m -> dg)) + 1 ) = src[1];
			
		*( (uint8_t*)(&(m -> node)) ) = src[2];
		*( (uint8_t*)(&(m -> node)) + 1 ) = src[3];
	}
	
	unpack_RoutingTable(m -> rt, src + 4);
	return;
}
/********************************************************************************************/
void unpack_NW_Packet_header(NW_Packet *pkt, uint8_t* src)
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(pkt -> src)) ) = src[1];
		*( (uint8_t*)(&(pkt -> src)) + 1 ) = src[0];
		
		*( (uint8_t*)(&(pkt -> dest)) ) = src[3];
		*( (uint8_t*)(&(pkt -> dest)) + 1 ) = src[2];
		
		*( (uint8_t*)(&(pkt -> prevHop)) ) = src[5];
		*( (uint8_t*)(&(pkt -> prevHop)) + 1 ) = src[4];
		
		*( (uint8_t*)(&(pkt -> nextHop)) ) = src[7];
		*( (uint8_t*)(&(pkt -> nextHop)) + 1 ) = src[6];
		
	}
	else  // BIG_ENDIAN
	{
		*( (uint8_t*)(&(pkt -> src)) ) = src[0];
		*( (uint8_t*)(&(pkt -> src)) + 1 ) = src[1];
		
		*( (uint8_t*)(&(pkt -> dest)) ) = src[2];
		*( (uint8_t*)(&(pkt -> dest)) + 1 ) = src[3];
		
		*( (uint8_t*)(&(pkt -> prevHop)) ) = src[4];
		*( (uint8_t*)(&(pkt -> prevHop)) + 1 ) = src[5];
		
		*( (uint8_t*)(&(pkt -> nextHop)) ) = src[6];
		*( (uint8_t*)(&(pkt -> nextHop)) + 1 ) = src[7];
		
	}
	
	pkt -> ttl = src[8];
	pkt -> type = src[9];
	pkt -> length = src[10];
	pkt -> prio = src[11];
	
	return;
}
/******************************************************************************************/

void unpack_NodeToGatewaySerial_Packet_header(NodeToGatewaySerial_Packet *pkt, uint8_t *src)
{
	pkt -> type = src[0];
	pkt -> length = src[1];
	
	return;
}
/********************************************************************************************/
void unpack_GatewayToNodeSerial_Packet_header(GatewayToNodeSerial_Packet *pkt, uint8_t *src)
{
	pkt -> type = src[0];
	pkt -> length = src[1];
	
	return;
}
/*********************************************************************************************/

uint16_t hton(uint16_t host)
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

uint16_t ntoh(uint16_t nw)
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
	
	
		
	
