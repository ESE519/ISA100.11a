#ifndef ISAMESSAGETYPES_H
#define ISAMESSAGETYPES_H

#include <dlmo.h>

// ********************************* DAUX Structure ********************************************

typedef struct {
	uint8_t adSelection;//DauxType 3 bits, ChMapOv 1 bit, DauxOptSlowHop 1 bit,Reserved 3 bit
	//Time Synchronization information
	uint32_t DauxTAIsecond;
	uint16_t DauxTAIfraction;
	//Superframe Information
	uint16_t DauxTsDur;
	uint16_t DauxChIndex;
	uint8_t DauxChBirth;
	uint16_t DauxSfPeriod;
	uint16_t DauxSfBirth;
	uint8_t DauxChRate;
	uint16_t DauxChMap;
	//Integrity Check
	uint16_t IntegrityCheck;
}DLMO_DAUX;

/***************************DROUT (COMPRESSED VARIANT)******************************************/



typedef struct{
	/*
	 * Compress. If this value is set to 1, the compressed variant of the DROUT format shall be
used.
• Priority. This shall be set to the DPDU’s 4-bit priority.
• DlForwardLimit and DlForwardLimitExt (forwarding limit) limit the number of times that a
   DPDU may be forwarded within a DL subnet. If the forwarding limit is less than 7, the
  value shall be transmitted in DlForwardLimit and DlForwardLimitExt shall be elided. If the
 forwarding limit is greater than or equal to 7, DlForwardLimit shall be transmitted as 7, and
the forwarding limit shall be transmitted in DlForwardLimitExt.
	 *
	 */
	uint8_t info; // Compress (1) Priority (4) DlForwardLimit (3)
	uint8_t DlForwardLimitExt;
	/*
	 * GraphID (8 bits). GraphIDs compliant with this standard are 12-bit unsigned integers. In
the common case where the route is a single graph ID in the range of 1-255, the
compressed variant of the DROUT sub-header shall be used. Additionally, the
compressed variant is used in single-hop source routing, wherein GraphID=0 shall indicate
that the destination is one hop away. Since the single hop destination address can be
found in the MHR, it does not need to be repeated in DROUT. GraphID=0 shall be used
during the join process for addressing to and from a neighboring proxy, and is the only
way in this standard to indicate a 64-bit destination address in DROUT.
	 *
	 */
	uint8_t GraphId;
}DLMO_DROUT;

/*
 * Thoughts on graph
 *
 * One we get the link, check the graph type. If graph type is 0, do what we were doing till now(later on-provided a neighbor is configured)
 * Else if graph type is type is 1 , then link is only for a graph. Find the neighbor (again oldest and highest priority)
 * that is in the queue and whose DROUT header specifies this graph. It should somehow store the number of tries it has had.
 * Based on this number, we select the neighbor to transmit to. If the tries exceed some number, remove from the queue.
 * If there is no one with this graph in the queue, then just do that normal sending to a neighbor (only if graph type is 2)
 *
 */




/*
 * Following are the message types that will go inside the payload
 * I am defining my own message types because they are a part of the
 * Application layer and this projects implementation is concerned with
 * data link layer implementation. These messages should gradually be
 * substituted by ISA complaint message types
 *
 */


typedef enum{
   NEIGHBOR_TABLE_REPORT,
   DUMMY_PAYLOAD,
   ADD_NEIGHBOR,
   ADD_GRAPH,
   ADD_LINK,
   FLUSH_CANDIDATE_TABLE
} message_type;


typedef struct {
	uint8_t numberOfNeighbors;		//can be an neighbor table report, or some other command
	uint8_t candidate;
}NEIGHBOR_TABLE;

typedef struct {
	uint8_t type;		//can be an neighbor table report, or some other command
	uint8_t data;
}MESSAGE;

typedef struct {    // This structure will be used to send and receive neighbor configuration information from uart
   uint16_t neighborId;
}CONFIG_NEIGHBOR;

typedef struct{   // This structure will be used to send and receive graph configuration information from uart
   uint8_t graphId;
   uint8_t neighborCount;
   uint16_t neigh1;
   uint16_t neigh2;
   uint16_t neigh3;
}CONFIG_GRAPH;

typedef struct {// This structure will be used to send and receive link configuration information from uart
   uint8_t slotNumber;
   uint16_t neighborId;
   uint8_t graphId;
   uint8_t linkType;
   uint8_t graphType;
}CONFIG_LINK;



#endif
