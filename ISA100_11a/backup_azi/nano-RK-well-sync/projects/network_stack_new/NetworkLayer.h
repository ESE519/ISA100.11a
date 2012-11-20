/* This file contains the data structures and constants needed for the implementation
   of the network layer of the network stack.
   
   Authors:
   Aditya Bhave 
*/

#ifndef _NETWORK_LAYER_H
#define _NETWORK_LAYER_H

#include "NWStackConfig.h"
#include <stdint.h>
#include <nrk_stats.h>
#include "TransportLayerUDP.h"

#define DEBUG_NL 0	// debug flag for the network layer 

/****************************** CONSTANTS *********************************************/

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
#define MAX_NETWORK_PAYLOAD 70						// maximum size of network packet payload
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
#define NUM_ROUTE_REPLY_ELEMENTS 5
#define SIZE_MSG_ROUTE_REPLY (SIZE_NODE_ADDR * 2 + 1 + SIZE_ROUTE_REPLY_ELEMENT * NUM_ROUTE_REPLY_ELEMENTS)
#define SIZE_MSG_ROUTE_REQUEST (SIZE_NODE_ADDR * 2)

#define SIZE_NODE_STATE 19	// size of one entry in the table of node state values
#define SIZE_NODE_STATE_QUEUE (2 + SIZE_NODE_STATE * MAX_STATE_VALUES)

// related to SeqNoCache
#define NUM_ENTRIES_SNC 5
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
#define VALID_PKT_LENGTH 0
#define INVALID_PKT_LENGTH 1

#define INFINITY 100	// When cost = INFINITY, this indicates an invalid routing table entry

/***************************** DATA STRUCTURES *****************************************/
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
	uint16_t nextHop;							// next hop node
	uint16_t prevHop;							// previous hop node 
	uint16_t prevprevHop;						// previous hop node of the node forwarding the packet
	int8_t ttl;									// time to live
	uint8_t type; 								// bit mask for packet type and subtype
													// bit 7:	APPLICATION OR NW_CONTROL 
													// bit 6,5:   Type of Tlayer 
													// bit 4,3,2,1: type of NW_CONTROL; HELLO, NGB_LIST, ROUTE_CONFIG
													// bit 0: unused, always set to 0    
	int8_t length;								// actual length of payload
	int8_t prio;								// priority of this packet
	uint16_t seqNo;								// sequence number of this packet 
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
	uint8_t seq_no;				// seq number of this message
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
}Msg_SendNwInfo;

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


/************************************** FUNCTION PROTOTYPES ***********************************/
int8_t add_neighbor(Neighbor n);
/*
This function adds a given Neighbor if possible and/or updates neighbor control information in 
the neighbor list 

	PARAMS:		n: The Neighbor to be added
	RETURNS:		NRK_OK if no error is encountered during processing
					NRK_ERROR if the maximum neighbor limit is reached and further addition is 
					not possible
	ERROR NOS:	MAX_NEIGHBOR_LIMIT_REACHED
	Comments:	private function 
*/

void adjust_routing_table(int8_t);

uint16_t route_addr(uint16_t addr);
/*
This function returns the next hop address for a given address from its routing table 

	PARAMS:		addr: The destination address for the packet
	RETURNS:		the next hop address 
	Comments:	private function
*/

void route_packet(NW_Packet *pkt);
/*
This helper function as of now simply calls multihop()

	PARAMS:		pkt: Pointer to the network packet
	RETURNS:		None
	Comments: 	private function
*/

int8_t sendToGateway(uint8_t *buf, int16_t len);
/*
This function sends the message pointed to by its parameter to the attached gateway device

	PARAMS:		ptr: The pointer to the message
					len: length of the message 
	RETURNS:		NRK_OK if the message is sent successfully
					NRK_ERROR otherwise
	
	ERROR NOS:	INVALID_ARGUMENT if the passed paramters are faulty
			
	Comments:	User API
					Length of the buffer should not exceed MAX_SERIAL_PAYLOAD
*/

uint8_t pkt_type(NW_Packet *pkt);
/*
This helper function returns the type of the network packet received from the link layer 

	PARAMS:		ptr: pointer to the network packet
	RETURNS:		APPLICATION or NW_CONTROL
	Comments:	private function
*/

uint8_t tl_type(uint8_t type);
/*
This helper function returns the type of transport layer this packet is destined for 

	PARAMS:	 type: The 'type' field in the received packet
	RETURNS:  UDP or INVALID
	Comments: private function. Currently only UDP is supported
*/

void process_app_pkt(NW_Packet *pkt, int8_t rssi);
/*
This function is used to process the received application-layer n/w packet 

	PARAMS:		pkt:  pointer to the network layer packet to be processed
					rssi:	signal strength with which this packet was received
					
	RETURNS:		None
	Comments:	private function. This buffers the transport layer segment as per the 
					destination port number.
*/

void process_nw_ctrl_pkt(NW_Packet *pkt, int8_t rssi);
/*
This function is used to process the received network control packet 

	PARAMS:		pkt:  pointer to the network layer packet to be processed
					rssi:	signal strength with which this packet was received
					
	RETURNS:		None
	Comments:	private function. This function's behavior differs depending on 
					the packet subtype
					
					HELLO: 		  The function will update its neighbor list information
					NGB_LIST: 	  The function will try and multihop this packet 
					ROUTE_CONFIG: The function will update its routing tables 
*/

void create_network_layer_tasks();
/*
This function creates the TCBs of the two network layer tasks rx, tx 

	PARAMS:		None
	RETURNS:		None
	Comments:	private function. Called during node start up.
*/
 
void initialise_network_layer();
/*
This function initialises the data structures required by the network layer tasks 

	PARAMS:	 None
	RETURNS:	 None
	Comments: private function. Called during node startup
*/

void record_tx_queue_full(NW_Packet *pkt);
/*
This function records the fact that the transmit queue of the system was full at a certain time

	PARAMS:		pkt: pointer to the packet that failed to get inserted into the queue
	RETURNS:		None
	Comments:	private function for statistics collection
*/

void record_unassociated_socket_pkt(NW_Packet *pkt);
/*
This function records the fact that a packet destined for an unassociated port was received

	PARAMS:		pkt: pointer to the packet
	RETURNS:		None
	Comments:	private function for statistics collection
*/

void record_max_ngb_limit_reached(NW_Packet *pkt);
/*
This function records the fact that the maximum neighbor limit was reached for this node 

	PARAMS:		pkt: pointer to the packet containing the HELLO msg
	RETURNS:		None
	Comments:	private function for statistics collection
*/

void collect_queue_statistics();
/*
This function is called randomly from nl_tx_task() and collects statistics about the different 
queues in the system

	PARAMS:		None
	RETURNS:		None
	Comments:	private function for statistics collection
*/

int8_t set_routing_algorithm(int8_t pref, int8_t type, int8_t pdist);
/*
This function allows an application to specify the type of routing algorithm to be used in the network layer

	PARAMS:		pref: The type of routing algorithm, FLOOD or LINK_STATE
					type: If pref == FLOOD, this argument can be used to specify the flooding type
							PROBABILISTIC or TTL_BASED 
					pdist:If pref == FLOOD and type == PROBABILISTIC, then this argument specifies the
							probability distribution to be used  
							
	RETURNS:		NRK_OK if the setting is recorded properly, NRK_ERROR if some error is encountered
	
	ERROR NOS:	INVALID_ARGUMENT if the passed parameter is faulty
	Comments:	User API. This function can be called by the application task at any time. However the 
					effects may not be immediately apparent
*/ 

void set_RoutingTable(Msg_RoutingTable *mrt);


void print_RoutingTable();
/*
This function allows an application task to view the routing tables built on the node

	PARAMS:		None
	RETURNS:	None
	Comments:	private function. 
*/

int8_t get_routing_table(RoutingTable *rt);
/*
This function allows an application task to retrieve the routing table from the node

	PARAMS:		rt: The network layer will copy its routing table into this argument
	RETURNS:		NRK_OK if the table is copied successfully, or NRK_ERROR otherwise
	
	ERROR_NOS:	INVALID_ARGUMENT if the passed paramter is faulty 
	Comments:	User API
*/ 

void print_pkt_header(NW_Packet *pkt);
/*
This helper function prints out the header of the given packet 

	PARAMS:		pkt:	The packet
	RETURNS:		None
	Comments:	private function for debugging 
*/

void print_pkt(NW_Packet *pkt);
/*
This helper function prints out the entire contents of the given packet
	
	PARAMS:		pkt: The packet
	RETURNS:	None
	Comments:	private function for debugging 
*/

void print_raw(uint8_t *buf, int8_t len);
/*
This helper function prints out the contents of a buffer 

	PARAMS:		buf: The buffer to be printed
	RETURNS:	None
	Comments:	private function for debugging
*/

void print_Msg_RouteReply(Msg_RouteReply *);
void print_NgbList(NeighborList *nlt);

void initialise_routing_table();
/*
This function initialises the node's routing table

	PARAMS:		None
	RETURNS:	None
	Comments:	private function
*/

void initialise_node_state();
void initialise_SeqNoCache();

int8_t get_continue_sending_ngblist();
void set_continue_sending_ngblist(int8_t);
int8_t get_continue_sending_nodeinfo();
void set_continue_sending_nodeinfo(int8_t);
void set_continue_sending_hello(int8_t);
int8_t get_continue_sending_hello();

int8_t get_eviction_index();
int8_t process_Msg_RouteReply(Msg_RouteReply *mrr);
int8_t process_Msg_NwInfoAcquired(Msg_NwInfoAcquired *);
int8_t process_Msg_SendNwInfo(Msg_SendNwInfo *);
int8_t process_Msg_SendNodeInfo(Msg_SendNodeInfo *);
int8_t process_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *);
void collect_node_statistics();
void remove_from_node_state();
void add_to_node_state(NodeState *);
void set_nw_pkt_header(NW_Packet *pkt, uint16_t src, uint16_t dest, uint16_t nextHop, uint16_t prevHop, uint16_t prevprevHop, int8_t ttl, uint8_t type, int8_t length, int8_t prio);
int8_t search_addr(uint16_t, uint16_t [], int8_t);
void send_nw_pkt_blocking(NW_Packet *);
void update_NgbList();
void print_nrk_time_t(nrk_time_t time);
void print_NodeState(NodeState *ns);
void print_NodeStateQueue(NodeStateQueue *);
void set_dg(uint16_t addr);
uint16_t get_dg();
void print_dg();

void build_Msg_NodeInfo(Msg_NodeInfo *m);
void build_Msg_NgbList(Msg_NgbList *m);
void build_Msg_Hello(Msg_Hello *m);

void add_to_SeqNoCache(uint16_t addr, uint16_t seqNo);
void clear_SeqNoCache();
int8_t search_SeqNoCache(uint16_t addr, uint16_t *seqNo, uint32_t *timestamp);
int8_t process_SeqNo(NW_Packet *pkt);




	


#endif 

