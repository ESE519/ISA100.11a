
/* This file should be edited by the user depneding on his application */

#ifndef _NW_STACK_CONFIG_H
#define _NW_STACK_CONFIG_H

#include "NWErrorCodes.h"

// configurable parameters for the buffer manager 
#define MAX_APP_PAYLOAD  32  					// maximum size of the application payload in bytes 
#define MAX_SERIAL_PAYLOAD 32  				// maximum size of serial data payload
#define MAX_GATEWAY_PAYLOAD 48 
#define MAX_RX_QUEUE_SIZE 8 					// maximum number of receive buffers available in the 
														// system for USER tasks 
#define MAX_TX_QUEUE_SIZE 4     				// maximum size of transmit queue for the system
#define DEFAULT_EXCESS_POLICY DROP 			// excess policy default


// configurable parameters for the transport layer 
#define NUM_PORTS 4 								// number of ports available on the node  

// configurable parameters for the network layer  
#define CONNECTED_TO_GATEWAY TRUE	// indicates whether this node connects to the gateway or not
#define MAX_NGBS 5						// maximum number of neighbors recorded
#define HELLO_PERIOD 2  				// every HELLO_PERIOD seconds, a HELLO msg will be sent
#define MAX_NETWORK_DIAMETER 3		// maximum number of hops to the gateway
#define NGB_LIST_PERIOD 6 				// every NGB_LIST_PERIOD seconds, a NGB_LIST msg will be sent
#define TIMEOUT_COUNTER	10				// Each node has to report to it's neighbor within this count
#define MAX_NODES 5 				// maximum number of nodes in a sensor subnet

// priority levels of messages 
#define MAX_PRIORITY 31
#define LOW_PRIORITY 1
#define NORMAL_PRIORITY 2  
#define HIGH_PRIORITY 3 

/********************************** FUNCTION PROTOTYPES *****************************************/
void nrk_init_nw_stack();
/*
This function initialises the network stack for the nodes. Call this function during node startup

	PARAMS:		None
	RETURNS:		None
	Comments: 	User API
*/

#endif 
