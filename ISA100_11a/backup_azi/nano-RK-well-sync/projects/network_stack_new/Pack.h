/* This file contains the constants required for the implementation of packing functions 
	for the various data structures of the network stack
	
	Authors:
	Aditya Bhave
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
				BIG_ENDIAN					depending on the endianness of the node
				ERROR_ENDIAN 				if any error is encountered during calculation 
	
	Comments:	Called during initialization of the node.  
*/

// packing functions 
void pack_Neighbor(uint8_t*, Neighbor*);
void pack_Msg_Hello(uint8_t*, Msg_Hello*);
void pack_NeighborList(uint8_t*, NeighborList*);
void pack_Msg_NgbList(uint8_t*, Msg_NgbList*);
void pack_RoutingTableEntry(uint8_t *dest, RoutingTableEntry *rt);
void pack_RoutingTable(uint8_t *dest, RoutingTable *rt);
void pack_Msg_RoutingTable(uint8_t *dest, Msg_RoutingTable *m);
void pack_RouteReplyElement(uint8_t *dest, RouteReplyElement *rre);
void pack_Msg_RouteReply(uint8_t *dest, Msg_RouteReply *m);
void pack_NW_Packet_header(uint8_t*, NW_Packet*);
void pack_NodeToGatewaySerial_Packet_header(uint8_t *dest, NodeToGatewaySerial_Packet *pkt);
void pack_GatewayToNodeSerial_Packet_header(uint8_t *dest, GatewayToNodeSerial_Packet *pkt);
void pack_TL_UDP_header(uint8_t *, Transport_Segment_UDP*);
void pack_NodeState(uint8_t*, NodeState*);
void pack_Msg_RouteRequest(uint8_t *, Msg_RouteRequest *);
void pack_Msg_NwInfoAcquired(uint8_t*, Msg_NwInfoAcquired*);
void pack_Msg_SendNwInfo(uint8_t*, Msg_SendNwInfo*);
void pack_Msg_SendNodeInfo(uint8_t*, Msg_SendNodeInfo*);
void pack_Msg_NodeInfoAcquired(uint8_t *, Msg_NodeInfoAcquired *);
void pack_nrk_time_t(uint8_t *, nrk_time_t *);
void pack_Msg_NodeInfo(uint8_t *dest, Msg_NodeInfo *mni);
void pack_NodeStateQueue(uint8_t *dest, NodeStateQueue *nsq);


// unpacking functions 
void unpack_Neighbor(Neighbor*, uint8_t*);
void unpack_Msg_Hello(Msg_Hello*, uint8_t*);
void unpack_NeighborList(NeighborList*, uint8_t*);
void unpack_Msg_NgbList(Msg_NgbList*, uint8_t*);
void unpack_RoutingTableEntry(RoutingTableEntry *rt, uint8_t *src);
void unpack_RoutingTable(RoutingTable *rt, uint8_t *src);
void unpack_Msg_RoutingTable(Msg_RoutingTable *m, uint8_t *src);
void unpack_RouteReplyElement(RouteReplyElement *rre, uint8_t *src);
void unpack_Msg_RouteReply(Msg_RouteReply *m, uint8_t *src);
void unpack_NW_Packet_header(NW_Packet *, uint8_t *);
void unpack_NodeToGatewaySerial_Packet_header(NodeToGatewaySerial_Packet *, uint8_t *);
void unpack_GatewayToNodeSerial_Packet_header(GatewayToNodeSerial_Packet *, uint8_t *);
void unpack_TL_UDP_header(Transport_Segment_UDP*, uint8_t *);
void unpack_NodeState(NodeState *, uint8_t *);
void unpack_Msg_NwInfoAcquired(Msg_NwInfoAcquired*, uint8_t *);
void unpack_Msg_SendNwInfo(Msg_SendNwInfo*, uint8_t *);
void unpack_Msg_SendNodeInfo(Msg_SendNodeInfo*, uint8_t *);
void unpack_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *, uint8_t *);
void unpack_nrk_time_t(nrk_time_t *, uint8_t *);
void unpack_Msg_NodeInfo(Msg_NodeInfo *m, uint8_t *src);
void unpack_NodeStateQueue(NodeStateQueue *nsq, uint8_t *src);



// conversion functions
uint16_t hton(uint16_t);		// host - network byte order 
uint16_t ntoh(uint16_t);		// network - host byte order 

// Network byte order is by convention BIG_ENDIAN 

				
#endif 

