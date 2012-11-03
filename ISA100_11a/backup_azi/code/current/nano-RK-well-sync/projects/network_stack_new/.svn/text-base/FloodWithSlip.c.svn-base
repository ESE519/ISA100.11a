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

/* SLIP character codes */
#define END             0300    /* indicates end of packet */
#define ESC             0333    /* indicates byte stuffing */
#define ESC_END         0334    /* ESC ESC_END means END data byte */
#define ESC_ESC         0335    /* ESC ESC_ESC means ESC data byte */

// Global data structures
NeighborList nl;					 					// to hold the neighbors of a node
uint8_t DEBUG;											// debug flag
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];			// receive buffer for network layer
uint8_t local_tx_buf[sizeof(NW_Packet)]; 		// to hold the packet to be multi-hopped
// to hold the packet to be sent to the gateway 
uint8_t local_to_gw_buf[sizeof(NodeToGatewaySerialPacket)];

uint8_t tx_buf_HELLO_msg_task[sizeof(NW_Packet)];	// transmit buffer for HELLO msg 
uint8_t tx_buf_NGB_LIST_msg_task[sizeof(NW_Packet)];	// transmit buffer for NGB_LIST msg

NW_Packet pkt_rx_task;							// to hold a received packet
NW_Packet pkt_tx_HELLO_msg_task;				// to hold the HELLO msg
NW_Packet pkt_tx_NGB_LIST_msg_task;			// to hold the NGB_LIST msg
NodeToGatewaySerialPacket ntg_pkt;			// to hold a packet for the gateway

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

Msg_NgbList *msg;

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
			//printf("%d found neighbor %d in list\n", NODE_ADDR, n.addr);
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
			//printf("%d added neighbor %d to list\n", NODE_ADDR, n.addr);			
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
	DEBUG = 0;
	
	// initialise a random number generator 
	srand(NODE_ADDR);
	/*************************************************************************************/
	 
	nrk_create_taskset();		// create the set of tasks
	nrk_start();					// start this node
	
	return 0;
}

/*************DEBUG*******************************************************************************/
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
   //printf("%d ", END);

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
               //printf("%d %d ",ESC, ESC_END); 
               
               break;

             /* if it's the same code as an ESC character,
             * we send a special two character code so as not
             * to make the receiver think we sent an ESC
             */
             case ESC:
                putchar(ESC);
                putchar(ESC_ESC);
                //printf("%d %d ",ESC, ESC_ESC);
                break;

             /* otherwise, we just send the character
             */
             default:
 	              putchar(*buf);
 	              //printf("%d ", *buf);
        } // end switch
        buf++;
        length--;
   } // end while
   
	/* tell the receiver that we're done sending the packet */   
    putchar(END);
    //printf("%d ", END);
	 return;
}

/**********************************************************************************************/
void sendToSerial1(uint8_t *buf, int len)
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
void sendToGateway(NodeToGatewaySerialPacket *pkt)
{
	memcpy(local_to_gw_buf, pkt, sizeof(NodeToGatewaySerialPacket));
	printf("Size of NodeToGatewaySerialPacket on node = %d\n", sizeof(NodeToGatewaySerialPacket));
	//sendToSerial(local_to_gw_buf, sizeof(NodeToGatewaySerialPacket));
	//printf("\n\n");
	sendToSerial1(local_to_gw_buf, sizeof(NodeToGatewaySerialPacket));
}

/*********************************************************************************************/
void rx_task()
{
	uint8_t len; 				   // to hold the size of the received packet, always = // sizeof(NW_Packet) 
	uint8_t rssi;  			 	   // to hold rssi of received packet
	uint8_t *local_rx_buf;	   // pointer to receive buffer of link layer
	Msg_Hello m;			   	// to hold a HELLO message
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
   	
		pkt_rx_task = *((NW_Packet*)local_rx_buf);
		// Release the RX buffer quickly so future packets can arrive 
	   bmac_rx_pkt_release();
	   
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
				m = *((Msg_Hello*)(pkt_rx_task.data));	// extract it
				m.n.rssi = rssi;
				nrk_int_disable(); 
				add_neighbor(m.n);		// ignore the return value here 
				nrk_int_enable();
				
				if(DEBUG >= 1)
					printf("DEBUG: Received HELLO msg from: %d with RSSI = %d\n", m.n.addr, m.n.rssi);
				break;

			case NGB_LIST:		// NGB_LIST msg
					
					//if(DEBUG >= 0)
					//if(WITH_SLIP == FALSE)
					if(0)
					{
						uint8_t i;
						NeighborList nlist;
						msg = (Msg_NgbList*)(pkt_rx_task.data);						
						nlist = msg -> nl;
						
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
						memcpy(local_tx_buf, &pkt_rx_task, sizeof(NW_Packet));
						while( bmac_tx_packet(local_tx_buf, sizeof(NW_Packet)) == NRK_ERROR)
  						{
   						nrk_event_wait (bmac_tx_pkt_done_signal);
   					}
					}
					
					//if(CONNECTED_TO_GATEWAY == TRUE)
					//if(NODE_ADDR == 1)
					//if(WITH_SLIP == TRUE)
					if(1)
					{					
						// construct a packet to be sent to the gateway
						ntg_pkt.packetType = NW_CONTROL;
						ntg_pkt.packetSubType = NGB_LIST;
						ntg_pkt.length = sizeof(Msg_NgbList);
						printf("Size of Msg_NgbList on node = %d\n", ntg_pkt.length);
						printf("Size of NeighborList on node = %d\n", sizeof(NeighborList));
						printf("Size of Neighbor on node = %d\n", sizeof(Neighbor));
						printf("Size of Msg_Hello on node = %d\n", sizeof(Msg_Hello));
						
						
						
						memcpy(ntg_pkt.data, pkt_rx_task.data, sizeof(Msg_NgbList));
					
						// send the packet to gateway
						sendToGateway(&ntg_pkt);
					} 
					break;
		} // end switch
		nrk_led_clr(GREEN_LED);	
		nrk_led_clr(ORANGE_LED);
	
	} // end while
}	// end rx_task
					
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
  pkt_tx_HELLO_msg_task.length = sizeof(Msg_Hello);
  memcpy(pkt_tx_HELLO_msg_task.data, &m, sizeof(Msg_Hello));

	//copy this packet to the transmit buffer of the network layer
  memcpy(tx_buf_HELLO_msg_task, &pkt_tx_HELLO_msg_task, sizeof(NW_Packet));
  
  while(!bmac_started()) nrk_wait_until_next_period();
  
  while(1)
  {
		// Transmit the packet 
  		nrk_led_set(BLUE_LED); 
		while( bmac_tx_packet(tx_buf_HELLO_msg_task, sizeof(NW_Packet)) == NRK_ERROR )
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
  	pkt_tx_NGB_LIST_msg_task.length = sizeof(Msg_NgbList);
	memcpy(pkt_tx_NGB_LIST_msg_task.data, &m, sizeof(Msg_NgbList));
	
	//copy this packet to the transmit buffer of the network layer
  	memcpy(tx_buf_NGB_LIST_msg_task, &pkt_tx_NGB_LIST_msg_task, sizeof(NW_Packet));

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
		while( bmac_tx_packet(tx_buf_NGB_LIST_msg_task, sizeof(NW_Packet)) == NRK_ERROR )
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
  RX_TASK.period.secs = 4;
  RX_TASK.period.nano_secs = 0;
  RX_TASK.cpu_reserve.secs = 3;
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
  
  TX_HELLO_MSG_TASK.cpu_reserve.secs = 2;
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

  TX_NGB_LIST_MSG_TASK.cpu_reserve.secs = 1;
  TX_NGB_LIST_MSG_TASK.cpu_reserve.nano_secs = 0;  
  TX_NGB_LIST_MSG_TASK.period.secs = TX_NGB_LIST_MSG_TASK.cpu_reserve.secs + NGB_LIST_PERIOD;
  TX_NGB_LIST_MSG_TASK.period.nano_secs = 0;
  

  TX_NGB_LIST_MSG_TASK.offset.secs = 0;
  TX_NGB_LIST_MSG_TASK.offset.nano_secs= 0;
  nrk_activate_task (&TX_NGB_LIST_MSG_TASK);


  if(DEBUG == 2)
	printf ("Task Creation done\r\n");
}


