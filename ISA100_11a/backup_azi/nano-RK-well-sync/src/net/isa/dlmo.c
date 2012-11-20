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
#include <dlmo.h>

//********************** Global variables*************************************
DLMO_LINK dlmoLink[DLMO_LINK_MAX_COUNT];
DLMO_NEIGHBOR dlmoNeighbor[DLMO_NEIGHBOR_MAX_COUNT];
ISA_QUEUE isaQueue[TRANSMIT_QUEUE_MAX_SIZE] ;
//ISA_QUEUE *isaQueuePointer[TRANSMIT_QUEUE_MAX_SIZE]
uint8_t isa_sched[ISA_SLOTS_PER_FRAME];
uint64_t isa_tdma_rx_mask;	//should not need this
uint64_t isa_tdma_tx_mask;	// should not need this
uint64_t isa_slot;	//Set to 1 if slot is in use
uint8_t isaTxQueueSize; //holds the number of elements present in the Queue

//********************Local function definitions***********************************
void configureSlot(uint8_t slotNumber, uint16_t neighborId, LinkType linkType, bool clockSource);
void dlmoInit();
int8_t addLink (uint8_t slotNumber, DLMO_NEIGHBOR* neighborIndex,LinkType linkType);
DLMO_NEIGHBOR* addNeighbor(uint16_t index,uint64_t EUI64, uint8_t groupCode1, uint8_t groupCode2, bool clockSource,uint8_t  linkBacklogIndex,uint8_t linkBacklogDur, uint8_t linkBacklogActivate);
int8_t dd_data_request(uint16_t srcAddr, uint16_t destAddr, uint8_t priority, bool discardEligible, uint8_t ecn, bool lh, uint8_t contractId, uint8_t length, uint8_t *payload,  void (*slot_callback)(ISA_QUEUE *entry));
void sendPacket(uint16_t destAddr, uint8_t length, uint8_t *payload, void (*slot_callback)(ISA_QUEUE *entry)) ;
int8_t enQueue(uint16_t neighbor, uint8_t priority, uint8_t length, uint8_t *payload,  void (*slot_callback)(ISA_QUEUE *entry));
void isaFreePacket(ISA_QUEUE *entry);

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
}


int8_t addLink(uint8_t slotNumber, DLMO_NEIGHBOR* neighborIndex,LinkType linkType)
{

	uint8_t index;
	//find if there exists a free link
	for (index = 0; index < DLMO_LINK_MAX_COUNT; index++)
		{
			if (dlmoLink[index].isPresent == false) break;
		}

	if (index == DLMO_LINK_MAX_COUNT)
		{
			 setIsaError(LINK_CAPACITY_ERROR);
			 return -1;
		}

	//we have found a free index
	 dlmoLink[index].isPresent = true;
	 dlmoLink[index].neighbor = neighborIndex;
	 dlmoLink[index].linkType = linkType;
	 dlmoLink[index].chOffset = slotNumber;
	 // channel offset implementation will change as the protocol develops
	return 0;
}

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

DLMO_NEIGHBOR* addNeighbor(uint16_t index,uint64_t EUI64, uint8_t groupCode1, uint8_t groupCode2,bool clockSource,uint8_t  linkBacklogIndex,uint8_t linkBacklogDur, uint8_t linkBacklogActivate)
{
	uint8_t i,free_index=0;
	bool free_index_present = false;
	for(int i=0;i<DLMO_NEIGHBOR_MAX_COUNT;i++)
	{
		if(index == dlmoNeighbor[i].index)
		{
			printf("Neighbor Exists in Table at %d\n",index);
			return &dlmoNeighbor[free_index];
		}
		if(dlmoNeighbor[i].isPresent == false)
		{
					free_index_present = true;
					free_index = i;
		}
	}
	if(free_index_present == false)
	{
			setIsaError(NEIGHBOR_CAPACITY_ERROR);
			return NULL;
	}
	else
	{
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



void configureSlot(uint8_t slotNumber, uint16_t neighborId, LinkType linkType, bool clockSource)
{
	DLMO_NEIGHBOR* neighborIndex; 		//store the neighbor index to pass to addLink()
	if (slotNumber >=  ISA_MAX_SLOTS) {
		printf ("Slot number not in range");
		return;
	}

	//Call the function to add a neighbor
	neighborIndex = addNeighbor(neighborId,0, 0, 0, clockSource,0,0, 0);
	if (neighborIndex == NULL)//
    {
		printIsaError();
		return;
	}

	if (addLink(slotNumber, neighborIndex, linkType) == -1)
	{
		printIsaError();
		return;
	}
//record that the slot is in use- used to calculate next wakeup
	isa_slot |= ((uint64_t) 1) << slotNumber;

}


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

    current_local_slot = current_global_slot%25;
    testVariable |= ((uint64_t) 1) << (current_local_slot+1);

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
/*
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


 int8_t dd_data_request(uint16_t srcAddr, uint16_t destAddr, uint8_t priority, bool discardEligible, uint8_t ecn, bool lh, uint8_t contractId, uint8_t length, uint8_t *payload,  void (*slot_callback)(ISA_QUEUE *entry))
 {
	 //Future - Lookup DestAddr and add into the Queue with for a particular next hop

//Configure the headers within the payload (whichever applicable)

return  enQueue (destAddr, priority, length, payload, slot_callback);


}

//Wrapper for dd_data_request

 void sendPacket(uint16_t destAddr, uint8_t length, uint8_t *payload, void (*slot_callback)(ISA_QUEUE *entry)){
	 if (dd_data_request(0, destAddr, 0, 0, 0, 0, 0,  length, payload,   slot_callback) == -1){
		 printIsaError();
	 }
 }

//****************************************Functions for Queue***********************************************

 /*
  * NOTE : We are using a layer of pointers so that we can easily reorder elements in the queue without having to copy entire entries
  * 		We are only swapping the pointers. Basically using a layer of indirection
  * Operation of addition into the queue is as follows
  * 	Iterate through the queue to find a free slot to insert
  * 	Make sure we don't pass any lower priority valid entries
  * 		If we dont and find a free entry, we insert into this entry and then fix from here onwards
  * 		Fix means swapping if the next valid entry is of higher priority, thus finding the right place in the queue for the new entry
  *
  * 		If we do pass a lower priority entry without being able to find a place to insert then
  * 		Record the place where we crossed this entry
  * 		Run till we can find a place to insert and insert
  * 		Fix in the backward direction till the recorded index
  */

 int8_t enQueue(uint16_t neighbor, uint8_t priority, uint8_t length, uint8_t *payload,  void (*slot_callback)(ISA_QUEUE *entry))
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
	 		 return -1;
	 	 }

	 	 //check if length of payload is within bounds
	 	 if (length >= RF_MAX_PAYLOAD_SIZE) {
	 		 setIsaError(MAX_PAYLOAD_ERROR);
	 		 return -1;
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
			 memcpy(isaQueue[i].tx_buf, payload, length );//copy the payload
			 nrk_time_get(&isaQueue[i].time);	//copy the time when I was inserted into Queue
			 isaTxQueueSize++;
			 break;
		 }

	 }
	 if ( i == TRANSMIT_QUEUE_MAX_SIZE){
		 printf(" Critical error 2");
		 return -1;
	 }
	 return 1;

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


 ISA_QUEUE * getHighPriorityEntry(uint16_t neighbor){

	 nrk_time_t time;
	 uint8_t priority = 0;
	 ISA_QUEUE* tempIndex;
	 bool found = false;
	 uint8_t i;
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

 //*********************************************************************************************************
 isaFreePacket(ISA_QUEUE *entry){
	 entry->usedSlot = false;
	 isaTxQueueSize--;
 }

