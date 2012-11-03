/* This program implements the controlled flooding algorithm as part of the network layer
   As of now this program runs over the BMAC link layer protocol but this will be replaced soon
	with a generic network layer 
*/
#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <bmac.h>
#include <nrk_error.h>
#include <Flood.h>

#define FALSE 0
#define TRUE 1
#define WITH_SLIP FALSE 
#define DEBUG 0

/* SLIP character codes */
#define END             0300    /* indicates end of packet */
#define ESC             0333    /* indicates byte stuffing */
#define ESC_END         0334    /* ESC ESC_END means END data byte */
#define ESC_ESC         0335    /* ESC ESC_ESC means ESC data byte */

// Global data structures
NeighborList nl;					 					// to hold the neighbors of a node
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];			// receive buffer for network layer
uint8_t local_tx_buf[SIZE_NW_PACKET]; 			// to hold the packet to be multi-hopped
// to hold the packet to be sent to the gateway 
uint8_t local_to_gw_buf[SIZE_NODETOGATEWAYSERIAL_PACKET];

uint8_t tx_buf_HELLO_msg_task[SIZE_NW_PACKET];		// transmit buffer for HELLO msg 
uint8_t tx_buf_NGB_LIST_msg_task[SIZE_NW_PACKET];   // transmit buffer for NGB_LIST msg

NW_Packet pkt_rx_task;							// to hold a received packet
NW_Packet pkt_tx_HELLO_msg_task;				// to hold the HELLO msg
NW_Packet pkt_tx_NGB_LIST_msg_task;			// to hold the NGB_LIST msg
NodeToGatewaySerial_Packet ntg_pkt;			// to hold a packet for the gateway
uint8_t endian;									// to hold the endianness of the node


// definitions of the tasks and the stacks in the network layer
nrk_task_type RX_TASK;
NRK_STK rx_task_stack[NRK_APP_STACKSIZE];
void rx_task(void);

nrk_task_type TX_HELLO_MSG_TASK;
NRK_STK tx_HELLO_msg_task_stack[NRK_APP_STACKSIZE];
void tx_HELLO_msg_task(void);

nrk_task_type TX_NGB_LIST_MSG_TASK;
NRK_STK tx_NGB_LIST_msg_task_stack[NRK_APP_STACKSIZE];
void tx_NGB_LIST_msg_task(void);

/* Function prototypes */
void nrk_create_taskset();
uint8_t add_neighbor(Neighbor);
uint8_t endianness();

void pack_Neighbor(uint8_t *, Neighbor *);
void pack_NeighborList(uint8_t *, NeighborList*);
void pack_Msg_Hello(uint8_t *, Msg_Hello*);
void pack_Msg_NgbList(uint8_t *, Msg_NgbList*);
void pack_NW_Packet_header(uint8_t *, NW_Packet*);
void pack_NodeToGatewaySerial_Packet_header(uint8_t *, NodeToGatewaySerial_Packet*);

void unpack_Neighbor(Neighbor *, uint8_t *);
void unpack_NeighborList(NeighborList*, uint8_t *);
void unpack_Msg_Hello(Msg_Hello*, uint8_t *);
void unpack_Msg_NgbList(Msg_NgbList*, uint8_t *);
void unpack_NW_Packet_header(NW_Packet*, uint8_t *);
void unpack_NodeToGatewaySerial_Packet_header(NodeToGatewaySerial_Packet*, uint8_t *);

/********************************************************************************************/
uint8_t add_neighbor(Neighbor n)
{
	uint8_t i;					// loop index
	uint8_t found = FALSE;	// flag to indicate whether neighbor was found in array
	
	if(nl.count == MAX_NGBS)		// cannot store more than MAX_NGBS neighbors
		return MAX_NEIGHBOR_LIMIT_REACHED;

	/* first pass through array checks to see 
		1. if the neighbor had been recorded before
		2. decrements the lastReport (and possibly removes) of each neighbor
	*/
	for(i = 0; i < MAX_NGBS; i++)	
	{
		// search to see if this neighbor has been recorded before		
		if(nl.ngbs[i].addr == n.addr)
		{
			found = TRUE;
			// update last reported
			nl.ngbs[i].lastReport = TIMEOUT_COUNTER;
			nl.ngbs[i].isNew = FALSE;
			nl.ngbs[i].rssi = n.rssi;
			nrk_led_set(ORANGE_LED);
		}
		else if(nl.ngbs[i].addr != BCAST_ADDR)		// entry at this position is valid
      {
			nl.ngbs[i].lastReport--;					// decrement lastReport
			if(nl.ngbs[i].lastReport == 0)			// should I remove this neighbor?
			{
				nl.ngbs[i].addr = BCAST_ADDR;			// invalidate this entry
				nl.ngbs[i].rssi = 0;						// zero out the remaining two entries 
				nl.ngbs[i].isNew = FALSE;
				nl.count--;									// decrement number of ngbs recorded
			}
		}
	}	

	if(found == TRUE)			// neighbor was already present in array; do nothing further
		return NEIGHBOR_ALREADY_IN_LIST;
	
	/* second pass through array adds the new neighbor */
	for(i = 0; i < MAX_NGBS; i++)	
	{
		if(nl.ngbs[i].addr == BCAST_ADDR)		// this position in array holds an invalid entry
		{
			n.lastReport = TIMEOUT_COUNTER;		// set lastReport to timeout value
			n.isNew = TRUE;							// a new neighbor has reported 
			nl.ngbs[i] = n;							// place the new neighbor at this array index
			nl.count++;									// increment the number of neighbors recorded
			break;
		}
	}
	return NEIGHBOR_ADDED; 
}
/*********************************************************************************************/

int main ()
{
	// set up environment  
	/**************************** initialise() *********************************/
	uint8_t i;		// loop index

	// initialise the UART	
	nrk_setup_ports();  
	nrk_setup_uart(UART_BAUDRATE_115K2);
	
	// initialise the OS
	nrk_init();

	// clear all LEDs	
	// this will toggle when information about an already received neighbor is received again
	nrk_led_clr(ORANGE_LED);		
   nrk_led_clr(BLUE_LED);			// will toggle whenever a packet is sent
   nrk_led_clr(GREEN_LED);			// will toggle whenever a packet is received
   nrk_led_clr(RED_LED);			// will light up on error
	  
	// start the clock	
	nrk_time_set(0,0);

	// configure the bmac task
	bmac_task_config();
		
	// initialise the data structures
	nl.count = 0;
	nl.my_addr = NODE_ADDR;
	for(i = 0; i < MAX_NGBS; i++)	// an addr = BCAST_ADDR indicates an invalid entry
	   nl.ngbs[i].addr = BCAST_ADDR;
		
	// initialise a random number generator 
	srand(NODE_ADDR);
	
	// get the endianness of the node
	endian = endianness();
	if(endian == ERROR_ENDIAN)
	{
		printf("PANIC: Endianness unknown\n");
		nrk_led_set(RED_LED);
	}	
	/*************************************************************************************/
	 
	nrk_create_taskset();		// create the set of tasks
	nrk_start();					// start this node
	
	return 0;
}

/********************************************************************************************/
// This function can be changed to support different flooding algorithms
uint8_t shouldIMultihop(NW_Packet *pkt)
{
	if(pkt -> ttl == 0)
		return MULTIHOP_NO;	
	(pkt -> ttl)--;
	return MULTIHOP_YES;
}

/*********************************************************************************************/

void sendToSerial(uint8_t *buf, int length)
{
	/* send an initial END character to flush out any data that may
   * have accumulated in the receiver due to line noise
   */
   putchar(END);
   if(DEBUG == 2)
   	printf("%d ", END);

   /* for each byte in the packet, send the appropriate character
   * sequence 
   */
   while(length > 0)
	{
   	switch(*buf)
		 {
       		/* if it's the same code as an END character, we send a
            * special two character code so as not to make the
            * receiver think we sent an END
            */
            case END:
            	putchar(ESC);
               putchar(ESC_END);
               if(DEBUG == 2)
               	printf("%d %d ",ESC, ESC_END); 
               
               break;

             /* if it's the same code as an ESC character,
             * we send a special two character code so as not
             * to make the receiver think we sent an ESC
             */
             case ESC:
                putchar(ESC);
                putchar(ESC_ESC);
                if(DEBUG == 2)
                	printf("%d %d ",ESC, ESC_ESC);
                break;

             /* otherwise, we just send the character
             */
             default:
 	              putchar(*buf);
 	              if(DEBUG == 2)	
 	              	 printf("%d ", *buf);
        } // end switch
        buf++;
        length--;
   } // end while
   
	/* tell the receiver that we're done sending the packet */   
    putchar(END);
    if(DEBUG == 2)
    	printf("%d ", END);
	 return;
}

/**********************************************************************************************/
void printBuffer(uint8_t *buf, int len)
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
/**********************************************************************************************/
void sendToGateway(uint8_t *buf)
{
	//sendToSerial(local_to_gw_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
	printBuffer(local_to_gw_buf, SIZE_NODETOGATEWAYSERIAL_PACKET);
	
	return;
}
/**********************************************************************************************/
void unpack_Neighbor(Neighbor *n, uint8_t* src)
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(n -> addr)) ) = src[1];
		*( (uint8_t*)(&(n -> addr)) + 1 ) = src[0];
	}
	else // BIG ENDIAN 
	{
		*( (uint8_t*)(&(n -> addr)) ) = src[0];
		*( (uint8_t*)(&(n -> addr)) + 1 ) = src[1];
	}
	
	n -> rssi = src[2];
	n -> lastReport = src[3];
	n -> isNew = src[4];
	
	return;
}

void unpack_Msg_Hello(Msg_Hello *m, uint8_t* src)
{
	unpack_Neighbor(&(m -> n), src);
	return;
}
/**********************************************************************************************/

void unpack_NeighborList(NeighborList *nlist, uint8_t *src)
{
	Neighbor n;
	uint8_t i,j; 		// loop indices 
	
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(nlist -> my_addr)) ) = src[1];
		*( (uint8_t*)(&(nlist -> my_addr)) + 1 ) = src[0];
	}
	else
	{
		*( (uint8_t*)(&(nlist -> my_addr)) ) = src[0];
		*( (uint8_t*)(&(nlist -> my_addr)) + 1 ) = src[1];
	}
	
	for(i = 0,j = 2; i < MAX_NGBS; i++, j += SIZE_NEIGHBOR)
	{
		unpack_Neighbor(&n, src + j);
		nlist -> ngbs[i] = n;
	}
	nlist -> count = src[2+ MAX_NGBS * SIZE_NEIGHBOR];
		
	return;
}

void unpack_Msg_NgbList(Msg_NgbList *m, uint8_t *src)
{
	unpack_NeighborList(&(m -> nl), src);
	return;
}

/********************************************************************************************/
void unpack_NW_Packet_header(NW_Packet *pkt, uint8_t* src)
{
	if(endian == LITTLE_ENDIAN)
	{
		*( (uint8_t*)(&(pkt -> src)) ) = src[1];
		*( (uint8_t*)(&(pkt -> src)) + 1 ) = src[0];
		
		*( (uint8_t*)(&(pkt -> dest)) ) = src[3];
		*( (uint8_t*)(&(pkt -> dest)) + 1 ) = src[2];
		
		*( (uint8_t*)(&(pkt -> seq)) ) = src[5];
		*( (uint8_t*)(&(pkt -> seq)) + 1 ) = src[4];
		
	}
	else  // BIG_ENDIAN
	{
		*( (uint8_t*)(&(pkt -> src)) ) = src[0];
		*( (uint8_t*)(&(pkt -> src)) + 1 ) = src[1];
		
		*( (uint8_t*)(&(pkt -> dest)) ) = src[2];
		*( (uint8_t*)(&(pkt -> dest)) + 1 ) = src[3];
		
		*( (uint8_t*)(&(pkt -> seq)) ) = src[4];
		*( (uint8_t*)(&(pkt -> seq)) + 1 ) = src[5];
	}
	
	pkt -> ttl = src[6];
	pkt -> ackRequested = src[7];
	pkt -> packetType = src[8];
	pkt -> packetSubType = src[9];
	pkt -> length = src[10];
	
	return;
}
/*********************************************************************************************/
void unpack_NodeToGatewaySerial_Packet_header(NodeToGatewaySerial_Packet *pkt, uint8_t *src)
{
	pkt -> packetType = src[0];
	pkt -> packetSubType = src[1];
	pkt -> length = src[2];
	
	return;
}
/**********************************************************************************************/	
void rx_task()
{
	uint8_t len; 				   // to hold the size of the received packet, always = // sizeof(NW_Packet) 
	uint8_t rssi;  			   // to hold rssi of received packet
	uint8_t *local_rx_buf;	   // pointer to receive buffer of link layer
	Msg_Hello mh;			   	// to hold a HELLO message
	Msg_NgbList mnlist;			// to hold a NGB_LIST message 
	NeighborList nlist;			// to hold the actual NeighborList
	int8_t val;						// status variable to hold the return type of function calls

	if(DEBUG >= 1)
		printf("DEBUG: RX_TASK PID=%d\r\n",nrk_get_pid());

	// initialise the BMAC layer	
	bmac_init(25);
	bmac_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE);

	while(1)
	{
		if(DEBUG == 2)
			nrk_kprintf(PSTR("DEBUG: Waiting for packet in RX task\r\n"));

		// wait for the next packet
		do
		{
			val = bmac_wait_until_rx_packet();
		
		} while(val == NRK_ERROR);
		
		if(DEBUG == 2)
			nrk_kprintf(PSTR("DEBUG: Received a packet\r\n"));    
		
		nrk_led_set(GREEN_LED);			// shine the LED 
   	
		// Get the packet 
   	do
   	{
   		local_rx_buf = bmac_rx_pkt_get(&len,&rssi);
   		
   	} while(local_rx_buf == NULL);
   	
		// unpack the packet header from the received buffer 
		unpack_NW_Packet_header(&pkt_rx_task, local_rx_buf);
		
		if(pkt_rx_task.packetType == APPLICATION)				// application layer data
		{
			//sendUp(pkt.data, pkt.length); // pass it on to the application layer
			if(DEBUG == 2)
				printf("DEBUG: Application layer packet received. \r\n");
			continue;
		}
		
		// if the code reaches here, it means the packet is for the network layer
		switch(pkt_rx_task.packetSubType)
		{
			case HELLO:		// HELLO msg
				
				// unpack the Msg_Hello from the receive buffer 
				unpack_Msg_Hello(&mh, local_rx_buf + SIZE_NW_PACKET_HEADER);
				// Release the RX buffer quickly so future packets can arrive 
			   bmac_rx_pkt_release();
	   			
				mh.n.rssi = rssi;
				nrk_int_disable(); 
				add_neighbor(mh.n);		// ignore the return value here 
				nrk_int_enable();
				
				if(DEBUG >= 1)
					printf("DEBUG: Received HELLO msg from: %d with RSSI = %d\n", mh.n.addr, mh.n.rssi);
				break;

			case NGB_LIST:		// NGB_LIST msg
					// unpack the Msg_NgbList from the receive buffer						
					unpack_Msg_NgbList(&mnlist, local_rx_buf + SIZE_NW_PACKET_HEADER);
					// Release the RX buffer quickly so future packets can arrive 
	 				bmac_rx_pkt_release();
	   					
					nlist = mnlist.nl;
					if(DEBUG >= 0)
					{
						uint8_t i;	// loop index 
												
						printf("Received NGB_LIST msg from %d with count = %d\n", nlist.my_addr, nlist.count);
						for(i = 0; i < MAX_NGBS; i++)		
						{
							if(nlist.ngbs[i].addr != BCAST_ADDR)	// valid entry
								printf("%d, ", nlist.ngbs[i].addr);
						}
						printf("\r\n");
					}
										
					if(shouldIMultihop(&pkt_rx_task) == MULTIHOP_YES)	// multihop
					{
						// pack the modified network header into the transmit buffer
						pack_NW_Packet_header(local_tx_buf, &pkt_rx_task);
						// pack the NGB_LIST message into the buffer
						pack_Msg_NgbList(local_tx_buf + SIZE_NW_PACKET_HEADER, &mnlist);						
						while( bmac_tx_packet(local_tx_buf, SIZE_NW_PACKET) == NRK_ERROR)
  						{
   						nrk_event_wait (bmac_tx_pkt_done_signal);
   					}
					}
					
					// if(CONNECTED_TO_GATEWAY == TRUE)					
					if(NODE_ADDR == 1)
					{					
						// construct a packet to be sent to the gateway over the serial connection 
						ntg_pkt.packetType = NW_CONTROL;
						ntg_pkt.packetSubType = NGB_LIST;
						ntg_pkt.length = SIZE_MSG_NGB_LIST;
						
						// pack the message in the data field of the NodeToGatewaySerialPacket 
   					pack_Msg_NgbList(ntg_pkt.data, &mnlist);
   					// pack the NodeToGatewaySerial_Packet header into the serial transmit buffer 
   					pack_NodeToGatewaySerial_Packet_header(local_to_gw_buf, &ntg_pkt);
   					// append the actual message into the serial transmit buffer 
   					memcpy(local_to_gw_buf + SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER, ntg_pkt.data, SIZE_MSG_NGB_LIST);
						
						// send the packet to gateway
						sendToGateway(local_to_gw_buf);
					} 
					break;
		} // end switch
		nrk_led_clr(GREEN_LED);	
		nrk_led_clr(ORANGE_LED);
	
	} // end while
}	// end rx_task
					
/*********************************************************************************************/
uint8_t endianness()
{
	uint16_t n = 0x0102;
	uint8_t *ptr = (uint8_t*)(&n);
	
	if(ptr[0] == 2 && ptr[1] == 1)
		return LITTLE_ENDIAN;
		
	if(ptr[0] == 1 && ptr[1] == 2)
		return BIG_ENDIAN;
		
	return ERROR_ENDIAN;
}

/*******************************************************************************************/
void pack_Neighbor(uint8_t *dest, Neighbor *n)
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(n -> addr)) + 1 );
		dest[1] = *( (uint8_t*)(&(n -> addr)) );
	}	
	else //  BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(n -> addr)) );
		dest[1] = *( (uint8_t*)(&(n -> addr)) + 1 );
	}
	
	dest[2] = n -> rssi;
	dest[3] = n -> lastReport;
	dest[4] = n -> isNew;

	return;
}

void pack_Msg_Hello(uint8_t *dest, Msg_Hello *m)
{
	pack_Neighbor(dest, &(m -> n));
	return;
}
/******************************************************************************************/

void pack_NeighborList(uint8_t *dest, NeighborList *n)
{
	uint8_t i,j;		// loop indices 	
	
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(n -> my_addr)) + 1 );
		dest[1] = *( (uint8_t*)(&(n -> my_addr)) );
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(n -> my_addr)) );
		dest[1] = *( (uint8_t*)(&(n -> my_addr)) + 1 );
	}
	
	// pack MAX_NGBS neighbors into the destination buffer	
	for(i = 0, j = 2; i < MAX_NGBS; i++, j += SIZE_NEIGHBOR)
		pack_Neighbor( dest + j, &(n -> ngbs[i]) );
		
	dest[27] = n -> count;
	
	return;
}

void pack_Msg_NgbList(uint8_t *dest, Msg_NgbList *m)
{
	pack_NeighborList(dest, &(m -> nl));
	return;
}

/*******************************************************************************************/
void pack_NW_Packet_header(uint8_t *dest, NW_Packet *pkt)
{
	if(endian == LITTLE_ENDIAN)
	{
		dest[0] = *( (uint8_t*)(&(pkt -> src)) + 1 );
		dest[1] = *( (uint8_t*)(&(pkt -> src)) );
				
		dest[2] = *( (uint8_t*)(&(pkt -> dest)) + 1 );
		dest[3] = *( (uint8_t*)(&(pkt -> dest)) );
			
		dest[4] = *( (uint8_t*)(&(pkt -> seq)) + 1 );
		dest[5] = *( (uint8_t*)(&(pkt -> seq)) );
	}
	else // BIG_ENDIAN
	{
		dest[0] = *( (uint8_t*)(&(pkt -> src)) );
		dest[1] = *( (uint8_t*)(&(pkt -> src)) + 1 );
				
		dest[2] = *( (uint8_t*)(&(pkt -> dest)) );
		dest[3] = *( (uint8_t*)(&(pkt -> dest)) + 1 );
				
		dest[4] = *( (uint8_t*)(&(pkt -> seq)) );
		dest[5] = *( (uint8_t*)(&(pkt -> seq)) + 1 );
		
	}
	dest[6] = pkt -> ttl;
	dest[7] = pkt -> ackRequested;
	dest[8] = pkt -> packetType;
	dest[9] = pkt -> packetSubType;
	dest[10] = pkt -> length;
	
	return;
}
/*******************************************************************************************/
void pack_NodeToGatewaySerial_Packet_header(uint8_t *dest, NodeToGatewaySerial_Packet *pkt)
{
	dest[0] = pkt -> packetType;
	dest[1] = pkt -> packetSubType;
	dest[2] = pkt -> length;
	
	return;
}
				
/*********************************************************************************************/

void tx_HELLO_msg_task()
{
  Msg_Hello m;
  
  if(DEBUG >= 1)
		printf( "TX_HELLO_MSG_TASK PID=%d\r\n",nrk_get_pid());

  // Build a "Hello" msg to be sent periodically
  m.n.addr = NODE_ADDR;
  
  // Build the network packet which will hold the Hello message
  pkt_tx_HELLO_msg_task.src = NODE_ADDR;
  pkt_tx_HELLO_msg_task.dest = BCAST_ADDR;
  pkt_tx_HELLO_msg_task.ttl = 1;								// single hop message
  pkt_tx_HELLO_msg_task.ackRequested = 0;
  pkt_tx_HELLO_msg_task.seq = 0;								// indicates an invalid seq number
  pkt_tx_HELLO_msg_task.packetType = NW_CONTROL;
  pkt_tx_HELLO_msg_task.packetSubType = HELLO;
  pkt_tx_HELLO_msg_task.length = SIZE_MSG_HELLO;
  
  // pack the message in the data field of the NW_Packet 
  pack_Msg_Hello(pkt_tx_HELLO_msg_task.data, &m);
  // pack the NW_Packet header into the transmit buffer of network layer 
  pack_NW_Packet_header(tx_buf_HELLO_msg_task, &pkt_tx_HELLO_msg_task);
  // append the actual message into the transmit buffer of network layer 
  memcpy(tx_buf_HELLO_msg_task + SIZE_NW_PACKET_HEADER, pkt_tx_HELLO_msg_task.data, SIZE_MSG_HELLO);
  
  while(!bmac_started()) nrk_wait_until_next_period();
  
  while(1)
  {
		// Transmit the packet 
  		nrk_led_set(BLUE_LED); 
		while( bmac_tx_packet(tx_buf_HELLO_msg_task, SIZE_NW_PACKET) == NRK_ERROR )
  		{
   		nrk_event_wait (bmac_tx_pkt_done_signal);
   	}
		// Task gets control again after TX complete
		if(DEBUG >= 1)
			printf( "TX_HELLO_MSG_TASK sent data!\r\n" );
  		nrk_led_clr(BLUE_LED); 
		nrk_wait_until_next_period();
	}

} //end tx_HELLO_msg_task

/*********************************************************************************************/
void build_Msg_NgbList()
{
	Msg_NgbList m;
	
	nrk_int_disable();
	m.nl = nl;	
	nrk_int_enable();
	
	// Build the network packet which will hold the NGB_LIST message
  	pkt_tx_NGB_LIST_msg_task.src = NODE_ADDR;
  	pkt_tx_NGB_LIST_msg_task.dest = BCAST_ADDR;
  	pkt_tx_NGB_LIST_msg_task.ttl = MAX_NETWORK_DIAMETER;		
  	pkt_tx_NGB_LIST_msg_task.ackRequested = 0;
  	pkt_tx_NGB_LIST_msg_task.seq = 0;								// indicates an invalid seq number
  	pkt_tx_NGB_LIST_msg_task.packetType = NW_CONTROL;
  	pkt_tx_NGB_LIST_msg_task.packetSubType = NGB_LIST;
  	pkt_tx_NGB_LIST_msg_task.length = SIZE_MSG_NGB_LIST;
		
	// pack the message in the data field of the NW_Packet 
   pack_Msg_NgbList(pkt_tx_NGB_LIST_msg_task.data, &m);
   // pack the NW_Packet header into the transmit buffer of network layer 
   pack_NW_Packet_header(tx_buf_NGB_LIST_msg_task, &pkt_tx_NGB_LIST_msg_task);
   // append the actual message into the transmit buffer of network layer 
   memcpy(tx_buf_NGB_LIST_msg_task + SIZE_NW_PACKET_HEADER, pkt_tx_NGB_LIST_msg_task.data, SIZE_MSG_NGB_LIST);

	return;
}
/**********************************************************************************************/

void tx_NGB_LIST_msg_task()
{
	if(DEBUG >= 1)
		printf( "TX_NGB_LIST_MSG_TASK PID=%d\r\n",nrk_get_pid());

	while(!bmac_started()) nrk_wait_until_next_period();

  	while(1)
	{
		// Build a "Ngb_List" msg to be sent periodically
		build_Msg_NgbList();		
		// Transmit the packet 
  		nrk_led_set(BLUE_LED); 
		while( bmac_tx_packet(tx_buf_NGB_LIST_msg_task, SIZE_NW_PACKET) == NRK_ERROR )
  		{
   		nrk_event_wait (bmac_tx_pkt_done_signal);
   	}
		if(DEBUG >= 1)
			printf( "TX_NGB_LIST_MSG_TASK sent data!\r\n" );
  		nrk_led_clr(BLUE_LED); 
		nrk_wait_until_next_period();
	}

	return;
} // end tx_NGB_LIST_msg_task

/******************************************************************************************/
void
nrk_create_taskset()
{
  RX_TASK.task = rx_task;
  RX_TASK.Ptos = (void *) &rx_task_stack[NRK_APP_STACKSIZE-1];
  RX_TASK.Pbos = (void *) &rx_task_stack[0];
  RX_TASK.prio = 3;
  RX_TASK.FirstActivation = TRUE;
  RX_TASK.Type = BASIC_TASK;
  RX_TASK.SchType = PREEMPTIVE;
  
  RX_TASK.period.secs = 3;
  RX_TASK.period.nano_secs = 0;
  RX_TASK.cpu_reserve.secs = 4;
  RX_TASK.cpu_reserve.nano_secs = 500*NANOS_PER_MS;
  
  RX_TASK.offset.secs = 0;
  RX_TASK.offset.nano_secs= 0;
  nrk_activate_task (&RX_TASK);

/*****************************************************************************************/  
  TX_HELLO_MSG_TASK.task = tx_HELLO_msg_task;
  TX_HELLO_MSG_TASK.Ptos = (void *) &tx_HELLO_msg_task_stack[NRK_APP_STACKSIZE-1];
  TX_HELLO_MSG_TASK.Pbos = (void *) &tx_HELLO_msg_task_stack[0];
  TX_HELLO_MSG_TASK.prio = 2;
  TX_HELLO_MSG_TASK.FirstActivation = TRUE;
  TX_HELLO_MSG_TASK.Type = BASIC_TASK;
  TX_HELLO_MSG_TASK.SchType = PREEMPTIVE;
  
  TX_HELLO_MSG_TASK.cpu_reserve.secs = 3;
  TX_HELLO_MSG_TASK.cpu_reserve.nano_secs = 0;	
  TX_HELLO_MSG_TASK.period.secs = TX_HELLO_MSG_TASK.cpu_reserve.secs + HELLO_PERIOD;
  TX_HELLO_MSG_TASK.period.nano_secs = 0;
  
  TX_HELLO_MSG_TASK.offset.secs = 0;
  TX_HELLO_MSG_TASK.offset.nano_secs= 0;
  nrk_activate_task (&TX_HELLO_MSG_TASK);

/****************************************************************************************/

  TX_NGB_LIST_MSG_TASK.task = tx_NGB_LIST_msg_task;
  TX_NGB_LIST_MSG_TASK.Ptos = (void *) &tx_NGB_LIST_msg_task_stack[NRK_APP_STACKSIZE-1];
  TX_NGB_LIST_MSG_TASK.Pbos = (void *) &tx_NGB_LIST_msg_task_stack[0];
  TX_NGB_LIST_MSG_TASK.prio = 1;
  TX_NGB_LIST_MSG_TASK.FirstActivation = TRUE;
  TX_NGB_LIST_MSG_TASK.Type = BASIC_TASK;
  TX_NGB_LIST_MSG_TASK.SchType = PREEMPTIVE;

  TX_NGB_LIST_MSG_TASK.cpu_reserve.secs = 2;
  TX_NGB_LIST_MSG_TASK.cpu_reserve.nano_secs = 0;  
  TX_NGB_LIST_MSG_TASK.period.secs = TX_NGB_LIST_MSG_TASK.cpu_reserve.secs + NGB_LIST_PERIOD;
  TX_NGB_LIST_MSG_TASK.period.nano_secs = 0;
  

  TX_NGB_LIST_MSG_TASK.offset.secs = 0;
  TX_NGB_LIST_MSG_TASK.offset.nano_secs= 0;
  nrk_activate_task (&TX_NGB_LIST_MSG_TASK);


  if(DEBUG == 2)
	printf ("Task Creation done\r\n");
}


