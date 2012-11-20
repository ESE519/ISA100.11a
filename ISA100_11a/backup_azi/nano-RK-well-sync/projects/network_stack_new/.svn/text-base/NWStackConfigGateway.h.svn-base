/* This file stores constants that control the various paramters of the network stack
 * It should be edited by the user depending on his application requirements

Authors:
Aditya Bhave
*/

#ifndef _NW_STACK_CONFIG_H
#define _NW_STACK_CONFIG_H

#include "NWErrorCodes.h"

/*************************************** CONSTANTS ******************************************/
// useful constants
#define TRUE 1
#define FALSE 0

// configurable parameters for the buffer manager 
#define MAX_APP_PAYLOAD  32  					// maximum size of the application payload in bytes 
#define MAX_SERIAL_PAYLOAD 64  					// maximum size of serial data payload (node to gateway)
#define MAX_GATEWAY_PAYLOAD 48 					// maximum size of serial data payload (gateway to node)
#define MAX_RX_QUEUE_SIZE 4 					// Number of receive buffers available in the 
												// system for USER tasks 
#define MAX_SUBNET_SIZE 10						// maximum size of one sensor subnet 
#define MAX_TX_QUEUE_SIZE 8    				// Size of transmit queue for the system
#define DEFAULT_EXCESS_POLICY DROP 				// excess policy default

// configurable parameters for the transport layer 
#define NUM_PORTS 4 							// number of ports available for application tasks  

// configurable parameters for the network layer  
//#define CONNECTED_TO_GATEWAY TRUE		// indicates whether this node connects to the gateway via serial port
#define MAX_NGBS 5						// maximum number of neighbors recorded
#define HELLO_PERIOD 3  				// every HELLO_PERIOD seconds, a HELLO msg will be sent
#define MAX_NETWORK_DIAMETER 5			// maximum number of hops to the gateway
#define NGB_LIST_PERIOD 30 				// every NGB_LIST_PERIOD seconds, a NGB_LIST msg will be sent
#define UPDATE_NGB_LIST_PERIOD 5		// every UPDATE_NGB_LIST_PERIOD, neighbors lastReport's will be refreshed
#define NODE_INFO_PERIOD 30				// every NODE_INFO_PERIOD, a NODE_INFO message will be sent
#define TIMEOUT_COUNTER	5				// Each node has to report to it's neighbor within this count
#define NUM_ROUTING_TABLE_ENTRIES 5		// size of the node's routing table
#define MAX_STATE_VALUES 3				// number of state values maintained by the node

// priority levels of messages 
#define MAX_PRIORITY 31
#define LOW_PRIORITY 1
#define NORMAL_PRIORITY 2  
#define HIGH_PRIORITY 3 
#endif 
 
