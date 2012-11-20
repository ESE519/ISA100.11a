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

/**************************** Global buffers required by all tasks ******************/

ReceiveBufferUDP rx_buf_udp[MAX_RX_QUEUE_SIZE];	// receive buffer pool for the application layer tasks
int8_t num_bufs_free;									// actual number of buffers free
 
static TransmitBuffer tx_buf[MAX_TX_QUEUE_SIZE];			// transmit queue for the entire system 

ReceiveBufferManager rx_buf_mgr[NUM_PORTS];		// create one receive manager per port
																// port 0 is reserved 
TransmitBufferManager tx_buf_mgr;					// one transmit manager for the whole system 

uint32_t excessPolicySettings;						// 32 bit unsigned integer to hold 
																// excess policy settings
																
nrk_sem_t *bm_sem;										// semaphore to access the above variables 

/***************************************************************************************/
void initialise_nw_packet(NW_Packet *pkt)
{
	int8_t i;
	
	pkt -> src = 0;
	pkt -> dest = 0;
	pkt -> ttl = 0; 
	pkt -> type = 0xFF;
	pkt -> length = 0;
	pkt -> prio = 0;
	
	for(i = 0; i < MAX_NETWORK_PAYLOAD; i++)
		(pkt -> data)[i] = 0;
		
	return;
}
/******************************************************************************************/

inline void enter_cr(nrk_sem_t *sem, int8_t fno)
{
	if( nrk_sem_pend(sem) == NRK_ERROR )
	{
		switch(fno)
		{
			/******************************** Transport Layer ******************************/
			case 1:
				nrk_kprintf(PSTR("Error sem pending on initialise_transport_layer_udp()\r\n"));
				break;
				
			case 2: 
				nrk_kprintf(PSTR("Error sem pending on get_next_available_socket()\r\n"));
				break;
				
			case 3: 
				nrk_kprintf(PSTR("Error sem pending on create_socket()\r\n"));
				break;
			
			case 4: 
				nrk_kprintf(PSTR("Error sem pending on get_next_available_port()\r\n"));
				break;
			
			case 5: 
				nrk_kprintf(PSTR("Error sem pending on check_port_available()\r\n"));
				break;
			
			case 6: 
				nrk_kprintf(PSTR("Error sem pending on assign_port()\r\n"));
				break;
			
			case 7: 
				nrk_kprintf(PSTR("Error sem pending on release_port()\r\n"));
				break;
			
			case 8: 
				nrk_kprintf(PSTR("Error sem pending on bind()\r\n"));
				break;
			
			case 9: 
				nrk_kprintf(PSTR("Error sem pending on get_rx_queue_size()\r\n"));
				break;
			
			case 10: 
				nrk_kprintf(PSTR("Error sem pending on set_rx_queue_size()\r\n"));
				break;
			
			case 11: 
				nrk_kprintf(PSTR("Error sem pending on release_buffer()\r\n"));
				break;
			
			case 12: 
				nrk_kprintf(PSTR("Error sem pending on close_socket()\r\n"));
				break;
			
			case 13: 
				nrk_kprintf(PSTR("Error sem pending on is_port_associated()\r\n"));
				break;
			
			case 14: 
				nrk_kprintf(PSTR("Error sem pending on send()\r\n"));
				break;
			
			case 15: 
				nrk_kprintf(PSTR("Error sem pending on set_timeout()\r\n"));
				break;
			
			case 16: 
				nrk_kprintf(PSTR("Error sem pending on receive()\r\n"));
				break;
				
			case 17: 
				nrk_kprintf(PSTR("Error sem pending on check_receive_queue()\r\n"));
				break;
						
			case 18: 
				nrk_kprintf(PSTR("Error sem pending on wait_until_send_done()\r\n"));
				break;
				
			/*************************** Network Layer ********************************/				
			case 19: 
				nrk_kprintf(PSTR("Error sem pending on add_neighbor()\r\n"));
				break;
			
			case 20: 
				nrk_kprintf(PSTR("Error sem pending on shouldIMultihop()\r\n"));
				break;
			
			case 21: 
				nrk_kprintf(PSTR("Error sem pending on multihop()\r\n"));
				break;
				
			case 22:
				nrk_kprintf(PSTR("Error sem pending on route_addr()\r\n"));
			
			case 23: 
				nrk_kprintf(PSTR("Error sem pending on route_packet()\r\n"));
				break;
				
			case 24: 
				nrk_kprintf(PSTR("Error sem pending on sendToGateway()\r\n"));
				break;
			
			case 25: 
				nrk_kprintf(PSTR("Error sem pending on pkt_type()\r\n"));
				break;
				
			case 26: 
				nrk_kprintf(PSTR("Error sem pending on tl_type()\r\n"));
				break;
				
			case 27: 
				nrk_kprintf(PSTR("Error sem pending on nw_ctrl_type()\r\n"));
				break;
			
			case 28: 
				nrk_kprintf(PSTR("Error sem pending on process_app_pkt()\r\n"));
				break;
			
			case 29: 
				nrk_kprintf(PSTR("Error sem pending on process_nw_ctrl_pkt()\r\n"));
				break;
				
			case 30:
				nrk_kprintf(PSTR("Error sem pending on process_other_pkt()\r\n"));
				break;
				
			case 31:
				nrk_kprintf(PSTR("Error sem pending on build_Msg_Hello()\r\n"));
				break;
				
			case 32:
				nrk_kprintf(PSTR("Error sem pending on build_Msg_NgbList()\r\n"));
				break;
				
			case 33:
				nrk_kprintf(PSTR("Error sem pending on nl_rx_task()\r\n"));
				break;
				
			case 34:
				nrk_kprintf(PSTR("Error sem pending on nl_tx_task()\r\n"));
				break;
				
			case 35: 
				nrk_kprintf(PSTR("Error sem pending on create_network_layer_tasks()\r\n"));
				break;
			
			case 36: 
				nrk_kprintf(PSTR("Error sem pending on initialise_network_layer()\r\n"));
				break;
				
			/****************************** BufferManager.c *********************************/
			case 37: 
				nrk_kprintf(PSTR("Error sem pending on initialise_buffer_manager()\r\n"));
				break;
			
			case 38: 
				nrk_kprintf(PSTR("Error sem pending on is_excess_policy_valid()\r\n"));
				break;
			
			case 39: 
				nrk_kprintf(PSTR("Error sem pending on set_excess_policy()\r\n"));
				break;
			
			case 40: 
				nrk_kprintf(PSTR("Error sem pending on get_excess_policy()\r\n"));
				break;
			
			case 41: 
				nrk_kprintf(PSTR("Error sem pending on get_index_unallocated_rx_buf()\r\n"));
				break;
			
			case 42: 
				nrk_kprintf(PSTR("Error sem pending on insert_rx_pq()\r\n"));
				break;
			
			case 43: 
				nrk_kprintf(PSTR("Error sem pending on remove_rx_pq()\r\n"));
				break;
			
			case 44: 
				nrk_kprintf(PSTR("Error sem pending on insert_rx_fq()\r\n"));
				break;
			
			case 45: 
				nrk_kprintf(PSTR("Error sem pending on remove_rx_fq()\r\n"));
				break;
				
			case 46: 
				nrk_kprintf(PSTR("Error sem pending on insert_tx_aq()\r\n"));
				break;
			
			case 47: 
				nrk_kprintf(PSTR("Error sem pending on remove_tx_aq()\r\n"));
				break;
			
			case 48: 
				nrk_kprintf(PSTR("Error sem pending on insert_tx_fq()\r\n"));
				break;
			
			case 49: 
				nrk_kprintf(PSTR("Error sem pending on remove_tx_fq()\r\n"));
				break;
			
			case 50: 
				nrk_kprintf(PSTR("Error sem pending on get_in_process_buf_count()\r\n"));
				break;
			
			/******************************************************************************/
			
			default:
				nrk_kprintf(PSTR("enter_cr(): Unknown function number\r\n"));
				break;
		} // end switch 
	} // end if 
			
	return;
}

/******************************************************************************************/
inline void leave_cr(nrk_sem_t *sem, int8_t fno)
{
	if( nrk_sem_post(sem) == NRK_ERROR )
	{
		switch(fno)
		{
			/******************************** Transport Layer ******************************/
			case 1:
				nrk_kprintf(PSTR("Error sem posting on initialise_transport_layer_udp()\r\n"));
				break;
				
			case 2: 
				nrk_kprintf(PSTR("Error sem posting on get_next_available_socket()\r\n"));
				break;
				
			case 3: 
				nrk_kprintf(PSTR("Error sem posting on create_socket()\r\n"));
				break;
			
			case 4: 
				nrk_kprintf(PSTR("Error sem posting on get_next_available_port()\r\n"));
				break;
			
			case 5: 
				nrk_kprintf(PSTR("Error sem posting on check_port_available()\r\n"));
				break;
			
			case 6: 
				nrk_kprintf(PSTR("Error sem posting on assign_port()\r\n"));
				break;
			
			case 7: 
				nrk_kprintf(PSTR("Error sem posting on release_port()\r\n"));
				break;
			
			case 8: 
				nrk_kprintf(PSTR("Error sem posting on bind()\r\n"));
				break;
			
			case 9: 
				nrk_kprintf(PSTR("Error sem posting on get_rx_queue_size()\r\n"));
				break;
			
			case 10: 
				nrk_kprintf(PSTR("Error sem posting on set_rx_queue_size()\r\n"));
				break;
			
			case 11: 
				nrk_kprintf(PSTR("Error sem posting on release_buffer()\r\n"));
				break;
			
			case 12: 
				nrk_kprintf(PSTR("Error sem posting on close_socket()\r\n"));
				break;
			
			case 13: 
				nrk_kprintf(PSTR("Error sem posting on is_port_associated()\r\n"));
				break;
			
			case 14: 
				nrk_kprintf(PSTR("Error sem posting on send()\r\n"));
				break;
			
			case 15: 
				while(1)
				{
					uint32_t i;					
					nrk_int_disable();
					for(i = 0; i < 30000000L; i++)
						;	
					nrk_kprintf(PSTR("Error sem posting on set_timeout()\r\n"));
					printf("%d ", nrk_errno_get());
				}				
				
				break;
			
			case 16: 
				nrk_kprintf(PSTR("Error sem posting on receive()\r\n"));
				break;
				
			case 17: 
				nrk_kprintf(PSTR("Error sem posting on check_receive_queue()\r\n"));
				break;
						
			case 18: 
				nrk_kprintf(PSTR("Error sem posting on wait_until_send_done()\r\n"));
				break;
				
			/*************************** Network Layer ********************************/				
			case 19: 
				nrk_kprintf(PSTR("Error sem posting on add_neighbor()\r\n"));
				break;
			
			case 20: 
				nrk_kprintf(PSTR("Error sem posting on shouldIMultihop()\r\n"));
				break;
			
			case 21: 
				nrk_kprintf(PSTR("Error sem posting on multihop()\r\n"));
				break;
				
			case 22:
				nrk_kprintf(PSTR("Error sem posting on route_addr()\r\n"));
			
			case 23: 
				nrk_kprintf(PSTR("Error sem posting on route_packet()\r\n"));
				break;
				
			case 24: 
				nrk_kprintf(PSTR("Error sem posting on sendToGateway()\r\n"));
				break;
			
			case 25: 
				nrk_kprintf(PSTR("Error sem posting on pkt_type()\r\n"));
				break;
				
			case 26: 
				nrk_kprintf(PSTR("Error sem posting on tl_type()\r\n"));
				break;
				
			case 27: 
				nrk_kprintf(PSTR("Error sem posting on nw_ctrl_type()\r\n"));
				break;
			
			case 28: 
				nrk_kprintf(PSTR("Error sem posting on process_app_pkt()\r\n"));
				break;
			
			case 29: 
				nrk_kprintf(PSTR("Error sem posting on process_nw_ctrl_pkt()\r\n"));
				break;
				
			case 30:
				nrk_kprintf(PSTR("Error sem posting on process_other_pkt()\r\n"));
				break;
				
			case 31:
				nrk_kprintf(PSTR("Error sem posting on build_Msg_Hello()\r\n"));
				break;
				
			case 32:
				nrk_kprintf(PSTR("Error sem posting on build_Msg_NgbList()\r\n"));
				break;
				
			case 33:
				nrk_kprintf(PSTR("Error sem posting on nl_rx_task()\r\n"));
				break;
				
			case 34:
				nrk_kprintf(PSTR("Error sem posting on nl_tx_task()\r\n"));
				break;
				
			case 35: 
				nrk_kprintf(PSTR("Error sem posting on create_network_layer_tasks()\r\n"));
				break;
			
			case 36: 
				nrk_kprintf(PSTR("Error sem posting on initialise_network_layer()\r\n"));
				break;
				
			/****************************** BufferManager.c *********************************/
			case 37: 
				nrk_kprintf(PSTR("Error sem posting on initialise_buffer_manager()\r\n"));
				break;
			
			case 38: 
				nrk_kprintf(PSTR("Error sem posting on is_excess_policy_valid()\r\n"));
				break;
			
			case 39: 
				nrk_kprintf(PSTR("Error sem posting on set_excess_policy()\r\n"));
				break;
			
			case 40: 
				nrk_kprintf(PSTR("Error sem posting on get_excess_policy()\r\n"));
				break;
			
			case 41: 
				nrk_kprintf(PSTR("Error sem posting on get_index_unallocated_rx_buf()\r\n"));
				break;
			
			case 42: 
				nrk_kprintf(PSTR("Error sem posting on insert_rx_pq()\r\n"));
				break;
			
			case 43: 
				nrk_kprintf(PSTR("Error sem posting on remove_rx_pq()\r\n"));
				break;
			
			case 44: 
				nrk_kprintf(PSTR("Error sem posting on insert_rx_fq()\r\n"));
				break;
			
			case 45: 
				nrk_kprintf(PSTR("Error sem posting on remove_rx_fq()\r\n"));
				break;
				
			case 46: 
				nrk_kprintf(PSTR("Error sem posting on insert_tx_aq()\r\n"));
				break;
			
			case 47: 
				nrk_kprintf(PSTR("Error sem posting on remove_tx_aq()\r\n"));
				break;
			
			case 48: 
				nrk_kprintf(PSTR("Error sem posting on insert_tx_fq()\r\n"));
				break;
			
			case 49: 
				nrk_kprintf(PSTR("Error sem posting on remove_tx_fq()\r\n"));
				break;
			
			case 50: 
				nrk_kprintf(PSTR("Error sem posting on get_in_process_buf_count()\r\n"));
				break;
			
			/******************************************************************************/
			
			default:
				nrk_kprintf(PSTR("leave_cr(): Unknown function number\r\n"));
				break;
		} // end switch 
	} // end if 
			
	return;
}
/***********************************************************************************/
void initialise_buffer_manager()
{
	int8_t i;	// loop index 
	
	// initialise the receive buffers
	for(i = 0; i < MAX_RX_QUEUE_SIZE; i++)
	{
		rx_buf_udp[i].status = UNALLOCATED;		// initially all rx buffers are unallocated 
		rx_buf_udp[i].next = NULL;					// no links formed as of yet
	}
	num_bufs_free = MAX_RX_QUEUE_SIZE;			// initially all rx buffers are unallocated 
	
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
		initialise_nw_packet( &(tx_buf[i].pkt) );
		insert_tx_fq( &(tx_buf[i]) );
	}
	
	// initialise the excess policy settings 
	excessPolicySettings = 0;						// by default, its OVERWRITE for all priority levels 	
	
	bm_sem = nrk_sem_create(1,MAX_TASK_PRIORITY);	// create the mutex  
	if(bm_sem == NULL)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
		nrk_kprintf(PSTR("initialise_buffer_manager(): Error creating the semaphore\r\n"));
	}	
	
	if(DEBUG_BM == 2)
	{
		nrk_kprintf(PSTR("Initial tx buffer\r\n"));
		print_tx_buffer();
	}
	
	return;
}
/*******************************************************************************************/
inline void print_tx_buffer()
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
void print_rx_buffers(uint8_t port)
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
inline int8_t convert_ptr_to_index(TransmitBuffer *ptr)
{
	int8_t i;
	
	for(i = 0; i < MAX_TX_QUEUE_SIZE; i++)
		if(ptr == &tx_buf[i])
			break;
			
	return i;
}
/*******************************************************************************************/
inline int8_t is_excess_policy_valid(int8_t pref)
{
	switch(pref)
	{
		case OVERWRITE:
		case DROP:
			return TRUE;
	}
	return FALSE; 
}
/*******************************************************************************************/
int8_t set_excess_policy(int8_t prio, int8_t pref)	// user API 
{
	if(prio <= 0 || prio > MAX_PRIORITY || is_excess_policy_valid(pref) == FALSE)
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		return NRK_ERROR;
	}
	
	enter_cr(bm_sem, 39);	// enter CR 
		
	if(pref == DROP)	// set bit number 'prio' in excessPolicySettings to 1
		excessPolicySettings |= (uint32_t)1 << prio;
	else					// set bit number 'prio' in excessPolicySettings to 0
		excessPolicySettings &= ~( (uint32_t)1 << prio );
	
	leave_cr(bm_sem, 39);	// leave CR 
		
	return NRK_OK;
}
/***********************************************************************************/
int8_t get_excess_policy(int8_t prio)	// user API 
{
	if(prio <= 0 || prio > MAX_PRIORITY)
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		return NRK_ERROR;
	}	
	
	enter_cr(bm_sem, 40);
		
	// check the state of bit number 'prio' in excessPolicySettings 
	if( ((excessPolicySettings >> prio) & ((uint32_t)1)) == 0 ) // is bit 'prio' cleared
	{
		leave_cr(bm_sem, 40);	
		return OVERWRITE;
	}
	
	leave_cr(bm_sem, 40);
	return DROP;
}
/************************************************************************************/
int8_t get_index_unallocated_rx_buf()
{
	int8_t i;	// loop index 
	
	for(i = 0; i < MAX_RX_QUEUE_SIZE; i++)
		if(rx_buf_udp[i].status == UNALLOCATED)	// the buffer at this index is free 
			return i;
			
	return NRK_ERROR;
}
/**************************************************************************************/
inline int8_t port_to_rbm_index(uint8_t port)
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
inline int8_t port_to_port_index(uint8_t port)
{
	int8_t i;	// loop index
	
	for(i = 0; i < NUM_PORTS; i++)
		if(ports[i].pno == port)	// match found
			return i;
	
	// if the code reaches here, it means no match was found
	return NRK_ERROR;
}
/**************************************************************************************/
void insert_rx_pq(Transport_Segment_UDP *seg, int8_t prio, uint16_t addr, int8_t rssi)
{
	int8_t rbm_index;						// index of the corresponding rbm element 
	ReceiveBufferUDP *buf, *ptr, *prev;
	
	rbm_index = port_to_rbm_index(seg -> destPort);	
	
	if(DEBUG_BM == 2)
	{
		nrk_kprintf(PSTR("BM: insert_rx_pq(): DestPort = "));
		printf("%d\r\n", seg -> destPort);
	}
	buf = remove_rx_fq(rbm_index, EMPTY);		// remove an EMPTY buffer from the free queue
	
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
	
	if(DEBUG_BM == 2)
	{
		nrk_kprintf(PSTR("BM: insert_rx_pq(): After ptr loop\r\n"));
	}
	
	if(buf != NULL)	// a buffer is available to hold the newly received segment 
	{
		if(DEBUG_BM == 2)
		{
			nrk_kprintf(PSTR("BM: insert_rx_pq(): There is space\r\n"));
		}		
		
		// fill up the members of ReceiveBufferUDP		
		buf -> seg = *seg;	
		buf -> status = FULL;
		buf -> prio = prio;
		buf -> srcAddr = addr;
		buf -> rssi = rssi;
		buf -> next = ptr;
		
		
		if(rx_buf_mgr[rbm_index].head_pq == NULL)			// rx pq is empty
		{
			rx_buf_mgr[rbm_index].head_pq = rx_buf_mgr[rbm_index].tail_pq = buf; 
		}
		else if(prev == NULL)	// the newly received segment is to be inserted at the head of the port queue 
					rx_buf_mgr[rbm_index].head_pq = buf;
			  else
			  {
			  		prev -> next = buf;
			  		if(prev == rx_buf_mgr[rbm_index].tail_pq)
			  			rx_buf_mgr[rbm_index].tail_pq = buf;
			  }
			
		// newly received segment has been inserted at the proper slot in the port queue
		if(DEBUG_BM == 2)
		{
			nrk_kprintf(PSTR("BM: insert_rx_pq(): Before returning (there is space)\r\n"));
		}
		return;  
	}
	
	// if the code reaches here, it means that a buffer is not available to hold this new segment 
	// There are three possibilities
	// 1. The free queue is empty and the port queue is full
	// 2. The port queue is empty and the free queue is full of 'FULL' buffers
	// 3. Some combination of the above 2	
	
	if(DEBUG_BM == 2)
	{
		nrk_kprintf(PSTR("BM: insert_rx_pq(): No space\r\n"));
	}
	
	// check for 2 
	if(rx_buf_mgr[rbm_index].head_pq == NULL)
	{
		if(rx_buf_mgr[rbm_index].countFree != 0)	// debugging purposes 
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("insert_rx_pq(): Bug found in implementation of countFree in rx_buf_mgr\r\n"));
		}
		if(rx_buf_mgr[rbm_index].tail_pq != NULL)	// debugging purposes 
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("insert_rx_pq(): Bug found in implementation of tail_pq in rx_buf_mgr\r\n"));
		}
		
		if(DEBUG_BM == 2)
		{
			nrk_kprintf(PSTR("BM: insert_rx_pq(): Before returning. No space but port queue also empty\r\n"));
		}
		return;		// drop the new segment
	}
	
	// the situation could be 1 or 3. The behavior is the same. The lowest priority message 
	// in the port queue has to be replaced	  
	// check if the last segment in the port queue has a lower priority
	
	if(DEBUG_BM == 2)
	{
		nrk_kprintf(PSTR("BM: insert_rx_pq(): Possibility of replacement\r\n"));
	}
	if( (rx_buf_mgr[rbm_index].tail_pq) -> prio < prio ) // this last segment should be replaced
	{
		// at this point we know that ptr cannot be NULL		
		// find the last but one segment in the port queue 
		if(DEBUG_BM == 2)
		{
			nrk_kprintf(PSTR("BM: insert_rx_pq(): Inside 1\r\n"));
		}
		
		ReceiveBufferUDP *qtr = rx_buf_mgr[rbm_index].head_pq;
		// corner case, see if there is only one buffer in the port queue 
		if(rx_buf_mgr[rbm_index].head_pq == rx_buf_mgr[rbm_index].tail_pq)
		{
			// replace the members of this last buffer with those of the new one 
			if(DEBUG_BM == 2)
			{
				nrk_kprintf(PSTR("BM: insert_rx_pq(): Inside 2\r\n"));
			}
			qtr -> seg = *seg;
			qtr -> prio = prio;
			qtr -> srcAddr = addr;
			qtr -> rssi = rssi;		// message inserted at the tail of the queue 
			
			return;
		}
		else	// there is more than one buffer in the port queue 
		{
			if(DEBUG_BM == 2)
			{
				nrk_kprintf(PSTR("BM: insert_rx_pq(): Before qtr loop\r\n"));
			}
						
			while(qtr -> next != rx_buf_mgr[rbm_index].tail_pq)	// qtr will now point to the last but one segment 
				qtr = qtr -> next;
				
			if(DEBUG_BM == 2)
			{
				nrk_kprintf(PSTR("BM: insert_rx_pq(): After qtr loop\r\n"));
			}
				
			// replace the members of the last buffer with those of the new one
			(rx_buf_mgr[rbm_index].tail_pq) -> seg = *seg;
			(rx_buf_mgr[rbm_index].tail_pq) -> prio = prio;
			(rx_buf_mgr[rbm_index].tail_pq) -> srcAddr = addr;
			(rx_buf_mgr[rbm_index].tail_pq) -> rssi = rssi;
				
			// adjust the pointers so that this new segment occupies its correct position in the port queue			
			(rx_buf_mgr[rbm_index].tail_pq) -> next = ptr;
			if(prev == NULL)
				rx_buf_mgr[rbm_index].head_pq = rx_buf_mgr[rbm_index].tail_pq;
			else
				prev -> next = rx_buf_mgr[rbm_index].tail_pq;
					
			qtr -> next = NULL;				
			rx_buf_mgr[rbm_index].tail_pq = qtr;	// change the tail of the port queue
			
			return;
		}
	} // end if 
	else if( (rx_buf_mgr[rbm_index].tail_pq) -> prio == prio )	
		  {
		  		// check the setting of the excess policy for this priority level 
		  		if(DEBUG_BM == 2)
				{
					nrk_kprintf(PSTR("BM: insert_rx_pq(): Inside 3\r\n"));
				}	
				
		  		if( ((excessPolicySettings >> prio) & ((uint32_t)1)) == OVERWRITE)	// new segment is more important 
		  		{
		  			if(DEBUG_BM == 2)
					{
						nrk_kprintf(PSTR("BM: insert_rx_pq(): Inside 4\r\n"));
					}
		  			// replace the members of the last buffer with those of the new one 
		  			(rx_buf_mgr[rbm_index].tail_pq) -> seg = *seg;
					(rx_buf_mgr[rbm_index].tail_pq) -> prio = prio;
					(rx_buf_mgr[rbm_index].tail_pq) -> srcAddr = addr;
					(rx_buf_mgr[rbm_index].tail_pq) -> rssi = rssi;
					
					return;
				}
				else			// new segments should be dropped 
				{
					if(DEBUG_BM == 2)
					{
						nrk_kprintf(PSTR("BM: insert_rx_pq(): Inside 5\r\n"));
					}
					return;	// do nothing
				} 
		  }
		  else				// new segment has lower priority 
		  {
		  		if(DEBUG_BM == 2)
				{
					nrk_kprintf(PSTR("BM: insert_rx_pq(): Inside 6\r\n"));
				}
		  		return;		// do nothing 
		  }
	//possibility 1 or 3 checked 
	
	if(DEBUG_BM == 2)
	{
		nrk_kprintf(PSTR("BM: insert_rx_pq(): Inside 7\r\n"));
	}
	return;
}
/*************************************************************************************************/
ReceiveBufferUDP* remove_rx_pq(int8_t rbm_index)
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

void insert_rx_fq(ReceiveBufferUDP *buf, int8_t rbm_index, int8_t status)
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
		rx_buf_mgr[rbm_index].countFree++;
		buf -> status = EMPTY;
	}
	
	return;
}

/***************************************************************************************************/
ReceiveBufferUDP* remove_rx_fq(int8_t rbm_index, int8_t status)
{
	// This function removes the next receive buffer from the free queue having a given status 	
	ReceiveBufferUDP *ptr;
	ReceiveBufferUDP *prev = NULL;
	
	if(rx_buf_mgr[rbm_index].head_fq == NULL)	// no buffers remaining in free queue	
		return NULL;
		
	ptr = rx_buf_mgr[rbm_index].head_fq;
	while(ptr != NULL)
	{
		if(status == EMPTY)
		{
			if(ptr -> status == EMPTY)
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

int8_t insert_tx_aq(NW_Packet *pkt)
{
	TransmitBuffer *buf = remove_tx_fq();		// remove an available buffer from the free queue
	TransmitBuffer *ptr;
	TransmitBuffer *prev;
	
	if(buf == NULL)									// no more free buffers available 
	{
		return NRK_ERROR;
	}
	
	if(tx_buf_mgr.head_aq == NULL)				// the active queue is empty. Corner case  
	{
		// fill up the members of TransmitBuffer 
		buf -> pkt = *pkt;
		buf -> status = FULL;
		buf -> prio = pkt -> prio;
		buf -> next = NULL;
		
		tx_buf_mgr.head_aq = tx_buf_mgr.tail_aq = buf;
		tx_buf_mgr.count_aq++;
		return NRK_OK;
	}
		
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
	// fill up the members of TransmitBuffer		
	buf -> pkt = *pkt;	
	buf -> status = FULL;
	buf -> prio = pkt -> prio;
	buf -> next = ptr;
		
	if(prev == NULL)	// the new packet is to be inserted at the head of the active queue 
		tx_buf_mgr.head_aq = buf;
	else
	{
		prev -> next = buf;
		if(prev == tx_buf_mgr.tail_aq)
			tx_buf_mgr.tail_aq = buf;
	}
			
	// new packet has been inserted at the proper slot in the active queue
	tx_buf_mgr.count_aq++;
	
	return NRK_OK; 
}
/***********************************************************************************************/

TransmitBuffer* remove_tx_aq()
{
	// since the active queue is sorted as per priority, the buffer at the head of the queue 
	// is removed	
	TransmitBuffer *ptr;
	
	if(tx_buf_mgr.head_aq == NULL)	// the active queue is empty 
	{
		return NULL;
	}
		
	if(tx_buf_mgr.head_aq == tx_buf_mgr.tail_aq)	// there is only one tx buffer in the active queue 
		tx_buf_mgr.tail_aq = NULL;
		
	ptr = tx_buf_mgr.head_aq;
	
	tx_buf_mgr.head_aq = tx_buf_mgr.head_aq -> next;
	tx_buf_mgr.count_aq--;
	
	return ptr;
}
/*************************************************************************************************/
void insert_tx_fq(TransmitBuffer *buf)
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
	initialise_nw_packet(&(buf -> pkt));
	tx_buf_mgr.count_fq++;
	
	return;	
}

/**************************************************************************************************/
TransmitBuffer* remove_tx_fq()
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

int8_t get_in_process_buf_count(int8_t rbm_index)
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
int8_t get_num_bufs_free()
{
	int8_t n;
		
	n = num_bufs_free;
	
	return n;
}
/****************************************************************************************************/

void set_num_bufs_free(int8_t n)
{
	num_bufs_free = n;
	return;
}
/*****************************************************************************************************/
	
	
	
	
	
	
