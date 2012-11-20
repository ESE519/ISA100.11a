/* This file contains the data structures needed for the implementation of the network gateway
 * 
   Authors:
   Aditya Bhave
*/ 

#ifndef _NETWORK_GATEWAY_THREADED_H
#define _NETWORK_GATEWAY_THREADED_H 

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>   /* various type definitions.           */
#include <sys/ipc.h>     /* general SysV IPC structures         */
#include <sys/sem.h>	 /* semaphore functions and structs.    */

#include "NWStackConfigGateway.h"
#include "NWStackDataStructures.h"

/************************************** CONSTANTS *******************************************/

#define SIZE_HT_SNL 65536			// size of the hash table to manage the SNL
#define SIZE_HT_VL SIZE_HT_SNL		// size of the hash table to manage the VL
#define SIZE_IP_ADDRESS	16			// size of a string capable of holding an IP address
#define MAX_STATE_VALUES_GATEWAY 5	// number of history values to record for each node
#define COLLECTION_PERIOD 45		// collection period of data from serial port 
#define NW_CONTROL_CHECK_PERIOD 30	// timeout to check for ACKS of network control messages
#define RX_POWER_THRESHOLD (-40) 	// minimum acceptable value of signal strength 
#define MAX_BUF 128					// buffer size passed to SLIPstream to read to from serial port
#define SEM_ID 250					// some ID number for the semaphore set
#define NUM_SERIAL_RETRIES 3		// maximum number of times to try sending over serial port

#define SIZE_HT_SNL 65536			// size of the hash table to manage the vertical list
#define SIZE_HT_VL SIZE_HT_SNL		// size of the hash table to manage the neighbor nodes

#define GATEWAY_ADDRESS ("127.0.0.1")
#define GATEWAY_PORT 4000
#define WEB_SERVER_ROOT "../public_html"
#define PATHNAME_MAX_SIZE 64
#define BUFF_SIZE 256
#define PLOT_SIZE 1024

#define RX_TIMEOUT 5				// used as timeout value for socket while reading from Firefly
#define ACK_TIMEOUT 10				// wait for ACK to come back

#define DATA_RATE 250

#ifndef FALSE 
#define FALSE 0 
#endif 

#ifndef TRUE
#define TRUE 1
#endif 
 
#define DEBUG_NG 0  

#define SIZE_MSG_NGBLIST_QUEUE 32
#define SIZE_MSG_ROUTEREQUEST_QUEUE 32
#define SIZE_MSG_NODEINFO_QUEUE 32

/****************************************** DATA STRUCTURES **************************************/
/* These buffers hold messages for each type of thread */
typedef struct
{
	int8_t front;
	int8_t rear;
	int8_t num;
	pthread_mutex_t *qm;
	pthread_cond_t *qc;
	Msg_NgbList q[SIZE_MSG_NGBLIST_QUEUE];
}Msg_NgbListQueue;

typedef struct
{
	int8_t front;
	int8_t rear;
	int8_t num;
	pthread_mutex_t *qm;
	pthread_cond_t *qc;
	Msg_RouteRequest q[SIZE_MSG_ROUTEREQUEST_QUEUE];
}Msg_RouteRequestQueue;

typedef struct
{
	int8_t front;
	int8_t rear;
	int8_t num;
	pthread_mutex_t *qm;
	pthread_cond_t *qc;
	Msg_NodeInfo q[SIZE_MSG_NODEINFO_QUEUE];
}Msg_NodeInfoQueue;

struct SensorNode;

/* This structure manages the state variables of the node */
typedef struct
{
	int8_t front;
	int8_t rear;
	NodeState ns[MAX_STATE_VALUES_GATEWAY];
	uint32_t last_timestamp;
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
	
	int8_t nw_info_acquired;			// flags to indicate that an ACK is to be sent back to the node
	int8_t node_info_acquired;
	
	uint32_t total_pkts_inserted;		// statistics variables for each sensor node
	uint32_t total_wait_time;
	uint32_t total_ack_time;
	uint32_t totalbytesSent;
	uint32_t totalbytesReceived;
	uint32_t totalactiveTime;
	
	struct SensorNode *next;			// pointer for vertical link list creation 
	struct SensorNode *prev;			// pointer for vertical link list creation

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
void cleanup_mutex(void *);
void* process_UserInput(void *);
void* process_Msg_NodeInfo(void *);
void* process_Msg_RouteRequest(void *);
void* process_Msg_NgbList(void *);
void send_dummy_pkt();
void start_collection_phase();
void start_listening_phase();
int8_t receiveFromSerial(uint8_t *p, uint8_t len, int timeout);
int8_t process_topology_desc(NeighborList nl);
void prepare_topology_desc_file();
SensorNode* create_node_SNL(uint16_t addr);
SensorNode* search_node_SNL(uint16_t addr);
SensorNode* add_node_to_SNL(uint16_t addr);
void remove_node_SNL(SensorNode *);
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
void create_NodePath(char *path, uint16_t addr, int8_t);
void generate_NodeGraph(uint16_t);
//void generate_html(int8_t);
void generate_html(char *, int8_t);
void send_nw_ctrl_ACKS();

int8_t add_to_node_state_gateway(NodeStateQueueGateway *, NodeState *);
int8_t remove_from_node_state_gateway(NodeStateQueueGateway *, NodeState *);
void clear_topology();
void writeTopToFile(FILE *fptr, int pf);
int8_t sendOverSerial(void *msg, uint8_t type);
int8_t sendToSensorNode(GatewayToNodeSerial_Packet*);
//void writeNSQToFile(uint16_t, NodeStateQueueGateway *);
void writeNSQToFile(uint16_t addr, SensorNode *snptr);
void add_queue(void *msg, uint8_t type);
int8_t del_queue(void *msg, uint8_t type);

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
int8_t build_Msg_NwInfoAcquired(Msg_NwInfoAcquired *m);
void build_Msg_SendNodeInfo(Msg_SendNodeInfo *, uint16_t [], int8_t);
int8_t build_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *m);

double calcEnergy(SensorNode *, double *E_TX, double *E_RX, double *E_CPU);

#endif

