#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "NetworkGateway.h"

/*********************************** Extern variables and functions ******************************/
// From TopologyGeneration.c 
extern void begin_topology_file(FILE *);
extern void end_topology_file(FILE *);
extern void add_link_to_topology_file(FILE *, uint16_t, uint16_t, int8_t);
extern void generate_graph();

// From NGPack.c
extern void unpack_Msg_NgbList(Msg_NgbList *, uint8_t *);
extern void unpack_NodeToGatewaySerial_Packet_header(NodeToGatewaySerial_Packet *, uint8_t *);
extern int8_t endianness();
extern void pack_Msg_RoutingTable(uint8_t *, Msg_RoutingTable*);

/*********************************** Global data structures **************************************/
static uint8_t rx_buf[SIZE_NODETOGATEWAYSERIAL_PACKET];// receive buffer to hold bytes from attached node
static FILE *topologyFile;								// pointer to file that stores topology information 

static TopologyManager top_mgr;						// to manage information about links
static uint16_t *Map;									// to hold a mapping between node IDS 
												// and graph IDs
static uint8_t **edge;							// edge cost matrix
static uint8_t **parent;								// parent matrix (graph IDs)
static uint8_t **cost;								// to hold the shortest path costs

static Msg_RoutingTable mrt;							// to hold the routing table message
static RoutingTable rt[MAX_NODES];						// to hold the actual routing table
static GatewayToNodeSerial_Packet gtn_pkt;				// to hold the ROUTE_CONFIG message
char tx_buf[SIZE_GATEWAYTONODESERIAL_PACKET] = {3};// to hold the routing table message
char buffer[128];

/*********************************** FUNCTION DEFINITIONS ****************************************/
void print_NgbList(NeighborList nl)
{
	int8_t i;
	
	printf("[%d -> ", nl.my_addr);
	for(i = 0; i < MAX_NGBS; i++)
	{
		if(nl.ngbs[i].addr != BCAST_ADDR)
			printf("%d, ",nl.ngbs[i].addr);
	}
	printf("]\n");
	
	return;
}
/***************************************************************************************************/	
uint8_t receiveFromSerial(uint8_t *p, int8_t len)
{
	int8_t ret;
	/*
	int c;								// to read a character from the serial port 
	uint8_t received = 0;				// holds the number of characters read in all 
	
	FILE *fromNode = (FILE*)fopen("/dev/ttyUSB0", "r");
	if(fromNode == NULL)
	{
		printf("Failed to open the serial port for reading\n");
		exit(1);
	}
	
	if(DEBUG_NG == 2)
		printf("Inside receiveFromSerial()\n");
		
	while(1)
	{
     // get a character to process 
	  c = fgetc(fromNode);
	  if(DEBUG_NG == 2)
	  		printf("%d ", c);
	
	  // handle bytestuffing if necessary
     switch(c)
	  {

        case END:
            if(received > 0)		// we received the packet
            {
					fclose(fromNode);
					return received;
				}
            else						// beginning synchronizing END
              break;

        case ESC:
             c = fgetc(fromNode);
             
             switch(c) 
				 {
               case ESC_END:
               	c = END;
                  break;
               case ESC_ESC:
                  c = ESC;
                  break;
             }

             
         default:
         	if(received < len)
            	p[received++] = (uint8_t)c;
            else
            	return received;		// no point collecting other characters
     } //end outer switch
  } // end infinite while
  */
  
  do
  {
  	 ret = slipstream_receive(p);
  	 printf("Received buffer length = %d\r\n", ret);
  	 sleep(2);
  }while(ret <= 0);
  
  if(ret != len)
  {
  	printf("Error in received buffer length in receiveFromSerial() = %d\r\n", ret);
  	sleep(2);
  	//exit(1);
  }
  
} // end receiveFromSerial()
/*************************************************************************************************/
void printBuffer(uint8_t *buf, int8_t len)
{
	printf("[");
	while(len > 0)
	{
		printf("%d ", *buf);
		buf++;
		len--;
	}
	printf("]\n\n");
	return;
}
/*************************************************************************************************/
uint8_t serial_pkt_type(NodeToGatewaySerial_Packet *pkt)
{
	/*if(DEBUG_NG == 2)
	{
		printf("Inside serial_pkt_type()\r\n");
		printf("Passed NToGSP = %d %d\r\n", pkt -> type, pkt -> length);
	}
	*/
	switch(pkt -> type)
	{
		case SERIAL_APPLICATION:
			return SERIAL_APPLICATION;
			
		case SERIAL_NGB_LIST:
			return SERIAL_NW_CONTROL;
			
	}
	return INVALID;									// unrecognized packet type 
}
/*************************************************************************************************/
uint8_t serial_nw_ctrl_type(NodeToGatewaySerial_Packet *pkt)
{
	switch(pkt -> type)
	{
		case SERIAL_NGB_LIST:
			return SERIAL_NGB_LIST;
	}
	
	while(1)
		printf("Bug detected in implementation of packet type\n");

	return INVALID;									// this should never happen
}
/*************************************************************************************************/
void process_serial_app_pkt(NodeToGatewaySerial_Packet *pkt)
{
	printf("Received an application layer packet\n");
	print_ntg_pkt(pkt);
	
	return;
}
/*************************************************************************************************/
int8_t process_serial_nw_ctrl_pkt(NodeToGatewaySerial_Packet *pkt)
{
	NeighborList nlist;
	Msg_NgbList mnlist;
	int8_t i;
	int8_t sendNow = FALSE;
		
	SensorNode *ptr;
	
	if(DEBUG_NG == 2)
	{
		printf("Inside process_serial_nw_ctrl_pkt()\r\n");
	}

	switch(serial_nw_ctrl_type(pkt))
	{
		case SERIAL_NGB_LIST:
		{
			
			// unpack the Msg_NgbList from the receive buffer						
			unpack_Msg_NgbList(&mnlist, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER);
			nlist = mnlist.nl;
			if(DEBUG_NG == 2)
			{
				printf("NGB_LIST message received from %d\r\n", nlist.my_addr);
				printf("%d -> ", nlist.my_addr);
				for(i = 0; i < MAX_NGBS; i++)
				{
					if(nlist.ngbs[i].addr != BCAST_ADDR)
					{
						printf("%d[%d], ", nlist.ngbs[i].addr, nlist.ngbs[i].lastReport);
					}
				}
				printf("\r\n");
			}
									 
			process_topology_desc(nlist);	// construct the current graph
			break;
		}
	}
	
	return 0;
}
/**********************************************************************************************/
void initialise_network_gateway()
{

	top_mgr.head = top_mgr.tail = NULL;
	top_mgr.count = 0;
	Map = NULL;
	edge = NULL;
	parent = NULL;
	cost = NULL;
	initialise_routing_table();
	if(endianness() == ERROR_ENDIAN)
	{
		printf("Error in computing endianness of the gateway\n");
		exit(1);
	}
	
	return;
}
/***********************************************************************************************/
void initialise_routing_table()
{
	int8_t i;

	for(i = 0; i < MAX_NODES; i++)
	{
		rt[i].dest = BCAST_ADDR;				  // indicates invalid entry 
		rt[i].nextHop = BCAST_ADDR;				  // initially no routes are known
		rt[i].cost = INFINITY;				  	  // no costs are known
	}
	return;
}
/***********************************************************************************************/
void process_topology_desc(NeighborList nl)
{
	// The below comments apply to a situation where node 1 reports its neighbor list
	// as 1 -- 3,2
	int8_t i;					// loop index 
	int8_t j;					// loop index 
	int8_t free_index;			// holds the index of next free element in the ngbs[]
	SensorNode *node;			// traversal pointer
	int8_t flag;				// used to assign a value to 'n' of SensorNode
	
	// first pass through the array attempts to add each neighbor 
	// at the right position in the vertical list
	//printf("Received NGBLIST: ");
	//print_NgbList(nl);
	
	// firstly clean up the data structures required for the Floyd Warshall
	// algorithm since we need to recreate them at the end of topology processing
	free_data_structures();
	
	// first create an entry in the vertical list for node '1'
	node = add_to_sensor_node_list(nl.my_addr);
	if(node == NULL)
	{
		printf("Not enough memory to add node %d in VL\r\n", nl.my_addr);
		exit(1);
	}
	
	// now begin to process the members in the neighbor list
	for(i = 0; i < MAX_NGBS; i++)
	{
		SensorNode *ptr;
		Neighbor n = nl.ngbs[i];	// 'n' contains information about '3'
		if(n.addr == BCAST_ADDR)	// ignore invalid entries in the neighbor list 	
			continue;
		
		// search for this neighbor in the vertical list 
		for(ptr = top_mgr.head; ptr != NULL; ptr = ptr -> next)
		{
			if(ptr -> addr == n.addr)	// found '3'
				break;
		}
				
		if(top_mgr.head == NULL)		// the vertical list is empty.  
		{		
		
			SensorNode *ptr1 = add_to_sensor_node_list(n.addr);	// add '3'
			if(ptr1 == NULL)	// not enough memory to hold another node
			{
				printf("Not enough memory to add node %d in VL\r\n", n.addr); 
				exit(1);
			}
			
			(ptr1 -> ngbs[0]).addr = nl.my_addr;		// add '1'
			(ptr1 -> ngbs[0]).rssi = n.rssi;		
			(ptr1 -> ngbs[0]).isNew = TRUE;
			(ptr1 -> ngbs[0]).lastReport = TIMEOUT_COUNTER + 1;
			ptr1 -> count++;	// increment number of neighbors recorded
		}		
		else 									// the VL contains at least one node 
		{
		
			if(ptr == NULL)				// '3' was not found in the VL
			{
				SensorNode *ptr2 = add_to_sensor_node_list(n.addr);	// add '3'
				if(ptr2 == NULL)
				{
					printf("Not enough memory to add node %d in VL\r\n", n.addr);
					exit(1);
				}
			
				(ptr2 -> ngbs[0]).addr = nl.my_addr;		// add '1'
				(ptr2 -> ngbs[0]).rssi = n.rssi;		
				(ptr2 -> ngbs[0]).isNew = TRUE;
				(ptr2 -> ngbs[0]).lastReport = TIMEOUT_COUNTER + 1;
				ptr2 -> count++;	// increment number of neighbors recorded
		
			}
			else								// '3' was found in the VL 
			{
				// go through the neighbor list of '3' to find '1'
				for(j = 0; j < MAX_NGBS; j++)
				{
					if( (ptr -> ngbs[j]).addr == BCAST_ADDR)	// entry at this index is free
						free_index = j;
						
					if( (ptr -> ngbs[j]).addr == nl.my_addr )	// found '1' in the neighbor list of 3 
					{
						(ptr -> ngbs[j]).rssi = n.rssi;			// update the state variables of '1'
						(ptr -> ngbs[j]).isNew = FALSE;
						(ptr -> ngbs[j]).lastReport = TIMEOUT_COUNTER + 1;
						break;											// break out of for loop	
					} // end if
				} // end for
					
				if(j == MAX_NGBS)	// '1' was not found in the neighbor list
				{
					(ptr -> ngbs[free_index]).addr = nl.my_addr;	// add 1 
					(ptr -> ngbs[free_index]).rssi = n.rssi;
					(ptr -> ngbs[free_index]).isNew = TRUE;
					(ptr -> ngbs[free_index]).lastReport = TIMEOUT_COUNTER + 1;
					ptr -> count++;		// increment number of neighbors recorded
				}	// end if 
			} // end else 
		} // end else 
	} // end for 
	
	node = top_mgr.head;
	while(node != NULL)
	{
		flag = FALSE;
		for(i = 0; i < MAX_NGBS; i++)
		{
			if( (node -> ngbs[i]).addr != BCAST_ADDR )	// entry at this index is valid 
			{
				(node -> ngbs[i]).lastReport--;
				if( (node -> ngbs[i]).lastReport == 0)		// time to remove this entry
				{
					(node -> ngbs[i]).addr = BCAST_ADDR;
					(node -> ngbs[i]).rssi = 0;
					(node -> ngbs[i]).isNew = FALSE;
					node -> count--;		// decrement number of neighbors recorded
				} // end if
				else
				{
					if(flag == FALSE)
					{
						node -> n = &(node -> ngbs[i]);
						flag = TRUE;
					} // end if
				
					else 
					{
						if(node -> n -> rssi < (node -> ngbs[i]).rssi)	// found a better neighbor
							node -> n = &(node -> ngbs[i]);
					} // end else
				} // end else   
			} // end if 
		} // end for 
		
		node = node -> next;
	} // end while 
	
	return;
}
/**********************************************************************************************/
SensorNode* create_sensor_node(uint16_t addr)
{
	int8_t i;	// loop index 
	SensorNode *node = (SensorNode*)malloc(sizeof(SensorNode));
	
	if(node == NULL)							// not enough memory to hold the node 
		return NULL;
				
	node -> addr = addr;						// fill in the contents of the node
	for(i = 0; i < MAX_NGBS; i++)
	{
		(node -> ngbs[i]).addr = BCAST_ADDR;
		(node -> ngbs[i]).rssi = 0;
		(node -> ngbs[i]).lastReport = 0;
		(node -> ngbs[i]).isNew = FALSE;
	}		
	node -> count = 0;
	node -> next = NULL;
	node -> n = NULL;
	
	return node;
}
/**********************************************************************************************/
SensorNode* add_to_sensor_node_list(uint16_t addr)
{
	SensorNode *ptr;								// pointer to traverse the sensor node list
	SensorNode *node;								// to point to a newly created node
		
	ptr = top_mgr.head;
	while(ptr != NULL)
	{
		if(ptr -> addr == addr)					// sensor node was recorded before 
			break;
			
		ptr = ptr -> next;
	}
	
	if(ptr == NULL)								// sensor node was not found 
	{
		node = create_sensor_node(addr);
		if(node == NULL)
			return NULL;
		
		if(top_mgr.head == NULL)			// queue is empty 
		{
			top_mgr.head = top_mgr.tail = node;
		}			
		else										// queue is non empty and the node was not found 
		{
			top_mgr.tail -> next = node;
			top_mgr.tail = node;
		}
		top_mgr.count++;
		return node;
	}
	
	// if the code reaches this point, it means the node was found in the list
	return ptr;
}
/**********************************************************************************************/
int8_t node_to_graph(uint16_t node_addr)
{
	int8_t i;
	for(i = 1; i <= top_mgr.count; i++)
	{
		if(Map[i] == node_addr)			// found the mapping
				return i;
	}
	return INVALID_ADDRESS;				// no mapping found
}
/***********************************************************************************************/
uint16_t graph_to_node(int8_t graph_addr)
{
	return Map[graph_addr];
}
/**********************************************************************************************/
void generate_routing_tables()
{
	SensorNode *node;
	int8_t i,j;		// loop indices
	int8_t row_index, column_index;
	
	if(DEBUG_NG == 2)
	{
			printf("Inside generate_routing_tables()\r\n");
	}
	
	// construct the necessary data structures
	Map = (uint16_t *)malloc( (top_mgr.count + 1) * sizeof(uint16_t) );
	if(Map == NULL)
	{
		printf("Not enough memory to construct the Map table\r\n");
		exit(1);
	}
	edge = (uint8_t **)malloc( (top_mgr.count + 1) * sizeof(uint8_t*) );
	if(edge == NULL)
	{
		printf("Not enough memory to construct the edge[] table\r\n");
		exit(1);
	}
	parent = (uint8_t **)malloc( (top_mgr.count + 1) * sizeof(uint8_t*) );
	if(parent == NULL)
	{
		printf("Not enough memory to construct the parent[] table\r\n");
		exit(1);
	}
	cost = (uint8_t **)malloc( (top_mgr.count + 1) * sizeof(uint8_t*) );
	if(cost == NULL)
	{
		printf("Not enough memory to construct the cost[] table\r\n");
		exit(1);
	}
	for(i = 1; i <= top_mgr.count; i++)
	{
		edge[i] = (uint8_t*)malloc( (top_mgr.count + 1) * sizeof(uint8_t) );
		if(edge[i] == NULL)
		{
			printf("Not enough memory to construct the edge[%d][] table\r\n", i);
			exit(1);
		}
		parent[i] = (uint8_t*)malloc( (top_mgr.count + 1) * sizeof(uint8_t) );
		if(parent[i] == NULL)
		{
			printf("Not enough memory to construct the parent[%d][] table\r\n", i);
			exit(1);
		}
		cost[i] = (uint8_t*)malloc( (top_mgr.count + 1) * sizeof(uint8_t) );
		if(cost[i] == NULL)
		{
			printf("Not enough memory to construct the cost[%d][] table\r\n", i);
			exit(1);
		}
	} // end for
	
	// prune the graph to include links with RSSI > RX_POWER_THRESHOLD. At the same time
	// map the node IDs to graph IDs
	for(node = top_mgr.head, j = 1; node != NULL; node = node -> next, j++)
	{
		for(i = 0; i < MAX_NGBS; i++)
		{
			if( (node -> ngbs[i]).rssi < RX_POWER_THRESHOLD && (node -> n -> addr != (node -> ngbs[i]).addr) )
			{
				(node -> ngbs[i]).addr = BCAST_ADDR;
				(node -> ngbs[i]).isNew = FALSE;
				(node -> ngbs[i]).lastReport = 0;
				(node -> ngbs[i]).rssi = 0;
				node -> count--;
			}
		}
		// map the node IDs to graph IDs
		Map[j] = node -> addr;
	} // end for
	
	if(j != top_mgr.count + 1)
	{
		while(1)
			printf("Bug detected in implementation of top_mgr.count\r\n");
	}
		
	// initialise the edge cost and path cost matrices
	for(i = 1; i <= top_mgr.count; i++)
		for(j = 1; j <= top_mgr.count; j++)
		{
			if(j == i)
			{
				edge[i][j] = 0;					// cost of node to itself is zero
				cost[i][j] = 0;
			}
			else
			{
				edge[i][j] = INFINITY;			// assume no links initially
				cost[i][j] = INFINITY;
			}
		}
			
	// initialise the parent matrix
	for(i = 1; i <= top_mgr.count; i++)
		for(j = 1; j <= top_mgr.count; j++)
			parent[i][j] = INVALID_ADDRESS;
			
		
	// fill up the edge cost and path cost matrices
	for(node = top_mgr.head; node != NULL; node = node -> next)
	{
		row_index = node_to_graph(node -> addr);
		if(row_index == INVALID_ADDRESS)		// debugging check
		{
			printf("Bug detected in creating Map table(row)\n");
		}
		for(i = 0; i < MAX_NGBS; i++)
		{
			if( (node -> ngbs[i]).addr != BCAST_ADDR )
			{
				column_index = node_to_graph( (node -> ngbs[i]).addr );
				if(column_index == INVALID_ADDRESS)
				{
					printf("Bug detected in creating Map table(column)\n");
				}
				edge[row_index][column_index] = 1;	// indicates presence of a link
				cost[row_index][column_index] = 1; // cost of one hop is 1
			}
		}
	}
	
	prepare_topology_desc_file();
	end_topology_file(topologyFile);
	fclose(topologyFile);
	generate_graph();
	
	if(DEBUG_NG == 2)
	{
		print_topology();
	}
		
	if(DEBUG_NG == 0)
	{
		printf("Map:\r\n");
		print_Map();
	}
	if(DEBUG_NG == 2)
	{
		printf("edge matrix:\r\n");
		print_edge_matrix();
	}
	if(DEBUG_NG == 2)
	{
		printf("Parent matrix\r\n");
		print_parent_matrix();
	}
	
	// inputs to floyd-warshall are prepared. Run the algorithm
	if(DEBUG_NG == 0)
	{
		printf("Beginning Floyd Warshall\r\n");
	}
	floyd_warshall(top_mgr.count, edge, parent, cost);
	if(DEBUG_NG == 0)
	{
		printf("cost matrix:\r\n");
		print_cost_matrix();
		
		printf("Parent matrix:\r\n");
		print_parent_matrix();
	}	
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
					parent[i][j] = k;
				}
	
	return;
}
/**********************************************************************************************/
void disseminate_routing_tables()
{
	int8_t ret;
	int8_t v;
	int8_t row, column, i, j;	// loop indices
	
	// cost and parent matrices are now prepared. Compute routing table for each
	// node
	
	// the 'row' selects a particular source node, the 'column' is all the destinations
	for(row = 1; row <= top_mgr.count; row++)
	{
		// initialise the routing table again
		initialise_routing_table();
		
		for(column = 1, i = 0; column <= top_mgr.count; column++, i++)
		{
			rt[i].dest = graph_to_node(column);
			rt[i].cost = cost[row][column];
			if(rt[i].cost == INFINITY)
				rt[i].nextHop = INVALID_ADDRESS;
			else
				rt[i].nextHop = graph_to_node(get_parent(row,column));
		}
		
		if(DEBUG_NG == 0)
		{
			printf("Routing table for %d:\r\n", graph_to_node(row));
			for(j = 0; j < top_mgr.count; j++)
			{
				printf("%d -> %d [nh = ", graph_to_node(row), rt[j].dest);
				if(rt[j].nextHop == INVALID_ADDRESS)
					printf("INV, ");
				else
					printf("%d, ", rt[j].nextHop);
					
				if(rt[j].cost != INFINITY)
					printf("dist = %d]\r\n", rt[j].cost);
				else
					printf("dist = INF]\r\n");
			}				
			printf("\r\n");
		}
		// At this stage, the routing table is prepared for node with graph ID 'row'
		// construct a ROUTE_CONFIG message
		build_Msg_RoutingTable(&mrt, graph_to_node(row), rt);
		pack_Msg_RoutingTable(gtn_pkt.data, &mrt);
		gtn_pkt.type = SERIAL_ROUTE_CONFIG;
		gtn_pkt.length = SIZE_MSG_ROUTING_TABLE;
		pack_GatewayToNodeSerial_Packet_header(tx_buf, &gtn_pkt);
		memcpy(tx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, gtn_pkt.data, MAX_GATEWAY_PAYLOAD);
		
		printf("Sending data to the firefly...\r\n");
		// send it to the gateway node
		if( slipstream_send(tx_buf, SIZE_GATEWAYTONODESERIAL_PACKET) == 0)
		{
			while(1)
				printf("Error in sending data to the gateway at [%s,%d]\r\n", GATEWAY_ADDRESS, GATEWAY_PORT);
			exit(1);
		}
		printf("Data sent successfully\r\n");
		sleep(2);	// sleep for some time
		
	}	
	return;
}
/**********************************************************************************************/
void build_Msg_RoutingTable(Msg_RoutingTable *mrtbl, uint16_t addr, RoutingTable rtbl[])
{
	int8_t i;
	
	mrtbl -> dg = BCAST_ADDR;			// as of now dg is unknown
	mrtbl -> node = addr;				// whose routing table is this?
	for(i = 0; i < MAX_NODES; i++)
	{
		mrtbl -> rt[i] = rtbl[i];
	}
	
	return;
}
/**********************************************************************************************/
int8_t get_parent(int8_t from, int8_t to)
{
	if(parent[from][to] == INVALID_ADDRESS)
		return to;
	
	return get_parent(from, parent[from][to]);
}	
/**********************************************************************************************/	
int main()
{
	int64_t start_time, end_time;				// to mark the start and end of the collection period 
	NodeToGatewaySerial_Packet ntg_pkt;			// to hold a packet received from the attached node
	GatewayToNodeSerial_Packet gtn_pkt;
	int8_t flag;								// flag to mark end of data collection
	int8_t ret;
	char gw_addr[16];							// temporary variable 
	int8_t cnt = 0;
	int8_t i;
	
	initialise_network_gateway();					// initialise the gateway 
	
	// make an UDP socket to connect to the SLIPStream server
	if( slipstream_open("127.0.0.1", 4000, 1) == 0 )
	{
		printf("Error in connecting to the gateway server at [%s,%d]\r\n", strcpy(gw_addr, GATEWAY_ADDRESS), GATEWAY_PORT);
		exit(1);
	}
	
	// construct a dummy packet
	//sprintf(gtn_pkt.data, "Startup message\r\n");
	printf("tx_buf = [");
	for(i = 0; i < SIZE_GATEWAYTONODESERIAL_PACKET; i++)
	{	tx_buf[i] = 5;
		printf("%d ", tx_buf[i]);
	}
	printf("]\r\n");
	
	gtn_pkt.data[0] = 1;
	gtn_pkt.data[1] = 2;
	gtn_pkt.type = SERIAL_APPLICATION;
	gtn_pkt.length = 2;
	//gtn_pkt.length = strlen(gtn_pkt.data);
	//pack_GatewayToNodeSerial_Packet_header(tx_buf, &gtn_pkt);
	//memcpy(tx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, gtn_pkt.data, MAX_GATEWAY_PAYLOAD);
	//printf("Dummy packet = ");
	printf("SIZE_GATEWAYTONODESERIAL_PACKET = %d\r\n", SIZE_GATEWAYTONODESERIAL_PACKET);
	//print_gtn_pkt(&gtn_pkt);
	// send the dummy packet
	tx_buf[0] = 1;
	tx_buf[1] = 2;
	//sprintf (buffer, "This is a sample slip string: Count %d\n", cnt);

	do{
	ret = slipstream_send(tx_buf, (int)SIZE_GATEWAYTONODESERIAL_PACKET); // send an inital message to the server
	//ret = slipstream_send(buffer, strlen(buffer));
	if(ret == 0)
	{
		printf("Error in sending message to firefly\r\n");
		exit(1);
	}
	printf("Sent message to the firefly %d\r\n", cnt);
	cnt = (cnt + 1) % 128;
	sleep(1);
	}while(1);
	
	while(1)												// listen from the attached node forever 
	{
		if(DEBUG_NG == 2)
		{
			printf("Within main while loop\r\n");
		}
		// reset the timers
		start_time = time(NULL);
		end_time = start_time;
		
		// assume no new link has been detected		
		flag = FALSE;
		
		// open the topology file
		topologyFile = (FILE*)fopen("SensorTopology.dot", "w");
		if(topologyFile == NULL)
		{
			printf("Topology generation file could not be opened\n");
			exit(1);
		}
		
		if(DEBUG_NG == 2)
			printf("Topology collection started\n");
		
		// begin topology generation				
		begin_topology_file(topologyFile);
		
		// collect data for COLLECTION_PERIOD or till a new link is detected 		
		while( (end_time - start_time < COLLECTION_PERIOD) && (flag == FALSE) )	 
		{
			if(DEBUG_NG == 2)
			{
				printf("Within collection loop\r\n");
			}
			ret = receiveFromSerial(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
			
			if(DEBUG_NG == 2)
			{
				printf("\r\nNo of characters received = %d\r\n", ret);
			}
		
			if(DEBUG_NG >= 0)
			{
				//int8_t len = SIZE_NODETOGATEWAYSERIAL_PACKET;
				//int8_t *buf = rx_buf;
				printf("Received rx_buf = ");
				printBuffer(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
				/*while(len > 0)
				{
					printf("%d ", *buf);
					buf++;
					len--;
				}
				printf("\n\n");*/
			}
			// unpack the packet
			unpack_NodeToGatewaySerial_Packet_header(&ntg_pkt, rx_buf);
									
			// what kind of packet is it 
			switch(serial_pkt_type(&ntg_pkt))
			{
				case SERIAL_APPLICATION:
					process_serial_app_pkt(&ntg_pkt);
					break;
					
				case SERIAL_NW_CONTROL:
					if(process_serial_nw_ctrl_pkt(&ntg_pkt) == 1)	// should end the topology description file 
						flag = TRUE; 
					break;
					
				case INVALID:
					// drop the packet and go receive another one 
					printf("NG: Invalid packet type received = %d\n", ntg_pkt.type);	 
					break;
			}
			
			// update the end time 
			end_time = time(NULL);
			
		} // end while(COLLECTION_PERIOD)
		
		generate_routing_tables();
		disseminate_routing_tables();
				
	} // end while(1) 
	 return 0;
} // end main()
/******************************************************************************/
void prepare_topology_desc_file()
{
	SensorNode *ptr;
	int8_t i;
	
	// prepare the topology description file
	ptr = top_mgr.head;
	while(ptr != NULL)
	{
		for(i = 0; i < MAX_NGBS; i++)
		{
			if( (ptr -> ngbs[i]).addr != BCAST_ADDR )
			{
				add_link_to_topology_file(topologyFile, ptr -> addr, (ptr -> ngbs[i]).addr, (ptr -> ngbs[i]).rssi);
			}
		}
		ptr = ptr -> next;
	} // end while 
	return;
}
/*****************************************************************************/
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
/*****************************************************************************/
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
		{
			if(edge[i][j] == INFINITY)
				printf("INF\t ");
			else
				printf("%d\t ", edge[i][j]);
		}			
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
			if(parent[i][j] == INVALID_ADDRESS)
				printf("INV\t ");
			else
				printf("%d\t ", graph_to_node(parent[i][j]));
		}			
		printf("\r\n");
	}
	return;
}	
/********************************************************************************/
void print_ntg_pkt(NodeToGatewaySerial_Packet *pkt)
{
	int8_t i;
	
	print_ntg_pkt_header(pkt);
	for(i = 0; i < pkt -> length; i++)
		printf("%d ", pkt -> data[i]);
	printf("\r\n");
}
/*********************************************************************************/

void print_ntg_pkt_header(NodeToGatewaySerial_Packet *pkt)
{
	printf("[%d %d] ", pkt -> type, pkt -> length);
	return;
}
/***********************************************************************************/
void print_topology()
{
	SensorNode *ptr;
	int8_t i;
	
	printf("Current topology list is:\r\n");
	for(ptr = top_mgr.head; ptr != NULL; ptr = ptr -> next)
	{
		printf("%d -> ", ptr -> addr);
		for(i = 0; i < MAX_NGBS; i++)
		{
			if( (ptr -> ngbs[i]).addr != BCAST_ADDR)	// entry is valid
			{
				printf("%d[%d], ", (ptr -> ngbs[i]).addr, (ptr -> ngbs[i]).lastReport);
			}
		}
		printf("\r\n");
	}
	
	return;
}
/************************************************************************************/
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
/************************************************************************************/
void print_gtn_pkt(GatewayToNodeSerial_Packet *pkt)
{
	int8_t i;
	print_gtn_pkt_header(pkt);
	
	for(i = 0; i < pkt -> length; i++)
	{
		printf("%d ", pkt -> data[i]);
	}
	printf("\r\n");

	return;
}

/********************************************************************************/
void print_gtn_pkt_header(GatewayToNodeSerial_Packet *pkt)
{
	printf("[");
	printf("%d %d]", pkt -> type, pkt -> length);
	
	return;
}
	
/**********************************************************************************/			
		
	
	
