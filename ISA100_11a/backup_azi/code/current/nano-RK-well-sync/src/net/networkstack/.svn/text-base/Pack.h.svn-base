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


/* This file contains the constants required for the implementation of packing functions 
	for the various data structures of the network stack
*/

#ifndef _PACK_H
#define _PACK_H

#include "NetworkLayer.h"
#include "TransportLayerUDP.h"
#include "Serial.h"
#include <stdint.h>


// return types of endianness()
#define LITTLE_ENDIAN 1
#define BIG_ENDIAN 2

/******************************** FUNCTION PROTOTYPES ***********************************/
int8_t endianness();
/*
This function calculates and returns the endianness of the sensor node 

	PARAMS:		None
	RETURNS: 	LITTLE_ENDIAN or 
					BIG_ENDIAN			depending on the endianness of the node
					NRK_ERROR if any error is encountered during calculation 
	
	ERROR NOS:	ERROR_ENDIAN
	Comments:	Called during initialization of the node.  
*/

// packing functions 
void pack_Neighbor(uint8_t*, Neighbor*);
void pack_Msg_Hello(uint8_t*, Msg_Hello*);
void pack_NeighborList(uint8_t*, NeighborList*);
void pack_Msg_NgbList(uint8_t*, Msg_NgbList*);
void pack_RoutingTableEntry(uint8_t *dest, RoutingTable *rt);
void pack_RoutingTable(uint8_t *dest, RoutingTable rt[]);
void pack_Msg_RoutingTable(uint8_t *dest, Msg_RoutingTable *m);
void pack_NW_Packet_header(uint8_t*, NW_Packet*);
void pack_NodeToGatewaySerial_Packet_header(uint8_t *dest, NodeToGatewaySerial_Packet *pkt);
void pack_GatewayToNodeSerial_Packet_header(uint8_t *dest, GatewayToNodeSerial_Packet *pkt);
void pack_TL_UDP_header(uint8_t *, Transport_Segment_UDP*);

// unpacking functions 
void unpack_Neighbor(Neighbor*, uint8_t*);
void unpack_Msg_Hello(Msg_Hello*, uint8_t*);
void unpack_NeighborList(NeighborList*, uint8_t*);
void unpack_Msg_NgbList(Msg_NgbList*, uint8_t*);
void unpack_RoutingTableEntry(RoutingTable *rt, uint8_t *src);
void unpack_RoutingTable(RoutingTable rt[], uint8_t *src);
void unpack_Msg_RoutingTable(Msg_RoutingTable *m, uint8_t *src);
void unpack_NW_Packet_header(NW_Packet*, uint8_t*);
void unpack_NodeToGatewaySerial_Packet_header(NodeToGatewaySerial_Packet*, uint8_t *);
void unpack_GatewayToNodeSerial_Packet_header(GatewayToNodeSerial_Packet*, uint8_t *);
void unpack_TL_UDP_header(Transport_Segment_UDP*, uint8_t *);

// conversion functions
uint16_t hton(uint16_t);		// host - network byte order 
uint16_t ntoh(uint16_t);		// network - host byte order 

// Network byte order is by convention BIG_ENDIAN 

				
#endif 

