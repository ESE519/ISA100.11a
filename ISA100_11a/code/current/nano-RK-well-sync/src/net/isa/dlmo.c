/******************************************************************************
*  Nano-RK, a real-time operating system for sensor networks.
*  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
*  All rights reserved.
*
*  This is the Open Source Version of Nano-RK included as part of a Dual
*  Licensing Model. If you are unsure which license to use please refer to:
*  http://www.nanork.org/nano-RK/wiki/Licensing
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, version 2.0 of the License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Contributing Authors (specific to this file):
*  Azriel Samson
*  Vignesh Anantha Subramanium
*******************************************************************************/

//#include <rtl_debug.h>
#include <include.h>
#include <ulib.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <nrk.h>
#include <nrk_events.h>
#include <nrk_timer.h>
#include <nrk_error.h>
//#include <rtl_defs.h>
#include <stdlib.h>
#include <isa_error.h>
#include <dmo.h>
#include <dlmo.h>
#include <isa_messagetypes.h>


//********************** Global variables*************************************
volatile DLMO_LINK dlmoLink[DLMO_LINK_MAX_COUNT];
volatile DLMO_NEIGHBOR dlmoNeighbor[DLMO_NEIGHBOR_MAX_COUNT];
volatile DLMO_GRAPH dlmoGraph[DLMO_GRAPH_MAX_COUNT];
volatile ISA_QUEUE isaQueue[TRANSMIT_QUEUE_MAX_SIZE] ;
volatile DLMO_CANDIDATE dlmoCandidate;
//ISA_QUEUE *isaQueuePointer[TRANSMIT_QUEUE_MAX_SIZE]
uint8_t isa_sched[ISA_SLOTS_PER_FRAME];
uint64_t isa_tdma_rx_mask;	//should not need this
uint64_t isa_tdma_tx_mask;	// should not need this
uint64_t isa_slot;	//Set to 1 if slot is in use
uint8_t isaTxQueueSize; //holds the number of elements present in the Queue

/* Device management object*/
 DMO dmo;
 DLMO_DISCOVERY_ALERT discoveryAlert;

 /* Variables for neighbor table transmission */
 nrk_time_t lastSentTime;
 uint8_t nbr_buf[RF_MAX_PAYLOAD_SIZE];

//********************Local function definitions***********************************
void configureSlot(uint8_t slotNumber, uint16_t neighborId, LinkType linkType, bool clockSource, uint16_t graphId, uint8_t neighborCount, uint16_t n1, uint16_t n2, uint16_t n3, uint8_t graphType);
void dlmoInit();
DLMO_GRAPH* addGraph(uint16_t graphId, uint8_t neighborCount, uint16_t n1, uint16_t n2, uint16_t n3);
int8_t addLink(uint8_t slotNumber, uint16_t neighborId,uint16_t graphId , LinkType linkType, GraphType graphType);
DLMO_NEIGHBOR* addNeighbor(uint16_t index,uint64_t EUI64, uint8_t groupCode1, uint8_t groupCode2, bool clockSource,uint8_t  linkBacklogIndex,uint8_t linkBacklogDur, uint8_t linkBacklogActivate);
int8_t dd_data_request(uint16_t srcAddr, uint16_t destAddr, uint8_t priority, bool discardEligible, uint8_t ecn, bool lh, uint8_t contractId, uint8_t length, uint8_t *payload,  void (*slot_callback)(ISA_QUEUE *entry ,status));
void dd_data_indication(uint16_t srcAddr,uint16_t destAddr,uint8_t priority,bool discardEligibile, bool lh, uint8_t length, uint8_t *payload);
void sendPacket(uint16_t destAddr,uint8_t graphId, uint8_t length, uint8_t *payload, void (*slot_callback)(ISA_QUEUE *entry, status)) ;
int8_t enQueue(uint16_t neighbor, uint8_t priority, uint8_t length, uint8_t *payload,  void (*slot_callback)(ISA_QUEUE *entry, status));
void isaFreePacket(ISA_QUEUE *entry);


/*------------------------------------------------- dlmoInit() -----
         |  Function dlmoInit()
         |
         |  Purpose:  Called during initialization. This can also be used later to reset to
         |		factory defaults if required. In its current state, it doesn't really do much
         | 		except set the neighbor table reporting duration
         |
         |
         |  Parameters:
         |      NONE
         |
         |  Returns:
         |		NONE
         *-------------------------------------------------------------------*/
void dlmoInit()
{
	uint8_t i;
	//for LINK
	for (i=0; i<DLMO_LINK_MAX_COUNT; i++)

		{
			(dlmoLink[i].isPresent = false);
		}

//for NEIGHBOR
	for(i=0; i<DLMO_NEIGHBOR_MAX_COUNT; i++)
	{
		dlmoNeighbor[i].isPresent = false;
	}

		//Initialization for Transmit Queue
	isaTxQueueSize = 0; //Explicitly initialize this to 0
/*
	for(i=0; i<TRANSMIT_QUEUE_MAX_SIZE; i++)
	{
		isaQueuePointer[i] = &isaQueue[i];
	}

*/

	// Initialize the dlmo.DiscoveryAlery field
	/* Device management object*/
	discoveryAlert.alertReport.alertReportDisabled = false;
	discoveryAlert.duration = 60; //in seconds

	//initialize the lastSentTime to the startTime
	 nrk_time_get(&lastSentTime);

}


DLMO_GRAPH* findGraph(uint16_t graphId){
	for(uint8_t i=0;i<DLMO_GRAPH_MAX_COUNT;i++)
		{
			if(graphId == dlmoGraph[i].index) return &dlmoGraph[i];
		}
	return NULL;
}

DLMO_NEIGHBOR* findNeighbor(uint16_t neighborId){
	for(uint8_t i=0;i<DLMO_NEIGHBOR_MAX_COUNT;i++)
		{
			if(neighborId == dlmoNeighbor[i].index) return &dlmoNeighbor[i];
		}
	return NULL;
}

/*------------------------------------------------- addLink() -----
         |  Function addLink()
         |
         |  Purpose:  Used to add/update a  link. If the link is already present, its contents will get updated.
         |
         |
         |  Parameters:
         |      (IN)uint8_t slotNumber - 	This is curretly stored as channelOffset and represents the slot at which this link appears.
         |		(IN)DLMO_NEIGHBOR* neighborIndex - Pointer to the neighbor that is stored in this link. This parameter can also be NULL in the case of
         |								an ADV or RX link.
         |		(IN)DLMO_GRAPH* graphPtr - 	Pointer to the graph that is stored in this link. This parameter can also be NULL in the case of an ADV or
		 |								 RX link.
		 |		(IN)LinkType linkType - Represents the linkType
		 |								JOIN_RESP,
		 |								TX_NO_ADV,
		 |								TX_RX_NO_ADV,
		 |								TX_ADV,
		 |								ADV,
		 |								BURST_ADV,
		 |								BURST_ADV_SCAN,
		 |								SOLICITATION,
		 |								RX
         |		(IN)GraphType graphType - Additional type of TX link. This represents whether the link can be used to forward directly to a NEIGHBOR, or
		 |								 if the link is used only for a message that is destined for a GRAPH or if both GRAPH and NEIGHBOR (GRAPH) being
		 |								given preference
         |  Returns:
         |		ISA_SUCCESS
		 |		ISA_ERROR
         *-------------------------------------------------------------------*/
int8_t addLink(uint8_t slotNumber, uint16_t neighborId, uint16_t graphId , LinkType linkType, GraphType graphType)
{
	uint8_t index;
	int8_t freeIndex = -1;
	DLMO_NEIGHBOR* neighborPtr = NULL;	// neighbor pointer to store
	DLMO_GRAPH* graphPtr = NULL;			// graph pointer to store

	if (slotNumber >=  ISA_MAX_SLOTS) {
			printf ("Slot number not in range\r\n");
			return;
		}

	for (index = 0; index < DLMO_LINK_MAX_COUNT; index++)
		{
		if (freeIndex==-1 && dlmoLink[index].isPresent == false) freeIndex = index;
		if (dlmoLink[index].chOffset == slotNumber) {
			printf ("Slot %d already configured- updating\r\n",slotNumber);
			freeIndex = index;
			goto UPDATE_LINK;
		}
		}

	if (freeIndex == -1)
		{

			 setIsaError(LINK_CAPACITY_ERROR);
			 printIsaError();
			 return ISA_ERROR;
		}


	//we have found a free index
	UPDATE_LINK:
	if (linkType == TX_NO_ADV){

	//find the neighbor pointer
	if (neighborId!=0)
		{
			neighborPtr = findNeighbor(neighborId);
		}
	//find the graph pointer
	if (graphId!=0)
	{
		graphPtr = findGraph(graphId);
	}

	}
	printf("Added link at index %d\r\n",freeIndex);
	 dlmoLink[freeIndex].isPresent = true;
	 dlmoLink[freeIndex].neighbor = neighborPtr;
	 dlmoLink[freeIndex].graphPtr = graphPtr;
	 dlmoLink[freeIndex].linkType = linkType;
	 //reset the previous slot number before updating
	// isa_slot &= ~(((uint64_t) 1) << dlmoLink[freeIndex].chOffset);
	 dlmoLink[freeIndex].chOffset = slotNumber;
	 dlmoLink[freeIndex].graphPtr = graphPtr;
	 dlmoLink[freeIndex].typeInfo = ISASET(dlmoLink[index].typeInfo, SHIFTLEFT(graphType, GRAPH_TYPE_BIT));
	 // channel offset implementation will change as the protocol develops
	 //record that the slot is in use- used to calculate next wakeup
	 if(slotNumber == 23) putchar('x');
	 	isa_slot |= ((uint64_t) 1) << slotNumber;
	return ISA_SUCCESS;
}




/*------------------------------------------------- findLink() -----
         |  Function findLink()
         |
         |  Purpose:  This returns a pointer to a link that corresponds to the slot that is passes to it.
         |			The functions runs through the dlmoLink[] array and compares the slot with the channel offset parameter
         |			for valid links.
         |
         |
         |  Parameters:
         |      (IN)uint8_t slot - Slot for which we want the link pointer
         |
         |  Returns:
         |		DLMO_LINK * - Pointer to the link that corresponds to the slot passed as an argument.
         *-------------------------------------------------------------------*/
DLMO_LINK * findLink(uint8_t slot){

	uint8_t index;

	for (index = 0; index < DLMO_LINK_MAX_COUNT; index++)
			{
				if (dlmoLink[index].isPresent == true && dlmoLink[index].chOffset == slot ) {
					return &dlmoLink[index];
				}
			}
	printf ("This slot is not configured yet: %d\n\r" , slot);
	return NULL;
}

/*------------------------------------------------- isTransmitLinkPresent() -----
         |  Function isTransmitLinkPresent()
         |
         |  Purpose:  This function is called before we enqueue something in the queue in order to determine if
         |			we have a link that can be used to send this message
         |			We know we have a TX link if either -
         | 			1) The graphType is NEIGHBOR or GRAPH_NEIGHBOR and the DEST_ID of the message is equal to the
		 | 			neighbor on the link or
		 | 			2) the graph_id of the message corresponds to the graphId of the link and the graphType(of the link) is GRAPH or GRAPH_NEIGHBOR
		 | 			This function runs through all the links to find if either 1 or 2 is true
         |
         |
         |  Parameters:
         |      (IN)uint8_t *payload - Pointer to the message payload
         |
         |  Returns:
         |		true
         |		false
         *-------------------------------------------------------------------*/


bool isTransmitLinkPresent (uint8_t *payload){
	uint8_t index;
	 uint8_t graphType;
		 //check the link type

	 DLMO_DROUT * dRout = &payload[DROUT_INDEX];

	for (index = 0; index < DLMO_LINK_MAX_COUNT; index++)
	{
		//first find the graph type
		graphType = ISAMASK(dlmoLink[index].typeInfo,GRAPH_TYPE_MASK);
		graphType = SHIFTRIGHT(graphType,GRAPH_TYPE_BIT);

				if (dlmoLink[index].isPresent == true && dlmoLink[index].linkType == TX_NO_ADV ) {	//this is a valid TX link
					//the link neighbor is equal to payload destID  			and the  graph type is NEIGHBOR or GRAPH_NEIGHBOR			or 	 									link graphId is equal to the message graphID and the link type is GRAPH or GRAPH_NEIGHBOR
					if ((dlmoLink[index].neighbor->index == payload[DEST_INDEX] && (graphType == NEIGHBOR || graphType == GRAPH_NEIGHBOR))|| (dlmoLink[index].graphPtr->index !=0 && dlmoLink[index].graphPtr->index==dRout->GraphId&& (graphType == GRAPH || graphType == GRAPH_NEIGHBOR)) )
					return true;
				}
			}
	return false;		//we do not have a link that is configured for this graphId
}


/*------------------------------------------------- addNeighbor() -----
         |  Function addNeighbor()
         |
         |  Purpose:  Used to add/update a neighbor. If the neighbor is already present, its contents will get updated.
         |
         |
         |  Parameters:
         |      (IN)uint16_t index - 		Neighbor ID to store/update
         |		(IN)uint64_t EUI64 - 		Currently unused
         |		(IN)uint8_t groupCode1 - 	Currently unused
		 |		(IN)uint8_t groupCode2 -	Currently unused
		 |      (IN)bool clockSource - 		TRUE  - if this neighbor is my clock source
		 | 									FALSE - if this neighbor is not my clock source
		 |		(IN)uint8_t  linkBacklogIndex - Currently unused
		 | 		(IN)uint8_t linkBacklogDur	  - Currently unused
		 |      (IN)uint8_t linkBacklogActivate - Currently unused
		 |
         |
         |  Returns:
         |		DLMO_NEIGHBOR* - pointer to the neighbor added/updated
         *-------------------------------------------------------------------*/
DLMO_NEIGHBOR* addNeighbor(uint16_t index,uint64_t EUI64, uint8_t groupCode1, uint8_t groupCode2, bool clockSource,uint8_t  linkBacklogIndex,uint8_t linkBacklogDur, uint8_t linkBacklogActivate)
{
	uint8_t i,free_index=0;
	bool free_index_present = false;
	for(uint8_t i=0;i<DLMO_NEIGHBOR_MAX_COUNT;i++)
	{
		if(index == dlmoNeighbor[i].index && dlmoNeighbor[i].isPresent == true)
		{
			printf("Neighbor %d Exists in Table - updating\n\r",index);
			free_index = i;
			goto UPDATE_NEIGHBOR;
		}
		if(dlmoNeighbor[i].isPresent == false && free_index_present == false)
		{
					free_index_present = true;
					free_index = i;
		}
	}
	if(free_index_present == false)
	{
			setIsaError(NEIGHBOR_CAPACITY_ERROR);
			printIsaError();
			return NULL;
	}
	else
	{
		printf("Added Neighbor at Index %d\r\n",free_index);
		UPDATE_NEIGHBOR:
		dlmoNeighbor[free_index].index = index;
		dlmoNeighbor[free_index].isPresent = true;
		dlmoNeighbor[free_index].EUI64 = EUI64;
		dlmoNeighbor[free_index].groupCode1 = groupCode1;
		dlmoNeighbor[free_index].groupCode2 = groupCode2;
	    if(clockSource == true) dlmoNeighbor[free_index].typeInfo = ISASET(dlmoNeighbor[free_index].typeInfo,CLOCK_PREFERRED);
		dlmoNeighbor[free_index].linkBacklogIndex = linkBacklogIndex;
		dlmoNeighbor[free_index].linkBacklogDur = linkBacklogDur;
		dlmoNeighbor[free_index].linkBacklogActivate = linkBacklogActivate;
		return &dlmoNeighbor[free_index];
	}
}

/*------------------------------------------------- addGraph() -----
         |  Function addGraph()
         |
         |  Purpose:  Used to add/update a graph . If the graph is already present, its contents will get updated.
         |
         |
         |  Parameters:
         |      (IN)uint16_t graphId - 			Graph ID to store/update
         |		(IN)uint8_t  neighborCount - 	Number of neighbors in the preference list
         |		(IN)uint16_t n1 - 	Neighbor 1 (higher priority)
		 |		(IN)uint16_t n2 -	Neighbor 2
		 |      (IN)uint16_t n3 - 	Neighbor 3
         |
         |  Returns:
         |		DLMO_GRAPH* - pointer to the graph added/updated
         *-------------------------------------------------------------------*/

DLMO_GRAPH* addGraph(uint16_t graphId, uint8_t neighborCount, uint16_t n1, uint16_t n2, uint16_t n3){

	//printf("AddGraph Graph ID: %d\r\n",graphId);

	uint8_t i,free_index=0;
	bool free_index_present = false;
	for(uint8_t i=0;i<DLMO_GRAPH_MAX_COUNT;i++)
	{
		if(graphId == dlmoGraph[i].index)
		{
			printf("Graph %d Exists in Table -updating\n\r",graphId);
			free_index = i;
			goto UPDATE_GRAPH;
		}
		if(dlmoGraph[i].index == 0 && free_index_present == false) 	//is not configured
		{
					free_index_present = true;
					free_index = i;
		}
	}
	if(free_index_present == false)
	{
			setIsaError(GRAPH_CAPACITY_ERROR);
			printIsaError();
			return NULL;
	}
	else
	{
		printf("Added graph at index %d\r\n",free_index);
		UPDATE_GRAPH:
		dlmoGraph[free_index].index = graphId;
		dlmoGraph[free_index].info = ISASET(SHIFTLEFT(neighborCount, NEIGHBOR_COUNT_LOWER_BIT), dlmoGraph[free_index].info );	//set the neighbor count
		dlmoGraph[free_index].neighbor[0] = n1;
		dlmoGraph[free_index].neighbor[1] = n2;
		dlmoGraph[free_index].neighbor[2] = n3;
		return &dlmoGraph[free_index];
	}

}


/*
void configureSlot(uint8_t slotNumber, uint16_t neighborId, LinkType linkType, bool clockSource, uint16_t graphId, uint8_t neighborCount, uint16_t n1, uint16_t n2, uint16_t n3, GraphType graphType)
{
	DLMO_NEIGHBOR* neighborIndex;//store the neighbor index to pass to addLink()
	DLMO_GRAPH* graphPtr ;
	if (slotNumber >=  ISA_MAX_SLOTS) {
		printf ("Slot number not in range");
		return;
	}
	if (linkType == TX_NO_ADV){
	//Call the function to add a neighbor as long as neighboe ID is not zero
		if (neighborId!=0)
		{
			putchar('z');
			neighborIndex = addNeighbor(neighborId,0, 0, 0, clockSource,0,0, 0);
			if (neighborIndex == NULL)//
			{
				printIsaError();
				return;
			}
		}
		if (graphId !=0){
			putchar('y');
			graphPtr = addGraph(graphId, neighborCount, n1, n2, n3);
			if (graphPtr == NULL)//
						{
							printIsaError();
							return;
						}

		}
	}
	if (addLink(slotNumber, neighborIndex,graphPtr, linkType, graphType) == -1)
	{
		printIsaError();
		return;
	}
//record that the slot is in use- used to calculate next wakeup
	isa_slot |= ((uint64_t) 1) << slotNumber;

}

*/


/**
 * isa_get_slots_until_next_wakeup()
 *
 * This function returns the absolute number of slots between the current_slot
 * and the next RX/TX related wakeup.
 *
 * Argument: current_slot is the current slot
 * Return: uint16_t number of slots until the next wakeup
 */
uint16_t isa_get_slots_until_next_wakeup (uint16_t current_global_slot)
{
    uint16_t min_slot;
    uint8_t test_slot;
    uint8_t wrapped_slot;
    uint8_t current_local_slot;
    uint64_t testVariable = 0;

    current_local_slot = current_global_slot%ISA_SLOTS_PER_FRAME;
  //  printf("current local slot %d\r\n",current_local_slot);
    testVariable |= ((uint64_t)1) << (current_local_slot+1);

    for (test_slot = current_local_slot+1; test_slot < ISA_SLOTS_PER_FRAME; test_slot++) {

            if(isa_slot & testVariable) { 	//slot is  scheduled
            	min_slot = test_slot-current_local_slot;
            	    	return min_slot;

            }
                testVariable = testVariable << 1;

        }

    // scheduled slot wrapped back

    testVariable = 1;
    for (test_slot = 0; test_slot<=current_local_slot;test_slot++){
	if(isa_slot & testVariable){ //slot is scheduled
		min_slot = (ISA_SLOTS_PER_FRAME - current_local_slot + test_slot);
			return min_slot;

	}
    testVariable = testVariable << 1;
	    }
}


// *** Data link layer service access points ****
/********dd_data_request : Service access point used to send data ******************
 * SrcAddr (NL source address)
 * DestAddr (NL destination address)
 * Priority (priority of the payload)
 * DE (discard eligible)
 * ECN (explicit congestion notification)
 * LH (last hop, NL)
 * ContractID (ContractID of the payload)
 * DSDULength (payload length)
 * DSDU (number of octets as per DSDULength)
 * DSDUHandle (uniquely identifies each invocation of this primitive)
 *
 */


 int8_t dd_data_request(uint16_t srcAddr, uint16_t destAddr, uint8_t priority, bool discardEligible, uint8_t ecn, bool lh, uint8_t contractId, uint8_t length, uint8_t *payload,  void (*slot_callback)(ISA_QUEUE *entry, status))
 {
	 //Future - Table lookup based on contract Id and dest address
	 //Current - contractId is considered as the graphID directly and dest is the destID

//Configure the headers within the payload (whichever applicable)
	 payload[DEST_INDEX] = destAddr;
	 //if (contractId!=0)
	 {
		 DLMO_DROUT * dRout;
		 dRout = &payload[DROUT_INDEX];
		 dRout->GraphId = contractId;
	 }
return  enQueue (destAddr, priority, length, payload, slot_callback);


}

//Wrapper for dd_data_request

 void sendPacket(uint16_t destAddr,uint8_t graphId, uint8_t length, uint8_t *payload, void (*slot_callback)(ISA_QUEUE *entry, status))
 {
	 if (dd_data_request(0, destAddr, 0, 0, 0, 0, graphId,  length, payload,   slot_callback) == -1)
	 {
		 printIsaError();
	 }
 }

 //*******************dd_data_indication: Service access point used to indicate received data************
 void dd_data_indication(uint16_t srcAddr,uint16_t destAddr,uint8_t priority,bool discardEligibile, bool lh, uint8_t length, uint8_t *payload)
 {

	// printf("packet is for me");
	  isa_rx_pkt_release();
 }
 /*
  * Add to queue. Find a free place and insert with current time
  */

 int8_t enQueue(uint16_t neighbor, uint8_t priority, uint8_t length, uint8_t *payload,  void (*slot_callback)(ISA_QUEUE *entry, status))
 {
	 uint8_t i;
	/*
	 bool passedLowerPriority = false;
	 bool fixRequired = false;
	 bool insertionDone = false;
	 uint8_t lowerPriorityIndex;
	 uint8_t usedIndex;
	 ISA_QUEUE * temp;
*/

	 	 if (isaTxQueueSize > TRANSMIT_QUEUE_MAX_SIZE){
	 		 setIsaError(TRANSMIT_QUEUE_CAPACITY_ERROR);
	 		 return ISA_ERROR;
	 	 }

	 	 //check if length of payload is within bounds
	 	 if (length >= RF_MAX_PAYLOAD_SIZE) {
	 		 setIsaError(MAX_PAYLOAD_ERROR);
	 		 return ISA_ERROR;
	 	 }

	 	 //if we are here, we should have place to add into the Queue
	 	 //find the first free index and insert
	 for (i = 0; i < TRANSMIT_QUEUE_MAX_SIZE; i++){

		 if (isaQueue[i].usedSlot == false){
			 isaQueue[i].length = length;
			 isaQueue[i].priority = priority;
			 isaQueue[i].transmitPending = true;
			 isaQueue[i].usedSlot = true;
			 isaQueue[i].neighbor = neighbor;
			 isaQueue[i].slot_callback =  slot_callback;
			 isaQueue[i].numTries = 0;
			 memcpy(isaQueue[i].tx_buf, payload, length );//copy the payload
			 nrk_time_get(&isaQueue[i].time);	//copy the time when I was inserted into Queue
			 isaTxQueueSize++;
			 break;
		 }

	 }
	 if ( i == TRANSMIT_QUEUE_MAX_SIZE){
		 printf(" Critical error 2\r\n");
		 return ISA_ERROR;
	 }
	 return ISA_SUCCESS;

		 //this if evaluates the event in which I have not copied into a slot and find an entry of lower priority
/*
		 if (isaQueuePointer[i]->usedSlot == true && isaQueuePointer[i]->transmitPending = false  && insertionDone == false && passedLowerPriority == false && isaQueuePointer[i]->priority < priority && isaQueuePointer[i]->neighbor == neighbor){
			 passedLowerPriority = true;
			 lowerPriorityIndex = i;
			 continue;
		 }
		 //if passedLowerPriority == true , then find a slot to insert and insert-> swap pointers for lowerPriority and free
		 //fix for every index till free index

		 if (insertionDone == false && isaQueuePointer[i]->usedSlot == false){
			 //find a free slot to insert
			 usedIndex = i;
			 isaQueuePointer[i]->length = length;
			 isaQueuePointer[i]->priority = priority;
			 isaQueuePointer[i]->transmitPending = true;
			 isaQueuePointer[i]->usedSlot = true;
			 isaQueuePointer[i]->neighbor = neighbor;
			 memcpy(isaQueuePointer[i]->tx_buf, payload, length );//copy the payload
			 isaTxQueueSize++;
			 insertionDone = true;
			 if (passedLowerPriority == true) break; //IF this is the case, I fix after this loop
			 continue;
		 }

		 if (insertionDone == true && isaQueuePointer[i]->usedSlot == true  && isaQueuePointer[i]->transmitPending = false && isaQueuePointer[i]->neighbor == neighbor && isaQueuePointer[i]->priority > isaQueuePointer[usedIndex]->priority  ){ //Swap
			 //we come here only if fix required
			 temp = isaQueuePointer[i];
			 isaQueuePointer[i] = isaQueuePointer[usedIndex];
			 isaQueuePointer[usedIndex] = temp;
			 usedIndex = i;
				 }


	 //we can return now if we did not come here through the condition where I inserted after a higher priority
	 if (passedLowerPriority == false) return 1;
	 //I am here if I inserted after lower priority. Now I need to take care of fixing that
// I iterate from usedIndex to lowerPriority Index in the backward direction and fix
	 for (i = usedIndex -1 ; i >= lowerPriorityIndex ; i--)
		 if (isaQueuePointer[i]->usedSlot == true  && isaQueuePointer[i]->transmitPending = false && isaQueuePointer[i]->neighbor == neighbor && isaQueuePointer[i]->priority < isaQueuePointer[usedIndex]->priority){
			 temp = isaQueuePointer[i];
			 isaQueuePointer[i] = isaQueuePointer[usedIndex];
			 isaQueuePointer[usedIndex] = temp;
			 usedIndex = i;
		 }
	 return 1;
 */
 }

/*
 * if numtries is 0 then we should have the preferred link, else take any of the other links if possible
 */

 bool isLinkNeigborApplicable(ISA_QUEUE* isaQueue, DLMO_LINK * link)
 {
	 uint8_t i;
	 if (isaQueue->numTries == 0){
		 if( link->graphPtr->neighbor[0] == link->neighbor->index ) return true;
	 return false;
	 }
	 //for the number of neighbors configured as alternate routes in this graph
	 for (i = 0; i< SHIFTRIGHT(ISAMASK(link->graphPtr->info, NEIGHBOR_COUNT_MASK),NEIGHBOR_COUNT_LOWER_BIT );i++){
		 if (link->graphPtr->neighbor[i] == link->neighbor->index) return true;
	 }
	 return false;
 }

 ISA_QUEUE * getHighPriorityEntry(DLMO_LINK * link){

	 uint16_t neighbor;
	 nrk_time_t time;
	 uint8_t priority = 0;
	 ISA_QUEUE* tempIndex;
	 bool found = false;
	 uint8_t i;
	 uint8_t graphType;
	 //check the link type

	 graphType = ISAMASK(link->typeInfo,GRAPH_TYPE_MASK);
	 graphType = SHIFTRIGHT(graphType,GRAPH_TYPE_BIT);



if (graphType == 0){	//if the graph type is 0
	DIRECT_NEIGHBOR:
	if (link->neighbor == NULL || link->neighbor->isPresent == false) return NULL;
	neighbor = link->neighbor->index;
	for (i = 0; i < TRANSMIT_QUEUE_MAX_SIZE; i++){
		 if (isaQueue[i].usedSlot == true && isaQueue[i].transmitPending == true && isaQueue[i].neighbor == neighbor){
			 if (found == false){
				 found = true;
				 priority = isaQueue[i].priority;
				 tempIndex = &isaQueue[i];
				 time.nano_secs = isaQueue[i].time.nano_secs;
				 time.secs = isaQueue[i].time.secs;
			 }
			 //			if the priority is greater					or	(	priority is the same    		and ( seconds is less					or 	nanosecs is less))
		 if (found == true && ( priority < isaQueue[i].priority  || ( (priority == isaQueue[i].priority) && ( time.secs > isaQueue[i].time.secs || (time.secs == isaQueue[i].time.secs && time.nano_secs > isaQueue[i].time.nano_secs ))))){
			 priority = isaQueue[i].priority;
			 tempIndex = &isaQueue[i];
			 time.nano_secs = isaQueue[i].time.nano_secs;
			 time.secs = isaQueue[i].time.secs;
		 }
	 }

 }
	 if (found == false) {
		 return NULL;
	 }
	 return tempIndex;
}
else if (graphType == 1 || graphType == 2){		//this link is only for graph routing or this link preferres graph over direct neighbor
	//get the graph from the link
	if (link->graphPtr == NULL || link->graphPtr->index == 0) {	//if the graph pointer is null and the graph type is 2, then check for neighbor, else return NULL
	if (graphType==2)goto DIRECT_NEIGHBOR;
	return NULL;
}
for (i = 0; i < TRANSMIT_QUEUE_MAX_SIZE; i++){
		 if (isaQueue[i].usedSlot == true && isaQueue[i].transmitPending == true ){
			 DLMO_DROUT * dRout = &isaQueue[i].tx_buf[DROUT_INDEX];
			 if (dRout->GraphId!=0 && dRout->GraphId == link->graphPtr->index)//If the GraphId matches  (graphId is 8 bits while index is 16 bits)
			 {//first time to be transmitted and top preference neighbor for is on this link or second time and second pref or third time and third pref

				 if (isLinkNeigborApplicable(&isaQueue[i], link))
				{
				 if (found == false){
					 found = true;
					 priority = isaQueue[i].priority;
					 tempIndex = &isaQueue[i];
					 time.nano_secs = isaQueue[i].time.nano_secs;
					 time.secs = isaQueue[i].time.secs;
				 }
			 //			if the priority is greater					or	(	priority is the same    		and ( seconds is less					or 	nanosecs is less))
		 if (found == true && ( priority < isaQueue[i].priority  || ( (priority == isaQueue[i].priority) && ( time.secs > isaQueue[i].time.secs || (time.secs == isaQueue[i].time.secs && time.nano_secs > isaQueue[i].time.nano_secs ))))){
			 priority = isaQueue[i].priority;
			 tempIndex = &isaQueue[i];
			 time.nano_secs = isaQueue[i].time.nano_secs;
			 time.secs = isaQueue[i].time.secs;
		 }
			 }
			 }
	 }

}
	 if (found == false) {//if no graph to use, then we can check for direct neighbor, if type is 2
		 if (graphType == 2) goto DIRECT_NEIGHBOR;	//we did not find a graph , so now we check for direct_neighbor
		 return NULL;
	 }
	 return tempIndex;
}
 }

 //*********************************************************************************************************
 void isaFreePacket(ISA_QUEUE *entry){
	 //Write 0 into the queue payload

	 entry->usedSlot = false;
	 entry->transmitPending = false;;
	 entry->slot_callback = NULL;
	 isaTxQueueSize--;
 }
//****************** Functions for dlmo.Candidate******************************


 /*
  * The protocol states that the system manager may ask the device to clear its
  * entire candidate table. Individual candidates are never removed
  */

 void clearCandidateTable(){
 uint8_t i;
 dlmoCandidate.n=0;
 for (i=0; i< DLMO_CANDIDATE_MAX_SIZE ; i++){
	 dlmoCandidate.candidate[i].neighbor = 0;  //setting to zero indicates that no neighbor is present
}
 }

 /*
  * This function adds a neighbor in the candidate table
  */

 int8_t addCandidate(uint16_t candidate){
	 uint8_t i;
	 for (i=0; i<dlmoCandidate.n; i++){
		 if (dlmoCandidate.candidate[i].neighbor == candidate){
			// printf ("Neighbor: %d already present", candidate);
			 return ISA_SUCCESS;
		 }
	 }

	 if (dlmoCandidate.n >= DLMO_CANDIDATE_MAX_SIZE) {
		 setIsaError(CANDIDATE_CAPACITY_ERROR);
		 return ISA_ERROR ;//we have reached max size
	 }

dlmoCandidate.candidate[dlmoCandidate.n].neighbor = candidate;
dlmoCandidate.n++;
printf ("Added %d to Candidate table at %d\r\n", candidate,dlmoCandidate.n-1 );
return ISA_SUCCESS;
 }


 bool isDiscoveryAlertDue(){
	 nrk_time_t currentTime;
	 nrk_time_get(&currentTime);
	 if (currentTime.secs - lastSentTime.secs > 60) return true;
	 else return false;
 }

 void updateLastSentTime(){
	 nrk_time_get(&lastSentTime);
 }

int8_t sendAdv (){
	uint8_t length;
	//		n +  n * number of neighbors
	length = 1 + dlmoCandidate.n * sizeof(CANDIDATE);
	MESSAGE *message;
	message = &nbr_buf[PKT_DATA_START];
	message->type = NEIGHBOR_TABLE_REPORT;
	memcpy(&message->data, &dlmoCandidate, length);

	nbr_buf[DEST_INDEX] = isa_clk_src_id;
	nbr_buf[SRC_INDEX] = dmo.dlAddress;
			//length of dlmo.candidate + PKT_DATA_START + message-> type (1)
	length = length + PKT_DATA_START + 1;
	return enQueue(isa_clk_src_id, 0, length, nbr_buf, NULL);

}



/*
 * This function is called to flush candidate table
 */

void flushCandidateEntries()
{

	for (uint8_t i = 0;  i <  dlmoCandidate.n ; i++ ){
		dlmoCandidate.candidate[i].neighbor = 0;
		dlmoCandidate.candidate[i].rsqi = 0;
		dlmoCandidate.candidate[i].rssi = 0;

	}
	dlmoCandidate.n=0;
}
