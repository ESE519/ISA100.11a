// This file contains the data structures needed for the implementation of the network gateway 

#ifndef _NETWORK_GATEWAY_H
#define _NETWORK_GATEWAY_H 

#include <stdio.h>
#include <stdint.h>
#include <slipstream.h>

#include "NWStackConfigGateway.h"
#include "NWStackDataStructures.h"


/************************************** CONSTANTS *******************************************/
#define COLLECTION_PERIOD 5		// collection period of data from serial port 
#define RX_POWER_THRESHOLD (-24) // minimmum acceptable value of signal strength 
#define INFINITY 100 			// used during floyd-warshall computation
#define INVALID_ADDRESS 0

#define GATEWAY_ADDRESS ("127.0.0.1")
#define GATEWAY_PORT 4000

#ifndef FALSE 
#define FALSE 0 
#endif 

#ifndef TRUE
#define TRUE 1
#endif 
 
#define DEBUG_NG 0  

/************************************** DATA STRUCTURES *************************************/

typedef struct SensorNode
{
	uint16_t addr;						// address of the node 
	Neighbor ngbs[MAX_NGBS];			// list of its neighbors 
	int8_t count;						// actual number of neighbors recorded 
	Neighbor *n;						// the neighbor with the strongest radio link 
	
	struct SensorNode *next;			// pointer for link list creation 

}SensorNode;

typedef struct
{
	SensorNode *head;						// pointer to head of node list 
	SensorNode *tail;						// pointer to tail of node list 
	int8_t count;							// actual number of nodes in list 
}TopologyManager;

/********************************** FUNCTION PROTOTYPES *********************************/
void initialise_network_gateway();
/*
This function initialises the data structures used by the network gateway 

		PARAMS:		None
		RETURNS:		None
		Comments:	User API
*/ 

uint8_t serial_pkt_type(NodeToGatewaySerial_Packet *pkt);
uint8_t serial_nw_ctrl_type(NodeToGatewaySerial_Packet *pkt);
void process_serial_app_pkt(NodeToGatewaySerial_Packet *pkt);
int8_t process_serial_nw_ctrl_pkt(NodeToGatewaySerial_Packet *pkt);
void process_topology_desc(NeighborList nl);
SensorNode* add_to_sensor_node_list(uint16_t addr);
void update_ngb_list(SensorNode *node, uint16_t ngb_addr, int8_t rssi);
SensorNode* create_sensor_node(uint16_t addr);
void printBuffer(uint8_t *buf, int8_t len);
void print_NgbList(NeighborList nl);
int8_t node_to_graph(uint16_t);
uint16_t graph_to_node(int8_t);
void generate_routing_tables();
void floyd_warshall(int8_t, uint8_t **edge, uint8_t **parent, uint8_t **cost);
void disseminate_routing_tables();
void print_RoutingTable(Msg_RoutingTable *);
void print_edge_matrix();
void print_parent_matrix();
void print_Map();
void print_ntg_pkt_header(NodeToGatewaySerial_Packet *pkt);
void print_ntg_pkt(NodeToGatewaySerial_Packet *pkt);
void print_gtn_pkt(GatewayToNodeSerial_Packet *pkt);
void print_gtn_pkt_header(GatewayToNodeSerial_Packet *pkt);
void print_topology();
void print_cost_matrix();
void free_data_structures();
void prepare_topology_desc_file();
int8_t get_parent(int8_t, int8_t);
void build_Msg_RoutingTable(Msg_RoutingTable *mrtbl, uint16_t node, RoutingTable rtbl[]);
void initialise_routing_table();


#endif 
