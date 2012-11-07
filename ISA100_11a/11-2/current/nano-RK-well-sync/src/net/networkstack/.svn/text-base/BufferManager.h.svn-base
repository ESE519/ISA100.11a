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
*  Aditya Bhave
*******************************************************************************/


/* This file contains the data structures and constants needed for the implementation
	of the buffer manager of the network stack
*/
#ifndef _BUFFER_MANAGER_H
#define _BUFFER_MANAGER_H

#include <stdint.h>

#include "TransportLayerUDP.h"
#include "NetworkLayer.h"



/***************************** CONSTANTS **********************************************/

#define DEBUG_BM 0 					// debug flag for the buffer manager 

#define DEFAULT_RX_QUEUE_SIZE 1 	// each port will be given at least this much receive buffer space 

// the constants below define the values of the 'status' field of ReceiveBufferUDP  
#define UNALLOCATED 			1	// has not been given to any user task 
#define EMPTY					2 	// allocated and free to receive the next segment from the network layer 
#define FULL					3	// allocated and contains a segment received from the network layer 

// the constants 'FULL' and 'EMPTY' also apply to the 'status' field of the TransmitBuffer
// with the obvious meanings 

// the constants below define the values of the excess policy  
#define OVERWRITE 0  
#define DROP 1 

// priority level of task ....SHIFT this to Anthony's code 
#define MAX_TASK_PRIORITY 19   
 
/***************************** DATA STRUCTURES ****************************************/

typedef struct ReceiveBufferUDP
{
	Transport_Segment_UDP seg;			// this holds one UDP segment for the application layer 
	int8_t status;						   // indicates the state of the receive buffer 
	int8_t prio;							// indicates the priority of the segment contained 
												// this is set by the sending node 
	int8_t rssi;							// signal strength with which the underlying packet was received
	uint16_t srcAddr;						// node from which this segment was sent  
	struct ReceiveBufferUDP *next;	// pointer for link list creation 
}ReceiveBufferUDP;

typedef struct
{
	int8_t pid;								// pid of task associated with the port
	int8_t pindex;							// index of associated port element 
	ReceiveBufferUDP *head_fq;			// pointer to head of free queue (EMPTY or FULL buffers)
	ReceiveBufferUDP *tail_fq;			// pointer to tail of free queue (EMPTY or FULL buffers) 
	ReceiveBufferUDP *head_pq; 		// pointer to head of port queue (FULL buffers)	
	ReceiveBufferUDP *tail_pq; 		// pointer to tail of port queue (FULL buffers) 
	int8_t countTotal;					// total number of buffers reserved for this port 
	int8_t countFree;						// number of buffers EMPTY at any time 
}ReceiveBufferManager;					// one for every port in the system 

typedef struct TransmitBuffer
{
	NW_Packet pkt;							// this holds the packet to be transmitted
	int8_t status;							// indicates the status of the transmit buffer 
	int8_t prio;							// priority of packet. This is set by the task that
												// inserts this packet in the transmit queue 
	
	struct TransmitBuffer *next;		// pointer for link list creation 
}TransmitBuffer;

typedef struct 
{
	TransmitBuffer *head_fq;		// pointer to head of free queue (EMPTY buffers) 
	TransmitBuffer *tail_fq;		// pointer to tail of free queue (EMPTY buffers)
	TransmitBuffer *head_aq;		// pointer to head of active queue (FULL buffers) 
	TransmitBuffer *tail_aq;		// pointer to tail of active queue (FULL buffers) 
	int8_t count_aq;					// count of number of buffers in active queue			
	int8_t count_fq;					// count of number of buffers in free queue 
}TransmitBufferManager;

/***************************** FUNCTION PROTOTYPES ************************************/

// Function prototypes defined in BufferManager.c

void initialise_buffer_manager();
/*
This function initialises the receive and transmit buffers of the system. 
This should be called during the initialization phase of the node

	PARAMS:		None
	RETURNS:		None
	COMMENTS:	None
*/

int8_t is_excess_policy_valid(int8_t pref);
/*
This function checks whether a given excess policy preference is valid or not

	PARAMS:		pref: The preference
	RETURNS		TRUE or FALSE depending on whether pref is valid or not 
	COMMENTS:	private function  
*/ 

int8_t set_excess_policy(int8_t prio, int8_t pref);
/*
This function sets the excess policy of a given priority level to a given value

	PARAMS:			prio: The priority level
						pref: The preference type..OVERWRITE or DROP
	RETURNS:			NRK_OK if the setting is assigned correctly
						NRK_ERROR otherwise
						
	ERROR NOS:		INVALID_ARGUMENT if one or more of the input parameters is invalid
	COMMENTS:		user API
*/

int8_t get_excess_policy(int8_t prio);
/*
This function returns the excess policy setting for a given priority level

	PARAMS:		prio: The priority level
	RETURNS: 	OVERWRITE or DROP depending on the setting
					NRK_ERROR otherwise
	
	ERROR NOS:	INVALID_ARGUMENT if the input paramter is invalid
	COMMENTS:	user API
*/

int8_t get_index_unallocated_rx_buf();
/*
This function returns the index of the next unallocated receive buffer from the pool

	PARAMS:		None
	RETURNS:		index of the unallocated receive buffer (0 - (MAX_RX_QUEUE_SIZE - 1) ) if found
					NRK_ERROR otherwise
	
	COMMENTS:	private function
*/

int8_t port_to_rbm_index(uint8_t port);
/*
This function finds the index of the rbm element corresponding to a given port number

	PARAMS:		port: The port number
	RETURNS:		the index of the rbm element if present or NRK_ERROR if no match is found 
	Comments:	private function
*/

int8_t port_to_port_index(uint8_t port);
/*
This function finds the index of the port element corresponding to a given port number

	PARAMS:		port: The port number
	RETURNS:		the index of the port element if present or NRK_ERROR if no match is found 
	Comments:	private function
*/

void insert_rx_pq(Transport_Segment_UDP *seg, int8_t prio, uint16_t addr, int8_t rssi);
/*
This function inserts a given transport layer segment into the port queue of a given port 
as per its priority level

	PARAMS:		seg:	pointer to the segment to be added to the queue
					prio: priority level of this segment. Set by the sending node
					addr: address of sending node
					rssi: signal strength with which the underlying packet was received over the radio 
					
	RETURNS:		None
	COMMENTS:	private function 
*/

ReceiveBufferUDP* remove_rx_pq(int8_t rbm_index);
/*
This function removes the receive buffer at the head of a given port queue

	PARAMS:		rbm_index: the index of the relevant receive buffer manager
	RETURNS:		pointer to the receive buffer or NULL if the port queue is empty 
	COMMENTS: 	private function
*/ 

void insert_rx_fq(ReceiveBufferUDP *buf, int8_t rbm_index, int8_t status);
/* 
This function inserts a given receive buffer into the free queue of a given port
and sets its status field

	PARAMS:		buf:	pointer to the receive buffer
					rbm_index: the index of the relevant receive buffer manager
					status: EMPTY or FULL
					
	RETURNS:		None
	COMMENTS:	private function 
*/ 

ReceiveBufferUDP* remove_rx_fq(int8_t rbm_index, int8_t status);
/*
This function removes a receive buffer from the free queue with the given status

	PARAMS:		rbm_index: the index of the relevant receive buffer manager
					status: EMPTY or FULL
					
	RETURNS:		pointer to the receive buffer or NULL if no such buffer is found
	COMMENTS:	private function
*/	

int8_t insert_tx_aq(NW_Packet *pkt);
/*
This function inserts a network layer packet into the active transmit queue of the system

	PARAMS:		pkt: The network layer packet to be inserted
	RETURNS:		NRK_OK if the packet was inserted successfully
					NRK_ERROR if there are no more free slots in the transmit buffer
					
	COMMENTS:	private function
*/

TransmitBuffer* remove_tx_aq();
/*
This function removes the highest priority buffer from the active transmit queue 

	PARAMS:		None
	RETURNS:		pointer to the transmit buffer if there is one
					NULL if the active queue is empty 
					
	COMMENTS:	private function
*/ 

void insert_tx_fq(TransmitBuffer *buf);
/*
This function adds a given transmit buffer to the free queue

	PARAMS:		buf: pointer to the buffer to be added
	RETURNS:		None
	COMMENTS:	private function 
*/

TransmitBuffer* remove_tx_fq();
/*
This function removes a transmit buffer from the free queue

	PARAMS:		None
	RETURNS:		pointer to the transmit buffer or NULL if the free queue is empty
	COMMENTS:	private function
	
*/ 

int8_t get_in_process_buf_count(int8_t rbm_index);
/*
This function returns the number of FULL buffers in the free queue associated with a given port

	PARAMS:		rbm_index: the index of the relevant receive buffer manager
	RETURNS:		number of FULL buffers in the free queue 
	COMMENTS:	private function
*/ 

int8_t get_num_bufs_free();
/*
This function returns the number of unallocated rx buffers in the system at any time

	PARAMS:		None
	RETURNS:		value of num_bufs_free
	Comments:	private function
*/

void set_num_bufs_free(int8_t n);
/*
This function sets the value of num_bufs_free

	PARAMS:		n: the value to be set
	RETURNS:		None
	Comments:	private function
*/

void print_tx_buffer();
/*
This debugging function prints out the contents of the transmit buffer at any time 

	PARAMS:		None
	RETURNS:		None
	Comments: 	private function
*/

int8_t convert_ptr_to_index(TransmitBuffer *ptr);
void print_rx_buffers(uint8_t port);



#endif
