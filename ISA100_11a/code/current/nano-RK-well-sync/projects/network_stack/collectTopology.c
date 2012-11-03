#include "Flood.h"
#include <stdio.h>	 		//standard I/O routines.              
#include <stdlib.h>      	// rand() and srand() functions       
#include <unistd.h>			// fork(), etc.                        
#include <time.h>				// nanosleep(), etc.                   
#include <sys/types.h>   	// various type definitions.           
#include <sys/ipc.h>     	// general SysV IPC structures         
#include <sys/sem.h>			// semaphore functions and structs.    
#include <sys/wait.h>	 	// wait(), etc. 

#define SEM_ID 250                       

#define COLLECTION_PERIOD 6
#define DEBUG 1

/* SLIP character codes */
	#define END             0300    /* indicates end of packet */
   #define ESC             0333    /* indicates byte stuffing */
   #define ESC_END         0334    /* ESC ESC_END means END data byte */
   #define ESC_ESC         0335    /* ESC ESC_ESC means ESC data byte */

/****************************************************************************************/
void printBuffer(uint8_t *buf, uint8_t len)
{
	while(len > 0)
	{
		printf("%d ", *buf);
		buf++;
		len--;
	}
	printf("\n\n");
	return;
}
/****************************************************************************************/
int receiveFromSerial(uint8_t *p, uint8_t len)
{
	uint8_t c;								// to read a character from the serial port 
	uint8_t received = 0;				// holds the number of characters read in all 
	
	FILE *fromNode = (FILE*)fopen("/dev/ttyUSB0", "r");
	if(fromNode == NULL)
	{
		printf("Failed to open the serial port for reading\n");
		exit(1);
	}
	
	if(DEBUG == 2)
		printf("Inside receiveFromSerial()\n");
		
	// clean up the junk characters in serial port 
	//while(fgetc(fromNode) != EOF);
	
   while(1)
	{
     // get a character to process 
	  c = fgetc(fromNode);
	  if(DEBUG == 2)
	  		printf("%d ", c);
	
	  // handle bytestuffing if necessary
     switch(c)
	  {

        /* if it's an END character then we're done with
        * the packet
        */
        case END:
            if(received > 0)		// we received the packet
            {
					fclose(fromNode);
					return received;
				}
            else						// beginning synchronizing END
              break;

        /* if it's the same code as an ESC character, wait
        * and get another character and then figure out
        * what to store in the packet based on that.
        */
        case ESC:
             c = fgetc(fromNode);
             
             /* if "c" is not one of these two, then we
              * have a protocol violation.  The best bet
              * seems to be to leave the byte alone and
              * just stuff it into the packet
             */
             switch(c) 
				 {
               case ESC_END:
               	c = END;
                  break;
               case ESC_ESC:
                  c = ESC;
                  break;
             }

             /* here we fall into the default handler and let
             * it store the character for us
             */
         default:
         	if(received < len)
            	p[received++] = c;
     } //end outer switch
  } // end infinite while
} // end receiveFromSerial()

/*******************************************************************************************/

void toString(uint16_t addr, int8_t *str)
{
	sprintf(str, "%d", addr);
}

/********************************************************************************************/
void add_link_to_topology_file(FILE *fp, uint16_t f, uint16_t t)
{
	char from[ADDR_LENGTH];
	char to[ADDR_LENGTH];
	 
	toString(f, from);
	toString(t, to);
										
	fprintf(fp, "\t%s -> %s;\n",from, to); 
	if(DEBUG >= 1)
		printf("\t%s -> %s;\n", from, to);
	
	/* 
	fprintf(fp, "\t%d -> %d;\n", f,t);
	if(DEBUG >= 1)
		printf("\t%d -> %d;\n", f, t);
	*/
		
	return;
}
/*************************************************************************************/

void begin_topology_file(FILE *fp)
{
	fputs("\ndigraph Topology\n{\nconcentrate=true;\n", fp);
	if(DEBUG >= 1)
		printf("digraph Topology\n{\nconcentrate=true;\n");
		
	return;
}
/*************************************************************************************/

void end_topology_file(FILE *fp)
{
	fputs("}\n", fp); // close the topology format
	if(DEBUG >= 1)
		printf("}\n");
		
	return;
}

/*************************************************************************************/
void generate_graph()
{
	// maintenance operations for testing
	remove("log.gif");
	remove("serverlog.gif");
	
	system("dot -Tgif serverlog.dot -o serverlog.gif");	// create the .gif file 
	//system("dot -Tgif log.dot -o log.gif"); 
	//system("firefox file:///home/ayb/nano-rk/projects/MyProjects/SensorTopology.html &");
			
	return;
}
/*************************************************************************************/		
void initialise(int sem_set_id)
{
	//union semun sem_val;							// semaphore value 
	
	if(endianness() == ERROR_ENDIAN)
	{
		printf("PANIC: Endianness error in main() of collectTopology\n");
		exit(1);
	}
	sem_set_id = semget(SEM_ID, 1, IPC_CREAT | 0600);	// create the semaphore 
	if(sem_set_id == -1)
	{
		printf("PANIC: error in creating a semaphore set\n");
		exit(1);
	}	
	
	//sem_val.val = 1;		// initialise the mutex to 1 
	//if( semctl(sem_set_id, 0, SETVAL, sem_val) == -1 )
	if( semctl(sem_set_id, 0, SETVAL, 1) == -1 )
	{
		printf("PANIC: error in setting semaphore value\n");
		exit(1);
	}
	
	return;
}

/*********************************************************************************************/
void catch_child(int sig_num)
{
    /* when we get here, we know there's a zombie child waiting */
    int child_status;

    wait(&child_status);
    //printf("child exited.\n");
}
	
/*********************************************************************************************/
int main()
{
	int64_t start_time, end_time;					// to record collection period of the data 
	NodeToGatewaySerial_Packet ntg_pkt;	// to hold the packet received from the node 
	uint8_t rx_buf[SIZE_NODETOGATEWAYSERIAL_PACKET];	// buffer to get the packet 
	int8_t i;										// loop index 
	int8_t sendNow;								// flag to indicate when the topology should be generated 
	int8_t pid;										// to store the process id of the forked child 
	FILE *toTopologyFile;						// pointer to topology file
	int sem_set_id;								// ID of the semaphore set
	int child_status;
	
	
	// maintenance operation for testing	
	if(remove("/home/ayb/nano-rk/projects/MyProjects/log.dot") != 0)
		if(DEBUG == 2)
			printf("File was not deleted\n");
	if(remove("/home/ayb/nano-rk/projects/MyProjects/serverlog.dot") != 0)
		if(DEBUG == 2)
			printf("File was not deleted\n");
	
	initialise(sem_set_id);
	signal(SIGCHLD, catch_child);		// define the signal handler for the CHLD signal

	if(DEBUG == 2)
		printf("Collection started\n");
	while(1) 		// collect information from attached node forever
	{
		toTopologyFile = fopen("/home/ayb/nano-rk/projects/MyProjects/log.dot", "w");
		if(toTopologyFile == NULL)
		{
			printf("Could not open topology file for writing\n");
			exit(1);
		}
		begin_topology_file(toTopologyFile);
		
		sendNow = FALSE;	// initialise sendNow 
		
		// get system time
		start_time = time(NULL);
		end_time = start_time;
		
		while(1)			// loop for collection period 
		{
			uint64_t elapsed_time = end_time - start_time;
			if(elapsed_time >= COLLECTION_PERIOD)	// stop collecting and send to server
				break;

			// retrieve in a buffer the packet from the node			
			receiveFromSerial(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
									
			if(DEBUG == 2)
			{	
				printf("\n\nExtracted rx_buf is\n");			
				printBuffer(rx_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
			}
							
			// unpack the packet header from the receive buffer
			unpack_NodeToGatewaySerial_Packet_header(&ntg_pkt, rx_buf);
			
			if(DEBUG == 2)
			{
				printf("Printing ntg_pkt header\n");
				printf("%d %d %d\n", ntg_pkt.packetType, ntg_pkt.packetSubType, ntg_pkt.length);
			}
									
			if(ntg_pkt.packetType == APPLICATION)
			{
				printf("Application layer packet received\n");
				continue;
			}
			else if(ntg_pkt.packetType == NW_CONTROL)
				  {
						NeighborList nlist;
						Msg_NgbList mnlist;
						
						switch(ntg_pkt.packetSubType)
						{
							case NGB_LIST:	// obtained a Msg_NgbList, construct the topology information
								
								// unpack the Msg_NgbList from the receive buffer						
								unpack_Msg_NgbList(&mnlist, rx_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER);
								nlist = mnlist.nl;
													 
								for(i = 0; i < MAX_NGBS; i++)
								{
									if(nlist.ngbs[i].addr != BCAST_ADDR)		// valid entry
									{
										if(nlist.ngbs[i].isNew == TRUE)			// still to be decided 
											sendNow = TRUE;							// still to be decided 
										
										add_link_to_topology_file(toTopologyFile, nlist.my_addr, nlist.ngbs[i].addr);
									}					
								 } // end for 
								 break;

							 default:;
							 
						 } // end switch
					} // end if  
					else
						printf("This should never happen\n");
										 
			end_time = time(NULL);	// update end time 
			//if(sendNow == TRUE)		// a new link has appeared, complete topology desc 
			//	break;		
		} // end inner while
		end_topology_file(toTopologyFile);		// end topology description 
		fclose(toTopologyFile);						// close the file 
		
		system("cp log.dot serverlog.dot");
		pid = fork();
		
		switch(pid)
		{
			case -1: 
				printf("PANIC: fork was unable to create a new process\n");
				exit(1);
				
			case 0: 	// this is the child process 
				generate_graph();
				exit(0);
				
			default:	// this is the parent process 
				break;	// go back and collect more data 		
		}
		//generate_graph(); 
		
	} // end outer while

} // end main

