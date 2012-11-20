/* tThis file contains the data structures needed for the implementation of the network gateway
 * 
   Authors:
   Aditya Bhave
*/ 

// CHECKED
#ifndef _NETWORK_GATEWAY_H
#define _NETWORK_GATEWAY_H 

/************************************* INCLUDE files ***************************************/
#include <stdint.h>

#include "NWStackConfigGateway.h"
#include "NWStackDataStructures.h"

/************************************** CONSTANTS *******************************************/

#define SIZE_HT_SNL 65536			// size of the hash table to manage the SNL
#define SIZE_HT_VL SIZE_HT_SNL		// size of the hash table to manage the VL
#define SIZE_IP_ADDRESS	16			// size of a string capable of holding an IP address
#define MAX_SUBNET_SIZE 20			// maximum size of a subnet
#define MAX_STATE_VALUES_GATEWAY 50	// number of history values to record for each node
#define COLLECTION_PERIOD 30		// collection period of data from serial port 
#define RX_POWER_THRESHOLD (-34) 	// minimum acceptable value of signal strength 
#define MAX_BUF 128					// buffer size passed to SLIPstream to read to from serial port

#define SIZE_HT_SNL 65536			// size of the hash table to manage the vertical list
#define SIZE_HT_VL SIZE_HT_SNL		// size of the hash table to manage the neighbor nodes

#define GATEWAY_ADDRESS ("127.0.0.1")
#define GATEWAY_PORT 4000

#define RX_TIMEOUT 5				// used as timeout value for socket while reading from Firefly

#ifndef FALSE 
#define FALSE 0 
#endif 

#ifndef TRUE
#define TRUE 1
#endif 
 
#define DEBUG_NG 0  

/************************************** DATA STRUCTURES *************************************/
struct SensorNode;

/* This structure manages the state variables of the node */
typedef struct
{
	int8_t front;
	int8_t rear;
	NodeState ns[MAX_STATE_VALUES_GATEWAY];
}NodeStateQueueGateway;

/* This structure stores information about one neighbor(NL) */
typedef struct NeighborGateway
{
	uint16_t addr;
	int8_t rssi;
	int8_t lastReport;
	int8_t refreshFlag;
	
	struct NeighborGateway *next;
	struct NeighborGateway *prev;
	struct SensorNode *toHead;
	struct NeighborGateway *up;
	struct NeighborGateway *down;			// pointer for VL creation
}NeighborGateway;

/* The structure stores information about the routing table for each sensor node */
typedef struct 
{
	RoutingTableEntry rte[MAX_SUBNET_SIZE];
}RoutingTableGateway;

/* This structure stores information about one sensor node in the subnet */
typedef struct SensorNode
{
	uint16_t addr;						// address of the node 
	NeighborGateway *head;				// pointer to the head of the queue of neighbors
	NeighborGateway *tail;				// pointer to the tail of the queue of neighbors
	int8_t count;						// actual number of neighbors recorded 
	NeighborGateway *nbest;				// the neighbor with the best link
	NeighborGateway *nsbest;			// the neighbor with the second best link
	NeighborGateway *nworst;			// the neighbor with the worst link
	RoutingTableGateway rt;				// to hold the routing table for the node 
	NodeStateQueueGateway nsqg;			// holds state variables for this node
		
	struct SensorNode *next;			// pointer for vertical link list creation 

}SensorNode;

/* This structure manages the topology of the subnet */
typedef struct
{
	SensorNode *head;						// pointer to head of node list 
	SensorNode *tail;						// pointer to tail of node list 
	int8_t count;							// total number of nodes in list 
}TopologyManager;

/* Hash table for the VL */
typedef struct
{
	NeighborGateway *head;					// pointer to head of VL
	NeighborGateway *tail;					// pointer to tail of VL
	int8_t count;							// number of members in the VL
}HashTableVLEntry;

/* Hash table for the sensor node list */
typedef struct
{
	SensorNode *ptr;						// pointer to the SensorNode in the SNL
}
HashTableSNLEntry;
/********************************** FUNCTION PROTOTYPES *********************************/

uint8_t serial_pkt_type(NodeToGatewaySerial_Packet *pkt);
void process_serial_app_pkt(NodeToGatewaySerial_Packet *pkt);
int8_t process_serial_nw_ctrl_pkt(NodeToGatewaySerial_Packet *pkt);
void process_Msg_NodeInfo(Msg_NodeInfo *);
void send_dummy_pkt();
void start_collection_phase();
void start_listening_phase();
int8_t receiveFromSerial(uint8_t *p, uint8_t len, int timeout);
int8_t process_topology_desc(NeighborList nl);
SensorNode* create_node_SNL(uint16_t addr);
SensorNode* search_node_SNL(uint16_t addr);
SensorNode* add_node_to_SNL(uint16_t addr);
NeighborGateway* create_node_NL(uint16_t addr, Neighbor *n);
void set_ngb_gw(NeighborGateway *ngw, Neighbor *n);
NeighborGateway* search_node_NL(SensorNode *snptr, uint16_t addr);
NeighborGateway* add_node_to_NL(SensorNode *snptr, uint16_t addr, Neighbor *n);
void add_node_to_VL(NeighborGateway *ngptr);
void free_data_structures();
void refresh_VL(uint16_t);
void remove_node_NL(SensorNode *, NeighborGateway *);
void remove_node_VL(uint16_t addr);
void floyd_warshall(int8_t n, uint8_t **edge, uint8_t **parent, uint8_t **cost);
void generate_routing_tables();
int8_t get_parent(int8_t from, int8_t to);
void rank_ngbs(SensorNode *);
uint16_t graph_to_node(int8_t graph_addr);
int8_t is_better_ngb(NeighborGateway *, NeighborGateway *);
int8_t node_to_graph(uint16_t node_addr);
void process_Msg_RouteRequest(Msg_RouteReply *, uint16_t src, uint16_t dest);
void add_to_node_state_gateway(NodeStateQueueGateway *, NodeState *);
void remove_from_node_state_gateway(NodeStateQueueGateway *);
void clear_topology();
void writeTopToFile(FILE *fptr, int pf);
int8_t sendOverSerial(void *msg, uint8_t type);
int8_t sendToSensorNode(GatewayToNodeSerial_Packet*);
void writeNSToFile(Msg_NodeInfo *);


void print_edge_matrix();
void print_parent_matrix();
void print_Map();
void print_cost_matrix();
void print_RoutingTableGateway(uint16_t addr);
void print_Msg_RouteReply(Msg_RouteReply *m);
void print_NodeStateQueueGateway(uint16_t addr);
void print_nrk_time_t(nrk_time_t);
void print_SNL();
void print_VL();
void printBuffer(uint8_t *buf, uint8_t len);
void print_NodeState(NodeState *ns);
void print_NodeStateQueue(NodeStateQueue *nsq);
void print_ntg_pkt_header(NodeToGatewaySerial_Packet *pkt);
void print_ntg_pkt(NodeToGatewaySerial_Packet *pkt);
void print_gtn_pkt(GatewayToNodeSerial_Packet *pkt);
void print_gtn_pkt_header(GatewayToNodeSerial_Packet *pkt);

void initialise_node_state_gateway(NodeStateQueueGateway *nsqg);
void initialise_Msg_RouteReply(Msg_RouteReply *);
void initialise_routing_table_gateway(RoutingTableGateway *rt);
void initialise_network_gateway();
void initialise_Msg_NwInfoAcquired(Msg_NwInfoAcquired *m);
void initalise_Msg_SendNwInfo(Msg_SendNwInfo *);
void initialise_Msg_SendNodeInfo(Msg_SendNodeInfo *);
void initialise_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *);

void build_Msg_SendNwInfo(Msg_SendNwInfo *, uint16_t *, int8_t);
void build_Msg_NwInfoAcquired(Msg_NwInfoAcquired *m);
void build_Msg_SendNodeInfo(Msg_SendNodeInfo *, uint16_t [], int8_t);
void build_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *m);


#endif 
