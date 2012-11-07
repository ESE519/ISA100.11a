/* This file contains the data structures and constants needed for the implementation
	of the network layer of the network stack. 
*/

#ifndef _NETWORK_LAYER_H
#define _NETWORK_LAYER_H

#include "NWStackConfig.h"
#include <stdint.h>
#include "TransportLayerUDP.h"

#define TRUE 1
#define FALSE 0
#define DEBUG_NL 0	// debug flag for the network layer 

/****************************** CONSTANTS *********************************************/

#define MAX_NETWORK_PAYLOAD  MAX_TRANSPORT_UDP_SEG // maximum size of network packet payload 
#define BCAST_ADDR 65535				 // Reserved address


#define LINK_STATE 1 					 // parameters for the routing algorithm 
#define FLOODING 2
#define PROBABILISTIC 1 				 // paramters for the type of flooding 
#define TTL_BASED 2 
#define RANDOM 1 							 // paramters for the probability distribution 
#define GAUSSIAN 2 

#define DEFAULT_ROUTING_ALGORITHM LINK_STATE
#define DEFAULT_FLOODING_TYPE TTL_BASED
#define DEFAULT_PDISTRIBUTION RANDOM

// sizes of various object types 
#define SIZE_NEIGHBOR 5
#define SIZE_NEIGHBORLIST (3 + MAX_NGBS * SIZE_NEIGHBOR)

#define SIZE_MSG_HELLO SIZE_NEIGHBOR
#define SIZE_MSG_NGB_LIST SIZE_NEIGHBORLIST

#define SIZE_NW_PACKET_HEADER 12  
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

#define INFINITY 100
#define INVALID_ADDRESS BCAST_ADDR

/***************************** DATA STRUCTURES *****************************************/

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
	uint16_t prevHop;							// previous hop node 
	uint16_t nextHop;							// next hop node
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

typedef struct
{
	uint16_t dg;						// address of the default gateway
	uint16_t node;						// whose routing table is this
	RoutingTable rt[MAX_NODES];
}Msg_RoutingTable;

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

int8_t shouldIMultihop(NW_Packet *pkt);
/*
This function checks whether a given packet should be multihopped or not

	PARAMS:		pkt: Pointer to the network packet
	RETURNS:		MULTIHOP_YES or MULTIHOP_NO 
	Comments:	private function
*/

void multihop(NW_Packet *pkt);
/*
This helper function first calls shouldIMultihop() and if return type is TRUE, inserts 
the packet into the transmit queue of the system

	PARAMS:		pkt: Pointer to the network packet
	RETURNS:		None
	Comments: 	private function
*/

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

int8_t sendToGateway(uint8_t *buf, int8_t len);
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

uint8_t nw_ctrl_type(uint8_t type);
/*
This helper function returns the sub-type of the packet if it is of type NW_CONTROL

	PARAMS:	 type: The 'type' field in the received packet
	RETURNS:	 HELLO, NGB_LIST, ROUTE_CONFIG or INVALID
	Comments: private function. Currently only three sub-types are supported
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


void print_RoutingTable(Msg_RoutingTable *mrt);
/*
This function allows an application task to view the routing tables built on the node

	PARAMS:		mrt: pointer to data structure holding information about the routing table
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

void initialise_routing_table();
/*
This function initialises the node's routing table

	PARAMS:		None
	RETURNS:	None
	Comments:	private function
*/


	


#endif 

