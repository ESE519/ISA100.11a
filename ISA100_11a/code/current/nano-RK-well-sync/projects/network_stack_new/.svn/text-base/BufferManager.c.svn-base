/* This file contains the implementation of the buffer manager of the network stack */

#include <stdint.h>
#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <nrk_error.h>

#include "BufferManager.h" 
#include "NWErrorCodes.h"
#include "Debug.h"

/***************************** External variables/ functions ************************/
// From TransportLayerUDP.c
extern Port ports[];
extern void print_seg(Transport_Segment_UDP *);

// From NetworkLayer.c 
extern void print_pkt_header();

// From CriticalRegion.c
extern void enter_cr(nrk_sem_t *, int8_t);
extern void leave_cr(nrk_sem_t *, int8_t);

/**************************** Global buffers required by all tasks ******************/
ReceiveBufferUDP rx_buf_udp[MAX_RX_QUEUE_SIZE];		// receive buffer pool in the system
ReceiveBufferManager rx_buf_mgr[NUM_PORTS];			// create one ReceiveBufferManager per port
													// port 0 is reserved lication layer tasks
int8_t num_bufs_free;								// actual number of buffers free
uint8_t total_wait_time;							// total waiting time of packets in the transmit queue
uint8_t total_pkts_ins;								// total number of packets inserted (mostly also transmitted) by this node
 
static TransmitBuffer tx_buf[MAX_TX_QUEUE_SIZE];	// transmit queue for the entire system 
TransmitBufferManager tx_buf_mgr;					// one transmit manager for the whole system 

uint32_t excessPolicySettings;						// 32 bit unsigned integer to hold excess policy settings
nrk_sem_t *bm_sem;									// semaphore to access the above variables

/***************************************************************************************/
void initialise_nw_packet(NW_Packet *pkt)	// CHECKED
{
	int8_t i;
	
	pkt -> src = INVALID_ADDR;
	pkt -> dest = INVALID_ADDR;
	pkt -> ttl = 0; 
	pkt -> type = INVALID;
	pkt -> length = 0;
	pkt -> prio = 0;
	
	for(i = 0; i < MAX_NETWORK_PAYLOAD; i++)
		(pkt -> data)[i] = 0;
		
	return;
}
/***********************************************************************************/
void initialise_buffer_manager()	// CHECKED
{
	int8_t i;	// loop index 
	
	bm_sem = nrk_sem_create(1,MAX_TASK_PRIORITY);	// create the mutex  
	if(bm_sem == NULL)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		nrk_kprintf(PSTR("initialise_buffer_manager(): Error creating the semaphore\r\n"));
	}	
	
	// initialise the receive buffers
	for(i = 0; i < MAX_RX_QUEUE_SIZE; i++)
	{
		rx_buf_udp[i].status = UNALLOCATED;		// initially all rx buffers are unallocated 
		rx_buf_udp[i].next = NULL;				// no links formed as of yet
	}
	num_bufs_free = MAX_RX_QUEUE_SIZE;			// initially all rx buffers are unallocated 
	total_wait_time = 0;						// initially no pkts are waiting in the TX queue
	total_pkts_ins = 0;
	
	// initialise the receive buffer managers
	for(i = 0; i < NUM_PORTS; i++)				 
	{
		rx_buf_mgr[i].pid = INVALID_PID;			// pid of task associated with the port 
		rx_buf_mgr[i].pindex = -1;					// indicates absence of rbm/port mapping 
		rx_buf_mgr[i].head_fq = NULL;
		rx_buf_mgr[i].tail_fq = NULL;
		rx_buf_mgr[i].head_pq = NULL;
		rx_buf_mgr[i].tail_pq = NULL;
		rx_buf_mgr[i].countTotal = 0;
		rx_buf_mgr[i].countFree = 0;
	}
	
	// initialise the transmit buffer manager 
	tx_buf_mgr.head_fq = NULL;
	tx_buf_mgr.tail_fq = NULL;
	tx_buf_mgr.head_aq = NULL;
	tx_buf_mgr.tail_aq = NULL;
	tx_buf_mgr.count_fq = 0;
	tx_buf_mgr.count_aq = 0;
	
	// initialise the transmit buffers
	for(i = 0; i < MAX_TX_QUEUE_SIZE; i++)
	{
		tx_buf[i].status = EMPTY;
		tx_buf[i].next = NULL;
		insert_tx_fq( &(tx_buf[i]) );	// form the free list of TX buffers
	}
	
	// initialise the excess policy settings 
	excessPolicySettings = OVERWRITE;	// by default, its OVERWRITE for all priority levels 	
	
	return;
}
/*******************************************************************************************/
void print_tx_buffer()	// FOR DEBUGGING
{
	int8_t i;
	
	nrk_kprintf(PSTR("Transmit Buffer: "));
	if(tx_buf_mgr.head_fq == NULL)
		printf("-1 ");
	else
		printf("%d ", convert_ptr_to_index(tx_buf_mgr.head_fq));
			
	if(tx_buf_mgr.tail_fq == NULL)
		printf("-1 ");
	else
		printf("%d ", convert_ptr_to_index(tx_buf_mgr.tail_fq));
	
	if(tx_buf_mgr.head_aq == NULL)
		printf("-1 ");
	else
		printf("%d ", convert_ptr_to_index(tx_buf_mgr.head_aq));

	if(tx_buf_mgr.tail_aq == NULL)
		printf("-1\n");
	else
		printf("%d\n", convert_ptr_to_index(tx_buf_mgr.tail_aq));
	
	for(i = 0; i < MAX_TX_QUEUE_SIZE; i++)
		print_pkt(&(tx_buf[i].pkt));
		
	return;
}
/********************************************************************************************/
void print_rx_buffers(uint8_t port)	// FOR DEBUGGING
{
	int8_t rbm_index;
	ReceiveBufferUDP *buf;
	
	rbm_index = port_to_rbm_index(port);
	
	if(DEBUG_BM == 0)
	{
		nrk_kprintf(PSTR("BM: rbm_index = "));
		printf("%d\r\n", rbm_index);
		
		nrk_kprintf(PSTR("Port queue:\r\n"));
		buf = rx_buf_mgr[rbm_index].head_pq;
		while(buf != NULL)
		{
			printf("%d ", buf -> srcAddr);
			print_seg( &(buf -> seg) );
		}
	}
	
	return;
}		
/********************************************************************************************/
// Given a pointer to a TransmitBuffer, what is it's index in the global array?
int8_t convert_ptr_to_index(TransmitBuffer *ptr)	// CHECKED
{
	int8_t i;	// loop index
	
	for(i = 0; i < MAX_TX_QUEUE_SIZE; i++)
		if(ptr == &tx_buf[i])
			break;
			
	return i;
}
/*******************************************************************************************/
int8_t is_excess_policy_valid(int8_t pref)	// CHECKED
{
	switch(pref)
	{
		case OVERWRITE:
		case DROP:
			return TRUE;
	}
	return FALSE; 
}
/**************************************************************************/
int8_t set_excess_policy(int8_t prio, int8_t pref)	// user API, CHECKED
{
	if( (prio <= 0) || (prio > MAX_PRIORITY) || (is_excess_policy_valid(pref) == FALSE) )
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		return NRK_ERROR;
	}
	
	enter_cr(bm_sem, 39);	// enter CR 
		
	if(pref == DROP)	// set bit number 'prio' in excessPolicySettings to 1
		excessPolicySettings |= (uint32_t)1 << prio;
	else				// set bit number 'prio' in excessPolicySettings to 0
		excessPolicySettings &= ~( (uint32_t)1 << prio );
	
	leave_cr(bm_sem, 39);	// leave CR 
		
	return NRK_OK;
}
/***********************************************************************************/
int8_t get_excess_policy(int8_t prio)	// user API, CHECKED
{
	int8_t policy;
	
	if( (prio <= 0) || (prio > MAX_PRIORITY) )
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		return NRK_ERROR;
	}	
	
	// check the state of bit number 'prio' in excessPolicySettings
	enter_cr(bm_sem, 40);
	policy = (excessPolicySettings >> prio) & (uint32_t)1;
	leave_cr(bm_sem, 40);
	
	if(policy == 0)		// bit 'prio' is cleared
		return OVERWRITE;
	
	return DROP;		// bit 'prio' is set
}
/************************************************************************************/
int8_t get_index_unallocated_rx_buf()	// CHECKED
{
	int8_t i;	// loop index 
	
	for(i = 0; i < MAX_RX_QUEUE_SIZE; i++)
		if(rx_buf_udp[i].status == UNALLOCATED)	// the buffer at this index is free 
			return i;
			
	return NRK_ERROR;
}
/**************************************************************************************/
// Given a port number, what RBM element is responsible for the port?
int8_t port_to_rbm_index(uint8_t port)	// CHECKED
{
	int8_t i;	// loop index
	
	for(i = 0; i < NUM_PORTS; i++)
		if(rx_buf_mgr[i].pindex != -1)	// valid rbm element 
			if(ports[rx_buf_mgr[i].pindex].pno == port)	// found the matching rbm element 
				return i;
	
	// if the code reaches here, it means no match was found
	return NRK_ERROR;
}
/**************************************************************************************/
// Given a port number, what Port element is responsible for it?
int8_t port_to_port_index(uint8_t port)	// CHECKED
{
	int8_t i;	// loop index
	
	for(i = 0; i < NUM_PORTS; i++)
		if(ports[i].pno == port)	// match found
			return i;
	
	// if the code reaches here, it means no match was found
	return NRK_ERROR;
}
/**************************************************************************************/

/**************************************************************************************/
// This function is called assuming the port and its resources exist
void insert_rx_pq(Transport_Segment_UDP *seg, int8_t prio, uint16_t addr, int8_t rssi)	// CHECKED
{
	int8_t rbm_index;						// index of the corresponding rbm element 
	ReceiveBufferUDP *buf, *ptr, *prev;		// temporary pointers
	
	rbm_index = port_to_rbm_index(seg -> destPort);	// retrieve the index of the RBM element	
	buf = remove_rx_fq(rbm_index, EMPTY);	// remove an EMPTY buffer from the free queue
	
	// pointers for traversing the port queue 
	ptr = rx_buf_mgr[rbm_index].head_pq;				 
	prev = NULL; 
	
	// find the buffer in the port queue holding the next lower priority segment	
	while(ptr != NULL)
	{
		if(ptr -> prio < prio)
			break;
			
		prev = ptr;
		ptr = ptr -> next;
	}
	// at this point, 'ptr' points to this buffer or possibly NULL	
	if(buf != NULL)	// a buffer is available to hold the newly received segment 
	{
		// fill up the members of ReceiveBufferUDP		
		buf -> seg = *seg;	
		buf -> status = FULL;
		buf -> prio = prio;
		buf -> srcAddr = addr;
		buf -> rssi = rssi;
		buf -> next = ptr;
		
		// adjust the pointers
		// case 1: the port queue is empty
		if(rx_buf_mgr[rbm_index].head_pq == NULL)			// rx pq is empty
		{
			rx_buf_mgr[rbm_index].head_pq = rx_buf_mgr[rbm_index].tail_pq = buf; 
		}
		// case 2: newly received segment is to be inserted at the head of the pq
		else if(prev == NULL)	 
					rx_buf_mgr[rbm_index].head_pq = buf;
			  else
			  {		// case 3: buf needs to be inserted somewhere within the queue
			  		prev -> next = buf;
			  		// case 4: buf needs to be inserted at the tail of the pq
			  		if(prev == rx_buf_mgr[rbm_index].tail_pq)
			  			rx_buf_mgr[rbm_index].tail_pq = buf;
			  }
			
		if(DEBUG_BM == 1)
		{
			nrk_kprintf(PSTR("Added a segment to free buffer: "));
			printf("%d\r\n", seg -> data[0]);
		}
		return;  
	}
	
	// if the code reaches here, it means that a buffer is not available to hold this new segment 
	// There are three possibilities
	// 1. The free queue is empty and the port queue is full
	// 2. The port queue is empty and the free queue is full of 'FULL' buffers
	// 3. Some combination of the above 2	
	
	// check for 2 
	if(rx_buf_mgr[rbm_index].head_pq == NULL)
	{
		if(rx_buf_mgr[rbm_index].countFree != 0)	// sanity check 
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			nrk_kprintf(PSTR("insert_rx_pq(): Bug found in implementation of countFree in rx_buf_mgr\r\n"));
		}
		if(rx_buf_mgr[rbm_index].tail_pq != NULL)	// sanity check 
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			nrk_kprintf(PSTR("insert_rx_pq(): Bug found in implementation of tail_pq in rx_buf_mgr\r\n"));
		}
		
		if(DEBUG_BM == 0)
		{
			nrk_kprintf(PSTR("BM: Free queue is full of FULL buffers\r\n"));
		}
		// no free space and the port queue is also empty. Drop the new segment
		return;		
	}	
	// the situation could be 1 or 3. The behavior is the same. The lowest priority message 
	// in the port queue has to be replaced	  
	// check if the last segment in the port queue has a lower priority
	if( (rx_buf_mgr[rbm_index].tail_pq) -> prio < prio ) // this last segment should be replaced
	{
		ReceiveBufferUDP *qtr;
		// at this point we know that ptr cannot be NULL		
		// find the last but one segment in the port queue 
		qtr = rx_buf_mgr[rbm_index].head_pq;
		// corner case, see if there is only one buffer in the port queue 
		if(rx_buf_mgr[rbm_index].head_pq == rx_buf_mgr[rbm_index].tail_pq)
		{
			// replace the members of this last buffer with those of the new one 
			qtr -> seg = *seg;
			qtr -> prio = prio;
			qtr -> srcAddr = addr;
			qtr -> rssi = rssi;		// message inserted at the tail of the queue 
			
			return;
		}
		else	// there is more than one buffer in the port queue 
		{
			while(qtr -> next != rx_buf_mgr[rbm_index].tail_pq)	// qtr will now point to the last but one segment 
				qtr = qtr -> next;
				
			// replace the members of the last buffer with those of the new one
			(rx_buf_mgr[rbm_index].tail_pq) -> seg = *seg;
			(rx_buf_mgr[rbm_index].tail_pq) -> prio = prio;
			(rx_buf_mgr[rbm_index].tail_pq) -> srcAddr = addr;
			(rx_buf_mgr[rbm_index].tail_pq) -> rssi = rssi;
				
			// adjust the pointers so that this new segment occupies its correct position in the port queue			
			(rx_buf_mgr[rbm_index].tail_pq) -> next = ptr;
			if(prev == NULL)
				rx_buf_mgr[rbm_index].head_pq = rx_buf_mgr[rbm_index].tail_pq;	// new segment has to be inserted at the head
			else
				prev -> next = rx_buf_mgr[rbm_index].tail_pq;	// new segment has to be inserted within the pq
					
			qtr -> next = NULL;						// qtr now ALWAYS points to the last element in the pq
			rx_buf_mgr[rbm_index].tail_pq = qtr;	// change the tail of the port queue
			
			return;
		}
	} // end if 
	else if( (rx_buf_mgr[rbm_index].tail_pq) -> prio == prio )	// last segment has the same priority	
		  {
		  		// check the setting of the excess policy for this priority level 
		  		if( ((excessPolicySettings >> prio) & ((uint32_t)1)) == OVERWRITE)	// new segment is more important 
		  		{
		  			if(DEBUG_BM == 0)
		  			{
		  				nrk_kprintf(PSTR("BM: Inside the section for OVERWRITE\r\n"));
		  			}
		  			// replace the members of the last buffer with those of the new one 
		  			(rx_buf_mgr[rbm_index].tail_pq) -> seg = *seg;
					(rx_buf_mgr[rbm_index].tail_pq) -> prio = prio;
					(rx_buf_mgr[rbm_index].tail_pq) -> srcAddr = addr;
					(rx_buf_mgr[rbm_index].tail_pq) -> rssi = rssi;
					
					return;
				}
				else			// setting is DROP. The new segment has to be dropped 
				{
					if(DEBUG_BM == 0)
		  			{
		  				nrk_kprintf(PSTR("BM: Dropping message: "));
		  				printf("%d\r\n", seg -> data[0]);		  				
		  			}
					return;	// do nothing
				} 
		  }
		  else				// new segment has lower priority. This has to be dropped 
		  {
		  		return;		// do nothing 
		  }
	//possibility 1 or 3 checked 
	
	return;
}
/*************************************************************************************************/
ReceiveBufferUDP* remove_rx_pq(int8_t rbm_index)	// CHECKED
{
	// The remove is always done from the head of the queue since its a priority queue	
	ReceiveBufferUDP *ptr;
	
	if(rx_buf_mgr[rbm_index].head_pq == NULL)	// no more queued segments
	{
		return NULL;
	}
		
	if(rx_buf_mgr[rbm_index].head_pq == rx_buf_mgr[rbm_index].tail_pq)	// only one buffer in queue 
		rx_buf_mgr[rbm_index].tail_pq = NULL;
		
	ptr = rx_buf_mgr[rbm_index].head_pq;
	
	rx_buf_mgr[rbm_index].head_pq = rx_buf_mgr[rbm_index].head_pq -> next;
		
	return ptr;
}
/*************************************************************************************************/
void insert_rx_fq(ReceiveBufferUDP *buf, int8_t rbm_index, int8_t status)	// CHECKED
{
	// the insert is always done at the tail of the queue 
	if(rx_buf_mgr[rbm_index].head_fq == NULL)	// the free queue is empty 
	{
		rx_buf_mgr[rbm_index].head_fq = rx_buf_mgr[rbm_index].tail_fq = buf;
		buf -> next = NULL;
	}	
	else
	{
		rx_buf_mgr[rbm_index].tail_fq -> next = buf;
		buf -> next = NULL;
		rx_buf_mgr[rbm_index].tail_fq = buf;
	}

	if(status == EMPTY)	// this function was called from bind() / setReceiveQueueSize()
	{
		rx_buf_mgr[rbm_index].countFree++;	// increment the status variables
		buf -> status = EMPTY;
	}
	
	return;
}
/***************************************************************************************************/
ReceiveBufferUDP* remove_rx_fq(int8_t rbm_index, int8_t status)		// CHECKED
{
	// This function removes the next receive buffer from the free queue having a given status 	
	ReceiveBufferUDP *ptr;
	ReceiveBufferUDP *prev = NULL;
	
	if(rx_buf_mgr[rbm_index].head_fq == NULL)	// no buffers remaining in free queue	
		return NULL;
		
	ptr = rx_buf_mgr[rbm_index].head_fq;
	while(ptr != NULL)							// traverse the free queue
	{
		if(status == EMPTY)
		{
			if(ptr -> status == EMPTY)	// status = EMPTY
				break;
		}
		else if(ptr -> status == FULL)	// status = FULL 
				break;
		
		prev = ptr;		
		ptr = ptr -> next;
	}	
	if(ptr == NULL)	// the EMPTY / FULL buffer was not found in the free queue
		return NULL;
	
	if(prev == NULL)	// the EMPTY / FULL buffer is at the head of the queue
	{
		if(rx_buf_mgr[rbm_index].head_fq == rx_buf_mgr[rbm_index].tail_fq)	// only one buffer in free queue
			rx_buf_mgr[rbm_index].tail_fq = NULL;
			
		rx_buf_mgr[rbm_index].head_fq = rx_buf_mgr[rbm_index].head_fq -> next;
		if(status == EMPTY)
			rx_buf_mgr[rbm_index].countFree--;
		return ptr;
	}
	
	// EMPTY / FREE buffer is somewhere in the free queue 
	prev -> next = ptr -> next;
	if(status == EMPTY)
		rx_buf_mgr[rbm_index].countFree--;
	return ptr;
}
/*********************************************************************************************/
int8_t insert_tx_aq(NW_Packet *pkt)				// CHECKED
{
	TransmitBuffer *buf = remove_tx_fq();		// remove an available buffer from the free queue
	TransmitBuffer *ptr;
	TransmitBuffer *prev;
	nrk_time_t curr_time;
	
	if(buf == NULL)								// no more free buffers available 
	{
		return NRK_ERROR;
	}
	
	// fill up the members of TransmitBuffer 
	buf -> pkt = *pkt;
	buf -> status = FULL;
	buf -> prio = pkt -> prio;
	nrk_time_get(&curr_time);
	nrk_time_compact_nanos(&curr_time);
	buf -> timestamp = curr_time.secs;
		
	if(tx_buf_mgr.head_aq == NULL)				// the active queue is empty. Corner case  
	{
		buf -> next = NULL;
		tx_buf_mgr.head_aq = tx_buf_mgr.tail_aq = buf;
		tx_buf_mgr.count_aq++;
				
		return NRK_OK;
	}
	// active queue is not empty. Find the correct position of insertion
	// pointers for traversing the active queue 
	ptr = tx_buf_mgr.head_aq;				 
	prev = NULL; 
	
	// find the buffer holding the next lower priority packet	
	while(ptr != NULL)
	{
		if(ptr -> prio < pkt -> prio)
			break;
			
		prev = ptr;
		ptr = ptr -> next;
	}
	buf -> next = ptr;	 // adjust the pointer
		
	if(prev == NULL)	// the new packet is to be inserted at the head of the active queue 
		tx_buf_mgr.head_aq = buf;
	else				// the new packet is to be inserted within the queue
	{
		prev -> next = buf;
		if(prev == tx_buf_mgr.tail_aq)	// the new packet is to be inserted at the tail of the queue
			tx_buf_mgr.tail_aq = buf;
	}
			
	// new packet has been inserted at the proper slot in the active queue
	tx_buf_mgr.count_aq++;	
	
	return NRK_OK; 
}
/***********************************************************************************************/
TransmitBuffer* remove_tx_aq()	// CHECKED
{
	// since the active queue is sorted as per priority, the buffer at the head of the queue 
	// is removed	
	TransmitBuffer *ptr;
	nrk_time_t curr_time;
	
	if(tx_buf_mgr.head_aq == NULL)	// the active queue is empty 
	{
		return NULL;
	}
		
	if(tx_buf_mgr.head_aq == tx_buf_mgr.tail_aq)	// there is only one tx buffer in the active queue 
		tx_buf_mgr.tail_aq = NULL;
		
	ptr = tx_buf_mgr.head_aq;
	
	tx_buf_mgr.head_aq = tx_buf_mgr.head_aq -> next;
	tx_buf_mgr.count_aq--;
	
	// update statistics variables
	nrk_time_get(&curr_time);
	nrk_time_compact_nanos(&curr_time);
	total_wait_time += (uint8_t)(curr_time.secs - ptr -> timestamp);
	total_pkts_ins++;
		
	return ptr;
}
/*************************************************************************************************/
void insert_tx_fq(TransmitBuffer *buf)	// CHECKED
{
	// the buffer is always added at the tail of the free queue 
	
	if(tx_buf_mgr.head_fq == NULL)	// the free queue is empty 
		tx_buf_mgr.head_fq = tx_buf_mgr.tail_fq = buf;
	else
	{
		tx_buf_mgr.tail_fq -> next = buf;
		tx_buf_mgr.tail_fq = buf;
	}
	buf -> status = EMPTY;
	buf -> next = NULL;
	tx_buf_mgr.count_fq++;
	
	return;	
}

/**************************************************************************************************/
TransmitBuffer* remove_tx_fq()	// CHECKED
{
	// The buffer is always removed from the head of the queue 	
	TransmitBuffer *ptr;
		
	if(tx_buf_mgr.head_fq == NULL)	// the free queue is empty 
		return NULL;
		
	if(tx_buf_mgr.head_fq == tx_buf_mgr.tail_fq)	// there is only one free tx buffer 
		tx_buf_mgr.tail_fq = NULL;
		
	ptr = tx_buf_mgr.head_fq;
	
	tx_buf_mgr.head_fq = tx_buf_mgr.head_fq -> next;
	tx_buf_mgr.count_fq--;
	
	return ptr;
}
/**************************************************************************************************/
// Given a port number, how many of it's buffers are FULL and already accepted by the task?
int8_t get_in_process_buf_count(int8_t rbm_index)	// CHECKED
{
	int8_t count = 0;
	ReceiveBufferUDP *ptr = rx_buf_mgr[rbm_index].head_fq;
	
	while(ptr != NULL)
	{
		if(ptr -> status == FULL)
			count++;
		ptr = ptr -> next;
	}
	
	return count;
}
/***************************************************************************************************/
// How many ReceiveBuffer's in the system are free?
int8_t get_num_bufs_free()	// CHECKED
{
	int8_t n;
		
	n = num_bufs_free;
	
	return n;
}
/****************************************************************************************************/
void set_num_bufs_free(int8_t n)	// CHECKED
{
	num_bufs_free = n;
	return;
}
/****************************************************************************************************/
int8_t get_tx_aq_size()	// CHECKED
{
	int8_t n;
	
	enter_cr(bm_sem, 34);
	n = tx_buf_mgr.count_aq;
	leave_cr(bm_sem, 34);
	
	return n;
}
/*******************************************************************************************************/
int8_t get_tx_fq_size()	// CHECKED
{
	int8_t n;
	
	enter_cr(bm_sem, 34);
	n = tx_buf_mgr.count_fq;
	leave_cr(bm_sem, 34);
	
	return n;
}
/********************************************************************************************************/
uint8_t get_avg_wait_time() // CHECKED
{
	uint8_t avg;
	
	enter_cr(bm_sem, 34);
	avg = total_wait_time / total_pkts_ins;
	leave_cr(bm_sem, 34);
	
	return avg;
}
/***********************************************************************************************************/
uint32_t get_total_pkts_ins()
{
	uint32_t num;
	
	enter_cr(bm_sem, 34);
	num = total_pkts_ins;
	leave_cr(bm_sem, 34);
	
	return num;
}
/************************************************************************************************************/
uint32_t get_total_wait_time()
{
	uint32_t wait;
	
	enter_cr(bm_sem, 34);
	wait = total_wait_time;
	leave_cr(bm_sem, 34);
	
	return wait;
}
/************************************ STATISTICS COLLECTION FUNCTIONS **********************************/
/********************************************************************************************************/
void collect_queue_stats(int8_t *txqs, int8_t *rxqs, uint8_t *tpi, uint8_t *twt)	// CHECKED
{
	enter_cr(bm_sem, 34);									// collect queue statistics
	
	*txqs = tx_buf_mgr.count_aq;						// size of TX queue
	*rxqs = MAX_RX_QUEUE_SIZE - num_bufs_free;			// size of RX queue
	*tpi = total_pkts_ins;								// pkts inserted
	*twt = total_wait_time;								// total wait time
	
	total_pkts_ins = 0;										// reset the variables
	total_wait_time = 0;
	
	leave_cr(bm_sem, 34);
	
	return;
}

	
	
	
	
	
	
