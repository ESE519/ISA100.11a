/* This file implements the threaded version of the network gateway for the sensor network */
/* Authors:
   Aditya Bhave
*/
#include "NetworkGatewayThreaded.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/*********************************** External variables and functions ******************************/
// From TopologyGeneration.c 
extern void add_link_to_topology_file(FILE *fp, uint16_t f, uint16_t t, int8_t rssi);
extern void begin_topology_file(FILE *fp);
extern void end_topology_file(FILE *fp);
extern void generate_TopGraph();

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

Msg_NgbListQueue mnl_queue;								// list of queues (one for each thread)
Msg_RouteRequestQueue mrrq_queue;
Msg_NodeInfoQueue mni_queue;

pthread_t thread_Msg_NgbList;							// list of threads (one per message type)
pthread_t thread_Msg_RouteRequest;
pthread_t thread_Msg_NodeInfo;
pthread_t thread_UserInput;								// to read and process user input
pthread_t thread_CollectStats;							// thread to update statistics on web page

pthread_mutex_t data_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;		// to protect the SNL and VL
pthread_mutex_t serial_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;		// to protect access to serial port
pthread_mutex_t mnl_queue_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;	// to protect access to queues
pthread_mutex_t mrrq_queue_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t mni_queue_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
//pthread_mutex_t cp_progress_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; // to protect access to 'cp_progress'

pthread_cond_t mnl_request = PTHREAD_COND_INITIALIZER;			// list of condition variables
pthread_cond_t mrrq_request = PTHREAD_COND_INITIALIZER;
pthread_cond_t mni_request = PTHREAD_COND_INITIALIZER;

uint8_t msnwi_seq_no = 0;										// list of sequence numbers for various BCAST_messages
uint8_t msni_seq_no = 0;
uint8_t mnwia_seq_no = 0;
uint8_t mnia_seq_no = 0;
uint8_t mrr_seq_no = 0;

//uint8_t cp_progress;											// flag to indicate if collection_phase is in progress

/************************************************ FUNCTION DEFINITIONS ***********************************/
void cleanup_mutex(void *m)
{
	pthread_mutex_t *mut = (pthread_mutex_t*)m;
	
	if(mut)
		pthread_mutex_unlock(mut);
		
	return;
}
/********************************************************************************************************/
/*
void generate_html(int8_t close)
{
	FILE *fp;
	char path[PATHNAME_MAX_SIZE];
	
	sprintf(path, "%s/demo_hot.html", WEB_SERVER_ROOT);
	fp = (FILE *)fopen(path, "w");
	if(fp == NULL)
	{
		perror("NG: generate_html(): fopen(): ");
		exit(1);
	}
	
	fprintf(fp, "<html> \n");
	fprintf(fp, "<head> \n"); 
	fprintf(fp, "<title> Sensor Network statistics </title> \n");
	fprintf(fp, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"5; URL=./demo.html\"> \n");
	fprintf(fp, "</head> \n");
	fprintf(fp, "<body style=\"color: rgb(204, 204, 204); background-color: rgb(0, 0, 0); \"link = \"#cccccc\" vlink = \"#cccccc\" alink = \"#cccccc\"> \n");
	fprintf(fp, "<h1>Node statistics for the sensor network at CIC</h1> \n");
	fprintf(fp, "<p> \n");
	fprintf(fp, "The picture below shows the node placement at CIC \n");
	fprintf(fp, "</p><br> \n");
	fprintf(fp, "<img src = \"./cic_testbed.png\"> \n\n");
	
	sprintf(path, "%s/SensorTopology.gif", WEB_SERVER_ROOT);
	if(access(path, F_OK) == 0)		// topology file SensorTopology.gif exists
	{
		fprintf(fp, "<p> \n");
		fprintf(fp, "<a name = \"Top\"> Current topology is </a> <br> \n");
		fprintf(fp, "</p> \n");
		fprintf(fp, "<img src = \"./SensorTopology.gif\"> \n");
	
	}
	else												// topology file does not exist
	{
		fprintf(fp, "<p> \n");
		fprintf(fp, "Current toplogy information is unavailable \n");
		fprintf(fp, "</p><br> \n");
	}
	
	if(close == 1)	// finish the HTML file
	{
		fprintf(fp, "</body> \n");
		fprintf(fp, "</html> \n");
	}	
	fclose(fp);
	
	return;
	
}
*/
/********************************************************************************************************/
void create_NodePath(char *path, uint16_t addr, int8_t flag)
{
	char name[30];
	
	sprintf(path, "%s/Node_%u", WEB_SERVER_ROOT, addr);
	if(flag == 0)
		sprintf(name, "/Node_%u.txt", addr);	// generate the file name
	else
		sprintf(name, "/Node_%u_temp.txt", addr);	// used for graph display purposes
	strcat (path, name);						// generate the complete path name
	
	return;
}
/********************************************************************************************************/
void generate_html(char *fname, int8_t close)
{
	FILE *fp;
	char path[PATHNAME_MAX_SIZE];
	
	sprintf(path, "%s/%s", WEB_SERVER_ROOT, fname);
	fp = (FILE *)fopen(path, "w");
	if(fp == NULL)
	{
		perror("NG: generate_html(): fopen(): ");
		exit(1);
	}
	
	if(strcmp(fname, "SensorAndrewLogo.html") == 0)	// it is the logo file
	{
		fprintf(fp, "<html> \n");
		fprintf(fp, "<head> \n");
		fprintf(fp, "</head> \n");
		fprintf(fp, "<body style=\"background-color: rgb(30, 45, 60);\" > \n");
		fprintf(fp, "<center> ");
		fprintf(fp, "<img src = \"Firefly_2.jpeg\" width = \"200\" height = \"130\"> \n");
		fprintf(fp, "<img src = \"SensorAndrewLogo.jpeg\" width=\"315\" height=\"130\"> \n");
		fprintf(fp, "<img src = \"Firefly_2.jpeg\" width = \"200\" height = \"130\"> \n");
		fprintf(fp, "</center> \n");
						
		if(close == 1)
		{
			fprintf(fp, "</body> \n");
			fprintf(fp, "</html> \n");
		}
	}
	else if(strcmp(fname, "Demo.html") == 0)		// it is the main HTML file
	{
		fprintf(fp, "<html> \n");
		fprintf(fp, "<head> \n");
		fprintf(fp, "<title> Sensor Andrew Project </title> \n");
		fprintf(fp, "</head> \n");
		fprintf(fp, "<frameset rows = \"30%%, 90%%\"> \n");
		fprintf(fp, "<frame src=\"SensorAndrewLogo.html\" name=\"SALogo\" scrolling=\"no\" frameborder=\"0\" noresize> \n");
		fprintf(fp, "<frameset cols = \"10%%, 40%%, 50%%\"> \n");
		fprintf(fp, "<frame src=\"Node_List.html\" name=\"Node_List\" frameborder=\"1\"> \n");
		fprintf(fp, "<frame src=\"Node_Topology.html\" name=\"Node_Topology\" frameborder=\"1\" noresize> \n");
		fprintf(fp, "<frame src=\"Node_Data.html\" name=\"Node_Data\" frameborder=\"1\" noresize> \n");
		fprintf(fp, "</frameset> \n");
		fprintf(fp, "</frameset> \n");
		fprintf(fp, "</html> \n");		
		/*
		fprintf(fp, "<frameset cols = \"10%%, 50%%, 40%%\"> \n");
		//fprintf(fp, "<frameset rows = \"110, *\"> \n");
		//fprintf(fp, "<frame src=\"SensorAndrewLogo.html\" name=\"SALogo\" scrolling=\"no\" frameborder=\"0\" noresize> \n");
		//fprintf(fp, "<frame src=\"Node_List.html\" name=\"Node_List\" frameborder=\"0\"> \n");
		//fprintf(fp, "</frameset> \n");
		//fprintf(fp, "<frame src=\"Node_Topology.html\" name=\"Node_Topology\" frameborder=\"0\" noresize> \n");
		//fprintf(fp, "<frame src=\"Node_Data.html\" name=\"Node_Data\" frameborder=\"0\" noresize> \n");
		//fprintf(fp, "</frameset> \n");
		//fprintf(fp, "</html> \n");
		*/
	}
	else if(strcmp(fname, "Node_List_hot.html") == 0)
	{
		fprintf(fp, "<html> \n");
		fprintf(fp, "<head> \n");
		fprintf(fp, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"20; URL=./Node_List.html\"> \n");
		fprintf(fp, "</head> \n");
		fprintf(fp, "<body style=\"color: rgb(204, 204, 204); background-color: rgb(0, 0, 0); \"link = \"#cccccc\" vlink = \"#cccccc\" alink = \"#cccccc\"> \n");
		fprintf(fp, "<p> \n");
		fprintf(fp, "Click on the links below to view individual node statistics in the right frame \n");
		fprintf(fp, "</p> \n");
		
		if(close == 1)	// finish the HTML file
		{
			fprintf(fp, "</body> \n");
			fprintf(fp, "</html> \n");
		}
	}
	else if(strcmp(fname, "Node_Topology_hot.html") == 0)
	{
		fprintf(fp, "<html> \n");
		fprintf(fp, "<head> \n");
		fprintf(fp, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"20; URL=./Node_Topology.html\"> \n");
		fprintf(fp, "</head> \n");
		fprintf(fp, "<body style=\"color: rgb(204, 204, 204); background-color: rgb(0, 0, 0); \"link = \"#cccccc\" vlink = \"#cccccc\" alink = \"#cccccc\"> \n");
		fprintf(fp, "<p> \n");
		fprintf(fp, "The picture below shows the node placement at CIC \n");
		fprintf(fp, "</p><br> \n");
		fprintf(fp, "<img src = \"./cic_testbed.png\" width = \"480\" height = \"400\"> \n\n");
		sprintf(path, "%s/SensorTopology.gif", WEB_SERVER_ROOT);
		if(access(path, F_OK) == 0)		// topology file SensorTopology.gif exists
		{
			fprintf(fp, "<p> \n");
			fprintf(fp, "<a name = \"Top\"> Current topology is </a> <br> \n");
			fprintf(fp, "</p> \n");
			fprintf(fp, "<img src = \"./SensorTopology.gif\" width = \"480\" height = \"400\"> \n");
		}
		else												// topology file does not exist
		{
			fprintf(fp, "<p> \n");
			fprintf(fp, "Current toplogy information is unavailable \n");
			fprintf(fp, "</p><br> \n");
		}
	
		if(close == 1)	// finish the HTML file
		{
			fprintf(fp, "</body> \n");
			fprintf(fp, "</html> \n");
		}
	}
	else if(strcmp(fname, "Node_Data_hot.html") == 0)
	{
		fprintf(fp, "<html> \n");
		fprintf(fp, "<head> \n");
		fprintf(fp, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"20; URL=./Node_Data.html\"> \n");
		fprintf(fp, "</head> \n");
		fprintf(fp, "<body style=\"color: rgb(204, 204, 204); background-color: rgb(0, 0, 0); \"link = \"#cccccc\" vlink = \"#cccccc\" alink = \"#cccccc\"> \n");
		fprintf(fp, "<p> \n");
		fprintf(fp, "Real-time node statistics are shown below \n");
		fprintf(fp, "</p> \n");
		
		if(close == 1)	// finish the HTML file
		{
			fprintf(fp, "</body> \n");
			fprintf(fp, "</html> \n");
		}
	}
	else
	{
		printf("NG: generate_html(): Wrong file name passed \n");
		exit(1);
	}
			
	fclose(fp);		// always close the file, so that it can be opened again in the calling function
	
	return;
}
/********************************************************************************************************/
void generate_NodeGraph(uint16_t addr)
{
	char path_temp[PATHNAME_MAX_SIZE];		// path of the Node_%u_tmp.txt file
	char plot_path[PATHNAME_MAX_SIZE];		// path of the file containing the Gnuplot commands
	char cmd[BUFF_SIZE];					// to hold a shell command
	FILE *plot_fp;							// pointer to file containing Gnuplot commands
	
	create_NodePath(path_temp, addr, 1);			  // create path to "temp" file of node with address = "addr"
	sprintf(plot_path, "%s/gpcmds", WEB_SERVER_ROOT); // create path to Gnuplot commands file
		
	plot_fp = (FILE *)fopen(plot_path,"w"); // open Gnuplot commands file
	if(plot_fp == NULL)
	{
		fprintf(stderr, "NG: process_CollectStats(): Error while opening gpcmds!\n");
		exit(1);
	}
	// fill up the file with commands for each of the statistics
	fprintf(plot_fp, "set terminal jpeg\n");
	fprintf(plot_fp, "set title \"Statistics for Node %u\"\n", addr);
	fprintf(plot_fp, "set output \"%s/Node_%u/Node_%u_temp.jpeg\" \n", WEB_SERVER_ROOT, addr, addr);
	fprintf(plot_fp, "set xlabel \"Time\"\n");
	fprintf(plot_fp, "set ylabel \"Statistic(B, T, R)\"\n");
	fprintf(plot_fp, "plot \"%s\" using 1:2 title \"Battery\" with lines, \"%s\" using 1:3 title \"TX queue size\" with lines, \"%s\" using 1:4 title \"RX queue size\" with lines, \"%s\" using 1:5 title \"Avg wait time in TX queue\" with lines, \"%s\" using 1:6 title \"Avg ACK time\" with lines\n", path_temp, path_temp, path_temp, path_temp, path_temp);
	fclose(plot_fp);	// close the file
	// construct the Gnuplot command
	sprintf(cmd, "gnuplot %s", plot_path, WEB_SERVER_ROOT, addr, addr);
	system(cmd);	// execute the command
	if(DEBUG_NG == 1)
		printf("NG: process_CollectStats(): Gnuplot command = %s\r\n", cmd);
	
	sprintf(cmd, "mv %s/Node_%u/Node_%u_temp.jpeg %s/Node_%u/Node_%u.jpeg", WEB_SERVER_ROOT, addr, addr, WEB_SERVER_ROOT, addr, addr);
	system(cmd);	// create the file read by the Web server
	
	
	return;
}
/********************************************************************************************************/
void* process_CollectStats(void *data)
{
	SensorNode *snptr;
	char path[PATHNAME_MAX_SIZE];
	char path_temp[PATHNAME_MAX_SIZE];
	char cmd[BUFF_SIZE];				// temporary buffers
	FILE *fp;
	int8_t i;							// loop index
		
	printf("Stats Collection thread started\r\n");
	generate_html("SensorAndrewLogo.html", 1);
	generate_html("Node_List_hot.html", 1);
	generate_html("Node_Topology_hot.html", 1);
	generate_html("Node_Data_hot.html", 1);
	sprintf(cmd, "mv %s/Node_List_hot.html %s/Node_List.html", WEB_SERVER_ROOT, WEB_SERVER_ROOT);
	system(cmd);						// rename it to the file used by the Web server
	sprintf(cmd, "mv %s/Node_Topology_hot.html %s/Node_Topology.html", WEB_SERVER_ROOT, WEB_SERVER_ROOT);
	system(cmd);						// rename it to the file used by the Web server
	sprintf(cmd, "mv %s/Node_Data_hot.html %s/Node_Data.html", WEB_SERVER_ROOT, WEB_SERVER_ROOT);
	system(cmd);						// rename it to the file used by the Web server
	generate_html("Demo.html", 1);					// generate the initial (closed) HTML file
		
	while(1)
	{	
		usleep(100);	// sleep for some time
		// create the second frame
		generate_html("Node_List_hot.html", 0);							// generate the headers of the HTML file	
		sprintf(path, "%s/Node_List_hot.html", WEB_SERVER_ROOT);
		fp = (FILE *)fopen(path, "a+");				// open the file again for appending node values
		if(fp == NULL)
		{
			perror("NG: process_CollectStats()1: fopen(): ");
			exit(1);
		}	
		pthread_mutex_lock(&data_mutex);
		// Add links to named anchors here
		for(snptr = top_mgr.head; snptr != NULL; snptr = snptr -> next)
		{
			create_NodePath(path, snptr -> addr, 0);
			if(access(path, F_OK) == 0)		// the data file Node_n.txt exists
				fprintf(fp, "<p> <a href = \"Node_Data.html#Node_%u\" target = \"Node_Data\"> Node %u </a> </p> \n", snptr -> addr, snptr -> addr);
		}
		pthread_mutex_unlock(&data_mutex);
		fprintf(fp, "</body> \n");		// finish the HTML file and close
		fprintf(fp, "</html> \n");
		fclose(fp);
		
		// create the third frame
		generate_html("Node_Topology_hot.html", 1);
		
		// create the fourth frame
		generate_html("Node_Data_hot.html", 0);
		sprintf(path, "%s/Node_Data_hot.html", WEB_SERVER_ROOT);
		fp = (FILE *)fopen(path, "a+");				// open the file again for appending node values
		if(fp == NULL)
		{
			perror("NG: process_CollectStats()2: fopen(): ");
			exit(1);
		}
		pthread_mutex_lock(&data_mutex);		
		// fill up the remaining lines of the HTML file
		for(snptr = top_mgr.head; snptr != NULL; snptr = snptr -> next)
		{
			create_NodePath(path, snptr -> addr, 0);	// create path to Node_n.txt
			if(access(path, F_OK) == 0)		// data file Node_n.txt exists
			{
				create_NodePath(path_temp, snptr -> addr, 1);	// create the temporary file for graph display
				// create the command to generate the temporary file
				sprintf(cmd, "tail -%d %s > %s", PLOT_SIZE, path, path_temp);
				system(cmd);								// execute the command
				generate_NodeGraph(snptr -> addr);			// generate the statistics graph for this node
				// fill up the remaining lines of the demo_hot.html file
				fprintf(fp, "<p> \n");
				fprintf(fp, "<a name = \"Node_%u\"> For Node %u </a> <br> \n", snptr -> addr, snptr -> addr);
				fprintf(fp, "<a href = \"./Node_%u/Node_%u.txt\"> Click here for actual data </a> \n", snptr -> addr, snptr -> addr);
				fprintf(fp, "<br><br> \n");
				fprintf(fp, "<a href = \"./Node_%u/Node_%u.jpeg\" <img src = \"./Node_%u/Node_%u.jpeg\" width = \"600\" height = \"450\"> </a> \n", snptr -> addr, snptr -> addr, snptr -> addr, snptr -> addr);
				fprintf(fp, "<br> \n");
				fprintf(fp, "<a href = \"#Top\"> Go back to Top </a> \n");
				fprintf(fp, "<br><br> \n");
				fprintf(fp, "Routing Table \n");
				fprintf(fp, "<br><br> \n");
				fprintf(fp, "<table border = \"1\">  \n");
				fprintf(fp, "\t <tr> \n");
				fprintf(fp, "\t\t <th>Destination</th> \n");
				fprintf(fp, "\t\t <th>Next Hop</th> \n");
				fprintf(fp, "\t\t <th>Cost</th> \n");
				fprintf(fp, "\t </tr> \n");
				for(i = 0; i < MAX_SUBNET_SIZE; i++)	// go through the routing table of the sensor node
				{
					if(snptr -> rt.rte[i].dest != INVALID_ADDR)	// a valid entry
					{
						fprintf(fp, "\t <tr> \n");
						fprintf(fp, "\t\t <td> %u </td> \n", snptr -> rt.rte[i].dest);
						
						if(snptr -> rt.rte[i].nextHop != INVALID_ADDR)	// a valid NEXT_HOP field exits
							fprintf(fp, "\t\t <td> %u </td> \n", snptr -> rt.rte[i].nextHop);
						else
							fprintf(fp, "\t\t <td> INV </td> \n");
							
						if(snptr -> rt.rte[i].cost != INFINITY)
							fprintf(fp, "\t\t <td> %u </td> \n", snptr -> rt.rte[i].cost);
						else
							fprintf(fp, "\t\t <td> INF </td> \n");
						fprintf(fp, "\t </tr> \n");
					}
				} // end inner for	
			
				fprintf(fp, "</table> \n");		// Routing table displayed
				fprintf(fp, "</p> \n");
			}
			else	// Node_%u.txt is not yet created.
				;	// nothing to do
		} // end for
		pthread_mutex_unlock(&data_mutex);
		fprintf(fp, "</body> \n");	// finish and close the Node_Data_hot.html file
		fprintf(fp, "</html> \n");
		fclose(fp);	
		
		// rename all files to what they are referenced by the main HTML page
		sprintf(cmd, "mv %s/Node_List_hot.html %s/Node_List.html", WEB_SERVER_ROOT, WEB_SERVER_ROOT);
		system(cmd);						// rename it to the file used by the Web server
		sprintf(cmd, "mv %s/Node_Topology_hot.html %s/Node_Topology.html", WEB_SERVER_ROOT, WEB_SERVER_ROOT);
		system(cmd);						// rename it to the file used by the Web server
		sprintf(cmd, "mv %s/Node_Data_hot.html %s/Node_Data.html", WEB_SERVER_ROOT, WEB_SERVER_ROOT);
		system(cmd);						// rename it to the file used by the Web server
	
	} // end while(1)
				
	return;
}	
/*******************************************************************************************************/
void* process_UserInput(void *data)
{
	Msg_SendNwInfo msnwi;
	Msg_SendNodeInfo msni;
	uint16_t addrs;
			
	printf("User Interaction thread started\r\n");
	while(1)
	{
		usleep(100);
	}
	
	/*
	while(1);
	
		printf("\n\nMain Menu");
		printf("1.  View topology information\r\n");
		printf("2.  Print routing table\r\n");
		printf("3.  Query a node for topology information\n");
		printf("4.  Query a node for state information\n");
		printf("5.  Exit\r\n");
		printf("Enter your choice: ");
		scanf("%d", &choice);
	
		switch(choice)
		{
			case 1:			// view topology information
				print_SNL();
				print_VL();
				break;
				
			case 2:			// view routing table of a node
				printf("Enter node ID: ");
				scanf("%d", &addr);
				print_RoutingTableGateway(addr);
				break;	
									
			case 3:			// Query a node for topology information
				printf("Enter node ID: ");
				scanf("%d", &addr);
				if(search_node_SNL(addr) == NULL)
				{
					printf("Node ID does not exist\r\n");
					break;
				}				
				
				addrs[0] = addr;				
				build_Msg_SendNwInfo(&msnwi, addrs, 1);
				ret = sendOverSerial(&msnwi, SERIAL_SEND_NW_INFO);
				if(ret)
				{
					perror("NG: process_UserInput(): Error in sending message to Firefly\r\n");
					pthread_exit(NULL);
				}
				
				break;
			
											
			default:
				printf("Invalid option: Try again\r\n");
		} // end outer switch
	
	} while(choice != 18);
	*/
	
	return;
}	
	
	
	
/*******************************************************************************************************/
/* This thread processes NGB_LIST messages */
void* process_Msg_NgbList(void *data)
{
	NeighborList nlist;
	Msg_NgbList mnlist;
	int rc;
	int i;
	char path[30];
	
	pthread_cleanup_push(cleanup_mutex, (void*)&mnl_queue_mutex);
	
	/* lock the mutex, to access the queue exclusively. */
    rc = pthread_mutex_lock(&mnl_queue_mutex);
    if(rc)
    {
    	perror("NG: process_Msg_NgbList(): pthread_mutex_lock()");
    	pthread_exit(NULL);
    }

	/* do forever.... */
    while (1)
    {
    	usleep(100);
		if (mnl_queue.num > 0)
		{ 
	    	rc = del_queue(&mnlist, SERIAL_NGB_LIST);
	    	if(rc == -1)	// this should never happen
	    	{
	    		printf("NG: process_Msg_NgbList(): Fatal bug detected\r\n");
	    		exit(1);
	    	}	
	    	/* unlock mutex - so that the main thread can add new requests to the queue parallely */
    		rc = pthread_mutex_unlock(&mnl_queue_mutex);
    		if(rc)
    		{
    			perror("NG: process_Msg_NgbList(): pthread_mutex_unlock(&mnl_queue_mutex)");
    			pthread_exit(NULL);
    		}
    			
			nlist = mnlist.nl;
			if(DEBUG_NG == 0)
			{
				printf("NG: NGB_LIST message received from %d\r\n", nlist.addr);
				//printf("NG: process_serial_nw_ctrl_pkt(): NGB_LIST message received from %d\r\n", nlist.addr);
				printf("%d -> ", nlist.addr);
				for(i = 0; i < MAX_NGBS; i++)
					if(nlist.ngbs[i].addr != INVALID_ADDR)	// valid entry
						printf("%d[%d], ", nlist.ngbs[i].addr, nlist.ngbs[i].rssi);
			}
			printf("\r\n");
			pthread_mutex_lock(&data_mutex);
			rc = process_topology_desc(nlist);	// construct the current graph
			if(rc == TRUE)						// major change in topology
				generate_routing_tables();
			if(DEBUG_NG >= 0)
				printf("NG:Before assigning the flag in htsnl\r\n");
				
			htsnl[nlist.addr].ptr -> nw_info_acquired = 1;		// set the flag
			
			if(DEBUG_NG >= 0)
				printf("NG: After assigning the flag in htsnl\r\n");
				
			
			// prepare the topology description file
			sprintf(path, "%s/SensorTopology.dot", WEB_SERVER_ROOT);
			if(DEBUG_NG == 1)
				printf("NG: process_Msg_NgbList(): Topology path = %s\r\n", path);
			topologyFile = (FILE*)fopen(path, "w");		// note that the topology file is re-written every time
			if(topologyFile == NULL)
			{
				printf("NG: process_Msg_NgbList(): Topology generation file could not be opened\n");
				exit(1);
			}
			begin_topology_file(topologyFile);					// start the file
			prepare_topology_desc_file();						// fill the file
			end_topology_file(topologyFile);					// end the file
			fclose(topologyFile);								// close the file
			generate_TopGraph();								// invoke a graph-drawing program
					
			pthread_mutex_unlock(&data_mutex);
		
			rc = pthread_mutex_lock(&mnl_queue_mutex);	// lock the mutex again
			if(rc)
			{
				perror("NG: process_Msg_NgbList(): pthread_mutex_lock(&mnl_queue_mutex)");
				pthread_exit(NULL);
			}
		} // end if
	    else
	    {
	    	/* the thread will wait on the condition variable */
	   		/* wait for a request to arrive. Note the mutex will be unlocked here, thus allowing the 
	   	       main thread to add to the queue.  
	   	    */
		 	pthread_cond_wait(&mnl_request, &mnl_queue_mutex);
	    	/* and after we return from pthread_cond_wait, the mutex  */
	    	/* is locked again, so we don't need to lock it ourselves */
	    }

    } // end while(1)
	
	/* remove thread cleanup handler. never reached, but we must use */
    /* it here, according to pthread_cleanup_push's manual page.     */
    pthread_cleanup_pop(0);	

}
/***************************************************************************************************/
/* This thread processes ROUTE_REQUEST messages */
void* process_Msg_RouteRequest(void *data)
{
	Msg_RouteRequest mrrq;
	Msg_RouteReply mrr;
	int8_t i, j, abandon, rc;
	uint16_t from;
	SensorNode *snptr;
	
	printf("NG: process_Msg_RouteRequest(): Thread started \r\n");
	
	pthread_cleanup_push(cleanup_mutex, (void*)&mrrq_queue_mutex);
	/* lock the mutex, to access the queue exclusively. */
    rc = pthread_mutex_lock(&mrrq_queue_mutex);
    if(rc)
    {
    	perror("NG: process_Msg_RouteRequest(): pthread_mutex_lock()");
    	pthread_exit(NULL);
    }

	/* do forever.... */
    while (1)
    {
    	usleep(100);
		if (mrrq_queue.num > 0)
		{ 
	    	rc = del_queue(&mrrq, SERIAL_ROUTE_REQUEST);
	    	if(rc == -1)	// this should never happen
	    	{
	    		printf("NG: process_Msg_RouteRequest(): Fatal bug detected\r\n");
	    		exit(1);
	    	}	
	    	//printf("NG: process_Msg_RouteRequest(): Starting to process message\r\n");
	    	/* unlock mutex - so that the main thread can add new requests to the queue parallely */
    		rc = pthread_mutex_unlock(&mrrq_queue_mutex);
    		if(rc)
    		{
    			perror("NG: process_Msg_RouteRequest(): pthread_mutex_unlock(&mrr_queue_mutex)");
    			pthread_exit(NULL);
    		}
    		
    		if(DEBUG_NG == 0)
				printf("NG: ROUTE_REQUEST message received from %u\r\n", mrrq.src);
			
    		// fill in the members of the Msg_RouteReply
    		initialise_Msg_RouteReply(&mrr);
			mrr.dest = mrrq.dest;
			from = mrrq.src;
			abandon = FALSE;
			
			pthread_mutex_lock(&data_mutex);
			for(i = 0; i < NUM_ROUTE_REPLY_ELEMENTS && abandon == FALSE; i++) // parse through Msg_RouteReply
			{
				if(from == mrrq.dest)	// message constructed
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
				for(j = 0; j < MAX_SUBNET_SIZE; j++)	// go through the routing table of 'from'
				{
					if(snptr -> rt.rte[j].dest == mrrq.dest)		// an entry for the destination exists
					{
						if(snptr -> rt.rte[j].nextHop != INVALID_ADDR)		// a path exists to the destination
						{
							// fill up one element in the Msg_RouteReply message
							mrr.rre[i].src = from;				
							mrr.rre[i].nextHop = snptr -> rt.rte[j].nextHop;
							mrr.rre[i].cost = snptr -> rt.rte[j].cost;
							from = snptr -> rt.rte[j].nextHop;
							break;
						} // end if
						else									// no path exists to the destination
						{
							abandon = TRUE;		// no point continuing
							break;
						}
					} // end if
				} // end inner for
				if(j == MAX_SUBNET_SIZE) // at this point we know that no entry is present for the destination
				{
					abandon = TRUE;		// no point continuing
					break;				
				} // end if									
			} // end for
			pthread_mutex_unlock(&data_mutex);			
			if(abandon == FALSE)		// send to the network only if a valid Msg_RouteReply is created
			{
				rc = sendOverSerial(&mrr, SERIAL_ROUTE_REPLY);
				if(rc == -1)
				{
					perror("NG: process_Msg_RouteRequest(): Error in sending data to Firefly\r\n");
					//exit(1);
				}
				print_Msg_RouteReply(&mrr);
			}			
			rc = pthread_mutex_lock(&mrrq_queue_mutex);	// lock the mutex again
			if(rc)
			{
				perror("NG: process_Msg_NgbList(): pthread_mutex_lock(&mnl_queue_mutex)");
				pthread_exit(NULL);
			}
		} // end if
		else
	    {
	    	/* the thread will wait on the condition variable */
	   		/* wait for a request to arrive. Note the mutex will be unlocked here, thus allowing the
	   	       main thread to add to the queue.  
	   	    */
		 	pthread_cond_wait(&mrrq_request, &mrrq_queue_mutex);
	    	/* and after we return from pthread_cond_wait, the mutex  */
	    	/* is locked again, so we don't need to lock it ourselves */
	    }

	} // end while(1)
	
	/* remove thread cleanup handler. never reached, but we must use */
    /* it here, according to pthread_cleanup_push's manual page.     */
    pthread_cleanup_pop(0);
	
}
/**************************************************************************************************/
/* This thread proceses NODE_INFO messages */
void* process_Msg_NodeInfo(void *data)
{
	SensorNode *snptr;
	int8_t i;			// loop index
	Msg_NodeInfo mni;
	int rc;
	
	printf("NG: process_Msg_NodeInfo(): Thread started \r\n");
	
	pthread_cleanup_push(cleanup_mutex, (void*)&mni_queue_mutex);
	/* lock the mutex, to access the queue exclusively. */
    rc = pthread_mutex_lock(&mni_queue_mutex);
    if(rc)
    {
    	perror("NG: process_Msg_NodeInfo(): pthread_mutex_lock()");
    	pthread_exit(NULL);
    }

	/* do forever.... */
    while (1)
    {
    	usleep(100);
		if (mni_queue.num > 0)
		{ 
	    	rc = del_queue(&mni, SERIAL_NODE_INFO);
	    	if(rc == -1)	// this should never happen
	    	{
	    		printf("NG: process_Msg_NodeInfo(): Fatal bug detected\r\n");
	    		exit(1);
	    	}	
	    	if(DEBUG_NG == 0)
			{
				printf("NG: NODE_INFO message received from %u\r\n", mni.addr);
				print_NodeStateQueue(&(mni.nsq));
			}
			
	    	/* unlock mutex - so that the main thread can add new requests to the queue parallely */
    		rc = pthread_mutex_unlock(&mni_queue_mutex);
    		if(rc)
    		{
    			perror("NG: process_Msg_NodeInfo(): pthread_mutex_unlock(&mni_queue_mutex)");
    			pthread_exit(NULL);
    		}
    		
    		// check to see if the node exists in the SNL
    		pthread_mutex_lock(&data_mutex);
    		snptr = htsnl[mni.addr].ptr;
			if(snptr == NULL)	
			{
				printf("NG: process_Msg_NodeInfo(): Adding node to SNL\r\n");
				free_data_structures();
				add_node_to_SNL(mni.addr);
				snptr = htsnl[mni.addr].ptr;	// assign the pointer to the newly allocated node
			}
			
			for(i = mni.nsq.front; ; i = (i + 1) % MAX_STATE_VALUES)	// iterate through NodeStateQueue
			{
				if(i == mni.nsq.rear)	// reached end of circular queue
					break;
					
				// update the state queue in the SNL
				rc = add_to_node_state_gateway( &(snptr -> nsqg), &(mni.nsq.ns[i]) );
				snptr -> total_pkts_inserted += mni.nsq.ns[i].total_pkts_inserted;
				snptr -> total_wait_time += mni.nsq.ns[i].total_wait_time;
				snptr -> total_ack_time += mni.nsq.ns[i].total_ack_time;
				snptr -> totalbytesSent += mni.nsq.ns[i].bytesSent;
				snptr -> totalbytesReceived += mni.nsq.ns[i].bytesReceived;
				snptr -> totalactiveTime += mni.nsq.ns[i].active_time;
				
				if(rc == -1)		// addition not possible because queue is full
				{
					//writeNSQToFile(mni.addr, &(snptr -> nsqg) );		// flush out values to the data file
					writeNSQToFile(mni.addr, snptr);
					rc = add_to_node_state_gateway( &(snptr -> nsqg), &(mni.nsq.ns[i]) );	// retry addition
					if(rc == -1)							// serious bug
					{
						printf("NG: process_Msg_NodeInfo(): Error in NodeStateQueueGateway management\r\n");
						exit(1);
					}
				}	// end if
			}
			htsnl[mni.addr].ptr -> node_info_acquired = 1;		// set the flag
			
			pthread_mutex_unlock(&data_mutex);		// release the data mutex
					
		    rc = pthread_mutex_lock(&mni_queue_mutex);	// lock the mutex again
			if(rc)
			{
				perror("NG: process_Msg_NodeInfo(): pthread_mutex_lock(&mni_queue_mutex)");
				pthread_exit(NULL);
			}
		} // end if
	    else
	    {
	    	/* the thread will wait on the condition variable */
	   		/* wait for a request to arrive. Note the mutex will be unlocked here, thus allowing the 
	   	       main thread to add to the queue.  
	   	    */
		 	pthread_cond_wait(&mni_request, &mni_queue_mutex);
	    	/* and after we return from pthread_cond_wait, the mutex  */
	    	/* is locked again, so we don't need to lock it ourselves */
	    }

    } // end while(1)
	
	/* remove thread cleanup handler. never reached, but we must use */
    /* it here, according to pthread_cleanup_push's manual page.     */
    pthread_cleanup_pop(0);	

}
/****************************************************************************************/
void add_queue(void *msg, uint8_t type)
{
	Msg_NgbList *mnl;
	Msg_RouteRequest *mrrq;
	Msg_NodeInfo *mni;
	int rc;
	
	switch(type)
	{
		case SERIAL_NGB_LIST:
		
			rc = pthread_mutex_lock(&mnl_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_lock");
				pthread_exit(NULL);
			}
			
			mnl = (Msg_NgbList*)msg;
			if( ((mnl_queue.rear + 1) % SIZE_MSG_NGBLIST_QUEUE) == mnl_queue.front )	// the queue is full
				printf("NGT: Msg_NgbList queue is full\r\n");
			else
			{
				mnl_queue.q[mnl_queue.rear] = *mnl;
				mnl_queue.rear = (mnl_queue.rear + 1) % SIZE_MSG_NGBLIST_QUEUE;
				mnl_queue.num++;		// increment number of requests pending
			}
			
		rc = pthread_mutex_unlock(&mnl_queue_mutex);
		if(rc)
		{
			perror("pthread_mutex_unlock");
			pthread_exit(NULL);
		}
    	/* signal the condition variable - there's a new request to handle */
    	pthread_cond_signal(&mnl_request);
			
		break;
			
		case SERIAL_ROUTE_REQUEST:
		
			rc = pthread_mutex_lock(&mrrq_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_lock");
				pthread_exit(NULL);
			}
			mrrq = (Msg_RouteRequest*)msg;
			if( ((mrrq_queue.rear + 1) % SIZE_MSG_ROUTEREQUEST_QUEUE) == mrrq_queue.front )	// the queue is full
				printf("NGT: Msg_RouteRequest queue is full\r\n");
			else
			{
				mrrq_queue.q[mrrq_queue.rear] = *mrrq;
				mrrq_queue.rear = (mrrq_queue.rear + 1) % SIZE_MSG_ROUTEREQUEST_QUEUE;
				mrrq_queue.num++;
			}
			
			rc = pthread_mutex_unlock(&mrrq_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_unlock");
				pthread_exit(NULL);
			}
			/* signal the condition variable - there's a new request to handle */
    	    pthread_cond_signal(&mrrq_request);
			
			break;
			
		case SERIAL_NODE_INFO:
		
			rc = pthread_mutex_lock(&mni_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_lock");
				pthread_exit(NULL);
			}
			mni = (Msg_NodeInfo*)msg;
			if( (mni_queue.rear + 1) % SIZE_MSG_NODEINFO_QUEUE == mni_queue.front )	// the queue is full
				printf("NGT: Msg_NodeInfo queue is full\r\n");
			else
			{
				mni_queue.q[mni_queue.rear] = *mni;
				mni_queue.rear = (mni_queue.rear + 1) % SIZE_MSG_NODEINFO_QUEUE;
				mni_queue.num++;
			}
						
			rc = pthread_mutex_unlock(&mni_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_unlock");
				pthread_exit(NULL);
			}
			/* signal the condition variable - there's a new request to handle */
    		pthread_cond_signal(&mni_request);
		
			
			break;
	}
	
	return;
}
/*******************************************************************************************************/
int8_t del_queue(void *msg, uint8_t type)
{
	int8_t rc;
	
	switch(type)
	{
		case SERIAL_NGB_LIST:
		
			rc = pthread_mutex_lock(&mnl_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_lock");
				pthread_exit(NULL);
			}
			if(mnl_queue.front == mnl_queue.rear)	// queue is empty
				msg = NULL;
			else
			{
				*((Msg_NgbList*)msg) = mnl_queue.q[mnl_queue.front];		// remove the first element from the queue
				mnl_queue.front = (mnl_queue.front + 1) % SIZE_MSG_NGBLIST_QUEUE;
				mnl_queue.num--;
			}
			rc = pthread_mutex_unlock(&mnl_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_unlock");
				pthread_exit(NULL);
			}
			
			return msg == NULL ? -1 : 0;
					
		case SERIAL_ROUTE_REQUEST:
		
			rc = pthread_mutex_lock(&mrrq_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_lock");
				pthread_exit(NULL);
			}
			if(mrrq_queue.front == mrrq_queue.rear)	// queue is empty
				msg = NULL;
			else
			{
				*((Msg_RouteRequest*)msg) = mrrq_queue.q[mrrq_queue.front];		// remove the first element from the queue
				mrrq_queue.front = (mrrq_queue.front + 1) % SIZE_MSG_ROUTEREQUEST_QUEUE;
				mrrq_queue.num--;
			}
			rc = pthread_mutex_unlock(&mrrq_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_unlock");
				pthread_exit(NULL);
			}
			
			return msg == NULL ? -1 : 0;
				
		case SERIAL_NODE_INFO:
		
			rc = pthread_mutex_lock(&mni_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_lock");
				pthread_exit(NULL);
			}
			if(mni_queue.front == mni_queue.rear)	// queue is empty
				msg = NULL;
			else
			{
				*((Msg_NodeInfo*)msg) = mni_queue.q[mni_queue.front];		// remove the first element from the queue
				mni_queue.front = (mni_queue.front + 1) % SIZE_MSG_NODEINFO_QUEUE;
				mni_queue.num--;
			}
			rc = pthread_mutex_unlock(&mni_queue_mutex);
			if(rc)
			{
				perror("pthread_mutex_unlock");
				pthread_exit(NULL);
			}
			return msg == NULL ? -1 : 0;
			
	} // end switch
	
}
/*******************************************************************************************************/
void init_queues()
{
	mnl_queue.front = mnl_queue.rear = mnl_queue.num = 0;
	mnl_queue.qc = &mnl_request;
	mnl_queue.qm = &mnl_queue_mutex;
	
	mrrq_queue.front = mrrq_queue.rear = mrrq_queue.num = 0;
	mrrq_queue.qc = &mrrq_request;
	mrrq_queue.qm = &mrrq_queue_mutex;
	
	mni_queue.front = mni_queue.rear = mni_queue.num = 0;
	mni_queue.qc = &mni_request;
	mni_queue.qm = &mni_queue_mutex;
	
	return;
}	
/*******************************************************************************************************/
void init_threads()
{
	if(pthread_create(&thread_Msg_NgbList, NULL, process_Msg_NgbList, NULL) != 0)
	{
		perror("pthread_create(thread_Msg_NgbList)");
		exit(1);
	}
	if(pthread_create(&thread_Msg_RouteRequest, NULL, process_Msg_RouteRequest, NULL) != 0)
	{
		perror("pthread_create(thread_Msg_RouteRequest)");
		exit(1);
	}
	if(pthread_create(&thread_Msg_NodeInfo, NULL, process_Msg_NodeInfo, NULL) != 0)
	{
		perror("pthread_create(thread_Msg_NodeInfo)");
		exit(1);
	}
	
	// create the user-input reading thread
	if(pthread_create(&thread_UserInput, NULL, process_UserInput, NULL) != 0)
	{
		perror("pthread_create(thread_UserInput)");
		exit(1);
	}
	
	// create the statistics collection thread
	if(pthread_create(&thread_CollectStats, NULL, process_CollectStats, NULL) != 0)
	{
		perror("pthread_create(thread_CollectStats)");
		exit(1);
	}
	
	return;
}
/*******************************************************************************************************/
void initialise_network_gateway()
{
	uint32_t i;									// loop variable
	char cmd[BUFF_SIZE];						// to store a command
	
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
	
	sprintf(cmd, "%s/cleanup.sh", WEB_SERVER_ROOT);
	system(cmd);
	
	// initialise the data structures associated with the threads
	printf("Initializing queue\r\n");
	init_queues();
	
	printf("Creating threads\r\n");
	init_threads();
	
	
	return;
}

/***************************************************************************************/
int8_t receiveFromSerial(uint8_t *p, uint8_t len, int timeout)
{
	int8_t nread;						// to hold number of characters read at a time from the serial port
	uint8_t bytesRemaining;				// to hold number of bytes remaining to be read from Firefly
	uint8_t index;						// loop variable
	int64_t st, et;						// needed to maintain timeout
	
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
  		nread = slipstream_receive(p + index, bytesRemaining);
  		if(nread == -1)
  		{
  			// nothing was received from the Firefly node
  			nread = 0;			// set the number of bytes read to zero
  		}
  		
  		index += nread;					// increment number of characters successfully read
  		bytesRemaining -= nread;		// successfully read 'nread' bytes
  		
  		if(timeout > 0)
  		{
  			et = time(NULL);
  			if(et - st >= timeout)			 // waited long enough
  				break;
  		} // end if  
    }while(bytesRemaining > 0);
  
	return index;						// return whatever number of characters successfully read
		 
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
			
		case SERIAL_ACK:
			return SERIAL_ACK;
			
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
		
			// add this message to the appropriate queue
			unpack_Msg_NgbList(&mnlist, pkt -> data);
			add_queue(&mnlist, SERIAL_NGB_LIST);
			break;
				
		case SERIAL_ROUTE_REQUEST:
		
			unpack_Msg_RouteRequest(&mrrq, pkt -> data);
			add_queue(&mrrq, SERIAL_ROUTE_REQUEST);
			break;
					
		case SERIAL_NODE_INFO:
			// unpack the Msg_NodeInfo from the receive buffer						
			unpack_Msg_NodeInfo(&mnode, pkt -> data);
			add_queue(&mnode, SERIAL_NODE_INFO);
			break;
			
		default:	// invalid packet type
			;		// do nothing. Discard the packet
		
	} // end switch
	
	return retValue;
}
/**********************************************************************************************/
void initialise_Msg_NwInfoAcquired(Msg_NwInfoAcquired *m)
{
	int8_t i;
	
	m -> dg = INVALID_ADDR;
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
		m -> addrs[i] = INVALID_ADDR;
	m -> seq_no = ++mnwia_seq_no;
	
	return;
}
/***********************************************************************************************/
void initialise_Msg_SendNwInfo(Msg_SendNwInfo *m)
{
	int8_t i;
	
	m -> dg = INVALID_ADDR;
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
		m -> addrs[i] = INVALID_ADDR;
	m -> seq_no = ++msnwi_seq_no;  
	//printf("????? %d %d\r\n", m -> seq_no, msnwi_seq_no); 
	
	return;
}
/***********************************************************************************************/
void initialise_Msg_SendNodeInfo(Msg_SendNodeInfo *m)
{
	int8_t i;
	
	m -> dg = INVALID_ADDR;
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
		m -> addrs[i] = INVALID_ADDR;
	m -> seq_no = ++msni_seq_no;
	return;
}
/***********************************************************************************************/
void initialise_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *m)
{
	int8_t i;
	
	m -> dg = INVALID_ADDR;
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
		m -> addrs[i] = INVALID_ADDR;
	m -> seq_no = ++mnia_seq_no;
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
			//printf("NG: Following the prev pointers\r\n");
			while(ptr -> prev != NULL)
				ptr = ptr -> prev;
		
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
	
	if(DEBUG_NG >= 0)
		printf("NG: Entered prepare_topology_desc_file()\r\n");
		
	pthread_mutex_lock(&data_mutex);
	// prepare the topology description file
	for(ptr = top_mgr.head; ptr != NULL; ptr = ptr -> next)
	{
		for(ngptr = ptr -> head; ngptr != NULL; ngptr = ngptr -> next)
			add_link_to_topology_file(topologyFile, ptr -> addr, ngptr -> addr, ngptr -> rssi);
	}
	pthread_mutex_unlock(&data_mutex);
	
	if(DEBUG_NG >= 0)
		printf("NG: Leaving prepare_topology_desc_file()\r\n");
	
	return;
}
/******************************************************************************************************/
int8_t process_topology_desc(NeighborList nl)
{
	// The below comments apply to a situation where node 1 reports its neighbor list
	// as 1 -- 3,2
	int8_t i,j;			// loop indices
	SensorNode *ptr;	// scratch variable
	SensorNode *first, *second;
	Neighbor n;
	NeighborGateway *node;
	int8_t retValue = FALSE;
	
	if(DEBUG_NG == 0)
		printf("NG: Entered process_topology_desc()\r\n");
	
	// flush all the entries in the VL
	refresh_VL(nl.addr);
	
	// check to see if '1' is present in the SNL
	if(search_node_SNL(nl.addr) == NULL)	// '1' was not found in the SNL
	{
		printf("Here2\n");
		free_data_structures();
		printf("Here3\n");
	
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
		
		printf("Here4\n");
		// search for '3' in the SNL 
		ptr = search_node_SNL(n.addr);
		printf("Here5\n");
		if(ptr == NULL)				// '3' was not found in the SNL
		{
				SensorNode *ptr2;
				printf("Here6\r\n");
				free_data_structures();
				printf("Here7\n");
				ptr2 = add_node_to_SNL(n.addr);	// add '3' to the SNL
				printf("Here8\n");
				// add '1' to the neighbor list of '3'
				node = add_node_to_NL(ptr2, nl.addr, &n);
				printf("Here10\n");
				retValue = TRUE;
		} // end if
		else								// '3' was found in the SNL and 'ptr' points to it 
		{
			printf("Here11\n");
			// go through the neighbor list of '3' to find '1'
			NeighborGateway *ptr3 = search_node_NL(ptr, nl.addr);
			printf("Here12\n");
			if(ptr3 == NULL)		// '1' was not found in the NeighborGateway list of '3'
			{
				printf("Here13\n");
				free_data_structures();
				printf("Here14\n");
				ptr3 = add_node_to_NL(ptr, nl.addr, &n);	// add '1' to the NeighborGateway list of '3'
				printf("Here15\n");
				retValue = TRUE;
			} //end if
			else				// '1' was found in the NeighborGateway list of '3' and 'ptr3' points to it
			{
				printf("Here16\n");
				set_ngb_gw(ptr3, &n);	// update the control variables of NeighborGateway '1'
				printf("Here17\n");
			}
		} // end else
	} // end for
	remove_node_VL(nl.addr);			// look at the refreshFlag values in each node and remove if necessary
	printf("Here18\r\n");
	// now check whether any SNs need to be removed
	first = NULL;
	second = top_mgr.head;
	while(second != NULL)
	{
		first = second;				// candidate node for removal
		second = second -> next;	// advance the traversing pointer
		if(first -> count == 0)		// this node needs to be removed
		{
			printf("Here19\r\n");
			refresh_VL(first -> addr);		// mark all the flags as FALSE again
			printf("Here20\r\n");
			remove_node_VL(first -> addr);	// remove all occurrences of this node in the VL and in the NL of other SNs
			printf("Here21\r\n");
			free_data_structures();
			printf("Here22\r\n");
			remove_node_SNL(first);			// finally remove this node from the SNL
			printf("Here23\r\n");
		} // end if
	} // end while
	
	if(DEBUG_NG == 0)
		printf("NG: Left process_topology_desc()\r\n");
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
	//node -> count = 0;						// number of NeighborGateway nodes
	node -> count = -1;							// NOTE: indicates a newly added SN
	node -> next = NULL;						// pointers for SNL creation
	node -> prev = NULL;						// pointers for SNL creation
	node -> nbest = NULL;
	node -> nsbest = NULL;
	node -> nworst = NULL;
	//node -> last_seq_no = 0;
	node -> nw_info_acquired = 0;				// not technically needed. Done only for completeness
	node -> node_info_acquired = 0;
	
	node -> total_pkts_inserted = 0;
	node -> total_wait_time = 0;
	node -> total_ack_time = 0;
	node -> totalbytesSent = 0;
	node -> totalbytesReceived = 0;
	node -> totalactiveTime = 0;
		
	initialise_node_state_gateway(&(node -> nsqg));
	initialise_routing_table_gateway(&(node -> rt));
		
	if(DEBUG_NG == 2)
		printf("TDS: Leaving create_node_SNL()\r\n");
	return node;
}
/**********************************************************************************************/
SensorNode* search_node_SNL(uint16_t addr)	// C
{
	SensorNode *ptr;
	
	if(DEBUG_NG == 2)
		printf("TDS: Entered search_node_SNL()\r\n");
		
	pthread_mutex_lock(&data_mutex);
	ptr =  htsnl[addr].ptr;		// if the node does not exist, the hash table entry will be NULL
	pthread_mutex_unlock(&data_mutex);
	
	if(DEBUG_NG == 2)
		printf("TDS: Leaving search_node_SNL()\r\n");
	
	return ptr;
}
/**********************************************************************************************/
SensorNode* add_node_to_SNL(uint16_t addr)	// C
{
	SensorNode *node;					// scratch variables
	SensorNode *ptr;
	char path[PATHNAME_MAX_SIZE];		// temporary buffer
	
	if(DEBUG_NG == 1)
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
			node -> prev = top_mgr.tail;
			top_mgr.tail = node;
		}
		
		top_mgr.count++;
		htsnl[addr].ptr = node;			// assign the hash table entry to this node
		
		sprintf(path, "%s/Node_%u", WEB_SERVER_ROOT, addr);
		if(mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH) == -1)	// try to create the directory
			if(errno !=  EEXIST)
			{
				perror("mkdir(): ");
				exit(1);
			}
		
		if(DEBUG_NG == 1)
			printf("TDS: Leaving add_node_to_SNL()\r\n");
		return node;
	}
	
	// if the code reaches this point, it means the node was found in the list
	if(DEBUG_NG == 1)
		printf("TDS: Leaving add_node_to_SNL()\r\n");
	return ptr;
}
/**********************************************************************************************/
void remove_node_SNL(SensorNode *snptr)
{
	char path[PATHNAME_MAX_SIZE];
	char cmd[BUFF_SIZE];
	
	// create the path to the data file for this node
	sprintf(cmd, "rm -rf %s/Node_%u/Node_%u.txt", WEB_SERVER_ROOT, snptr -> addr, snptr -> addr);
	system(cmd);	
	
	if(snptr -> next == NULL)	// the last node is to be removed
	{
		if(snptr -> prev == NULL) // this is also the first (and only) node in the SNL
			top_mgr.head = top_mgr.tail = NULL;
		else					 // at least one node exists above this one
		{
			snptr -> prev -> next = NULL;
			top_mgr.tail = snptr -> prev;   // move the tail pointer
		}
	}
	else if(snptr -> prev == NULL)	// this is the first node to be removed
		 {
		 	if(snptr -> next == NULL)	// this is also the last (and only) node in the SNL
		 		top_mgr.head = top_mgr.tail = NULL;
			else					// at least one node exists below this one
		 	{
		 		snptr -> next -> prev = NULL;		// assign the next pointer 
		 		top_mgr.head = snptr -> next;		// move the head pointer
		 	}
		 }
		 else			// a 'middle' node is to be removed
		 {
		 	snptr -> prev -> next = snptr -> next;
		 	snptr -> next -> prev = snptr -> prev;
		 }
	top_mgr.count--;					// decrement number of nodes in the SNL
	htsnl[snptr -> addr].ptr = NULL;	// mark the hash table entry as NULL
	free(snptr);						// free this node	
		
	return;
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
		if(snptr -> count == 0)				// NOTE: This is the first NL node to be added
			snptr -> count++;				// increment once more
			
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
	
	if(DEBUG_NG >= 0)
		printf("NG: Entered free_data_structures()\n");
	
	if(Map != NULL)
	{
		free(Map);
		Map = NULL;
		
	}
	if(edge != NULL)
	{
		for(i = 0; i <= top_mgr.count; i++)
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
		for(i = 0; i <= top_mgr.count; i++)
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
		for(i = 0; i <= top_mgr.count; i++)
			if(cost[i] != NULL)
			{
				free(cost[i]);
				cost[i] = NULL;
			}
		free(cost);
		cost = NULL;
	}
	
	if(DEBUG_NG >= 0)
		printf("NG: Leaving free_data_structures()\n");
		
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
		printf("NG: generate_routing_tables(): Entered\r\n");
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
	
	edge[0] = NULL;
	parent[0] = NULL;
	cost[0] = NULL;
	
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
			
			//cost[row][column] = ngptr -> rssi;				// only for debugging
			
		} // end for
	} // end for
	
	if(DEBUG_NG >= 0)
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
	
	if(DEBUG_NG == 0)
		printf("NG: generate_routing_tables(): Leaving\r\n");	
	
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
int8_t build_Msg_NwInfoAcquired(Msg_NwInfoAcquired *m)
{
	int8_t i;
	SensorNode *snptr;
	int8_t ret;
	
	ret = 0;	
	pthread_mutex_lock(&data_mutex);
	initialise_Msg_NwInfoAcquired(m);
	for(snptr = top_mgr.head, i = 0; snptr != NULL; snptr = snptr -> next, i++)
	{
		if(snptr -> nw_info_acquired == 1)	// need to send an ACK back to this node
		{
			ret = 1;
			m -> addrs[i] = snptr -> addr;
			snptr -> nw_info_acquired = 0;	// reset the flag
		}
		//printf("Inserted address %d\n", m -> addrs[i]);
	}
	if(ret == 0)
		mnwia_seq_no--;
		
	pthread_mutex_unlock(&data_mutex);
	
	return ret;
}	
/**********************************************************************************************/
int8_t build_Msg_NodeInfoAcquired(Msg_NodeInfoAcquired *m)
{
	int8_t i;
	SensorNode *snptr;
	int8_t ret;
	
	ret = 0;	// assume initially no ACK message is to be sent
	pthread_mutex_lock(&data_mutex);
	//printf("NG: build_Msg_NwInfoAcquired(): Building this packet\n");
	initialise_Msg_NodeInfoAcquired(m);
	for(snptr = top_mgr.head, i = 0; snptr != NULL; snptr = snptr -> next, i++)
	{
		if(snptr -> node_info_acquired == 1)	// need to send ACK back to this node
		{
			ret = 1;							// need to send the ACK message
			m -> addrs[i] = snptr -> addr;
			snptr -> node_info_acquired = 0;	// reset the flag
		}
		//printf("Inserted address %d\n", m -> addrs[i]);
	}
	if(ret == 0)	// no need to send message
		mnia_seq_no--;
		
	pthread_mutex_unlock(&data_mutex);
	
	return ret;
}
/**********************************************************************************************/
void send_dummy_pkt()
{
	int8_t ret;					// to hold the return value of function calls
	static int8_t count1 = 1;	// temporary variable for debugging purposes
	GatewayToNodeSerial_Packet gtn_pkt;
		
	// send the dummy packet. Keep trying till successful
	/*do
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
	*/
	
	gtn_pkt.type = SERIAL_APPLICATION;
	gtn_pkt.length = 1;
	gtn_pkt.data[0] = count1;
	ret = sendOverSerial(&gtn_pkt, SERIAL_APPLICATION);
	if(ret == -1)
	{
		perror("NG: send_dummy_pkt(): Error in sending message to Firefly\r\n");
		//exit(1);
		return;
	}
	printf("NG: send_dummy_pkt(): Sent dummy message to the firefly: %d\r\n", count1);
	count1 = (count1 + 1) % 120;
	
	return;
}
/*************************************************************************************************/
void start_collection_phase()
{
	int64_t start_time, end_time;					 // to mark the start and end of the collection period 
	NodeToGatewaySerial_Packet ntg_pkt;				 // to hold a packet received from the attached node
	uint8_t rx_buf[SIZE_NODETOGATEWAYSERIAL_PACKET]; // to hold a message from the attached Firefly
	int8_t ret;										 // holds the return value of function calls
	int8_t count;
	
	//set the timers
	start_time = time(NULL);
	end_time = start_time;
	
	//printf("NG: start_collection_phase(): Collecting topology information. Please wait..");
	
	// collect data for COLLECTION_PERIOD  		
	while( end_time - start_time < COLLECTION_PERIOD )
	//while(1) // debugging	 
	{
		pthread_mutex_lock(&serial_mutex);
		// receive a packet from the attached node
		ret = receiveFromSerial(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET, RX_TIMEOUT);
		pthread_mutex_unlock(&serial_mutex);
		
		if(ret == 0)	// no data received
		{
			end_time = time(NULL);	// update the end time
			continue;
		}		
		/*if(ret != SIZE_NODETOGATEWAYSERIAL_PACKET)	// wrong length packet received
		{
			printf("NG: start_collection_phase(): Shorter length packet received: %d\r\n", ret);
			exit(1);
		}*/
		
		if(ret < SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER)	// wrong length packet received
		{
			printf("NG: start_collection_phase(): Shorter length packet received: %d\r\n", ret);
			exit(1);
		}			
		// unpack the packet
		unpack_NodeToGatewaySerial_Packet_header(&ntg_pkt, rx_buf);
		if(ret != (SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER + ntg_pkt.length) )
		{
			printf("NG: start_collection_phase(): Shorter length packet received2: %d %x\r\n", ret, ntg_pkt.type);
			
			//exit(1);
		}
		//memcpy(ntg_pkt.data, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, MAX_SERIAL_PAYLOAD);
		memcpy(ntg_pkt.data, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.length);
													
		// what kind of packet is it 
		switch(serial_pkt_type(&ntg_pkt))
		{
			case SERIAL_APPLICATION:
				process_serial_app_pkt(&ntg_pkt);
				break;
					
			case SERIAL_NW_CONTROL:
				process_serial_nw_ctrl_pkt(&ntg_pkt);		// ignore the return value here 	
				break;
				
			default:
				// drop the packet and go receive another one 
				printf("NG: start_collection_phase(): Invalid packet type received = %x\n", ntg_pkt.type);	 
		}
		
		// update the end time 
		end_time = time(NULL);
	} // end while(COLLECTION_PERIOD)
	
	return;
} // end function
/*************************************************************************************************/
void start_listening_phase()
{
	int8_t ret;							// to hold the return value of various function calls
	NodeToGatewaySerial_Packet ntg_pkt;	// to hold a packet sent by the attached node
	uint8_t rx_buf[SIZE_NODETOGATEWAYSERIAL_PACKET];
	int64_t st, et;					 // to mark the start and end of the check period 
		
	printf("NG: start_listening_phase(): Begin.....\r\n");
	st = time(NULL);
	et = st;
	
	while(1)		// listen to attached node forever	 
	{
		if(et - st >= NW_CONTROL_CHECK_PERIOD)
		{
			send_nw_ctrl_ACKS();
			st = et;					// reset the timer
		}		
		pthread_mutex_lock(&serial_mutex);
		// receive a packet from the attached node
		ret = receiveFromSerial(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET, RX_TIMEOUT);
		pthread_mutex_unlock(&serial_mutex);
		
		if(ret == 0)	
		{
			et = time(NULL);
			continue; // do nothing
		}	
		
		/*if(ret != SIZE_NODETOGATEWAYSERIAL_PACKET)	// wrong length packet received
		{
			printf("NG: start_listening_phase(): Shorter length packet received: %d\r\n", ret);
			exit(1);
		}
		*/
		
		if(ret < SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER)	// wrong length packet received
		{
			printf("NG: start_collection_phase(): Shorter length packet received: %d\r\n", ret);
			exit(1);
		}	
		// unpack the packet
		unpack_NodeToGatewaySerial_Packet_header(&ntg_pkt, rx_buf);
		//memcpy(ntg_pkt.data, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, MAX_SERIAL_PAYLOAD);
		memcpy(ntg_pkt.data, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.length);
									
		// what kind of packet is it 
		switch(serial_pkt_type(&ntg_pkt))
		{
			case SERIAL_APPLICATION:
				process_serial_app_pkt(&ntg_pkt);
				break;
					
			case SERIAL_NW_CONTROL:
				process_serial_nw_ctrl_pkt(&ntg_pkt); 
				break;
				
			default:
				// drop the packet and go receive another one 
				printf("NG: start_listening_phase(): Invalid packet type received = %d\n", ntg_pkt.type);	 
				break;
		} // end switch
		
		et = time(NULL);
	} // end outer while(1)

} // end function
/**********************************************************************************************/
int main()
{
	Msg_SendNwInfo msnwi;						// to hold a message of type Msg_SendNwInfo;
	Msg_SendNodeInfo msni;						// to hold a message of type Msg_SendNodeInfo
	Msg_NwInfoAcquired mnwia;					// to hold a message
	Msg_NodeInfoAcquired mnia;		            // to hold a message
	
	char gw_addr[SIZE_IP_ADDRESS];				// temporary variable to hold the IP address of the gateway
	int8_t ret;									// holds the return value of function calls
	uint32_t count = 3, i;						// temporary variables
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
	
	/*
	// construct and send a SERIAL_SEND_NW_INFO packet into the network
	addrs[0] = BCAST_ADDR;
	build_Msg_SendNwInfo(&msnwi, addrs, 1);
	printf("NG: main(): Sending SERIAL_SEND_NW_INFO: %u\r\n", msnwi.seq_no);
	ret = sendOverSerial(&msnwi, SERIAL_SEND_NW_INFO);
	if(ret == -1)
	{
		perror("NG: main()1: Error while sending data to Firefly");
		//exit(1);
	}
	// go into COLLECTION PHASE
	start_collection_phase();	// start collecting topology information
	print_SNL();
	generate_routing_tables(); // generate the topology description file and routing tables for each node
	
	// tell the network to stop sending NGB_LIST messages. Send a NW_INFO_ACQUIRED message
	build_Msg_NwInfoAcquired(&mnwia);
	printf("NG: main(): Sending SERIAL_NW_INFO_ACQUIRED: %u\r\n", mnwia.seq_no);
	ret = sendOverSerial(&mnwia, SERIAL_NW_INFO_ACQUIRED);
	if(ret == -1)
	{
		perror("NG: main()2: Error in sending data to Firefly\r\n");
		//exit(1);
	}
	
	// construct and send a SERIAL_SEND_NODE_INFO packet to the network
	addrs[0] = BCAST_ADDR;		// require each node's state information
	build_Msg_SendNodeInfo(&msni, addrs, 1);
	printf("NG: main(): Sending SERIAL_SEND_NODE_INFO: %u\r\n", msni.seq_no);
	ret = sendOverSerial(&msni, SERIAL_SEND_NODE_INFO);
	if(ret == -1)
	{
		perror("NG: main()3: Error while sending data to Firefly");
		//exit(1);
	}
	// go into COLLECTION PHASE
	start_collection_phase();	// start collecting node state information
	
	// tell the network to stop sending NODE_INFO messages. Send a NODE_INFO_ACQUIRED message
	build_Msg_NodeInfoAcquired(&mnia);
	printf("NG: main(): Sending SERIAL_NODE_INFO_ACQUIRED: %u\r\n", mnia.seq_no);
	ret = sendOverSerial(&mnia, SERIAL_NODE_INFO_ACQUIRED);
	if(ret == -1)
	{
		perror("NG: main()4: Error in sending data to Firefly\r\n");
		//exit(1);
	}
	
	*/
	
	do {	// for debugging
	// construct and send a SERIAL_SEND_NW_INFO packet to the network
	addrs[0] = BCAST_ADDR;		// require each node's NW information
	build_Msg_SendNwInfo(&msnwi, addrs, 1);
	printf("NG: main(): Sending SERIAL_SEND_NW_INFO\r\n");
	ret = sendOverSerial(&msnwi, SERIAL_SEND_NW_INFO);
	if(ret == -1)
	{
		perror("NG: main()1: Error while sending data to Firefly");
		//exit(1);
	}
	// go into COLLECTION PHASE
	start_collection_phase();	// start collecting topology information
	print_SNL();
	generate_routing_tables(); // generate the topology description file and routing tables for each node
	
	
	// tell the network to stop sending NGB_LIST messages. Send a NW_INFO_ACQUIRED message
	build_Msg_NwInfoAcquired(&mnwia);
	printf("NG: Sending SERIAL_NW_INFO_ACQUIRED\r\n");
	ret = sendOverSerial(&mnwia, SERIAL_NW_INFO_ACQUIRED);
	if(ret == -1)
	{
		perror("NG: main()2: Error in sending data to Firefly\r\n");
		//exit(1);
	}
		
	for(i = 1; i <= 5; i++)
	{
		printf("NG: After sending NW_INFO related messages %d\r\n", i);
		sleep(1);
	}
	
	//do{
	// construct and send a SERIAL_SEND_NODE_INFO packet to the network
	addrs[0] = BCAST_ADDR;		// require each node's state information
	build_Msg_SendNodeInfo(&msni, addrs, 1);
	printf("NG: main(): Sending SERIAL_SEND_NODE_INFO\r\n");
	ret = sendOverSerial(&msni, SERIAL_SEND_NODE_INFO);
	if(ret == -1)
	{
		perror("NG: main()3: Error while sending data to Firefly");
		//exit(1);
	}
			
	// go into COLLECTION PHASE
	start_collection_phase();	// start collecting node state information
	
	// tell the network to stop sending NODE_INFO messages. Send a NODE_INFO_ACQUIRED message
	build_Msg_NodeInfoAcquired(&mnia);
	printf("NG: Sending SERIAL_NODE_INFO_ACQUIRED\r\n");
	ret = sendOverSerial(&mnia, SERIAL_NODE_INFO_ACQUIRED);
	if(ret == -1)
	{
		perror("NG: main()4: Error in sending data to Firefly\r\n");
		//exit(1);
	}
	
	for(i = 1; i <= 5; i++)
	{
		printf("NG: After sending NODE_INFO related messages %d\r\n", i);
		sleep(1);
	}
	
//	} while(1);	// inner

	printf("NG: Sleeping for 5 seconds\n");
	usleep(100);
	
	} while(1);	// for debugging
	
		
	// go into LISTENING PHASE
	// the code never leaves this function
	start_listening_phase();
	
	return 0;
} // end main()
/*************************************************************************************************/
void send_nw_ctrl_ACKS()
{
	Msg_NwInfoAcquired mnwia;					// to hold a message
	Msg_NodeInfoAcquired mnia;		            // to hold a message
	int8_t ret;
	
	// tell the network to stop sending NGB_LIST messages. Send a NW_INFO_ACQUIRED message
	ret = build_Msg_NwInfoAcquired(&mnwia);
	if(ret == 1)	// need to send the ACK message
	{
		printf("NG: send_nw_ctrl_ACKS(): Sending SERIAL_NW_INFO_ACQUIRED: %u\r\n", mnwia.seq_no);
		ret = sendOverSerial(&mnwia, SERIAL_NW_INFO_ACQUIRED);
		if(ret == -1)
		{
			perror("NG: send_nw_ctrl_ACKS()1: Error in sending data to Firefly\r\n");
			//exit(1);
		}
	}
		
	// tell the network to stop sending NODE_INFO messages. Send a NODE_INFO_ACQUIRED message
	ret = build_Msg_NodeInfoAcquired(&mnia);
	if(ret == 1)
	{
		printf("NG: send_nw_ctrl_ACKS(): Sending SERIAL_NODE_INFO_ACQUIRED: %u\r\n", mnia.seq_no);
		ret = sendOverSerial(&mnia, SERIAL_NODE_INFO_ACQUIRED);
		if(ret == -1)
		{
			perror("NG: send_nw_ctrl_ACKS()2: Error in sending data to Firefly\r\n");
			//exit(1);
		}
	}
	return;
}

/*************************************************************************************************/
void initialise_Msg_RouteReply(Msg_RouteReply *m)
{
	int8_t i;
	
	m -> dest = INVALID_ADDR;
	m -> dg = INVALID_ADDR;
	m -> seq_no = ++mrr_seq_no;
	
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
	nsqg -> last_timestamp = 0;
	return;
}
/**************************************************************************************************/
int8_t add_to_node_state_gateway(NodeStateQueueGateway *nsqg, NodeState *ns)
{	
	int8_t i, j;					// loop variables
	int8_t rear = nsqg -> rear;
	int8_t front = nsqg -> front;
	
	if(DEBUG_NG ==  1)
		printf("NG: entered add_to_node_state_gateway()\r\n");
		
	if( (rear + 1) % MAX_STATE_VALUES_GATEWAY == front )// node's state queue is full
		return -1;										// addition not possible
	if(ns -> timestamp < nsqg -> last_timestamp)	// old entry. Will never be written to file. No point in adding to NodeStateQueueGateway
		return 0;
			
	// now add the new entry
	for(i = front; ; i = (i + 1) % MAX_STATE_VALUES_GATEWAY)
	{
		if(i == rear)											// reached the end of the circular queue
			break;
		if(nsqg -> ns[i].timestamp > ns -> timestamp)	// check whether current entry in list is more recent
			break;
	}
	
	// check when we came out
	if(i == rear)	// reached end of circular queue
		nsqg -> ns[rear] = *ns;						// add the NodeState object into the queue
	else // need to shift elements
	{
		if(rear == 0)	// the last element in the queue is at the last index in the array
			j = MAX_STATE_VALUES_GATEWAY - 1;
		else
			j = rear - 1;
		
		for( ; j != i; j = ( (j == 0) ? (MAX_STATE_VALUES_GATEWAY - 1) : j - 1 ) ) 
			nsqg -> ns[(j + 1) % MAX_STATE_VALUES_GATEWAY] = nsqg -> ns[j];
		nsqg -> ns[(j + 1) % MAX_STATE_VALUES_GATEWAY] = nsqg -> ns[j];	// do it one more time
		
		nsqg -> ns[j] = *ns;			// insert the new element in the correct place
	}
	// finally adjust the rear pointer
	nsqg -> rear = (nsqg -> rear + 1) % MAX_STATE_VALUES_GATEWAY;
	
	if(DEBUG_NG == 1)
		printf("NG: leaving add_to_node_state_gateway()\r\n");
	
	return 0;													// addition successful
}	
/**********************************************************************************************/
int8_t remove_from_node_state_gateway(NodeStateQueueGateway *nsqg, NodeState *ns)
{
	if(nsqg -> front == nsqg -> rear)	// queue is empty
		return -1;						// removal not possible
			
	*ns = nsqg -> ns[nsqg -> front];	// retrieve the NodeState element at the head of the queue
	nsqg -> front = (nsqg -> front + 1) % MAX_STATE_VALUES_GATEWAY;
	
	return 0;							// removal successful
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
//void writeNSQToFile(uint16_t addr, NodeStateQueueGateway *nsqg)
void writeNSQToFile(uint16_t addr, SensorNode *snptr)
{
	double E_TX, E_RX, E_CPU;
	double LIFETIME;
	
	FILE *fp;
	char path[PATHNAME_MAX_SIZE];   // temporary buffer
	NodeState ns;
		
	if(DEBUG_NG == 1)
		printf("NG: entered WriteNSQToFile()\r\n");
	
	create_NodePath(path, addr, 0);		// create the entire path name to file 
	printf("NG: writeNSToFile(): Path name = %s\r\n", path); 
	
	fp = (FILE*)fopen(path, "a");
	if(fp == NULL)
	{
		printf("NodeState file for %d could not be opened\r\n", addr);
		exit(1);
	}
	
	//while( remove_from_node_state_gateway(nsqg, &ns) == 0 )			// as long as queue has entries
	while( remove_from_node_state_gateway(&(snptr -> nsqg), &ns) == 0 )
	{
		(snptr -> nsqg).last_timestamp = ns.timestamp;
		LIFETIME = calcEnergy(snptr, &E_TX, &E_RX, &E_CPU);		
		//fprintf(fp, "%u\t%2f\t%d\t%d\t%4f\t%4f\n", ns.timestamp, bat, ns.tx_queue_size, ns.rx_queue_size, (float)ns.total_wait_time / ns.total_pkts_inserted, (float)ns.total_ack_time / ns.total_pkts_inserted);
		//nsqg -> last_timestamp = ns.timestamp;						// update the last timestamp
		fprintf(fp, "%u\t%2f\t%d\t%d\t%4f\t%4f\n", ns.timestamp, ns.battery/100.0, ns.tx_queue_size, ns.rx_queue_size, (double)(snptr -> total_wait_time) / (snptr -> total_pkts_inserted), (double)(snptr -> total_ack_time) / (snptr -> total_pkts_inserted));
	}
	
	fclose(fp);														// close the file
	
	if(DEBUG_NG == 1)
		printf("NG: leaving WriteNSQToFile()\r\n");
	
	return;
}	
/***************************************************************************************************/
double calcEnergy(SensorNode *snptr, double *E_TX, double *E_RX, double *E_CPU)
{
	double I_TX = 0;
	double I_RX = 0; 
	double I_CPU = 0;
	double V = 3.3;
	double R = DATA_RATE * 1024 / 8.0;
	double E_BMAC_RX;
	double E_BATTERY;
	double E_TOT;
	uint32_t TBS = snptr -> totalbytesSent;
	uint32_t TBR = snptr -> totalbytesReceived;
	uint32_t end_time;
	double LIFETIME;
	
	E_BATTERY = 3 * 60 * 60 * V;
	*E_TX = V * I_TX * TBS / R;
	*E_RX = V * I_RX * TBS / R;
	E_BMAC_RX = V * I_RX * 150E-6 / 100E-3 * (snptr -> nsqg).last_timestamp;
	//E_BMAC_TX = 
	*E_CPU = V * I_CPU * snptr -> totalactiveTime;
	
	E_TOT = *E_TX + *E_RX + E_BMAC_RX + *E_CPU;
	
	end_time = E_TOT * (snptr -> nsqg).last_timestamp;
	LIFETIME = end_time - (snptr -> nsqg).last_timestamp;
	LIFETIME = LIFETIME / 60 / 60 / 24;		// convert to days
	
	return LIFETIME;
}
	
/***************************************************************************************************/
int8_t sendOverSerial(void *msg, uint8_t type)
{
	// this function packs the passed message into a GatewayToNodeSerial_Packet
	Msg_RouteReply *mrr;
	Msg_SendNwInfo *msnwi;
	Msg_NwInfoAcquired *mnwia;
	Msg_SendNodeInfo *msni;
	Msg_NodeInfoAcquired *mnia;
	GatewayToNodeSerial_Packet gtn_pkt;
	int8_t ret;
	
	pthread_mutex_lock(&serial_mutex);	// to protect access to serial port
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
			
		case SERIAL_APPLICATION:		// required for now for the initial dummy packet
			gtn_pkt = *((GatewayToNodeSerial_Packet*)(msg));
			break;
	}
	ret = sendToSensorNode(&gtn_pkt);
	pthread_mutex_unlock(&serial_mutex);
	return ret;
}	
/***************************************************************************************************/
int8_t sendToSensorNode(GatewayToNodeSerial_Packet *pkt)
{
	// this function reliably sends the serial packet to the Firefly
	char tx_buf[SIZE_GATEWAYTONODESERIAL_PACKET];	// transmit buffer to send data to attached Firefly
	char rx_buf[SIZE_NODETOGATEWAYSERIAL_PACKET];	// receive buffer to receive data from Firefly
	NodeToGatewaySerial_Packet ntg_pkt;
	int8_t ret;										// return type of function calls
	int8_t ack_rcvd;								// needed for flow control
	int8_t num_retries;								// to hold number of retries
	int64_t st, et;									// needed to maintain timeout
	
	
	pack_GatewayToNodeSerial_Packet_header(tx_buf, pkt);
	//memcpy(tx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, pkt -> data, MAX_GATEWAY_PAYLOAD);
	memcpy(tx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, pkt -> data, pkt -> length);
	
	ack_rcvd = 0;		// initialise
	num_retries = -1;
	
	while( (num_retries < NUM_SERIAL_RETRIES) && (ack_rcvd == 0) )
	{
		num_retries++;
		//ret = slipstream_send(tx_buf, (int)SIZE_GATEWAYTONODESERIAL_PACKET);
		ret = slipstream_send(tx_buf, SIZE_GATEWAYTONODESERIAL_PACKET_HEADER + pkt -> length); 
		if(ret == 0)	// problem in sending over UDP
		{
			perror("NG: sendToSensorNode(): Error in sending data to Firefly: ");
			exit(1);
		}
		
		st = time(NULL);	// reset the timeout period
		et = st;
		while(et - st < ACK_TIMEOUT)	// wait for ACK_TIMEOUT seconds. Need both timeouts here since
										// receiveFromSerial() could return other packets as well
		{
			ret = receiveFromSerial(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET, RX_TIMEOUT);	
			if(ret == 0)	// nothing received from Firefly
			{
				//printf("Nothing received from Firefly\r\n");
				et = time(NULL);	// update the end time for ACK wait
				continue;
			}
			
			/*if(ret != SIZE_NODETOGATEWAYSERIAL_PACKET)	// wrong length packet received
			{
				printf("NG: sendToSensorNode(): Wrong length packet received: %d\r\n", ret);
				exit(1);
			}
			*/
			
			if(ret < SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER)	// wrong length packet received
			{
				printf("NG: sendToSensorNode(): Wrong length packet received: %d\r\n", ret);
				exit(1);
			}
						
			unpack_NodeToGatewaySerial_Packet_header(&ntg_pkt, rx_buf);
			//memcpy(ntg_pkt.data, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, MAX_SERIAL_PAYLOAD);
			memcpy(ntg_pkt.data, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.length);
			
			// what kind of packet is it 
			switch(serial_pkt_type(&ntg_pkt))
			{
				case SERIAL_APPLICATION:
					process_serial_app_pkt(&ntg_pkt);
					break;
					
				case SERIAL_NW_CONTROL:
					process_serial_nw_ctrl_pkt(&ntg_pkt);		// ignore the return value here 	
					break;
				
				case SERIAL_ACK:
					if(DEBUG_NG >= 1)
						printf("NG: sendToSensorNode(): ACK received\r\n");
					ack_rcvd = 1;
					break;
				
				default:
					// drop the packet and go receive another one 
					printf("NG: sendToSensorNode(): Invalid packet type received = %x\n", ntg_pkt.type);	 
			} // end switch
		
			if(ack_rcvd == 1)
				break;	// packet sent reliably. Break out of inner loop
			
			et = time(NULL);
		} // end inner while
	} // end outer while
	
	if(ack_rcvd == 1)		// successful transmission
		return 0;
	return -1;				// unsuccessful
}	
/***************************************************************************************************/
/********************************** PRINTING FUNCTIONS *********************************************/
void print_SNL()
{
	SensorNode *snptr; 
	NeighborGateway *ngptr;
	
	pthread_mutex_lock(&data_mutex);	
	
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
	pthread_mutex_unlock(&data_mutex);
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
		printf("%u %d %d %d ", snptr -> nsqg.ns[index].timestamp, snptr -> nsqg.ns[index].battery, snptr -> nsqg.ns[index].tx_queue_size, snptr -> nsqg.ns[index].rx_queue_size);
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
	
	pthread_mutex_lock(&data_mutex);
	
	rt = htsnl[addr].ptr -> rt;
	printf("Routing Table for %d:\n", addr);
	printf("Dest\tNextHop\tCost\n");
	for(i = 0; i < MAX_SUBNET_SIZE; i++)
	{
		if(rt.rte[i].dest == INVALID_ADDR)	// ignore invalid destination
			continue;
			
		printf("%u\t", rt.rte[i].dest);
		
		if(rt.rte[i].nextHop == INVALID_ADDR)
			printf("INV\t");
		else
			printf("%u\t", rt.rte[i].nextHop);
					
		if(rt.rte[i].cost == INFINITY)
			printf("INF\r\n");
		else
			printf("%d\r\n", rt.rte[i].cost);
	}
	
	pthread_mutex_unlock(&data_mutex);
	return;
}	
/*************************************************************************************************/
void print_NodeState(NodeState *ns)
{
	printf("%lu %u %d %d %u %u %u\r\n", ns -> timestamp, ns -> battery, ns -> tx_queue_size, ns -> rx_queue_size, ns -> total_pkts_inserted, ns -> total_wait_time, ns -> total_ack_time);
	
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
	
	pthread_mutex_lock(&data_mutex);	
	
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
	
	pthread_mutex_unlock(&data_mutex);	
	return;
}
/**************************************************************************************************/
