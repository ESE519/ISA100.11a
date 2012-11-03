#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<time.h>

#include "NWStackDataStructures.h"

/******************************** CONSTANTS *********************************************************/
#define SIZE_HT_SNL 65536			// size of the hash table to manage the SNL
#define SIZE_HT_VL SIZE_HT_SNL		// size of the hash table to manage the VL
#define MAX_STATE_VALUES_GATEWAY 3	// number of history values to record for each node
#define RX_POWER_THRESHOLD 0
#define TRUE 1
#define FALSE 0

#define DEBUG_NG 0
/******************************* Data Structures ***************************************************/
struct SensorNode;

/* This structure manages the state variables of the node */
typedef struct
{
	uint16_t addr;				// address of the node
	int8_t front;
	int8_t rear;
	NodeState ns[MAX_STATE_VALUES_GATEWAY];
}NodeStateQueueGateway;

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

typedef struct 
{
	RoutingTableEntry rte[MAX_SUBNET_SIZE];
}RoutingTableGateway;

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

typedef struct
{
	SensorNode *head;						// pointer to head of node list 
	SensorNode *tail;						// pointer to tail of node list 
	int8_t count;							// total number of nodes in list 
}TopologyManager;

typedef struct
{
	NeighborGateway *head;					// pointer to head of VL
	NeighborGateway *tail;					// pointer to tail of VL
	int8_t count;							// number of members in the VL
}HashTableVLEntry;

typedef struct
{
	SensorNode *ptr;						// pointer to the SensorNode in the SNL
}
HashTableSNLEntry;

/************************************* Global Data Variables ****************************************/
static uint16_t *Map;									// to hold a mapping between node IDS 
														// and graph IDs
static uint8_t **edge;									// edge cost matrix
static uint8_t **parent;								// parent matrix (graph IDs)
static uint8_t **cost;									// to hold the shortest path costs

/* The following interpretation is to be applied to the contents of these matrices 
  
  * edge matrix:
   A value of 1 indicates there is an edge between node 'row' and node 'column'. 
   If row == column, it is the same node and the matrix entry is a dummy value (=1)
   
  * cost matrix:
   A value of INFINITY indicates there is no path from node 'row' to node 'column'.
   if row == column, it is the same node and the matrix entry is equal to 0
   
  * parent matrix:
   A value of INVALID_ADDR indicates there is no path from node 'row' to node 'column'. 
   If row == column, it is the same node and the matrix entry is a dummy value (=INVALID_ADDR)
*/

static TopologyManager top_mgr;							// to manage information about the network
HashTableSNLEntry htsnl[SIZE_HT_SNL];					// hash table to manage the sensor node list
HashTableVLEntry htvl[SIZE_HT_VL];						// hash table to manage the vertical list
NeighborList nl;										// to hold one NeighborList
FILE *topologyFile;										// pointer to topology file
NW_Packet pkt_tx;
NW_Packet pkt_rx;
uint8_t tx_buf[SIZE_NW_PACKET];
uint8_t rx_buf[SIZE_NW_PACKET];
Msg_RouteReply mrr;
NodeState ns;

/*************************** FUNCTION PROTOTYPES *******************************************************/
int8_t process_topology_desc(NeighborList nl);
SensorNode* create_node_SNL(uint16_t addr);
SensorNode* search_node_SNL(uint16_t addr);
SensorNode* add_node_to_SNL(uint16_t addr);
NeighborGateway* create_node_NL(uint16_t addr, Neighbor *n);
void set_ngb_gw(NeighborGateway *ngw, Neighbor *n);
NeighborGateway* search_node_NL(SensorNode *snptr, uint16_t addr);
NeighborGateway* add_node_to_NL(SensorNode *snptr, uint16_t addr, Neighbor *n);
void add_node_to_VL(NeighborGateway *ngptr);
void initialise_network_gateway();
void print_SNL();
void print_VL();
void free_data_structures();
void refresh_VL(uint16_t);
void remove_node_NL(SensorNode *, NeighborGateway *);
void remove_node_VL(uint16_t addr);
void initalise_nl();
void end_program();
void floyd_warshall(int8_t n, uint8_t **edge, uint8_t **parent, uint8_t **cost);
void generate_routing_tables();
int8_t get_parent(int8_t from, int8_t to);
void rank_ngbs(SensorNode *, NeighborGateway*);
uint16_t graph_to_node(int8_t graph_addr);
int8_t is_better_ngb(NeighborGateway *, NeighborGateway *);
int8_t node_to_graph(uint16_t node_addr);
void initialise_routing_table_gateway(RoutingTableGateway *rt);
void print_edge_matrix();
void print_parent_matrix();
void print_Map();
void print_cost_matrix();
//void print_RoutingTableGateway(RoutingTableGateway *rt, int8_t row);
//void print_RoutingTableGateway(RoutingTableGateway *rt);
void writeTopToFile(FILE *fptr, int);
void print_RoutingTableGateway(uint16_t addr);

extern void add_link_to_topology_file(FILE *fp, uint16_t f, uint16_t t, int8_t rssi);
extern void begin_topology_file(FILE *fp);
extern void end_topology_file(FILE *fp);
extern void generate_graph();

void process_Msg_RouteRequest(uint16_t src, uint16_t dest);
void print_Msg_RouteReply(Msg_RouteReply *m);
void initialise_msg_route_reply(Msg_RouteReply *m);
void initialise_node_state_gateway(NodeStateQueueGateway *nsqg);
void add_to_node_state_gateway(NodeStateQueueGateway *, NodeState *);
void remove_from_node_state_gateway(NodeStateQueueGateway *);
void print_NodeStateQueueGateway(uint16_t addr);
void print_nrk_time_t(nrk_time_t);
void writeNodeStateToFile(FILE *fptr, uint16_t addr, int pf);
				
/* Firefly functions */
void build_Msg_NgbList(Msg_NgbList *m);
void initialise_node_state(NodeStateQueue *nsqg);
void add_to_node_state(NodeStateQueue *, NodeState *);
void remove_from_node_state(NodeStateQueue *);


/*************************** FUNCTION DEFINITIONS ****************************************************/

void initialise_network_gateway()
{
	uint32_t i;									// loop variable
	
	top_mgr.head = top_mgr.tail = NULL;			// initially no nodes are recorded
	top_mgr.count = 0;		
	Map = NULL;
	cost = NULL;
	edge = NULL;
	parent = NULL;					
	
	// initialise the two hash tables
	for(i = 0; i < SIZE_HT_SNL; i++)
		htsnl[i].ptr = NULL;
	
	for(i = 0; i < SIZE_HT_VL; i++)
	{
		htvl[i].head = htvl[i].tail = NULL;
		htvl[i].count = 0;
	}	

	return;
}
/******************************************************************************************************/
void initialise_nl()
{
	int8_t i;
	
	for(i = 0; i < MAX_NGBS; i++)
		nl.ngbs[i].addr = INVALID_ADDR;
		
	return;
}
/******************************************************************************************************/
void refresh_VL(uint16_t addr)	// C
{
	NeighborGateway *ngptr;
	
	if(DEBUG_NG == 0)
		printf("TDS: Entering refresh_VL:%d\r\n", addr);
	
	ngptr = htvl[addr].head;
	while(ngptr != NULL)
	{
		ngptr -> refreshFlag = FALSE;
		ngptr = ngptr -> down;
	}
	
	if(DEBUG_NG == 0)
		printf("TDS: Leaving refresh_VL\r\n");
	return;
}
/*******************************************************************************************************/
int8_t is_better_ngb(NeighborGateway *first, NeighborGateway *second)	// C
{
	int8_t retValue;
	
	if(DEBUG_NG == 0)
		printf("NG: Entered is_better_ngb()\r\n");
	
	if(first == NULL)
	{
		printf("First is NULL\r\n");
		exit(1);
	}
	else if(second == NULL)
	{
		printf("Second is NULL\r\n");
		exit(1);
	}
	
	retValue = (first -> rssi > second -> rssi) ? TRUE : FALSE;
	
	if(DEBUG_NG == 0)
		printf("NG: Leaving is_better_ngb()\r\n");
		
	return retValue;	
}
/*******************************************************************************************************/
void rank_ngbs(SensorNode *snptr, NeighborGateway *ngptr)
{
	if(DEBUG_NG == 0)
		printf("NG: Entered rank_ngbs() %d %d\r\n", snptr -> addr, ngptr -> addr);
	
	if(ngptr == NULL)
	{
		printf("Fatal bug\r\n");
		exit(1);
	}
	if(snptr -> count == 1)			// this is the only neighbor to be added / updated
	{
		printf("Entered for %d %d\r\n", snptr -> addr, ngptr -> addr);
		snptr -> nbest = ngptr;		// assign the best, sbest and worst neighbors
		snptr -> nworst = ngptr;
		snptr -> nsbest = ngptr;
	}
	else
	{
		if(is_better_ngb(ngptr, snptr -> nworst) == FALSE)	// new neighbor is worse than the 'worst' neighbor
		{
			snptr -> nworst = ngptr;						// assign a new worst neighbor
			if(snptr -> count == 2)							// only two neighbors
				snptr -> nsbest = ngptr;
		}// end if	
		else if(is_better_ngb(ngptr, snptr -> nsbest) == TRUE)	// found a better 'sbest' neighbor
			 {
		 		if(is_better_ngb(ngptr, snptr -> nbest) == TRUE)	// this is better than the 'best' neighbor
				{
					snptr -> nsbest = snptr -> nbest;		// change the 'best' and 'sbest' neighbors
					snptr -> nbest = ngptr;
				}
				else									// new neighbor lies inbetween 'best' and 'sbest'
					snptr -> nsbest = ngptr;			// change only the 'sbest' neighbors
		 	 }  // end if
		 	
	} // end else
	if(DEBUG_NG == 0)
		printf("NG: Leaving rank_ngbs()\r\n");
	
	return;
}
/*******************************************************************************************************/
void remove_node_VL(uint16_t addr)
{
	NeighborGateway *first, *second;
	NeighborGateway *ptr;
	int8_t count;
	
	if(DEBUG_NG == 0)
		printf("TDS: Entered remove_node_VL() with %d %d\r\n", addr, htvl[addr].count);

	first = NULL;
	second = htvl[addr].head;
	
	while(second != NULL)
	{
		if(DEBUG_NG == 2)
			printf("Entered here\n");
		first = second;
		second = second -> down;
		
		if(first -> refreshFlag == FALSE)			// this node needs to be removed
		{
			if(DEBUG_NG == 0)
				printf("NG: remove_node_VL(): Need to remove %d\r\n", first -> addr);
			
			if(first -> down != NULL)				// there exists a node below this one
				first -> down -> up = first -> up;
			if(first -> up == NULL)					// this is the first node in the VL
				htvl[addr].head = first -> down;	
			else									// there is a node above this one
				first -> up -> down = first -> down;
			htvl[addr].count--;						// decrement size of VL
			
			
			if(first -> next != NULL)				// there exists a node to the right of this one
				first -> next -> prev = first -> prev;
			if(first -> prev == NULL)				// this is the first node in the NeighborGatewayList
				first -> toHead -> head = first -> next;
			else
				first -> prev -> next = first -> next;
			
			// go back to head of the NL
			ptr = first;
			count = 0;
			while(ptr -> prev != NULL)
			{
				printf("What\r\n");
				count++;
				if(count > 2)	// debugging
				{
					print_SNL();
					print_VL();
					exit(1);	
				}				
				ptr = ptr -> prev;
			}
			ptr -> toHead -> count--;
			
			free(first);							// All variables updated. Free this node
			
		} // end if
	} // end while
	
	if(DEBUG_NG == 0)
		printf("TDS: Leaving remove_node_VL() with count = %d\r\n", htvl[addr].count);

	return;
}
/******************************************************************************************************/
void prepare_topology_desc_file()
{
	SensorNode *ptr;			// iterator variable
	NeighborGateway *ngptr;		// iterator variable
	int8_t i;					// loop variable
	
	// prepare the topology description file
	for(ptr = top_mgr.head; ptr != NULL; ptr = ptr -> next)
	{
		for(ngptr = ptr -> head; ngptr != NULL; ngptr = ngptr -> next)
			add_link_to_topology_file(topologyFile, ptr -> addr, ngptr -> addr, ngptr -> rssi);
	}
	return;
}
/******************************************************************************************************/
int8_t process_topology_desc(NeighborList nl)
{
	// The below comments apply to a situation where node 1 reports its neighbor list
	// as 1 -- 3,2
	int8_t i,j;			// loop indices
	SensorNode *ptr;	// scratch variable
	Neighbor n;
	NeighborGateway *node;
	int8_t retValue = FALSE;
	
	if(DEBUG_NG == 0)
		printf("NG: Entered process_topology_desc()\r\n");
	
	// flush all the entries in the VL
	refresh_VL(nl.addr);
	//printf("Here1\n");
	
	// check to see if '1' is present in the SNL
	if(search_node_SNL(nl.addr) == NULL)	// '1' was not found in the SNL
	{
		//printf("Here2\n");
		free_data_structures();
		//printf("Here3\n");
	
		add_node_to_SNL(nl.addr);
		retValue = TRUE;				// change in topology description
	}
	
	// '1' is now present in the SNL	
	// now begin to process the members in the neighbor list of '1'
	for(i = 0; i < MAX_NGBS; i++)	// goes through the neighbor list of '1'
	{
		n = nl.ngbs[i];				// 'n' contains information about '3' (i.e. the link 3 --> 1)
		if(n.addr == INVALID_ADDR)	// ignore invalid entries in the neighbor list 	
			continue;
		
		//printf("Here4\n");
		// search for '3' in the SNL 
		ptr = search_node_SNL(n.addr);
		//printf("Here5\n");
		if(ptr == NULL)				// '3' was not found in the SNL
		{
				SensorNode *ptr2;
				//printf("Here6\n");
				free_data_structures();
				//printf("Here7\n");
				ptr2 = add_node_to_SNL(n.addr);	// add '3' to the SNL
				//printf("Here8\n");
				// add '1' to the neighbor list of '3'
				node = add_node_to_NL(ptr2, nl.addr, &n);
				//printf("Here10\n");
				retValue = TRUE;
		} // end if
		else								// '3' was found in the SNL and 'ptr' points to it 
		{
			//printf("Here11\n");
			// go through the neighbor list of '3' to find '1'
			NeighborGateway *ptr3 = search_node_NL(ptr, nl.addr);
			//printf("Here12\n");
			if(ptr3 == NULL)		// '1' was not found in the NeighborGateway list of '3'
			{
				//printf("Here13\n");
				free_data_structures();
				//printf("Here14\n");
				ptr3 = add_node_to_NL(ptr, nl.addr, &n);	// add '1' to the NeighborGateway list of '3'
				//printf("Here15\n");
				retValue = TRUE;
			} //end if
			else				// '1' was found in the NeighborGateway list of '3' and 'ptr3' points to it
			{
				//printf("Here16\n");
				set_ngb_gw(ptr3, &n);	// update the control variables of NeighborGateway '1'
				//printf("Here17\n");
			}
		} // end else
	} // end for
	remove_node_VL(nl.addr);			// look at the refreshFlag values in each node
	//printf("Here18\n\n");
	return retValue;
}
/**********************************************************************************************/
SensorNode* create_node_SNL(uint16_t addr)	// C
{
	int8_t i;	// loop index 
	SensorNode *node;
	int8_t retvalue;
	
	if(DEBUG_NG == 2)
		printf("TDS: Entered create_node_SNL(): %d\r\n", addr);
	
	node = (SensorNode*)malloc(sizeof(SensorNode));
	if(node == NULL)							// not enough memory to hold the node 
	{
		if(DEBUG_NG == 2)
			printf("TDS: Leaving create_node_SNL()\r\n");
		return NULL;
	}				
	node -> addr = addr;						// fill in the contents of the node
	node -> head = node -> tail = NULL;	
	node -> count = 0;
	node -> next = NULL;
	node -> nbest = node -> nsbest = node -> nworst = NULL;
	initialise_node_state_gateway(&(node -> nsqg));
		
	if(DEBUG_NG == 2)
		printf("TDS: Leaving create_node_SNL()\r\n");
	return node;
}
/**********************************************************************************************/
SensorNode* search_node_SNL(uint16_t addr)	// C
{
	if(DEBUG_NG == 2)
		printf("TDS: Entered search_node_SNL()\r\n");
	return htsnl[addr].ptr;		// if the node does not exist, the hash table entry will be NULL
	if(DEBUG_NG == 2)
		printf("TDS: Leaving search_node_SNL()\r\n");
}
/**********************************************************************************************/
SensorNode* add_node_to_SNL(uint16_t addr)	// C
{
	SensorNode *node;					// scratch variables
	SensorNode *ptr;
	
	if(DEBUG_NG == 0)
		printf("TDS:Entered add_node_to_SNL() %d\r\n", addr);
	
	ptr = search_node_SNL(addr);
	if(ptr == NULL)						//  'addr' is not present in the SNL
	{
		node = create_node_SNL(addr);
		if(node == NULL)
		{
			printf("NG: add_node_to_SNL(): Not enough memory to create the sensor node\r\n");
			exit(1);
		}			
		if(top_mgr.head == NULL)		// SNL is empty
			top_mgr.head = top_mgr.tail = node;
		else							// SNL is not empty
		{
			top_mgr.tail -> next = node;
			top_mgr.tail = node;
		}
		
		top_mgr.count++;
		htsnl[addr].ptr = node;			// assign the hash table entry to this node
		if(DEBUG_NG == 0)
			printf("TDS: Leaving add_node_to_SNL()\r\n");
		return node;
	}
	
	// if the code reaches this point, it means the node was found in the list
	if(DEBUG_NG == 0)
		printf("TDS: Leaving add_node_to_SNL()\r\n");
	return ptr;
}
/**********************************************************************************************/
NeighborGateway* create_node_NL(uint16_t addr, Neighbor *n)	// C
{
	NeighborGateway *ptr;
	
	if(DEBUG_NG == 2)
		printf("TDS: Entered create_node_NL() %d\r\n", addr);
	
	ptr = (NeighborGateway*)malloc(sizeof(NeighborGateway));
	if(ptr == NULL)			// not enough memory to create the new node
	{
		return NULL;
	}
	
	// fill up the member of the node
	ptr -> addr = addr;			// address of '1'
	ptr -> rssi = n -> rssi;
	ptr -> lastReport = n -> lastReport;
	ptr -> refreshFlag = TRUE;		// update the refresh flag
	ptr -> next = NULL;
	ptr -> prev = NULL;
	ptr -> toHead = NULL;
	ptr -> up = NULL;
	ptr -> down = NULL;
	
	if(DEBUG_NG == 2)
		printf("TDS: Leaving create_node_NL()\r\n");
	
	return ptr;
}
/**********************************************************************************************/
void set_ngb_gw(NeighborGateway *ngw, Neighbor *n)	// C
{
	if(DEBUG_NG == 0)
		printf("TDS: Entered set_ngb_gw()\r\n");
		
	ngw -> rssi = n -> rssi;				// assign the control variables
	ngw -> lastReport = n -> lastReport;
	ngw -> refreshFlag = TRUE;
	
	if(DEBUG_NG == 0)
		printf("TDS: Leaving set_ngb_gw()\r\n");
	return;
}
/**********************************************************************************************/
NeighborGateway* search_node_NL(SensorNode *snptr, uint16_t addr) // C
{
	NeighborGateway *ngptr;
	
	if(DEBUG_NG == 2)
		printf("TDS: Entered search_node_NL()\r\n");
		
	if(snptr == NULL)		// sanity check
	{
		printf("NG: search_node_NL(): Bug detected in implementation of hash table SNL\r\n");
		exit(1);
	}
	ngptr = snptr -> head;		// assign a pointer to the head of the horizontal list
	
	while(ngptr != NULL)
		if(ngptr -> addr == addr)	// found the neighbor
			break;
		else
			ngptr = ngptr -> next;	// move the pointer ahead
			
	if(DEBUG_NG == 2)
		printf("TDS: Leaving search_node_NL()\r\n");
		
	return ngptr;					// if neighbor is not found, ngptr will equal NULL
}	
/**********************************************************************************************/
NeighborGateway* add_node_to_NL(SensorNode *snptr, uint16_t addr, Neighbor *n) // C
{
	NeighborGateway *ngptr;
	
	if(DEBUG_NG == 0)
		printf("TDS: Entered add_node_to_NL() %d %d\r\n", snptr -> addr, addr);
	
	ngptr = search_node_NL(snptr, addr);
	
	if(ngptr == NULL)	// node not found in NL
	{
		ngptr = create_node_NL(addr, n);	// create a new NeighborGateway node
		if(ngptr == NULL)					// no memory to create the NeighborGateway node
		{
			printf("NG: add_node_to_NL(): Not enough memory to create new NeighborGateway node\r\n");
			exit(1);
		}		
		if(snptr -> head == NULL)			// NL is empty
		{
			snptr -> head = snptr -> tail = ngptr;
			ngptr -> toHead = snptr;				// assign the special back pointer
		}
		else								// NL is not empty
		{
			snptr -> tail -> next = ngptr;
			ngptr -> prev = snptr -> tail;
			snptr -> tail = ngptr;
		}
		snptr -> count++;					// increment the number of neighbors recorded
		if(DEBUG_NG == 2)
			printf("Calling add_node_to_VL() with SNL addr = %d\r\n", snptr -> addr);
		add_node_to_VL(ngptr);				// add this node to the vertical list
		
	}
	else									// neighbor is found in the NL
	{
		set_ngb_gw(ngptr, n);			// update information about this neighbor
	}
	
	rank_ngbs(snptr, ngptr);				// set the 'best', 'sbest' and 'worst' neighbors
	if(DEBUG_NG == 0)
		printf("TDS: Leaving add_node_to_NL()\r\n");
	return ngptr;
}	
/***********************************************************************************************/
void remove_node_NL(SensorNode *snptr, NeighborGateway *ngptr)	// C
{
	if(DEBUG_NG == 0)
		printf("NG: Entering remove_node_NL(): %d %d\r\n", snptr -> addr, ngptr -> addr);
	
	// remove this node from the NL first
	if(ngptr -> next != NULL)		// there exists a node to the right of this one
		ngptr -> next -> prev = ngptr -> prev;
	if(ngptr -> prev == NULL)		// this is the first node in the NL
		ngptr -> toHead -> head = ngptr -> next;
	else							// there exists a node to the left of this one
		ngptr -> prev -> next = ngptr -> next;
	snptr -> count--;				// decrement number of nodes in NeighborGateway list
	
	// now remove this node from the VL
	if(ngptr -> down != NULL)		// there exists a node below this one
		ngptr -> down -> up = ngptr -> up;
	if(ngptr -> up == NULL)			// this is the first node in the VL
		htvl[ngptr -> addr].head = ngptr -> down;
	else
		ngptr -> up -> down = ngptr -> down;	// there exists a node above this one
	htvl[ngptr -> addr].count--;	// decrement number of nodes in the VL
		
	// this function is only called from generate_routing_tables(). Hence we only need to 
	// check for the worst neighbor
	if(snptr -> nworst == ngptr)			// this happens to be the 'worst' neighbor
		snptr -> nworst = NULL;
		
	free(ngptr);	// all pointers adjusted. Free this node
	
	if(DEBUG_NG == 0)
		printf("NG: Leaving remove_node_NL()\r\n");
	
	return;
}
/**********************************************************************************************/
void add_node_to_VL(NeighborGateway *ngptr)		// C
{
	if(DEBUG_NG == 0)
		printf("TDS: Entered add_node_to_VL() with addr = %d\r\n", ngptr -> addr);

	if(ngptr -> down != NULL)	// sanity check
	{
		printf("Fatal bug\r\n");
		exit(1);
	}	
	if(htvl[ngptr -> addr].head == NULL)	// VL is empty
	{
		htvl[ngptr -> addr].head = htvl[ngptr -> addr].tail = ngptr;
	}
	else	// VL is not empty
	{
		htvl[ngptr -> addr].tail -> down = ngptr;
		ngptr -> up = htvl[ngptr -> addr].tail;
		htvl[ngptr -> addr].tail = ngptr;
		
	}
	htvl[ngptr -> addr].count++;
	
	if(DEBUG_NG == 0)
		printf("TDS: Leaving add_node_to_VL()\r\n");
	return;
}
/************************************************************************************************/
void print_SNL()
{
	SensorNode *snptr; 
	NeighborGateway *ngptr;
	
	printf("SensorNode List\n");
	snptr = top_mgr.head;
	while(snptr != NULL)
	{
		printf("%d -> ", snptr -> addr);
		ngptr = snptr -> head;
		while(ngptr != NULL)
		{
			printf("[%d %d],  ", ngptr -> addr, ngptr -> rssi);
			ngptr = ngptr -> next;
		}
		printf("\r\n");
		snptr = snptr -> next;
	}
	return;
} 
/************************************************************************************************/
void print_VL()
{
	int i;
	NeighborGateway *ptr;
	
	printf("Vertical list:\n");
	//for(i = 0; i < SIZE_HT_VL; i++)
	for(i = 0; i < 100; i++)
	{
		if(htvl[i].head == NULL)
			continue;
			
		ptr = htvl[i].head;
		while(ptr != NULL)
		{
			printf("%d, ", ptr -> addr);
			ptr = ptr -> down;
		}
		printf("\r\n");
	}
	
	return;
}
/************************************************************************************************/
void end_program()
{
	SensorNode *prev, *ptr;
	NeighborGateway *prev1, *ptr1;
	
	prev = NULL;
	ptr = top_mgr.head;
	
	while(ptr != NULL)
	{
		prev = ptr;
		ptr = ptr -> next;
		
		prev1 = NULL;
		ptr1 = prev -> head;
		
		while(ptr1 != NULL)
		{
			prev1 = ptr1;
			ptr1 = ptr1 -> next;
			free(prev1);
		}
		
		free(prev);
	}
		
	return;
}
/************************************************************************************************/
void free_data_structures()
{
	int8_t i;	// loop index
	
	if(Map != NULL)
	{
		free(Map);
		Map = NULL;
		
	}
	if(edge != NULL)
	{
		for(i = 1; i <= top_mgr.count; i++)
			if(edge[i] != NULL)
			{
				free(edge[i]);
				edge[i] = NULL;
			}
		free(edge);
		edge = NULL;
	}
	if(parent != NULL)
	{
		for(i = 1; i <= top_mgr.count; i++)
			if(parent[i] != NULL)
			{
				free(parent[i]);
				parent[i] = NULL;
			}
		free(parent);
		parent = NULL;
	}
	if(cost != NULL)
	{
		for(i = 1; i <= top_mgr.count; i++)
			if(cost[i] != NULL)
			{
				free(cost[i]);
				cost[i] = NULL;
			}
		free(cost);
		cost = NULL;
	}
	return;
}
/************************************************************************************************/
int8_t node_to_graph(uint16_t node_addr)
{
	int8_t i;	// loop variable
	
	for(i = 1; i <= top_mgr.count; i++)
	{
		if(Map[i] == node_addr)			// found the mapping
				return i;
	}
	return INVALID_ADDR;				// no mapping found
}
/***********************************************************************************************/
uint16_t graph_to_node(int8_t graph_addr)
{
	return Map[graph_addr];
}
/**********************************************************************************************/
void generate_routing_tables()
{
	SensorNode *node;						// iterator variable
	int8_t i,j;								// loop indices
	int8_t row, column;						// used to index the various matrices
	RoutingTableGateway rt;					// to hold the routing table for one node
	SensorNode *ptr;						// to hold return types of functions
	NeighborGateway *ngptr;					// iterator variable
	char ch;	
	
	if(DEBUG_NG == 1)
		printf("Entered generate_routing_tables()\n");
	// firstly clean up the data structures required for the Floyd Warshall
	// algorithm since we need to recreate them at the end of process_topology_desc
	//free_data_structures();	
	
	// construct the necessary data structures
	Map = (uint16_t*)malloc( (top_mgr.count + 1) * sizeof(uint16_t) );
	if(Map == NULL)
	{
		printf("NG: generate_routing_tables(): Not enough memory to construct the Map table\r\n");
		exit(1);
	}
	edge = (uint8_t **)malloc( (top_mgr.count + 1) * sizeof(uint8_t*) );
	if(edge == NULL)
	{
		printf("NG: generate_routing_tables(): Not enough memory to construct the edge[] table\r\n");
		exit(1);
	}
	parent = (uint8_t **)malloc( (top_mgr.count + 1) * sizeof(uint8_t*) );
	if(parent == NULL)
	{
		printf("NG: generate_routing_tables(): Not enough memory to construct the parent[] table\r\n");
		exit(1);
	}
	cost = (uint8_t **)malloc( (top_mgr.count + 1) * sizeof(uint8_t*) );
	if(cost == NULL)
	{
		printf("NG: generate_routing_tables(): Not enough memory to construct the cost[] table\r\n");
		exit(1);
	}
	
	for(i = 1; i <= top_mgr.count; i++)
	{
		edge[i] = (uint8_t*)malloc( (top_mgr.count + 1) * sizeof(uint8_t) );
		if(edge[i] == NULL)
		{
			printf("NG: generate_routing_tables(): Not enough memory to construct the edge[%d][] row\r\n", i);
			exit(1);
		}
		parent[i] = (uint8_t*)malloc( (top_mgr.count + 1) * sizeof(uint8_t) );
		if(parent[i] == NULL)
		{
			printf("NG: generate_routing_tables(): Not enough memory to construct the parent[%d][] row\r\n", i);
			exit(1);
		}
		cost[i] = (uint8_t*)malloc( (top_mgr.count + 1) * sizeof(uint8_t) );
		if(cost[i] == NULL)
		{
			printf("NG: generate_routing_tables(): Not enough memory to construct the cost[%d][] row\r\n", i);
			exit(1);
		}
	} // end for
	if(DEBUG_NG == 1)
		printf("Finished creating matrices\r\n");
	
	// prune the graph to include links with RSSI > RX_POWER_THRESHOLD. At the same time
	// map the node IDs to graph IDs
	for(node = top_mgr.head, j = 1; node != NULL; node = node -> next, j++)
	{		
		for(ngptr = node -> head; ngptr != NULL; ngptr = ngptr -> next)
		{
			if( (ngptr -> rssi < RX_POWER_THRESHOLD) && (ngptr != node -> nbest) && (ngptr != node -> nsbest) )
			{
				if(node -> nworst == ngptr)			// this happens to be the 'worst' neighbor
					node -> nworst = NULL;
			
				remove_node_NL(node, ngptr);		// remove this neighbor
			}
		}
		Map[j] = node -> addr;
	} // end for
	
	if(j != top_mgr.count + 1)	// sanity check for debugging
		while(1)
			printf("NG: generate_routing_tables(): Bug detected in implementation of top_mgr.count: ERROR\r\n");
			
	if(DEBUG_NG == 1)
		printf("Finished pruning the graph and creating Map\n");
		
	// initialise the edge cost, path cost and parent matrices
	for(i = 1; i <= top_mgr.count; i++)
		for(j = 1; j <= top_mgr.count; j++)
		{
			if(j == i)
			{
				edge[i][j] = 0;					// cost of node to itself is zero
				cost[i][j] = 0;
				parent[i][j] = INVALID_ADDR;	// self loop
			}
			else
			{
				edge[i][j] = 0;					// assume no links initially
				cost[i][j] = INFINITY;
				parent[i][j] = INVALID_ADDR;
			}
			
		}
			
	// fill up the edge cost, path cost and parent matrices
	for(node = top_mgr.head; node != NULL; node = node -> next)
	{
		row = node_to_graph(node -> addr);
		if(row == INVALID_ADDR)		// debugging check
		{
			printf("NG: generate_routing_tables(): Bug detected in creating Map table(row)\n");
		}
		for(ngptr = node -> head; ngptr != NULL; ngptr = ngptr -> next)
		{
			column = node_to_graph(ngptr -> addr);
			if(column == INVALID_ADDR)
			{
				printf("NG: generate_routing_tables(): Bug detected in creating Map table(column)\n");
			}
			edge[row][column] = 1;	 // indicates presence of a link
			parent[row][column] = row;
			/*if(ngptr -> rssi < RX_POWER_THRESHOLD)	
				cost[row][column] = RX_POWER_THRESHOLD - ngptr -> rssi;
			else
				cost[row][column] = 1;						// cost of a strong neighbor is 1
			*/
			cost[row][column] = ngptr -> rssi;				// only for debugging
			
		} // end for
	} // end for
	
	if(DEBUG_NG == 1)
		printf("Finished assigning all matrices\r\n");
	
	if(DEBUG_NG == 1)
	{
		printf("Map:\r\n");
		print_Map();
	}
	if(DEBUG_NG == 1)
	{
		printf("edge matrix:\r\n");
		print_edge_matrix();
	}
	if(DEBUG_NG == 1)
	{
		printf("Cost matrix\r\n");
		print_cost_matrix();
	}
	if(DEBUG_NG == 1)
	{
		printf("Parent matrix\r\n");
		print_parent_matrix();
	}
	
	printf("Press any key to continue: ");
	getchar();
	// inputs to floyd-warshall are prepared. Run the algorithm
	if(DEBUG_NG == 1)
		printf("\nBeginning Floyd Warshall\r\n");
	
	floyd_warshall(top_mgr.count, edge, parent, cost);
	
	if(DEBUG_NG == 1)
	{
		printf("cost matrix:\r\n");
		print_cost_matrix();
		
		printf("Parent matrix:\r\n");
		print_parent_matrix();
	}	
	printf("Press any key to continue: ");
	getchar();
	
	// cost and parent matrices are now prepared. Compute routing table for each
	// node
	// the 'row' selects a particular source node, the 'column' is all the destinations
	for(row = 1; row <= top_mgr.count; row++)
	{
		// initialise the routing table structure
		initialise_routing_table_gateway(&rt);
		
		for(column = 1, i = 0; column <= top_mgr.count; column++, i++)
		{
			rt.rte[i].dest = graph_to_node(column);
			rt.rte[i].cost = cost[row][column];
			if(rt.rte[i].cost == INFINITY)				// required since parent[row][column] == INVALID_ADDR is ambiguous
				rt.rte[i].nextHop = INVALID_ADDR;
			else if(rt.rte[i].cost == 0)				// if column == row
				 {
				 	if(column != row)					// this should not happen. Cost has to be non-zero for a link joining two distinct nodes
				 		while(1){ printf("NG: generate_routing_tables(): Bug1 detected in routing table construction: ERROR\r\n"); exit(1);}
				 	rt.rte[i].nextHop = INVALID_ADDR;
				 }
				 else										// cost is non-zero
				 {			
					int8_t par = get_parent(row, column);	
					// sanity check for debugging. This should never happen since the path is valid
					if(par == INVALID_ADDR)
						while(1){ printf("NG: generate_routing_tables(): Bug2 detected in routing table construction: ERROR\r\n"); exit(1); }
					else
						rt.rte[i].nextHop = graph_to_node(par);
				 } // end else
		} // end for
		
		// At this stage, the routing table is prepared for node with graph ID 'row'
		// insert it into the appropriate place in the VL
		ptr = search_node_SNL(graph_to_node(row));
		if(ptr == NULL)		// sanity check. This should never happen
			while(1)
				printf("NG: generate_routiing_tables(): Bug detected in implementation of top_mgr: ERROR\r\n");
			
		ptr -> rt = rt;								// assign the node its routing table
		if(DEBUG_NG == 1)
			print_rt(graph_to_node(row));
		
	} // end for		
	
	return;
}
/**********************************************************************************************/
void floyd_warshall(int8_t n, uint8_t **edge, uint8_t **parent, uint8_t **cost)
{
	int8_t i, j, k;
	
	for(k = 1; k <= n; k++)
		for(i = 1; i <= n; i++)
			for(j = 1; j <= n; j++)
				if(cost[i][k] + cost[k][j] < cost[i][j])
				{
					cost[i][j] = cost[i][k] + cost[k][j];
					parent[i][j] = parent[k][j];
				}
	
	return;
}
/**********************************************************************************************/
int8_t get_parent(int8_t from, int8_t to)
{
	if(parent[from][to] == INVALID_ADDR)
		if(from == to)		// same node
			return from;
		else
			return INVALID_ADDR;
	
	if(parent[from][to] == from)	// direct link
		return to;					// the nextHop node is the one on the other side of the link
	
	return get_parent(from, parent[from][to]);
}	
/**********************************************************************************************/
void initialise_routing_table_gateway(RoutingTableGateway *rt)
{
	int8_t i;	// loop variable
	
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
	{
		rt -> rte[i].dest = INVALID_ADDR;
		rt -> rte[i].nextHop = INVALID_ADDR;
		rt -> rte[i].cost = 0;
	}
	return;
}	
/************************************************************************************************/
void writeNodeStateToFile(FILE *fptr, uint16_t addr, int pf)
{
	SensorNode *snptr;
	int8_t index;
	
	if(pf == TRUE)
		printf("Wrote to file the contents:\n");
	
	snptr = htsnl[addr].ptr;
	for(index = snptr -> nsqg.front; index != snptr -> nsqg.rear; index = (index + 1) % MAX_STATE_VALUES_GATEWAY)
		fprintf(fptr, "%d.%d -> %d %d %d\n", snptr -> nsqg.ns[index].timestamp.secs, snptr -> nsqg.ns[index].timestamp.nano_secs, snptr -> nsqg.ns[index].battery, snptr -> nsqg.ns[index].tx_queue_size, snptr -> nsqg.ns[index].rx_queue_size);
	printf("\n");
	
	return;
}
/************************************************************************************************/
void writeTopToFile(FILE *fptr, int pf)
{
	SensorNode *snptr;
	NeighborGateway *ngptr;
	
	if(pf == TRUE)
		printf("Wrote to file the contents:\n");
	
	fprintf(fptr, "%d\n", top_mgr.count);
	
	if(pf == TRUE)
		printf("%d\n", top_mgr.count);
				
	for(snptr = top_mgr.head; snptr != NULL; snptr = snptr -> next)
	{
		fprintf(fptr, "%d(%d) -> ", snptr -> addr, snptr -> count);
		if(pf == TRUE)
			printf("%d(%d) -> ", snptr -> addr, snptr -> count);
		for(ngptr = snptr -> head; ngptr != NULL; ngptr = ngptr -> next)
		{
			fprintf(fptr, "[%d %d], ", ngptr -> addr, ngptr -> rssi);
			if(pf == TRUE)
				printf("[%d %d], ", ngptr -> addr, ngptr -> rssi);
		}						
		fprintf(fptr, "\n");
		if(pf == TRUE)
			printf("\n");
	}
	return;
}
/************************************************************************************************/
int main()
{
	int num, i,j, choice, count;
	char ch, fileName[100];
	uint16_t addr, src, dest;
	FILE *fptr, *fp;
	Neighbor n;
	SensorNode *snptr;
	NeighborGateway *ngptr;
	Msg_NgbList mn;
			
	initialise_network_gateway();
	system("clear");
	printf("This program tests the data structure used to maintain toplogy description of the sensor subnet\r\n");
	
	do
	{
		printf("\n\n");
		printf("1.  Create NeighborList\r\n");
		printf("2.  Print SNL and VL\r\n");
		printf("3.  Clear topology information\r\n");
		printf("4.  Load topology from a file\r\n");
		printf("5.  Write topology to a file\r\n");
		printf("6.  Display file contents\r\n");
		printf("7.  Delete topology file\r\n");
		printf("8.  Generate routing tables\r\n");
		printf("9.  Print routing table\r\n");
		printf("10. Generate graph\r\n");
		printf("11. Create Msg_RouteRequest\r\n");
		printf("12. Create Msg_NodeInfo\r\n");
		printf("13. Print Node State information\n");
		printf("14. Query a node for topology information\n");
		printf("15. Query a node for state information\n");
		printf("16. Write node state values to a file\r\n");
		printf("17. Load node state values from a file\r\n");
		printf("18. Exit\r\n");
		printf("Enter your choice: ");
		scanf("%d", &choice);
	
		switch(choice)
		{
			case 1:				// create a NeighborList message
				initialise_nl();					// invalidate all neighbors
				printf("Enter number of neighbors: ");
				scanf("%d", &num);
				if(num > MAX_NGBS)
				{
					printf("Invalid number of neighbors entered\n");
					break;
				}
				nl.count = num;
				
				printf("Enter source node addr: ");
				scanf("%d", &nl.addr);
				
				for(i = 0; i < num; i++)
				{
					printf("Enter information about Neighbor %d: ", i + 1);
					scanf("%d %d", &nl.ngbs[i].addr, &nl.ngbs[i].rssi);
				} 
				/* pack and unpack the message */
				build_Msg_NgbList(&mn);
				pack_NW_Packet_header(tx_buf, &pkt_tx);
				memcpy(tx_buf + SIZE_NW_PACKET_HEADER, pkt_tx.data, MAX_NETWORK_PAYLOAD);
				for(i = 0; i < SIZE_NW_PACKET; i++)
					rx_buf[i] = tx_buf[i];
				unpack_NW_Packet_header(&pkt_rx, rx_buf);
				memcpy(pkt_rx.data, rx_buf + SIZE_NW_PACKET_HEADER, MAX_NETWORK_PAYLOAD);
				unpack_Msg_NgbList(&mn, pkt_rx.data);				
				
				process_topology_desc(mn.nl);
				fp = (FILE*)fopen("top_desc.txt", "a");
				if(fp == NULL)
				{
					perror("File cannot be opened");
					exit(1);
				}
				writeTopToFile(fp, FALSE);
				fclose(fp);				
				generate_routing_tables();
				break;
			
			case 2:			// print topology information
				print_SNL();
				print_VL();
				break;
				
			case 3:			// clear topology description
				end_program();
				initialise_network_gateway();
				if(remove("SensorTopology.dot") < 0)
				{
					perror("File cannot be deleted");
					exit(1);
				}
				if(remove("SensorTopology.gif") < 0)
				{
					perror("File cannot be deleted");
					exit(1);
				}				
				break;
				
			case 4:			// read topology from a file
				printf("Enter file name with extension: ");
				scanf("%s", fileName);
				
				fptr = (FILE*)fopen(fileName, "r");
				if(fptr == NULL)
				{
					perror("File cannot be opened");
					break;
				}
				end_program();
				initialise_network_gateway();
				
				fscanf(fptr, "%d", &num);			// read number of lines to read
				printf("Number of lines to read = %d\n", num);
				
				for(j = 0; j < num; j++)
				{
					fscanf(fptr, "%d(%d) -> ", &addr, &count);
					printf("%d(%d) -> ", addr, count);
					snptr = add_node_to_SNL(addr);
					for(i = 0; i < count; i++)
					{
						fscanf(fptr, "[%d %d], ", &addr, &(n.rssi));
						printf("[%d %d], ", addr, n.rssi);
						add_node_to_NL(snptr,addr, &n);
					}
				}
				printf("File read successfully\r\n");
				fclose(fptr);
				break;
				
			case 5:			// Writing topology description to a file
				printf("Enter file name with extension: ");
				scanf("%s", fileName);
				fptr = (FILE*)fopen(fileName, "w");
				if(fptr == NULL)
				{
					perror("File cannot be opened");
					break;
				}			
				writeTopToFile(fptr, TRUE);
				printf("File written to successfully\n");
				fclose(fptr);
				break;	
				
			case 6:		// display contents of a file
				printf("Enter name of the file with extension: ");
				scanf("%s", fileName);
				fptr = (FILE*)fopen(fileName, "r");
				if(fptr == NULL)
				{
					perror("File cannot be opened");
					break;
				}
				printf("File contents are:\n");
				while(!feof(fptr))
					fputc(fgetc(fptr), stdout);
				fclose(fptr);
				break;
				
			case 7:		// delete a topology description file
				printf("Enter name of the file with extension: ");
				scanf("%s", fileName);
				if(remove(fileName) < 0)
					perror("File cannot be deleted");
				else
					printf("File deleted\n");
				break;
				
			case 8:		// generate routing table
				if(top_mgr.count == 0)	// no topology recorded yet
					printf("ERROR: Enter topology information first\r\n");
				else
				{
					generate_routing_tables();
				}
				break;
			
			case 9:		// print routing table
				printf("Enter node ID: ");
				scanf("%d", &addr);
				print_RoutingTableGateway(addr);
				break;	
				
			case 10:		// generate topology graph
				topologyFile = (FILE*)fopen("SensorTopology.dot", "w");
				if(topologyFile == NULL)
				{
					perror("Topology generation file could not be opened");
					exit(1);
				}
				begin_topology_file(topologyFile);
				prepare_topology_desc_file();						
				end_topology_file(topologyFile);					
				fclose(topologyFile);								// close the file
				generate_graph();									// invoke a graph-drawing program
				break;
	
			case 11:					// Msg_RouteRequest
				printf("Enter destination and source node: ");
				scanf("%d %d", &dest, &src);
				printf("You entered %d %d\n", dest, src);
				
				process_Msg_RouteRequest(src, dest);				
				print_Msg_RouteReply(&mrr);
				break;	
				
			case 12: 					// Msg_NodeState
				printf("Enter address of the node: ");
				scanf("%d", &addr);
				if(htsnl[addr].ptr == NULL)		// node not present in the SNL
					add_node_to_SNL(addr);
				
				printf("Enter number of values to report: ");
				scanf("%d", &num);
				
				for(i = 0; i < num; i++)
				{
					printf("Enter battery, tx_queue_size, rx_queue_size, timestamp for value %d: ", i + 1);
					scanf("%d %d %d %d %d", &ns.battery, &ns.tx_queue_size, &ns.rx_queue_size, &ns.timestamp.secs, &ns.timestamp.nano_secs);
					add_to_node_state_gateway(&(htsnl[addr].ptr -> nsqg), &ns);
				}
				print_NodeStateQueueGateway(addr);
				break;
					
			case 13:
				printf("Enter node address: ");
				scanf("%d", &addr);
				if(htsnl[addr].ptr == NULL)
					printf("Node %d does not exist in the SNL\n", addr);
				else
				{
					print_NodeStateQueueGateway(addr);
				}
				break;
								
			case 14:
				break;
			
			case 15:
				break;
				
			case 16: 			// write node state values to a file
				printf("Enter file name with extension: ");
				scanf("%s", fileName);
				fptr = (FILE*)fopen(fileName, "w");
				if(fptr == NULL)
				{
					perror("File cannot be opened");
					break;
				}	
				printf("Enter node address: ");
				scanf("%d", &addr);		
				if(htsnl[addr].ptr == NULL)		// this node does not exist in the SNL
				{
					printf("Node %d does not exist in the SNL\n", addr);
					break;
				}
				writeNodeStateToFile(fptr, addr, TRUE);
				printf("File written to successfully\n");
				fclose(fptr);
				break;	
				
			case 17:
				break;
				
			case 18:					// Exit the gateway
				 break;
				
			default:
				printf("Invalid option: Try again\r\n");
		} // end outer switch
	
	} while(choice != 18);
	
	end_program();						// clean up the data structures in memory
	return 0;
}	 // end main()

/****************************************** PRINTING FUNCTIONS ******************************************/
void print_Map()
{
	int8_t i;
	
	for(i = 1; i <= top_mgr.count; i++)
	{
		printf("%d  %d\r\n", i, Map[i]);
	}
	return;
}
/*****************************************************************************/
void print_edge_matrix()
{
	int8_t i, j;
	
	// first print all the column headings
	printf("\t");
	for(i = 1; i <= top_mgr.count; i++)
		printf("%d\t", Map[i]);
	printf("\r\n");
	
	for(i = 1; i <= top_mgr.count; i++)
	{
		printf("%d\t", Map[i]);
		for(j = 1; j <= top_mgr.count; j++)
			printf("%d\t ", edge[i][j]);
				
		printf("\r\n");
	}
	return;
}
/******************************************************************************/
void print_parent_matrix()
{
	int8_t i, j;
	
	// first print all the column headings
	printf("\t");
	for(i = 1; i <= top_mgr.count; i++)
		printf("%d\t", Map[i]);
	printf("\r\n");
	
	for(i = 1; i <= top_mgr.count; i++)
	{
		printf("%d\t", Map[i]);
		for(j = 1; j <= top_mgr.count; j++)
		{
			if(parent[i][j] == INVALID_ADDR)
				printf("INV\t ");
			else
				printf("%d\t ", graph_to_node(parent[i][j]));
		}			
		printf("\n");
	}
	return;
}	
/***********************************************************************************/
void print_cost_matrix()
{
	int8_t i, j;
	
	// first print all the column headings
	printf("\t");
	for(i = 1; i <= top_mgr.count; i++)
		printf("%d\t", Map[i]);
	printf("\r\n");
	
	for(i = 1; i <= top_mgr.count; i++)
	{
		printf("%d\t", Map[i]);
		for(j = 1; j <= top_mgr.count; j++)
		{
			if(cost[i][j] == INFINITY)
				printf("INF\t ");
			else
				printf("%d\t ", cost[i][j]);
		}			
		printf("\r\n");
	}
	return;
}
/*************************************************************************************************/
/************************************************************************************/
void print_RoutingTableGateway(uint16_t addr)
{
	RoutingTableGateway rt;
	int8_t i;
	
	rt = htsnl[addr].ptr -> rt;
	printf("Routing Table for %d:\n", addr);
	printf("Dest\t\tNextHop\t\tCost\n");
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
		if(rt.rte[i].dest != INVALID_ADDR)		// valid entry in the routing table
		{
			if(rt.rte[i].dest == addr)
				printf("%d\t\tINV\t\t%d\n", rt.rte[i].dest, rt.rte[i].cost);
			else
				printf("%d\t\t%d\t\t%d\n", rt.rte[i].dest, rt.rte[i].nextHop, rt.rte[i].cost);
		}
	printf("\n");
	return;
}
/*************************************************************************************/
/*******************************************************************************************/
void process_Msg_RouteRequest(uint16_t src, uint16_t dest)
{
	int8_t i, j, abandon = FALSE;;
	uint16_t from;
	SensorNode *snptr;
	
	// fill in the members of the Msg_RouteReply
	initialise_msg_route_reply(&mrr);
	mrr.dest = dest;
	from = src;
	for(i = 0; i < NUM_ROUTE_REPLY_ELEMENTS && abandon == FALSE; i++)
	{
		if(from == dest)	// message constructed
		{
			//printf("Finishing message construction. From = %d\n", from);
			break;
		}
		// search the routing table for node 'from' to get nextHop address to node 'dest'
		snptr = htsnl[from].ptr;
		if(snptr == NULL)		// source node is not found
		{
			printf("From node is not found in the SNL\n");
			abandon = TRUE;		// no point in continuing
			break;
		}
		for(j = 0; j < MAX_SUBNET_SIZE; j++)
		{
			if(snptr -> rt.rte[j].dest == dest)		// an entry for the destination exists
			{
				if(snptr -> rt.rte[j].nextHop != INVALID_ADDR)		// a path exists to the destination
				{
					// fill up one element in the Msg_RouteReply message
					mrr.rre[i].src = from;				
					mrr.rre[i].nextHop = snptr -> rt.rte[j].nextHop;
					mrr.rre[i].cost = snptr -> rt.rte[j].cost;
					from = snptr -> rt.rte[j].nextHop;
					break;
				}
				else									// no path exists to the destination
				{
					abandon = TRUE;						// no point continuing
					break;
				}
			} // end if
		} // end inner for
		if(j == MAX_SUBNET_SIZE) // at this point we know that no entry is present for the destination
			break;				// no point continuing									
	} // end for

	return;
}		
/**************************************************************************************************/
void print_Msg_RouteReply(Msg_RouteReply *m)
{
	int8_t i;
	
	printf("Msg_RouteReply is:\n");
	
	if(m -> dest == INVALID_ADDR)
		printf("dest = INV\n");
	else
		printf("dest = %d\n", m -> dest);
	for(i = 0; i < NUM_ROUTE_REPLY_ELEMENTS; i++)
		if(m -> rre[i].src != INVALID_ADDR)
			printf("%d[%d %d]\n", m -> rre[i].src, m -> rre[i].nextHop, m -> rre[i].cost);
		else
			printf("INV[]\n");

	printf("\n");
}

/**************************************************************************************************/
void initialise_msg_route_reply(Msg_RouteReply *m)
{
	int8_t i;
	
	m -> dest = INVALID_ADDR;
	m -> dg = INVALID_ADDR;
	for(i = 0; i < NUM_ROUTE_REPLY_ELEMENTS; i++)
	{
		m -> rre[i].src = INVALID_ADDR;
		m -> rre[i].nextHop = INVALID_ADDR;
		m -> rre[i].cost = 0;
	}
	return;
}
/**************************************************************************************************/
void initialise_node_state_gateway(NodeStateQueueGateway *nsqg)
{
	nsqg -> front = nsqg -> rear = 0;			// initialise the circular queue
	return;
}
/**************************************************************************************************/
void add_to_node_state_gateway(NodeStateQueueGateway *nsqg, NodeState *ns)
{	
	int8_t rear = nsqg -> rear;
	int8_t front = nsqg -> front;
	
	if( (rear + 1) % MAX_STATE_VALUES == front )		// node's state queue is full
		remove_from_node_state_gateway(nsqg);					// remove the oldest entry
		
	// now add the new entry
	nsqg -> ns[rear] = *ns;
	nsqg -> rear = (nsqg -> rear + 1) % MAX_STATE_VALUES;
	return;
}	
/**********************************************************************************************/
void remove_from_node_state_gateway(NodeStateQueueGateway *nsqg)
{
	if(nsqg -> front == nsqg -> rear)	// queue is empty
		return;
		
	nsqg -> front = (nsqg -> front + 1) % MAX_STATE_VALUES;
	
	return;
}
/***********************************************************************************************/
void print_NodeStateQueueGateway(uint16_t addr)
{
	int8_t index;
	SensorNode *snptr;
	
	snptr = htsnl[addr].ptr;
	if(snptr == NULL)
	{
		printf("Node %d does not exist in the SNL\n");
		return;
	}
	printf("Queue values for %d are:\n", addr);
	for(index = snptr -> nsqg.front; index != snptr -> nsqg.rear; index = ((index + 1) % MAX_STATE_VALUES_GATEWAY))
	{
		printf("%d %d %d ", snptr -> nsqg.ns[index].battery, snptr -> nsqg.ns[index].tx_queue_size, snptr -> nsqg.ns[index].rx_queue_size);
		print_nrk_time_t(snptr -> nsqg.ns[index].timestamp);
		printf("\n");
	}
	
	printf("\n");
	return;
}
/**************************************************************************************************/ 
void print_nrk_time_t(nrk_time_t time)
{
	printf("%d %d", time.secs, time.nano_secs);
	return;
}
/*************************************************************************************************/
	
	
	
				
/**********************************************************************************************/

/************************************ Firefly node functions ************************************/
void initialise_node_state(NodeStateQueue *nsq)
{
	nsq -> front = nsq -> rear = 0;			// initialise the circular queue
	return;
}
/**************************************************************************************************/
void add_to_node_state(NodeStateQueue *nsq, NodeState *ns)
{	
	int8_t rear = nsq -> rear;
	int8_t front = nsq -> front;
	
	if( (rear + 1) % MAX_STATE_VALUES == front )		// node's state queue is full
		remove_from_node_state(nsq);					// remove the oldest entry
		
	// now add the new entry
	nsq -> ns[rear] = *ns;
	nsq -> rear = (nsq -> rear + 1) % MAX_STATE_VALUES;
	return;
}	
/**********************************************************************************************/
void remove_from_node_state(NodeStateQueue *nsq)
{
	if(nsq -> front == nsq -> rear)	// queue is empty
		return;
		
	nsq -> front = (nsq -> front + 1) % MAX_STATE_VALUES;
	
	return;
}
/*******************************************************************************************/
void build_Msg_NgbList(Msg_NgbList *m)	
{
	m -> nl = nl;			// copy the NeighborList into the message
	
	// Build the network packet which will hold the NGB_LIST message
  	/*pkt_tx.src = (uint16_t)NODE_ADDR;
  	enter_cr(nl_sem, 25);	// acquire the semaphore
  	pkt_tx.dest = DEFAULT_GATEWAY;	// assign the default gateway
  	leave_cr(nl_sem, 25);	// release the semaphore
  	pkt_tx.nextHop = route_addr(pkt_tx.dest);
  	pkt_tx.prevHop = (uint16_t)NODE_ADDR;
  	pkt_tx.prevprevHop = (uint16_t)NODE_ADDR;
  	*/
  	pkt_tx.ttl = MAX_NETWORK_DIAMETER;		
  	pkt_tx.type = (uint8_t)NGB_LIST;
  	pkt_tx.length = SIZE_MSG_NGB_LIST;
  	pkt_tx.prio = NORMAL_PRIORITY;
  	
  	pack_Msg_NgbList(pkt_tx.data, m);
  	
  	return;
}		
/********************************************************************************************/


/**************************************************************************************************/
	
		
		
		
