#include "NWStackConfigGateway.h"
#include <stdint.h>

/*********************************** Nano-RK **************************************************/
typedef struct {
   uint32_t secs;
   uint32_t nano_secs;
} nrk_time_t;

typedef int8_t nrk_sig_t;
typedef uint8_t nrk_sem_t;

/************************************ TransportLayerUDP *************************************/
// Types of sockets supported by the network stack 
#define SOCK_DGRAM 1 				// UDP-like 
#define SOCK_SEQPACKET 2 			// TCP-like 
#define SOCK_RAW 3 					// if the application wants to bypass the transport and network layers
#define SOCK_IPC 4 

#define EPHEMERAL_PORT_NUM_START 11		// 11 is the smallest port number available to user tasks
#define MAX_PORT_NUM 255
#define MAX_PORTS 127			 			// maximum number of ports that can be opened in the system 


// limits on various object types 
#define SIZE_TRANSPORT_UDP_HEADER 3
#define MAX_TRANSPORT_UDP_SEG (MAX_APP_PAYLOAD + SIZE_TRANSPORT_UDP_HEADER)

#define INVALID_PORT			0   // indicates an invalid port number 
#define INVALID_PID			0   // indicates an invalid pid (reserved for the kernel)

typedef struct
{
	uint8_t srcPort;						// source port of the application
	uint8_t destPort;						// destination port of the application
	int8_t length;							// actual length of the data payload
	uint8_t data[MAX_APP_PAYLOAD];	// application layer data 
		
}Transport_Segment_UDP;

typedef struct 
{
	uint8_t pno;							// the actual port number 
	nrk_sig_t data_arrived_signal;	// indicates that data has arrived for this port in the receive queue 
	nrk_sig_t send_done_signal;		// indicates that the highest priority message from this port 
												// has been sent over the radio 
}Port;

typedef struct
{
	int8_t pindex;							// index of the port element for this socket
	int8_t rbmindex;						// index of the rbm element for this socket	 
	int8_t pid;								// pid of the process holding this socket descriptor
	int8_t type;							// type of socket
	nrk_time_t timeout;					// to store a timeout value for the socket 
}Socket;

/********************************* NetworkLayer **********************************************/

#define MAX_NETWORK_PAYLOAD  MAX_TRANSPORT_UDP_SEG // maximum size of network packet payload 
#define BCAST_ADDR 65535				 // Reserved address
#define ADDR_LENGTH 5					 // length of a node address formatted as a character string 

#define LINK_STATE 1 					 // parameters for the routing algorithm 
#define FLOODING 2
#define PROBABILISTIC 1 
#define TTL_BASED 2 
#define RANDOM 1 
#define GAUSSIAN 2 

#define DEFAULT_ROUTING_ALGORITHM LINK_STATE
#define DEFAULT_FLOODING_TYPE TTL_BASED
#define DEFAULT_PDISTRIBUTION RANDOM

// sizes of various object types 
#define SIZE_NEIGHBOR 5
#define SIZE_NEIGHBORLIST (3 + MAX_NGBS * SIZE_NEIGHBOR)

#define SIZE_MSG_HELLO SIZE_NEIGHBOR
#define SIZE_MSG_NGB_LIST SIZE_NEIGHBORLIST

#define SIZE_NW_PACKET_HEADER 8  
#define SIZE_NW_PACKET (SIZE_NW_PACKET_HEADER + MAX_NETWORK_PAYLOAD)

#define SIZE_ROUTING_TABLE_ENTRY 5
#define SIZE_ROUTING_TABLE (SIZE_ROUTING_TABLE_ENTRY * MAX_NODES)
#define SIZE_MSG_ROUTING_TABLE (4 + SIZE_ROUTING_TABLE)

#define RF_BUFFER_SIZE 92 	// size of the receive buffer given to the link layer 

/* possible bit mask values of the packet type */
#define APPLICATION 0x00 	// 0 
#define NW_CONTROL 0x80 	// 1
 
/* possible bit mask values of the packet sub type for NW_CONTROL */
#define HELLO	0x80	 		// 000 
#define NGB_LIST 0x88		// 001 
#define ROUTE_CONFIG 0x90	// 010   
#define INVALID	0xFF		// 111 if the type is invalid

/* possible bit mask values of the packet sub type for APPLICATION */
#define UDP 0x00 	// 0 

/* possible values of return type of shouldIMultihop */
#define MULTIHOP_YES TRUE 
#define MULTIHOP_NO FALSE 

/* This structure stores information about one neighbor */
typedef struct 
{
	uint16_t addr;			// Node ID
	int8_t rssi;			// strength of received signal
	int8_t lastReport;	// last time the node reported
	int8_t isNew;			// flag to indicate if the neighbor appeared newly 
}Neighbor;

/* This structure holds the current information about all the neighbors of a node */
typedef struct 
{
	uint16_t my_addr;				// own node ID	
	Neighbor ngbs[MAX_NGBS];	// array of MAX_NGBS neighbors 
	int8_t count;					// actual number of neighbors recorded 
}NeighborList;

/* This structure represents the "Hello" message */
typedef struct 
{
	Neighbor n;
}Msg_Hello;

/* This structure represents the "NGB_LIST" message */
typedef struct
{
	NeighborList nl;
}Msg_NgbList;

/* This structure represents a network layer packet */
typedef struct
{
	uint16_t src;								// source address
	uint16_t dest;								// destination address
	uint16_t nextHop;							// next hop node 
	uint16_t prevHop;							// previous hop node
	int8_t ttl;									// time to live
	uint8_t type; 								// bit mask for packet type and subtype
													// bit 7:	APPLICATION OR NW_CONTROL 
													// bit 6:   Type of Tlayer 
													// bit 5,4,3: type of NW_CONTROL; HELLO, NGB_LIST, ROUTE_CONFIG
													// bit 2-0: unused, always set to 0        
	int8_t length;								// actual length of payload
	int8_t prio;								// priority of this packet 
	uint8_t data[MAX_NETWORK_PAYLOAD];	// payload
}NW_Packet;

/* This structure represents the routing table stored by the network layer */
typedef struct 
{
	uint16_t dest;
	uint16_t nextHop;
	uint8_t cost;
}RoutingTable;

/* This structure represents the routing table message propagated throughout the network*/
typedef struct
{
	uint16_t dg;						// address of the default gateway
	uint16_t node;
	RoutingTable rt[MAX_NODES];
}Msg_RoutingTable;

/*************************************** BufferManager *************************************/
#define DEFAULT_RX_QUEUE_SIZE 1 	// each port will be given at least this much receive buffer space 

// the constants below define the values of the 'status' field of ReceiveBufferUDP  
#define UNALLOCATED 			1	// has not been given to any user task 
#define EMPTY					2 	// allocated and free to receive the next segment from the network layer 
#define FULL					3	// allocated and contains a segment received from the network layer 

// the constants 'FULL' and 'EMPTY' also apply to the 'status' field of the TransmitBuffer
// with the obvious meanings 

// the constants below define the values of the excess policy  
#define OVERWRITE 0  
#define DROP 1 

// priority level of task ....SHIFT this to Anthony's code 
#define MAX_TASK_PRIORITY 19   

typedef struct ReceiveBufferUDP
{
	Transport_Segment_UDP seg;			// this holds one UDP segment for the application layer 
	int8_t status;						   // indicates the state of the receive buffer 
	int8_t prio;							// indicates the priority of the segment contained 
												// this is set by the sending node 
	int8_t rssi;							// signal strength with which the underlying packet was received
	uint16_t srcAddr;						// node from which this segment was sent  
	struct ReceiveBufferUDP *next;	// pointer for link list creation 
}ReceiveBufferUDP;

typedef struct
{
	int8_t pid;								// pid of task associated with the port
	int8_t pindex;							// index of associated port element 
	ReceiveBufferUDP *head_fq;			// pointer to head of free queue (EMPTY or FULL buffers)
	ReceiveBufferUDP *tail_fq;			// pointer to tail of free queue (EMPTY or FULL buffers) 
	ReceiveBufferUDP *head_pq; 		// pointer to head of port queue (FULL buffers)	
	ReceiveBufferUDP *tail_pq; 		// pointer to tail of port queue (FULL buffers) 
	int8_t countTotal;					// total number of buffers reserved for this port 
	int8_t countFree;						// number of buffers EMPTY at any time 
}ReceiveBufferManager;					// one for every port in the system 

typedef struct TransmitBuffer
{
	NW_Packet pkt;							// this holds the packet to be transmitted
	int8_t status;							// indicates the status of the transmit buffer 
	int8_t prio;							// priority of packet. This is set by the task that
												// inserts this packet in the transmit queue 
	
	struct TransmitBuffer *next;		// pointer for link list creation 
}TransmitBuffer;

typedef struct 
{
	TransmitBuffer *head_fq;		// pointer to head of free queue (EMPTY buffers) 
	TransmitBuffer *tail_fq;		// pointer to tail of free queue (EMPTY buffers)
	TransmitBuffer *head_aq;		// pointer to head of active queue (FULL buffers) 
	TransmitBuffer *tail_aq;		// pointer to tail of active queue (FULL buffers) 
	int8_t count_aq;					// count of number of buffers in active queue			
	int8_t count_fq;					// count of number of buffers in free queue 
}TransmitBufferManager;

/************************************** Serial *********************************************/

// SLIP character codes 
#define END             0300     /* indicates end of packet */
#define ESC             0333     /* indicates byte stuffing */
#define ESC_END         0334     /* ESC ESC_END means END data byte */
#define ESC_ESC         0335     /* ESC ESC_ESC means ESC data byte */

// possible bit mask values of the packet type 
#define SERIAL_APPLICATION 0x00	// 0 
#define SERIAL_NW_CONTROL	0x80  // 1 

// possible bit mask values of the packet sub type for SERIAL_NW_CONTROL
#define SERIAL_NGB_LIST 0x80		// 000  
#define SERIAL_ROUTE_CONFIG 0x80	// 000 
#define INVALID	0xFF				// 111 if the type is invalid

#define SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER 2  
#define SIZE_NODETOGATEWAYSERIAL_PACKET (SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER + MAX_SERIAL_PAYLOAD)

#define SIZE_GATEWAYTONODESERIAL_PACKET_HEADER 2
#define SIZE_GATEWAYTONODESERIAL_PACKET (SIZE_GATEWAYTONODESERIAL_PACKET_HEADER + MAX_GATEWAY_PAYLOAD)

// This structure represents a packet sent by a node to the gateway over a serial connection
typedef struct
{
	uint8_t type;						// bit mask for packet type and subtype
											// bit 7:	SERIAL_APPLICATION OR SERIAL_NW_CONTROL 
											// bit 6,5,4:  subtype for SERIAL_APPLICATION; set to 000 for now 
											// bit 3,2,1:  subtype of SERIAL_NW_CONTROL;
											//             SERIAL_NGB_LIST is the only option for now 
											// bit 0: unused, always set to 0 
	
	int8_t length;						// actual length of the serial payload 
	uint8_t data[MAX_SERIAL_PAYLOAD]; // serial payload 

}NodeToGatewaySerial_Packet;

typedef struct
{
	uint8_t type;						// bit mask for packet type and subtype
										// bit 7:	SERIAL_APPLICATION OR SERIAL_NW_CONTROL 
										// bit 6,5,4:  subtype for SERIAL_APPLICATION; set to 000 for now 
										// bit 3,2,1:  subtype of SERIAL_NW_CONTROL;
										//             SERIAL_ROUTE_CONFIG is the only option for now 
										// bit 0: unused, always set to 0 
	int8_t length;
	uint8_t data[MAX_GATEWAY_PAYLOAD];
}GatewayToNodeSerial_Packet;




