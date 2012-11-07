/* This file implements the network gateway for the sensor network */
/* Authors:
   Aditya Bhave
*/

/*********************************** INCLUDE FILES **************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "NetworkGateway.h"

/*********************************** External variables and functions ******************************/
// From TopologyGeneration.c 
extern void add_link_to_topology_file(FILE *fp, uint16_t f, uint16_t t, int8_t rssi);
extern void begin_topology_file(FILE *fp);
extern void end_topology_file(FILE *fp);
extern void generate_graph();

// From NGPack.c
extern void unpack_Msg_NgbList(Msg_NgbList *, uint8_t *);
extern void unpack_NodeToGatewaySerial_Packet_header(NodeToGatewaySerial_Packet *, uint8_t *);
extern int8_t endianness();
extern void pack_Msg_RoutingTable(uint8_t *, Msg_RoutingTable*);

/*********************************** Global data structures **************************************/
static FILE *topologyFile;								// pointer to file that stores topology information 

static TopologyManager top_mgr;							// to manage information about the network
static uint16_t *Map;									// to hold a mapping between node/graph IDs
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

HashTableSNLEntry htsnl[SIZE_HT_SNL];					// hash table to manage the sensor node list
HashTableVLEntry htvl[SIZE_HT_VL];						// hash table to manage the vertical list

/*********************************** FUNCTION DEFINITIONS ****************************************/
int8_t receiveFromSerial(uint8_t *p, uint8_t len, int timeout)
{
	int8_t nread;						// to hold number of characters read at a time from the serial port
	uint8_t bytesRemaining;
	uint8_t index;												// loop variable
	//int8_t i, j;												// loop variables
	//uint8_t temp_buffer[SIZE_NODETOGATEWAYSERIAL_PACKET];		// temporary buffer
	int64_t st, et;												// needed to maintain timeout
	
 
	if(timeout > 0)	// the caller wants to set a timeout
  	{
  		st = time(NULL);
  		et = st;  	
  	}
  
   	// initalise the loop variables
  	bytesRemaining = len;
  	index = 0;
 	do
  	{
  		//printf("NG: RX..\r\n");
  		nread = slipstream_receive(p + index, bytesRemaining);
  		if(nread == -1)
  		{
  			//continue;
  			//perror("NG: receiveFromSerial(): Error returned by slipstream_receive()");
  			//printf("\r\n");
  			//sleep(1);
  			return 0;					// did not read any bytes
  		}
  		index += nread;					// increment number of characters successfully read
  		bytesRemaining -= nread;		// successfully read 'nread' bytes
  		
  		if(timeout > 0)
  		{
  			et = time(NULL);
  			if(et - st >= timeout)			 // waited long enough
  				return len - bytesRemaining; // return whatever number of bytes were read
  		} // end if  
    }while(bytesRemaining > 0);
  
	return len;
	 
  /*
  
  do
  {
  	index = 0;	// reset the variable before doing the read operation
  	 
  	do
  	{
  		printf("NG: RX ...\r\n");
  		//printf("NG: receiveFromSerial(): Reading from the firefly....\r\n");
  		nread = slipstream_receive(temp_buffer, );
  		if(nread == -1)	// error
  		{
  			printf("NG: receiveFromSerial(): Error returned by slipstream_receive()\r\n");
  			return 0;
  		}
  		for(i = index, j = 0; j < nread; i++, j++)	// copy the contents of the temp buffer 
			p[i] = temp_buffer[j];  
	
		index += nread;								// increment number of characters successfully read
		
		et = time(NULL);
		if(et - st >= timeout)						// waited long enough
			return 0;
	
    }while(index < len);		// read exactly 'len' number of characters from the serial port
  
  	if(index == len)			// 'index' has to equal 'len' for successful reception
   		return len;
   	
   	printf("NG: receiveFromSerial(): Error in received buffer length = %d\r\n", index);
   	 
  }while(1);
  */
    
} // end receiveFromSerial()
/*************************************************************************************************/
uint8_t serial_pkt_type(NodeToGatewaySerial_Packet *pkt)
{
	switch(pkt -> type)
	{
		case SERIAL_APPLICATION:					// application layer packet
			return SERIAL_APPLICATION;
			
		case SERIAL_NGB_LIST:						// network control packet
		case SERIAL_ROUTE_REQUEST:
		case SERIAL_NODE_INFO:
			return SERIAL_NW_CONTROL;
			
	}
	return INVALID;									// unrecognized packet type 
}
/*************************************************************************************************/
void process_serial_app_pkt(NodeToGatewaySerial_Packet *pkt)
{
	printf("NG: process_serial_app_pkt(): Received an application layer packet\n");
	print_ntg_pkt(pkt);
	
	return;
}
/*************************************************************************************************/
int8_t process_serial_nw_ctrl_pkt(NodeToGatewaySerial_Packet *pkt)
{
	NeighborList nlist;				// variables to hold different type of NW_CTRL packet data
	Msg_NgbList mnlist;
	Msg_NodeInfo mnode;
	Msg_RouteRequest mrrq;
	Msg_RouteReply mrr;
	
	int8_t i;						// loop variable
	int8_t retValue = FALSE;		// to store the return value of this function
		
	switch(pkt -> type)
	{
		case SERIAL_NGB_LIST:
		{
			// unpack the Msg_NgbList from the receive buffer						
			unpack_Msg_NgbList(&mnlist, pkt -> data);
			nlist = mnlist.nl;
			if(DEBUG_NG == 0)
			{
				printf("NG: NGB_LIST message received from %d\r\n", nlist.addr);
				//printf("NG: process_serial_nw_ctrl_pkt(): NGB_LIST message received from %d\r\n", nlist.addr);
				printf("%d -> ", nlist.addr);
				for(i = 0; i < MAX_NGBS; i++)
				{
					if(nlist.ngbs[i].addr != INVALID_ADDR)	// valid entry
					{
						printf("%d[%d], ", nlist.ngbs[i].addr, nlist.ngbs[i].rssi);
					}
				}
				printf("\r\n");
			}
									 
			retValue = process_topology_desc(nlist);	// construct the current graph
			if(retValue == TRUE)						// major change in topology
				generate_routing_tables();
			break;
		}
		
		/*
		case SERIAL_ROUTE_REQUEST:
		{
				unpack_Msg_RouteRequest(&mrrq, pkt -> data);
				process_Msg_RouteRequest(&mrr, mrrq.src, mrrq.dest);
				sendOverSerial(&mrr, SERIAL_MSG_ROUTE_REPLY);
				break;
		}
		*/
		
		case SERIAL_NODE_INFO:
			// unpack the Msg_NodeInfo from the receive buffer						
			unpack_Msg_NodeInfo(&mnode, pkt -> data);
			if(DEBUG_NG == 0)
			{
				printf("NG: NODE_INFO message received from %u\r\n", mnode.addr);
				process_Msg_NodeInfo(&mnode);
				print_NodeStateQueue(&(mnode.nsq));
			}
									 
			break;
			
		default:	// invalid packet type
			;		// do nothing. Discard the packet
		
	} // end switch
	
	return retValue;
}
/**********************************************************************************************/
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
/***********************************************************************************************/
void initialise_Msg_NwInfoAcquired(Msg_NwInfoAcquired *m)
{
	int8_t i;
	
	m -> dg = INVALID_ADDR;
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
		m -> addrs[i] = INVALID_ADDR;
	return;
}
/***********************************************************************************************/
void initialise_Msg_SendNwInfo(Msg_SendNwInfo *m)
{
	int8_t i;
	
	m -> dg = INVALID_ADDR;
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
		m -> addrs[i] = INVALID_ADDR;
	return;
}
/***********************************************************************************************/
void initialise_Msg_SendNodeInfo(Msg_SendNodeInfo *m)
{
	int8_t i;
	
	m -> dg = INVALID_ADDR;
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
		m -> addrs[i] = INVALID_ADDR;
	return;
}
/***********************************************************************************************/
void initialise_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *m)
{
	int8_t i;
	
	m -> dg = INVALID_ADDR;
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
		m -> addrs[i] = INVALID_ADDR;
	return;
}
/***********************************************************************************************/
void refresh_VL(uint16_t addr)	// C
{
	NeighborGateway *ngptr;
	
	if(DEBUG_NG >= 1)
		printf("NG: Entering refresh_VL:%d\r\n", addr);
	
	ngptr = htvl[addr].head;
	while(ngptr != NULL)
	{
		ngptr -> refreshFlag = FALSE;
		ngptr = ngptr -> down;
	}
	
	if(DEBUG_NG >= 1)
		printf("NG: Leaving refresh_VL\r\n");
	return;
}
/***********************************************************************************************/
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
/***********************************************************************************************/
void rank_ngbs(SensorNode *snptr)
{
	NeighborGateway *ngptr;
	
	if(DEBUG_NG >= 1)
		printf("NG: Entering rank_ngbs()\r\n");
	
	// handle the special cases first
	if(snptr -> count == 0)		// no Neighbors
		snptr -> nbest = snptr -> nsbest = snptr -> nworst = NULL;
	else if(snptr -> count == 1) // only one neighbor
			snptr -> nbest = snptr -> nsbest = snptr -> nworst = snptr -> head;
		else if(snptr -> count == 2) // two neighbors
			  	if( (snptr -> head -> rssi) >= (snptr -> tail -> rssi) )	// first node is better than the second
			 	{
			 		snptr -> nbest = snptr -> head;
			 		snptr -> nsbest = snptr -> nworst = snptr -> tail;
			 	}
			 	else	// second node is better than the first
			 	{
			 		snptr -> nbest = snptr -> tail;
			 		snptr -> nsbest = snptr -> nworst = snptr -> head;
			 	}
			 else	// general case: There are at least three neighbors.
			 {
			 	int8_t best = -100;		// arbitarily small value
			 	int8_t worst = 100;		// arbitarily large value
			 	
			 	for(ngptr = snptr -> head; ngptr != NULL; ngptr = ngptr -> next)
			 	{
			 		if(ngptr -> rssi < worst)
			 		{
			 			snptr -> nworst = ngptr;		// assign the 'worst' neighbor
			 			worst = ngptr -> rssi;			// update the 'worst' value
			 		}
			 		if(ngptr -> rssi > best)
			 		{
			 			snptr -> nbest = ngptr;			// assign the 'best' neighbor
			 			best = ngptr -> rssi;			// update the 'best' value
			 		}
			 	} //end for	
			 
			 	best = -100;	// reinitialise the 'best' value
			 	// now 'best' and 'worst' neighbors are assigned
			 	for(ngptr = snptr -> head; ngptr != NULL; ngptr = ngptr -> next)
			 		if( (ngptr -> rssi > best) && (snptr -> nbest != ngptr) )
			 		{
			 			snptr -> nsbest = ngptr;
			 			best = ngptr -> rssi;
			 		}// end if
			 } // end else			
						 	
	if(DEBUG_NG >= 1)
		printf("NG: Leaving rank_ngbs()\r\n");
	
	return;
}
/***********************************************************************************************/
void remove_node_VL(uint16_t addr)
{
	NeighborGateway *first, *second;
	NeighborGateway *ptr;
	int8_t count;
	
	if(DEBUG_NG >= 1)
		printf("NG: Entered remove_node_VL() with %d %d\r\n", addr, htvl[addr].count);

	first = NULL;
	second = htvl[addr].head;
	
	while(second != NULL)
	{
		first = second;
		second = second -> down;
		
		if(first -> refreshFlag == FALSE)			// this node needs to be removed
		{
			if(DEBUG_NG == 0)
				printf("NG: remove_node_VL(): Need to remove %d\r\n", first -> addr);
			
			// go back to head of the NL to find the corresponding SNL entry
			ptr = first;
			//count = 0;
			printf("NG: Following the prev pointers\r\n");
			while(ptr -> prev != NULL)
			{
				printf("%u ", ptr -> addr);
				/*
				count++;
				if(count > 5)	// debugging
				{
					print_SNL();
					print_VL();
					exit(1);	
				}
				*/			
				ptr = ptr -> prev;
			}
			remove_node_NL(ptr -> toHead, first);		// remove this node from the NL and VL
		} // end if
	} // end while
	
	if(DEBUG_NG >= 1)
		printf("NG: Leaving remove_node_VL() with count = %d\r\n", htvl[addr].count);

	return;
}
/***********************************************************************************************/
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
void process_Msg_NodeInfo(Msg_NodeInfo *mni)
{
	SensorNode *snptr;
	int8_t i;			// loop index
	
	// check to see if the node exists in the SNL
	snptr = htsnl[mni -> addr].ptr;
	if(snptr == NULL)	
	{
		printf("NG: process_Msg_NodeInfo(): Adding node to SNL\r\n");
		add_node_to_SNL(mni -> addr);
	}
	
	for(i = mni -> nsq.front; ; i = (i + 1) % MAX_STATE_VALUES)	// iterate through NodeStateQueue
	{
		if(i == mni -> nsq.rear)	// reached end of circular queue
			break;
			
		// update the state queue in the SNL
		add_to_node_state_gateway( &(snptr -> nsqg), &(mni -> nsq.ns[i]) );
	}
	writeNSToFile(mni);		// write the new NodeState values to a file
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
	
	if(DEBUG_NG == 1)
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
	node -> head = node -> tail = NULL;			// list of NeighborGateway nodes	
	node -> count = 0;							// number of NeighborGateway nodes
	node -> next = NULL;						// SNL
	node -> nbest = NULL;
	node -> nsbest = NULL;
	node -> nworst = NULL;
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
		if(DEBUG_NG == 2)
			printf("TDS: Leaving create_node_NL()\r\n");
		return NULL;
	}
	
	// fill up the members of the node
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
	if(DEBUG_NG >= 1)
		printf("NG: Entered set_ngb_gw()\r\n");
		
	ngw -> rssi = n -> rssi;				// assign the control variables
	ngw -> lastReport = n -> lastReport;
	ngw -> refreshFlag = TRUE;
	
	if(DEBUG_NG >= 1)
		printf("NG: Leaving set_ngb_gw()\r\n");
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
	NeighborGateway *prev, *ptr;
	
	if(DEBUG_NG == 1)
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
			snptr -> tail -> next = ngptr;	// add the node to the tail of the list
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
	
	rank_ngbs(snptr);					// set the 'best', 'sbest' and 'worst' neighbors
	if(DEBUG_NG == 1)
		printf("TDS: Leaving add_node_to_NL()\r\n");
	return ngptr;
}	
/***********************************************************************************************/
void remove_node_NL(SensorNode *snptr, NeighborGateway *ngptr)	// C
{
	if(DEBUG_NG == 1)
		printf("NG: Entering remove_node_NL(): %d %d\r\n", snptr -> addr, ngptr -> addr);
	
	// first remove the node from the NL
	if(ngptr -> next == NULL)	// the last node is to be removed
	{
		if(ngptr -> prev == NULL) // this is also the first (and only) node in the NL
			snptr -> head = snptr -> tail = NULL;
		else					 // at least one node exists to the left of this one
		{
			ngptr -> prev -> next = NULL;
			snptr -> tail = ngptr -> prev;	// move the tail pointer
		}
	}
	else if(ngptr -> prev == NULL)	// this is the first node to be removed
		 {
		 	if(ngptr -> next == NULL)	// this is also the last (and only) node in the NL
		 		snptr -> head = snptr -> tail = NULL;
			else					// at least one node exists to the right of this one
		 	{
		 		ngptr -> next -> toHead = snptr;	// assing the special back pointer
		 		ngptr -> next -> prev = NULL;		// assing the prev pointer 
		 		snptr -> head = ngptr -> next;		// move the head pointer
		 	}
		 }
		 else			// a 'middle' node is to be removed
		 {
		 	ngptr -> prev -> next = ngptr -> next;
		 	ngptr -> next -> prev = ngptr -> prev;
		 }
	snptr -> count--;			// decrement number of neighbors in the NL		 

	/************************ Now remove the node from the VL *************************/
	if(ngptr -> down == NULL)	// the last node is to be removed
	{
		if(ngptr -> up == NULL) // this is also the first (and only) node in the VL
			htvl[ngptr -> addr].head = htvl[ngptr -> addr].tail = NULL;
		else					 // at least one node exists above this one
		{
			ngptr -> up -> down = NULL;
			htvl[ngptr -> addr].tail = ngptr -> up;	// move the tail pointer
		}
	}
	else if(ngptr -> up == NULL)	// this is the first node to be removed
		 {
		 	if(ngptr -> down == NULL)	// this is also the last (and only) node in the VL
		 		htvl[ngptr -> addr].head = htvl[ngptr -> addr].tail = NULL;
			else					// at least one node exists below this one
		 	{
		 		ngptr -> down -> up = NULL;					// assign the up pointer						
		 		htvl[ngptr -> addr].head = ngptr -> down;	// move the head pointer
		 	}
		 }
		 else			// a 'middle' node is to be removed
		 {
		 	ngptr -> up -> down = ngptr -> down;
		 	ngptr -> down -> up = ngptr -> up;
		 }
	htvl[ngptr -> addr].count--;	// decrement number of nodes in the VL
	
	if(snptr -> nworst == ngptr)	// update the neighbor statistics
		snptr -> nworst = NULL;
	if(snptr -> nsbest = ngptr)
		snptr -> nsbest = NULL;
	if(snptr -> nbest == ngptr)
	 	snptr -> nbest = NULL;
		 	
	rank_ngbs(snptr);				// update neighbor ranks of this SN	
	
	free(ngptr);					// finally free memory occupied by this node
	
	if(DEBUG_NG >= 1)
		printf("NG: Leaving remove_node_NL()\r\n");
	
	return;
}
/**********************************************************************************************/
void add_node_to_VL(NeighborGateway *ngptr)		// C
{
	if(DEBUG_NG >= 1)
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
	
	if(DEBUG_NG >= 1)
		printf("TDS: Leaving add_node_to_VL()\r\n");
	return;
}
/***********************************************************************************************/
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
	SensorNode *snptr;						// iterator variable
	int8_t i,j;								// loop indices
	int8_t row, column;						// used to index the various matrices
	RoutingTableGateway rt;					// to hold the routing table for one node
	SensorNode *ptr;						// to hold return types of functions
	NeighborGateway *ngptr;					// iterator variable
	char ch;	
	
	if(DEBUG_NG >= 0)
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
	if(DEBUG_NG >= 0)
		printf("Finished creating matrices\r\n");
	
	// prune the graph to include links with RSSI > RX_POWER_THRESHOLD. At the same time
	// map the node IDs to graph IDs
	for(snptr = top_mgr.head, j = 1; snptr != NULL; snptr = snptr -> next, j++)
	{		
		for(ngptr = snptr -> head; ngptr != NULL; )		// note this
		{
			if( (ngptr -> rssi < RX_POWER_THRESHOLD) && (ngptr != snptr -> nbest) && (ngptr != snptr -> nsbest) )
			{
				NeighborGateway *ngwptr;
				
				ngwptr = ngptr;						// let "ngwptr" point to node to be removed
				ngptr = ngptr -> next;				// move ngptr one place to the right
				remove_node_NL(snptr, ngwptr);		// remove this neighbor
			}
			else
				ngptr = ngptr -> next;				// move one node to the right
		}
		Map[j] = snptr -> addr;
	} // end for
	
	if(j != top_mgr.count + 1)	// sanity check for debugging
		while(1)
			printf("NG: generate_routing_tables(): Bug detected in implementation of top_mgr.count: ERROR\r\n");
			
	if(DEBUG_NG >= 0)
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
	for(snptr = top_mgr.head; snptr != NULL; snptr = snptr -> next)
	{
		row = node_to_graph(snptr -> addr);
		if(row == INVALID_ADDR)		// debugging check
		{
			printf("NG: generate_routing_tables(): Bug detected in creating Map table(row)\n");
		}
		for(ngptr = snptr -> head; ngptr != NULL; ngptr = ngptr -> next)
		{
			column = node_to_graph(ngptr -> addr);
			if(column == INVALID_ADDR)
			{
				printf("NG: generate_routing_tables(): Bug detected in creating Map table(column)\n");
			}
			edge[row][column] = 1;	 // indicates presence of a link
			parent[row][column] = row;
			if(ngptr -> rssi < RX_POWER_THRESHOLD)	
				cost[row][column] = RX_POWER_THRESHOLD - ngptr -> rssi;		// cost of a weak neighbor
			else
				cost[row][column] = 1;						// cost of a strong neighbor is 1
			
		} // end for
	} // end for
	
	if(DEBUG_NG >= 1)
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
	if(DEBUG_NG >= 1)
	{
		printf("Cost matrix\r\n");
		print_cost_matrix();
	}
	if(DEBUG_NG == 1)
	{
		printf("Parent matrix\r\n");
		print_parent_matrix();
	}
	
	//printf("Press any key to continue: ");
	//getchar();
	// inputs to floyd-warshall are prepared. Run the algorithm
	if(DEBUG_NG >= 1)
		printf("\nBeginning Floyd Warshall\r\n");
	
	floyd_warshall(top_mgr.count, edge, parent, cost);
	
	if(DEBUG_NG >= 1)
	{
		printf("cost matrix:\r\n");
		print_cost_matrix();
		
		printf("Parent matrix:\r\n");
		print_parent_matrix();
	}	
	
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
						while(1){ printf("NG: generate_routing_tables(): Bug2 detected in routing table construction: ERROR %d %d %u %u\r\n", row, column, graph_to_node(row), graph_to_node(column)); exit(1); }
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
		if(DEBUG_NG >= 1)
			print_RoutingTableGateway(graph_to_node(row));
		
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
/**********************************************************************************************/
void build_Msg_RoutingTable(Msg_RoutingTable *mrtbl, uint16_t addr, RoutingTable *rtbl)
{
	int8_t i;
	
	mrtbl -> dg = BCAST_ADDR;			// as of now dg is unknown
	mrtbl -> addr = addr;				// whose routing table is this?
	for(i = 0; i < NUM_ROUTING_TABLE_ENTRIES; i++)
	{
		(mrtbl -> rt).rte[i] = (rtbl -> rte)[i];
	}
	
	return;
}
/***********************************************************************************************/
void build_Msg_SendNwInfo(Msg_SendNwInfo *m, uint16_t addrs[], int8_t size)
{
	int8_t i;
	SensorNode *snptr;
	
	initialise_Msg_SendNwInfo(m);
	
	if(addrs[0] == BCAST_ADDR)		// each nodes's topology information is needed
		m -> addrs[0] = BCAST_ADDR;
	else
	{
		for(i = 0; i < size; i++)
		{
			m -> addrs[i] = addrs[i];
			printf("Inserted address %d\n", m -> addrs[i]);
		}
		/*for(snptr = top_mgr.head, i = 0; snptr != NULL; snptr = snptr -> next, i++)
		{
			m -> addrs[i] = snptr -> addr;
			printf("Inserted address %d\n", m -> addrs[i]);
		}
		*/
	}
	
	return;
}	
/**********************************************************************************************/
void build_Msg_SendNodeInfo(Msg_SendNodeInfo *m, uint16_t addrs[], int8_t size)
{
	int8_t i;
	
	initialise_Msg_SendNodeInfo(m);
	
	if(addrs[0] == BCAST_ADDR)		// each nodes's topology information is needed
		m -> addrs[0] = BCAST_ADDR;
	else
	{
		for(i = 0; i < size; i++)
		{
			m -> addrs[i] = addrs[i];
			printf("Inserted address %d\n", m -> addrs[i]);
		}
	}
	
	return;
}	
/**********************************************************************************************/
void build_Msg_NwInfoAcquired(Msg_NwInfoAcquired *m)
{
	int8_t i;
	SensorNode *snptr;
	
	//printf("NG: build_Msg_NwInfoAcquired(): Building this packet\n");
	initialise_Msg_NwInfoAcquired(m);
	for(snptr = top_mgr.head, i = 0; snptr != NULL; snptr = snptr -> next, i++)
	{
		m -> addrs[i] = snptr -> addr;
		//printf("Inserted address %d\n", m -> addrs[i]);
	}
	
	return;
}	
/**********************************************************************************************/
void build_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *m)
{
	int8_t i;
	SensorNode *snptr;
	
	//printf("NG: build_Msg_NwInfoAcquired(): Building this packet\n");
	initialise_Msg_NodeInfoAcquired(m);
	for(snptr = top_mgr.head, i = 0; snptr != NULL; snptr = snptr -> next, i++)
	{
		m -> addrs[i] = snptr -> addr;
		//printf("Inserted address %d\n", m -> addrs[i]);
	}
	
	return;
}
/**********************************************************************************************/
void send_dummy_pkt()
{
	int8_t ret;					// to hold the return value of function calls
	static int8_t count1 = 0;	// temporary variable for debugging purposes
	char tx_buf[SIZE_GATEWAYTONODESERIAL_PACKET];	// transmit buffer to send data to attached Firefly
	GatewayToNodeSerial_Packet gtn_pkt;
		
	// send the dummy packet. Keep trying till successful
	do
	{
		// construct the dummy packet
		gtn_pkt.type = SERIAL_APPLICATION;
		gtn_pkt.length = 1;
		gtn_pkt.data[0] = count1;
		pack_GatewayToNodeSerial_Packet_header(tx_buf, &gtn_pkt);
		memcpy(tx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, gtn_pkt.data, MAX_GATEWAY_PAYLOAD);
		ret = slipstream_send(tx_buf, (int)SIZE_GATEWAYTONODESERIAL_PACKET); // send an inital message to the server
		if(ret == 0)
		{
			printf("NG: send_dummy_pkt(): Error in sending data to Firefly\r\n");
			continue;
		}
		printf("NG: send_dummy_pkt(): Sent dummy message to the firefly: %d\r\n", count1);
		count1 = (count1 + 1) % 120;
		break;
	}while(1);
	
	return;
}
/*************************************************************************************************/
void start_collection_phase()
{
	int64_t start_time, end_time;					 // to mark the start and end of the collection period 
	NodeToGatewaySerial_Packet ntg_pkt;				 // to hold a packet received from the attached node
	uint8_t rx_buf[SIZE_NODETOGATEWAYSERIAL_PACKET]; // to hold a message from the attached Firefly
	Msg_NwInfoAcquired mnwia;						 // to hold a message
	Msg_NodeInfoAcquired mnia;						 // to hold a message
	int8_t ret;										 // holds the return value of function calls
	int8_t retValue;								 // to decide whether major change has occured in topology
	int8_t count;
	
	//set the timers
	start_time = time(NULL);
	end_time = start_time;
	
	//printf("NG: start_collection_phase(): Collecting topology information. Please wait..");
	
	retValue = FALSE;
	// collect data for COLLECTION_PERIOD  		
	while( end_time - start_time < COLLECTION_PERIOD )
	//while(1) // debugging	 
	{
		// receive a packet from the attached node
		ret = receiveFromSerial(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET, RX_TIMEOUT);
		if(ret == 0)	// no data received
		{
			end_time = time(NULL);	// update the end time
			continue;
		}		
		if(ret != SIZE_NODETOGATEWAYSERIAL_PACKET)	// wrong length packet received
		{
			printf("NG: start_collection_phase(): Shorter length packet received: %d\r\n", ret);
			exit(1);
		}			
		// unpack the packet
		unpack_NodeToGatewaySerial_Packet_header(&ntg_pkt, rx_buf);
		memcpy(ntg_pkt.data, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, MAX_SERIAL_PAYLOAD);
											
		// what kind of packet is it 
		switch(serial_pkt_type(&ntg_pkt))
		{
			case SERIAL_APPLICATION:
				process_serial_app_pkt(&ntg_pkt);
				break;
					
			case SERIAL_NW_CONTROL:
				retValue = process_serial_nw_ctrl_pkt(&ntg_pkt);		// ignore the return value here 	
				break;
				
			default:
				// drop the packet and go receive another one 
				printf("NG: start_collection_phase(): Invalid packet type received = %d\n", ntg_pkt.type);	 
		}
		
		// update the end time 
		end_time = time(NULL);
	} // end while(COLLECTION_PERIOD)
	
	
	// tell the network to stop sending NGB_LIST messages. Send a NW_INFO_ACQUIRED message
	build_Msg_NwInfoAcquired(&mnwia);
	printf("NG: Sending SERIAL_NW_INFO_ACQUIRED\r\n");
	ret = sendOverSerial(&mnwia, SERIAL_NW_INFO_ACQUIRED);
	if(ret == 0)
	{
		perror("NG: start_collection_phase(): Error in sending data to Firefly\r\n");
		exit(1);
	}
		
	if(retValue == TRUE)		
	{
		// open the topology description file
		topologyFile = (FILE*)fopen("SensorTopology.dot", "w");
		if(topologyFile == NULL)
		{
			printf("NG: start_collection_phase(): Topology generation file could not be opened\n");
			exit(1);
		}
		begin_topology_file(topologyFile);
		prepare_topology_desc_file();						
		end_topology_file(topologyFile);					
		fclose(topologyFile);								// close the file
		//generate_graph();									// invoke a graph-drawing program
	}
	
	for(count = 1; count <= 5; count++)
		{
			printf("%d\r\n", count);
			sleep(1);
		}
	// tell the network to stop sending NODE_INFO messages. Send a NODE_INFO_ACQUIRED message
	build_Msg_NodeInfoAcquired(&mnia);
	printf("NG: Sending SERIAL_NODE_INFO_ACQUIRED\r\n");
	ret = sendOverSerial(&mnia, SERIAL_NODE_INFO_ACQUIRED);
	if(ret == 0)
	{
		perror("NG: start_collection_phase(): Error in sending data to Firefly\r\n");
		exit(1);
	}
	
	return;
} // end function
/*************************************************************************************************/
void start_listening_phase()
{
	int8_t retValue = FALSE;			// to hold the return value of various function calls
	NodeToGatewaySerial_Packet ntg_pkt;	// to hold a packet sent by the attached node
	uint8_t rx_buf[SIZE_NODETOGATEWAYSERIAL_PACKET];
	uint32_t i,j;						// loop variables
	
	printf("NG: start_listening_phase(): Begin.....\r\n");
	
	while(1)		// listen to attached node forever	 
	{
		for(i=0; i< 1000000UL; i++)
			for(j = 0; j < 300; j++)
				;
		// receive a packet from the attached node
		retValue = receiveFromSerial(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET, RX_TIMEOUT);
		if(retValue == 0)	
		{
			continue; // do nothing
		}		
		// unpack the packet
		unpack_NodeToGatewaySerial_Packet_header(&ntg_pkt, rx_buf);
		memcpy(ntg_pkt.data, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, MAX_SERIAL_PAYLOAD);
									
		// what kind of packet is it 
		switch(serial_pkt_type(&ntg_pkt))
		{
			case SERIAL_APPLICATION:
				process_serial_app_pkt(&ntg_pkt);
				break;
					
			case SERIAL_NW_CONTROL:
				retValue = process_serial_nw_ctrl_pkt(&ntg_pkt); 
				break;
				
			case INVALID:
				// drop the packet and go receive another one 
				printf("NG: start_listening_phase(): Invalid packet type received = %d\n", ntg_pkt.type);	 
				break;
		}
		// is there any change to be made to the topology description file
		if(retValue == TRUE)
		{
			topologyFile = (FILE*)fopen("SensorTopology.dot", "w");
			if(topologyFile == NULL)
			{
				printf("NG: start_listening_phase(): Topology generation file could not be opened\n");
				exit(1);
			}
			begin_topology_file(topologyFile);
			prepare_topology_desc_file();						
			end_topology_file(topologyFile);					
			fclose(topologyFile);				// close the file
			generate_graph();					// invoke a graph-drawing program
			
			retValue = FALSE;					// reset the flag variable				
		}
	} // end while(1)
} // end function
/**********************************************************************************************/
int main()
{
	Msg_SendNwInfo msnwi;						// to hold a message of type Msg_SendNwInfo;
	Msg_SendNodeInfo msni;						// to hold a message of type Msg_SendNodeInfo
	GatewayToNodeSerial_Packet gtn_pkt;
	char gw_addr[SIZE_IP_ADDRESS];				// temporary variable to hold the IP address of the gateway
	int8_t ret;									// holds the return value of function calls
	uint32_t i, j, count = 3;
	uint16_t addrs[MAX_SUBNET_SIZE];			// temporary array
	
	initialise_network_gateway();				// initialise the gateway 
	// make an UDP socket to connect to the SLIPStream server
	if( slipstream_open(GATEWAY_ADDRESS, GATEWAY_PORT, 1) == 0 )
	{
		printf("NG: main(): Error in connecting to the gateway server at [%s,%d]\r\n", strcpy(gw_addr, GATEWAY_ADDRESS), GATEWAY_PORT);
		exit(1);
	}
	
	// construct and send a dummy packet
	while(count-- > 0)
	{
		send_dummy_pkt();
		sleep(1);
	}
	
	/*do
	{
		printf("\n\n");
		printf("1.  Print SNL and VL\r\n");
		printf("2.  Clear topology information\r\n");
		printf("3.  Write topology to a file\r\n");
		printf("4.  Print routing table\r\n");
		printf("5.  Query a node for topology information\n");
		printf("6.  Query a node for state information\n");
		printf("7.  Exit\r\n");
		printf("Enter your choice: ");
		scanf("%d", &choice);
	
		switch(choice)
		{
			case 1:			// print topology information
				print_SNL();
				print_VL();
				break;
				
			case 2:			// clear topology description
				clear_topology();
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
				
			case 3:			// Writing topology description to a file
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
				
			case 4:		// print routing table
				printf("Enter node ID: ");
				scanf("%d", &addr);
				print_RoutingTableGateway(addr);
				break;	
				
			case 5:		// Query a node for topology information
				printf("Enter node ID: ");
				scanf("%d", &addr);
				if(search_node_SNL(addr) == NULL)
				{
					printf("Node ID does not exist\r\n");
					break;
				}				
				build_Msg_SendNwInfo(&msnwi, INVALID_ADDR, addr);
				gtn_pkt.type = SERIAL_SEND_NW_INFO;
				gtn_pkt.length = SIZE_MSG_SEND_NW_INFO;
				pack_Msg_SendNwInfo(gtn_pkt.data, &msnwi);
				pack_GatewayToNodeSerial_Packet_header(tx_buf, &gtn_pkt);
				memcpy(tx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, gtn_pkt.data, MAX_GATEWAY_PAYLOAD);
				ret = slipstream_send(tx_buf, (int)SIZE_GATEWAYTONODESERIAL_PACKET); // send a data collection message into the network
				if(ret == 0)
				{
					printf("NG: main(): Error in sending message to firefly\r\n");
					exit(1);
				}
				printf("Waiting for Node:%d to respond ....\r\n", addr);
				ret = receiveFromSerial(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
									
				// unpack the packet
				unpack_NodeToGatewaySerial_Packet_header(&ntg_pkt, rx_buf);
				memcpy(ntg_pkt.data, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, MAX_SERIAL_PAYLOAD);
				
											
		// what kind of packet is it 
		switch(serial_pkt_type(&ntg_pkt))
		{
			case SERIAL_APPLICATION:
				process_serial_app_pkt(&ntg_pkt);
				break;
					
			case SERIAL_NW_CONTROL:
				
				
				
								
				
			default:
				printf("Invalid option: Try again\r\n");
		} // end outer switch
	
	} while(choice != 18);
	
	
	*/
		
	
	do // debugging
	{ 
		// construct and send a SERIAL_SEND_NW_INFO packet to the network
		addrs[0] = BCAST_ADDR;		// require each node's NW information
		build_Msg_SendNwInfo(&msnwi, addrs, 1);
		printf("NG: main(): Sending SERIAL_SEND_NW_INFO\r\n");
		ret = sendOverSerial(&msnwi, SERIAL_SEND_NW_INFO);
		if(ret == 0)
		{
			perror("NG: main(): Error while sending data to Firefly");
			exit(1);
		}
			
		
		/*// go into COLLECTION PHASE
		//start_collection_phase();
	
		printf("Sleeping ");
		for(count = 1; count <= 20; count++)
		{
			printf("%d\r\n", count);
			sleep(1);
		}
		print_SNL();
	//}while(1);		
	*/
	
	//do // debugging
	//{ 
		
		for(count = 1; count <= 5; count++)
		{
			printf("%d\r\n", count);
			sleep(1);
		}
			
		// construct and send a SERIAL_SEND_NODE_INFO packet to the network
		addrs[0] = BCAST_ADDR;		// require each node's state information
		build_Msg_SendNodeInfo(&msni, addrs, 1);
		printf("NG: main(): Sending SERIAL_SEND_NODE_INFO\r\n");
		ret = sendOverSerial(&msni, SERIAL_SEND_NODE_INFO);
		if(ret == 0)
		{
			perror("NG: main(): Error while sending data to Firefly");
			exit(1);
		}
			
		// go into COLLECTION PHASE
		start_collection_phase();
	
		printf("Sleeping ");
		for(count = 1; count <= 30; count++)
		{
			printf("%d\r\n", count);
			sleep(1);
		}
		print_SNL();
	}while(1);
	
		
	// generate the topology description file and routing tables for each node
	generate_routing_tables();
	
	// go into LISTENING PHASE
	// the code never leaves this function
	start_listening_phase();
	
	return 0;
} // end main()
/******************************************************************************/
void process_Msg_RouteRequest(Msg_RouteReply *mrr, uint16_t src, uint16_t dest)
{
	int8_t i, j, abandon = FALSE;;
	uint16_t from;
	SensorNode *snptr;
	
	// fill in the members of the Msg_RouteReply
	initialise_Msg_RouteReply(mrr);
	mrr -> dest = dest;
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
					mrr -> rre[i].src = from;				
					mrr -> rre[i].nextHop = snptr -> rt.rte[j].nextHop;
					mrr -> rre[i].cost = snptr -> rt.rte[j].cost;
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
/*********************************************************************************************/
void initialise_Msg_RouteReply(Msg_RouteReply *m)
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
	
	if( (rear + 1) % MAX_STATE_VALUES_GATEWAY == front )		// node's state queue is full
		remove_from_node_state_gateway(nsqg);			// remove the oldest entry
		
	// now add the new entry
	nsqg -> ns[rear] = *ns;
	nsqg -> rear = (nsqg -> rear + 1) % MAX_STATE_VALUES_GATEWAY;
	return;
}	
/**********************************************************************************************/
void remove_from_node_state_gateway(NodeStateQueueGateway *nsqg)
{
	if(nsqg -> front == nsqg -> rear)	// queue is empty
		return;
		
	nsqg -> front = (nsqg -> front + 1) % MAX_STATE_VALUES_GATEWAY;
	
	return;
}
/***************************************************************************************************/
void clear_topology()
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
/***************************************************************************************************/
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
/***************************************************************************************************/
void writeNSToFile(Msg_NodeInfo *mni)
{
	FILE *fp;
	char name[20];		// temporary string to hold the file name
	int8_t i;			// loop variable
	
	sprintf(name, "Node%d.dat", mni -> addr);	// generate the file name
	
	fp = (FILE*)fopen(name, "a");
	if(fp == NULL)
	{
		printf("NodeState file for %d could not be opened\r\n", mni -> addr);
		exit(1);
	}
	
	for(i = mni -> nsq.front; ; i = (i + 1) % MAX_STATE_VALUES)	// iterate through NodeStateQueue
	{
		if(i == mni -> nsq.rear)
			break;
		fprintf(fp, "%u\t%d\t%d\t%d\n", mni -> nsq.ns[i].timestamp.secs, mni -> nsq.ns[i].battery, mni -> nsq.ns[i].tx_queue_size, mni -> nsq.ns[i].rx_queue_size);
	}
	fclose(fp);
	
	return;
}	
/***************************************************************************************************/
int8_t sendOverSerial(void *msg, uint8_t type)
{
	Msg_RouteReply *mrr;
	Msg_SendNwInfo *msnwi;
	Msg_NwInfoAcquired *mnwia;
	Msg_SendNodeInfo *msni;
	Msg_NodeInfoAcquired *mnia;
	GatewayToNodeSerial_Packet gtn_pkt;
	int8_t ret;
	
	switch(type)
	{
		case SERIAL_ROUTE_REPLY:
			mrr = (Msg_RouteReply*)msg;
			gtn_pkt.type = SERIAL_ROUTE_REPLY;
			gtn_pkt.length = SIZE_MSG_ROUTE_REPLY;
			pack_Msg_RouteReply(gtn_pkt.data, mrr);
			break;
			
		case SERIAL_SEND_NW_INFO:
			msnwi = (Msg_SendNwInfo*)msg;
			gtn_pkt.type = SERIAL_SEND_NW_INFO;
			gtn_pkt.length = SIZE_MSG_SEND_NW_INFO;
			pack_Msg_SendNwInfo(gtn_pkt.data, msnwi);
			break;
			
		case SERIAL_NW_INFO_ACQUIRED:
			mnwia = (Msg_NwInfoAcquired*)msg;
			gtn_pkt.type = SERIAL_NW_INFO_ACQUIRED;
			gtn_pkt.length = SIZE_MSG_NW_INFO_ACQUIRED;
			pack_Msg_NwInfoAcquired(gtn_pkt.data, mnwia);
			break;
			
		case SERIAL_SEND_NODE_INFO:
			msni = (Msg_SendNodeInfo*)msg;
			gtn_pkt.type = SERIAL_SEND_NODE_INFO;
			gtn_pkt.length = SIZE_MSG_SEND_NODE_INFO;
			pack_Msg_SendNodeInfo(gtn_pkt.data, msni);
			break;
			
		case SERIAL_NODE_INFO_ACQUIRED:
			mnia = (Msg_NodeInfoAcquired*)msg;
			gtn_pkt.type = SERIAL_NODE_INFO_ACQUIRED;
			gtn_pkt.length = SIZE_MSG_NODE_INFO_ACQUIRED;
			pack_Msg_NodeInfoAcquired(gtn_pkt.data, mnia);
			break;
	}
	ret = sendToSensorNode(&gtn_pkt);
	return ret;
}	
/***************************************************************************************************/
int8_t sendToSensorNode(GatewayToNodeSerial_Packet *pkt)
{
	char tx_buf[SIZE_GATEWAYTONODESERIAL_PACKET];	// transmit buffer to send data to attached Firefly
	int8_t ret;										// return type of function calls
	
	pack_GatewayToNodeSerial_Packet_header(tx_buf, pkt);
	memcpy(tx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, pkt -> data, MAX_GATEWAY_PAYLOAD);
	
	ret = slipstream_send(tx_buf, (int)SIZE_GATEWAYTONODESERIAL_PACKET); 
	return ret;
}	
/********************************** PRINTING FUNCTIONS *********************************************/
void print_SNL()
{
	SensorNode *snptr; 
	NeighborGateway *ngptr;
	
	printf("SensorNode List\n");
	//printf("PHere0 ");
	snptr = top_mgr.head;
	//printf("PHere0 ");
	while(snptr != NULL)
	{
		//printf("PHere1\n");
		printf("%d -> ", snptr -> addr);
		//printf("PHere2\n ");
		ngptr = snptr -> head;
		while(ngptr != NULL)
		{
			//printf("PHere3\n");
			printf("[%d %d],  ", ngptr -> addr, ngptr -> rssi);
			//printf("PHere4\n");
			ngptr = ngptr -> next;
		}
		//printf("PHere5\n");
		if(snptr -> nbest != NULL)
			printf("%u ", snptr -> nbest -> addr);
		else
			printf("NULL ");
			
		if(snptr -> nsbest != NULL)
			printf("%u ", snptr -> nsbest -> addr);
		else
			printf("NULL ");
			
		if(snptr -> nworst != NULL)
			printf("%u ", snptr -> nworst -> addr);
		else
			printf("NULL ");	
			
		printf("\r\n");
		snptr = snptr -> next;
	}
	//end: printf("PHere6\n");
	return;
}
/***************************************************************************************************/
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
	printf("%lu %lu ", time.secs, time.nano_secs);
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
void print_Map()
{
	int8_t i;
	
	for(i = 1; i <= top_mgr.count; i++)
	{
		printf("%d  %d\r\n", i, Map[i]);
	}
	return;
}
/**************************************************************************************************/
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
/**************************************************************************************************/
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
/**************************************************************************************************/
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
/*************************************************************************************************/
void print_NodeState(NodeState *ns)
{
	print_nrk_time_t(ns -> timestamp);
	printf("%d %d %d\r\n", ns -> battery, ns -> tx_queue_size, ns -> rx_queue_size);
	
	return;
}
/*************************************************************************************************/
void print_NodeStateQueue(NodeStateQueue *nsq)
{
	int8_t i;
	
	for(i = nsq -> front; ; i = ((i + 1) % MAX_STATE_VALUES))
	{
		if(i == nsq -> rear)
			break;
		print_NodeState( &(nsq -> ns[i]) );
	}
	printf("\r\n");
	
	return;
}
/*************************************************************************************************/
void print_ntg_pkt(NodeToGatewaySerial_Packet *pkt)
{
	int8_t i;
	
	print_ntg_pkt_header(pkt);
	printf("[ ");
	for(i = 0; i < pkt -> length; i++)
		printf("%d ", pkt -> data[i]);
	printf("]\r\n");
}
/*************************************************************************************************/
void print_ntg_pkt_header(NodeToGatewaySerial_Packet *pkt)
{
	printf("[%d %d] ", pkt -> type, pkt -> length);
	return;
}
/*************************************************************************************************/
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
/*************************************************************************************************/
void print_gtn_pkt_header(GatewayToNodeSerial_Packet *pkt)
{
	printf("[");
	printf("%d %d]", pkt -> type, pkt -> length);
	
	return;
}
/*************************************************************************************************/	
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
/**************************************************************************************************/
