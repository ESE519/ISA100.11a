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

#define INVALID_PORT		0   // indicates an invalid port number 
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
#define BCAST_ADDR 65535						   // Reserved address
#define INVALID_ADDR 0							   // invalid address

/* The following interpretation is to to be followed for addresses 
 * 
 * In Ngb_List:
   ngbs[].addr = INVALID_ADDR means that the corresponding entry in the Neighbor array is free
   
 * In RoutingTableEntry:
   dest = INVALID_ADDR means that the corresponding entry in the array is free. Valid combinations
   (dest, nextHop, cost) = (INVALID_ADDR, INVALID_ADDR, 0)
   						 = (<valid address>, <valid address>, <valid cost>
 
 * In RouteReplyElement:
   src = INVALID_ADDR means the corresponding entry in the array is free. Valid combinations are
   (src, nextHop, cost) = (INVALID_ADDR, INVALID_ADDR, 0)
   						= (<valid address>, <valid address>, <valid cost>
 
 */

// sizes of various object types
#define SIZE_NODE_ADDR 2							// size of a sensor node address
#define MAX_NETWORK_PAYLOAD 64						// maximum size of network packet payload
#define SIZE_NEIGHBOR (SIZE_NODE_ADDR + 3)		   // size of the Neighbor struct
#define SIZE_NEIGHBORLIST (SIZE_NODE_ADDR + MAX_NGBS * SIZE_NEIGHBOR + 1)	// size of the NeigborList struct	

#define SIZE_MSG_HELLO (SIZE_NODE_ADDR * 2)
#define SIZE_MSG_NGB_LIST SIZE_NEIGHBORLIST

#define SIZE_NW_PACKET_HEADER (5 * SIZE_NODE_ADDR + 6)  
#define SIZE_NW_PACKET (SIZE_NW_PACKET_HEADER + MAX_NETWORK_PAYLOAD)

#define SIZE_ROUTING_TABLE_ENTRY (SIZE_NODE_ADDR * 2 + 1)
#define SIZE_ROUTING_TABLE (SIZE_ROUTING_TABLE_ENTRY * NUM_ROUTING_TABLE_ENTRIES)
#define SIZE_MSG_ROUTING_TABLE (SIZE_NODE_ADDR * 2 + SIZE_ROUTING_TABLE)

#define SIZE_ROUTE_REPLY_ELEMENT (SIZE_NODE_ADDR * 2 + 1)
#define NUM_ROUTE_REPLY_ELEMENTS 8
#define SIZE_MSG_ROUTE_REPLY (SIZE_NODE_ADDR * 2 + 1 + SIZE_ROUTE_REPLY_ELEMENT * NUM_ROUTE_REPLY_ELEMENTS)
#define SIZE_MSG_ROUTE_REQUEST (SIZE_NODE_ADDR * 2)

#define SIZE_NODE_STATE 19	// size of one entry in the table of node state values
#define SIZE_NODE_STATE_QUEUE (2 + SIZE_NODE_STATE * MAX_STATE_VALUES)

// related to SeqNoCache
#define NUM_ENTRIES_SNC MAX_SUBNET_SIZE
#define SNC_TIMEOUT 5

#define SIZE_MSG_SEND_NW_INFO (SIZE_NODE_ADDR + SIZE_NODE_ADDR * MAX_SUBNET_SIZE + 1)
#define SIZE_MSG_NW_INFO_ACQUIRED (SIZE_NODE_ADDR + SIZE_NODE_ADDR * MAX_SUBNET_SIZE + 1)
#define SIZE_MSG_SEND_NODE_INFO (SIZE_NODE_ADDR + SIZE_NODE_ADDR * MAX_SUBNET_SIZE + 1)
#define SIZE_MSG_NODE_INFO_ACQUIRED (SIZE_NODE_ADDR + SIZE_NODE_ADDR * MAX_SUBNET_SIZE + 1)
#define SIZE_MSG_NODE_INFO (SIZE_NODE_ADDR + SIZE_NODE_STATE_QUEUE)

#define RF_BUFFER_SIZE 92 	// size of the receive buffer given to the link layer 

/* possible bit mask values of the packet type */
#define APPLICATION 0x00 	// 0 
#define NW_CONTROL 0x80 	// 1
 
/* possible bit mask values of the packet sub type for NW_CONTROL */
#define HELLO	0x80	 		// 100 0 000 0 
#define NGB_LIST 0x82			// 100 0 001 0 
#define ROUTE_CONFIG 0x84		// 100 0 010 0   
#define ROUTE_REPLY 0x86		// 100 0 011 0
#define NW_INFO_ACQUIRED 0x88	// 100 0 100 0
#define SEND_NW_INFO 0x8A		// 100 0 101 0
#define SEND_NODE_INFO 0x8C		// 100 0 110 0
#define NODE_INFO 0x8E			// 100 0 111 0
#define NODE_INFO_ACQUIRED 0x90	// 100 1 000 0
#define INVALID	0x92			// 100 1 001 0

/* possible bit mask values of the packet sub type for APPLICATION */
#define UDP 0x00 		 		// 0 00 0000 0
#define TCP 0x20	 			// 0 01 0000 0

/* possible values of return type of various internal function calls */
#define MULTIHOP_YES TRUE 
#define MULTIHOP_NO FALSE 
#define VALID_PKT_LENGTH 0
#define INVALID_PKT_LENGTH 1

#define INFINITY 100	// When cost = INFINITY, this indicates an invalid routing table entry

/* This structure stores the sequence number cache */
typedef struct
{
	uint16_t addr;
	uint16_t seqNo;
	uint32_t timestamp;
}SequenceNoCacheEntry;

typedef struct
{
	SequenceNoCacheEntry snce[NUM_ENTRIES_SNC];
	int8_t count;
}SequenceNoCache;

/* This structure stores information about one neighbor */
typedef struct 
{
	uint16_t addr;				// Node ID
	int8_t rssi;				// strength of received radio signal
	int8_t lastReport;			// last time the node reported
	int8_t isNew;		 		// flag to indicate if the neighbor appeared newly
}Neighbor;

/* This structure stores the current information about the neighbors of a node */
typedef struct 
{
	uint16_t addr;					// own node ID	
	Neighbor ngbs[MAX_NGBS];		// array of MAX_NGBS neighbors 
	int8_t count;					// actual number of neighbors recorded 
}NeighborList;

/* This structure represents the "Hello" message */
typedef struct 
{
	uint16_t dg;			// address of the default gateway in the sensor subnet
	uint16_t addr;			// address of the node sending the HELLO message
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
	uint16_t prevHop;							// previous hop node 
	uint16_t nextHop;							// next hop node
	uint16_t prevprevHop;						// previous hop node of the node forwarding the packet
	int8_t ttl;									// time to live
	uint8_t type; 								// bit mask for packet type and subtype
													// bit 7:	APPLICATION OR NW_CONTROL 
													// bit 6,5:   Type of Tlayer 
													// bit 4,3,2,1: type of NW_CONTROL
													// bit 0: unused, always set to 0        
	int8_t length;								// actual length of payload
	int8_t prio;								// priority of this packet 
	uint16_t seqNo;
	uint8_t data[MAX_NETWORK_PAYLOAD];			// payload
}NW_Packet;

/* This structure represents a single entry in the routing table of a node */
typedef struct 
{
	uint16_t dest;			// destination address
	uint16_t nextHop;		// address of the next hop node
	uint8_t cost;			// cost associated with taking the path through the above next hop node
}RoutingTableEntry;

/* This structure represents the routing table stored by the network layer */
typedef struct
{
	RoutingTableEntry rte[NUM_ROUTING_TABLE_ENTRIES];
}RoutingTable;

/* This structure represents a message sent by the gateway to each node giving its routing table */
typedef struct
{
	uint16_t dg;						// address of the default gateway
	uint16_t addr;						// whose routing table is this
	RoutingTable rt;					// the actual routing table
}Msg_RoutingTable;

/* This structure represents a single element of the ROUTE_REPLY message sent by a gateway */
typedef struct
{
	uint16_t src;				// source address
	uint16_t nextHop;			// next hop address
	uint8_t cost;				// cost of route
}RouteReplyElement;

/* This structure represents a ROUTE_REPLY message sent by the gateway to the network */
typedef struct
{
	uint16_t dg;				// address of the default gateway
	uint16_t dest;				// destination address
	uint8_t seq_no;				// sequence number of the message
	RouteReplyElement rre[NUM_ROUTE_REPLY_ELEMENTS];
}Msg_RouteReply;

/* This structure represents a ROUTE_REQUEST message */
typedef struct
{
	uint16_t src;				// source of the route
	uint16_t dest;				// destination of the route
}Msg_RouteRequest;

/* This structure carries state variables for a node */
typedef struct
{
	uint16_t battery;				// reports the battery level of the node
	uint32_t timestamp;				// timestamp of the record collection
	uint32_t active_time;			// active time of the node
	uint16_t bytesSent;				// total number of bytes sent by the node
	uint16_t bytesReceived;			// total number of bytes received by the node
	uint8_t total_pkts_inserted;	// count of packets inserted
	uint8_t total_wait_time;		// total wait time of packets in queue
	uint8_t total_ack_time; 		// total acknowledgment time
	int8_t tx_queue_size;			// Size of the transmit queue (in use)
	int8_t rx_queue_size;			// Size of the receive queue (in use)
}NodeState;

/* This structure manages the state variables of the node */
typedef struct
{
	int8_t front;
	int8_t rear;
	NodeState ns[MAX_STATE_VALUES];
}NodeStateQueue;

typedef struct
{
	uint16_t dg;
	uint16_t addrs[MAX_SUBNET_SIZE];
	uint8_t seq_no;
}Msg_NwInfoAcquired;

typedef struct
{
	uint16_t dg;
	uint16_t addrs[MAX_SUBNET_SIZE];
	uint8_t seq_no;
}Msg_SendNwInfo;

typedef struct
{
	uint16_t dg;
	uint16_t addrs[MAX_SUBNET_SIZE];
	uint8_t seq_no;
}Msg_SendNodeInfo;

typedef struct
{
	uint16_t dg;
	uint16_t addrs[MAX_SUBNET_SIZE];
	uint8_t seq_no;
}Msg_NodeInfoAcquired;

typedef struct
{
	uint16_t addr;		// address of the node 
	NodeStateQueue nsq; // queue containing the actual state values
}Msg_NodeInfo;
		
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
#define MAX_TASK_PRIORITY 20  

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
	NW_Packet pkt;						// this holds the packet to be transmitted
	int8_t status;						// indicates the status of the transmit buffer 
	int8_t prio;						// priority of packet. This is set by the task that
										// inserts this packet in the transmit queue
	uint32_t timestamp;					// time at which the packet was inserted into the tx queue 
	struct TransmitBuffer *next;		// pointer for link list creation 
}TransmitBuffer;

typedef struct 
{
	TransmitBuffer *head_fq;		// pointer to head of free queue (EMPTY buffers) 
	TransmitBuffer *tail_fq;		// pointer to tail of free queue (EMPTY buffers)
	TransmitBuffer *head_aq;		// pointer to head of active queue (FULL buffers) 
	TransmitBuffer *tail_aq;		// pointer to tail of active queue (FULL buffers) 
	int8_t count_aq;				// count of number of buffers in active queue			
	int8_t count_fq;				// count of number of buffers in free queue 
}TransmitBufferManager;

/************************************** Serial *********************************************/
// SLIP character codes 
#define END             0300     /* indicates end of packet */
#define ESC             0333     /* indicates byte stuffing */
#define ESC_END         0334     /* ESC ESC_END means END data byte */
#define ESC_ESC         0335     /* ESC ESC_ESC means ESC data byte */

// possible bit mask values of the packet type (bit 7) 
#define SERIAL_APPLICATION  0x00  // 0  
#define SERIAL_NW_CONTROL	0x80  // 1

// possible bit mask values of the packet sub type for SERIAL_NW_CONTROL (bits 4,3,2,1)
#define SERIAL_NGB_LIST 0x80			// 100 0 000 0 (sent from node to gateway)
#define SERIAL_ROUTE_REQUEST 0x82		// 100 0 001 0 (sent from node to gateway)
#define SERIAL_ROUTE_CONFIG 0x84		// 100 0 010 0 (sent from gateway to the node)
#define SERIAL_ROUTE_REPLY 0x86			// 100 0 011 0 (sent from gateway to node)
#define SERIAL_NW_INFO_ACQUIRED 0x88	// 100 0 100 0 (sent from gateway to node)
#define SERIAL_SEND_NW_INFO 0x8A		// 100 0 101 0 (sent from gateway to node)
#define SERIAL_SEND_NODE_INFO 0x8C		// 100 0 110 0 (sent from gateway to node)
#define SERIAL_NODE_INFO 0x8E			// 100 0 111 0 (sent from node to gateway)
#define SERIAL_NODE_INFO_ACQUIRED 0x90		// 100 1 000 0 (sent from gateway to node)

#define SERIAL_SEND_NW_INFO_ACK	0x92		// 100 1 001 0 (sent from node to gateway)
#define SERIAL_NW_INFO_ACQUIRED_ACK	0x94	// 100 1 010 0 (sent from node to gateway)
#define SERIAL_INVALID 0x96					// 100 1 011 0 (invalid packet type)
#define SERIAL_ACK 0x98					

#define SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER 2  
#define SIZE_NODETOGATEWAYSERIAL_PACKET (SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER + MAX_SERIAL_PAYLOAD)

#define SIZE_GATEWAYTONODESERIAL_PACKET_HEADER 2
#define SIZE_GATEWAYTONODESERIAL_PACKET (SIZE_GATEWAYTONODESERIAL_PACKET_HEADER + MAX_GATEWAY_PAYLOAD)

// This structure represents a packet sent by a node to the gateway over a serial connection
typedef struct
{
	uint8_t type;						// bit mask for packet type and subtype
											// bit 7:	SERIAL_APPLICATION OR SERIAL_NW_CONTROL 
											// bit 6,5: subtype for SERIAL_APPLICATION; set to 00 for now 
											// bit 4,3,2,1: subtype of SERIAL_NW_CONTROL;
											// bit 0: unused, always set to 0 
	
	int8_t length;					   // actual length of the serial payload 
	uint8_t data[MAX_SERIAL_PAYLOAD];  // serial payload 

}NodeToGatewaySerial_Packet;

typedef struct
{
	uint8_t type;						// bit mask for packet type and subtype
											// bit 7:	SERIAL_APPLICATION OR SERIAL_NW_CONTROL 
											// bit 6,5: subtype for SERIAL_APPLICATION; set to 00 for now 
											// bit 4,3,2,1: subtype of SERIAL_NW_CONTROL;
											// bit 0: unused, always set to 0 
	
	int8_t length;					   // actual length of the serial payload 
	uint8_t data[MAX_GATEWAY_PAYLOAD]; // serial payload 

}GatewayToNodeSerial_Packet;

/*************************************** Link Layer *************************************************************/
#define TX_RADIO_POWER 10	// transmit power of the radio
#define TX_WAIT_PERIOD 10
#define TX_CHANNEL 25	



