gvdfgdf//#ifdef _FLOOD_H
//#define _FLOOD_H

#include <stdint.h>

#define FALSE 0
#define TRUE 1	

#define MAX_NGBS 5					// maximum number of neighbors recorded
#define HELLO_PERIOD 2  			// every HELLO_PERIOD seconds, a HELLO msg will be sent
#define MAX_NETWORK_DIAMETER 3	// maximum number of hops to the gateway
#define MAX_NETWORK_PAYLOAD 24	// maximum size of network layer payload
#define NGB_LIST_PERIOD 5			// every NGB_LIST_PERIOD seconds, a NGB_LIST msg will be sent
#define MAX_DATA_PAYLOAD 32		// maximum size of application layer payload
#define BCAST_ADDR 255				// Reserved address
#define TIMEOUT_COUNTER	3			// Each node has to report to it's neighbor within this //count
#define MAX_SERIAL_PAYLOAD 32		// maximum size of serial data payload
#define ADDR_LENGTH 5				// length of a node address in the sensor network 
#define CONNECTED_TO_GATEWAY TRUE

/* This structure stores information about one neighbor */

typedef struct
{
	uint16_t addr;			// Node ID
	int8_t rssi;			// strength of received signal
	uint8_t lastReport;	// last time the node reported
	uint8_t isNew;			// flag to indicate if the neighbor appeared newly 
}Neighbor;

/* This structure holds the current information about all the neighbors of a node */
typedef struct
{
	uint16_t my_addr;	
	Neighbor ngbs[MAX_NGBS];
	uint8_t count;
}NeighborList;

/* This structure represents the "Hello" message */
typedef struct
{
	Neighbor n;
}Msg_Hello;

typedef struct
{
	NeighborList nl;
}Msg_NgbList;

/* This structure represents a network layer packet */

typedef struct 
{
	uint16_t src;							// source address
	uint16_t dest;							// destination address
	uint16_t seq;							// seq number of packet
	uint8_t ttl;							// time to live
	uint8_t ackRequested;		  		// whether the node wants an ACK back or not
	uint8_t packetType;					// type of packet
	uint8_t packetSubType;				// sub-type of packet
	uint8_t length;						// length of payload
	uint8_t data[MAX_DATA_PAYLOAD];	// payload
}NW_Packet;

// This structure represents a packet sent by a node to the gateway over a serial connection
typedef struct
{
	uint8_t packetType;
	uint8_t packetSubType;
	uint8_t length;
	uint8_t data[MAX_SERIAL_PAYLOAD];

}NodeToGatewaySerial_Packet;

typedef struct
{
	uint8_t a;
}GatewayToNodeSerial_Packet;

/* possible values of the packet type */
#define APPLICATION 0
#define NW_CONTROL 1

/* possible values of the packet sub type */
#define HELLO	0
#define NGB_LIST 1

/* possible values of the return type of add_neighbor */
#define MAX_NEIGHBOR_LIMIT_REACHED 1
#define NEIGHBOR_ALREADY_IN_LIST 2
#define NEIGHBOR_ADDED 3

/* possible values of return type of shouldIMultihop */
#define MULTIHOP_YES 1
#define MULTIHOP_NO 2

/* possible values of return type of endianness */
#define LITTLE_ENDIAN 1
#define BIG_ENDIAN 2
#define ERROR_ENDIAN 3

/* actual sizes of the various structs without padding */
#define SIZE_NEIGHBOR 5
#define SIZE_NEIGHBORLIST (3 + MAX_NGBS * SIZE_NEIGHBOR)

#define SIZE_MSG_HELLO SIZE_NEIGHBOR
#define SIZE_MSG_NGB_LIST SIZE_NEIGHBORLIST

#define SIZE_NW_PACKET_HEADER 11 
#define SIZE_NW_PACKET (SIZE_NW_PACKET_HEADER + MAX_DATA_PAYLOAD)

#define SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER 3 
#define SIZE_NODETOGATEWAYSERIAL_PACKET (SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER + MAX_SERIAL_PAYLOAD)
