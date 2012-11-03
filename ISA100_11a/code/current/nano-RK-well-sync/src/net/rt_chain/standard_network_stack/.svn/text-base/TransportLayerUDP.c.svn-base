/* This file implements the transport layer (UDP-like) of the network stack */

#include "TransportLayerUDP.h"
#include "NetworkLayer.h"
#include "BufferManager.h"
#include "NWErrorCodes.h"
#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_error.h>
#include <stdint.h>


/****************** Global data structures required for port and socket management ***************/

Socket sock[NUM_PORTS];				// array to store available socket descriptors 0 - (NUM_PORTS - 1)

Port ports[NUM_PORTS];				// array to store available port numbers (1 - MAX_PORT_NUM)

int8_t tlayer_init_done;			// flag to indicate whether initialization has been done correctly 

Transport_Segment_UDP udp_seg;	// to hold a segment to be sent 
NW_Packet pkt;							// to hold a network packet to be sent 

nrk_sem_t *tl_sem;					// semaphore to protect access to the above variables 

/********************* Variables / functions defined in other files used here *****************/

// From BufferManager.c											 
extern ReceiveBufferManager rx_buf_mgr[]; 
extern int8_t num_bufs_free; 
extern ReceiveBufferUDP rx_buf_udp[];
extern TransmitBuffer tx_buf[];
extern TransmitBufferManager tx_buf_mgr;
extern nrk_sem_t *bm_sem;

extern void enter_cr(nrk_sem_t *, int8_t);
extern void leave_cr(nrk_sem_t *, int8_t);
extern int8_t get_num_bufs_free();
extern void insert_rx_fq(ReceiveBufferUDP*, int8_t, int8_t);
extern int8_t insert_tx_aq(NW_Packet *);
extern int8_t get_in_process_buf_count(int8_t);
extern ReceiveBufferUDP* remove_rx_pq(int8_t);
extern void print_tx_buffer();

// From NetworkLayer.c
extern uint16_t route_addr(uint16_t);
extern nrk_sem_t *nl_sem;

// From Pack.c
extern void pack_TL_UDP_header(uint8_t*, Transport_Segment_UDP*);

// From Debug.c 
extern void go_into_panic(int8_t *);

/************************** Function definitions ***************************************************/
void initialise_transport_layer_udp()
{
	int8_t i;	// loop index 
	
	// make the socket descriptors and ports unassigned 
	for(i = 0; i < NUM_PORTS; i++)
	{
		sock[i].pindex = -1;					// indicates the absence of socket-port mapping 
		sock[i].rbmindex = -1;				// indicates the absence of socket-port mapping 
		sock[i].pid = INVALID_PID;			// indicates an unassigned socket descriptor 
		sock[i].timeout.secs = 0;			// indicates no timeout 
		sock[i].timeout.nano_secs = 0; 	// indicates no timeout 
		
		ports[i].pno = INVALID_PORT;		// indicates an unassigned port number		
	}
	
	// create the mutex with priority ceiling equal to MAX_TASK_PRIORITY	
	tl_sem = nrk_sem_create(1,MAX_TASK_PRIORITY);	
	if(tl_sem == NULL)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("initialise_transport_layer_udp(): Error creating the semaphore\r\n"));
	}	
	
	if(NUM_PORTS > MAX_PORTS)	// too many ports opened in the system
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("initialise_transport_layer_udp(): Too many ports opened in system\r\n"));
	}
	
	// set the init_done flag 
	tlayer_init_done = 1;
	return;
}
/**************************************************************************************************/
int8_t get_next_available_socket()
{
	int8_t i;		// loop index
	
	// search through the 'sock' array to see if any socket numbers are left unassigned 
	for(i = 0; i < NUM_PORTS; i++)
	{
		if(sock[i].pid == INVALID_PID)	// this socket descriptor is available 
			return i;
	}
	_nrk_errno_set(NO_SOCKET_DESC_AVAILABLE);	
	
	return NRK_ERROR;
}
		
/***************************************************************************************************/
int8_t create_socket(int8_t type)
{
	int8_t result;		// temporary variable
	
	enter_cr(tl_sem, 3);
		
	if(tlayer_init_done != 1)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("create_socket(): Transport layer not initialised\r\n"));
	}
	
	switch(type)
	{
		case SOCK_DGRAM:
		case SOCK_IPC:
		case SOCK_RAW:
				result = get_next_available_socket();
				if(result != NRK_ERROR)
				{
					sock[result].pid = nrk_get_pid();
					sock[result].type = type;
				}
				break;
				
		default: 
				_nrk_errno_set(UNSUPPORTED_SOCK_TYPE);
				result = NRK_ERROR;
	}
		
	leave_cr(tl_sem, 3);
	return result; 
}

/*****************************************************************************************/
uint8_t get_next_available_port()
{
	uint16_t i;	// loop index for ephemeral port numbers
	int8_t j;	// loop index
	
	for(i = EPHEMERAL_PORT_NUM_START; i <= MAX_PORT_NUM; i++)
	{
		// check to see if port 'i' is available
		for(j = 0; j < NUM_PORTS; j++)
			if(ports[j].pno == i)	// port 'i' has been taken
				break;
				
		if(j == NUM_PORTS)			// port 'i' is free
			return i; 					// return this ephemeral port number back
	} 
	
	// if the code reaches there, it means that all ephemeral port numbers have been taken.
	// This also means that the 'ports' array is full 
	 
	_nrk_errno_set(NO_PORTS_AVAILABLE);	
	return INVALID_PORT;							 
}
/******************************************************************************************/
int8_t check_port_available(uint8_t pt)
{
	int8_t i;	// loop index
	
	for(i = 0; i < NUM_PORTS; i++)
		if(ports[i].pno == pt)	// port 'pt' has been taken
			break;
			
	if(i == NUM_PORTS)			// port 'pt' is available 
		return NRK_OK;
		
	_nrk_errno_set(PORT_UNAVAILABLE);	
	return NRK_ERROR;
}
/*****************************************************************************************/
void assign_port(int8_t pindex, uint8_t pt)
{
	int8_t ret1, ret2;	
	
	// fill up the members of the port element
	ports[pindex].pno = pt;
	ports[pindex].send_done_signal = nrk_signal_create();
	ports[pindex].data_arrived_signal = nrk_signal_create();
	
	if( ports[pindex].send_done_signal == NRK_ERROR )
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("assign_port(): Error creating the send_done signal\r\n"));
	}
	
	if( ports[pindex].data_arrived_signal == NRK_ERROR )
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("assign_port(): Error creating the data_arrived signal\r\n"));
	}
	
	ret1 = nrk_signal_register(ports[pindex].send_done_signal);
	ret2 = nrk_signal_register(ports[pindex].data_arrived_signal);
	
	if(ret1 == NRK_ERROR)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("assign_port(): Error registering the send_done signal\r\n"));
	}
	
	if(ret2 == NRK_ERROR)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("assign_port(): Error registering the data_arrived signal\r\n"));
	}
		
	return;
}
/******************************************************************************************/
void release_port(int8_t pindex)
{
	int8_t ret1, ret2;

	// invalidate members of the port element 
	ports[pindex].pno = INVALID_PID;
	ret1 = nrk_signal_delete(ports[pindex].send_done_signal);
	ret2 = nrk_signal_delete(ports[pindex].data_arrived_signal);
	
	if(ret1 == NRK_ERROR)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("assign_port(): Error creating the send_done signal\r\n"));
	}

	if(ret2 == NRK_ERROR)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("assign_port(): Error creating the data_arrived signal\r\n"));
	}	
	
	return;	
}
/*****************************************************************************************/
int8_t get_index_unassigned_port_element()
{
	int8_t i;
	
	for(i = 0; i < NUM_PORTS; i++)
		if(ports[i].pno == INVALID_PORT)
			return i;
			
	_nrk_errno_set(NO_PORT_ELEMENT_AVAILABLE);
	return NRK_ERROR;
}
/*******************************************************************************************/
int8_t get_index_unassigned_rbm_element()
{
	int8_t i;
	
	for(i = 0; i < NUM_PORTS; i++)
		if(rx_buf_mgr[i].pindex == -1)
			return i;
			
	_nrk_errno_set(NO_RBM_ELEMENT_AVAILABLE);
	return NRK_ERROR;
}
/*****************************************************************************************/
uint8_t get_port_num(int8_t sock_num)
{
	// ERROR checking
	if(sock_num < 0 || sock_num >= NUM_PORTS)
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		return INVALID_PORT;
	}
	
	if(sock[sock_num].pid != nrk_get_pid())
	{
		_nrk_errno_set(INVALID_SOCKET);
		return INVALID_PORT;
	}
	
	if(sock[sock_num].type == SOCK_RAW)
	{
		_nrk_errno_set(INVALID_SOCKET);
		return INVALID_PORT;
	}
	
	if(sock[sock_num].pindex == -1)
	{
		_nrk_errno_set(UNMAPPED_SOCKET);
		return INVALID_PORT;
	}
	
	return ports[sock[sock_num].pindex].pno;
}

/*****************************************************************************************/
int8_t bind(int8_t sock_num, int16_t port)
{
	int8_t buf_index;						// stores the index of next available receive buffer
	int8_t port_index;					// stores the index of the next unassigned port element
	int8_t rbm_index;						// stores the index of the next unassigned rbm element 			 
	int8_t i;								// loop index 
	int8_t size;							// size of receive queue allocated for this port 
	
	enter_cr(bm_sem, 8);	
	enter_cr(tl_sem, 8);
			
	if(tlayer_init_done != 1)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("bind(): Transport layer not initialised\r\n"));
	}
				
	// ERROR CHECKING 
	if(sock_num < 0 || sock_num >= NUM_PORTS)	// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 8);	
		leave_cr(bm_sem, 8);	
		return NRK_ERROR;
	}
		
	if(port <= 0 || port > MAX_PORT_NUM)			// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 8);	
		leave_cr(bm_sem, 8);
		return NRK_ERROR;
	}
			
	if(sock[sock_num].pid != nrk_get_pid())	// if a wrong sock_num is passed 
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 8);	
		leave_cr(bm_sem, 8);
		return NRK_ERROR;
	}
	
	if(sock[sock_num].type == SOCK_RAW)
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 8);	
		leave_cr(bm_sem, 8);
		return NRK_ERROR;
	}		
	
	if(sock[sock_num].rbmindex != -1)			// bind() / set_rx_queue_size() was called earlier
	{
		_nrk_errno_set(INVALID_CALL);
		
		leave_cr(tl_sem, 8);	
		leave_cr(bm_sem, 8);
		return NRK_ERROR;
	}
	
	if(check_port_available((uint8_t)port) == FALSE)	// check to see if port is already taken 
	{
		leave_cr(tl_sem, 8);	
		leave_cr(bm_sem, 8);	
		return NRK_ERROR;										// error no is already set = PORT_UNAVAILABLE
	} 
	
	if(get_num_bufs_free() < DEFAULT_RX_QUEUE_SIZE)			// receive buffer pool is exhausted 
	{
		_nrk_errno_set(NO_RX_BUFFERS_AVAILABLE);
		
		leave_cr(tl_sem, 8);	
		leave_cr(bm_sem, 8);
		return NRK_ERROR;
	}
	
	if(sock[sock_num].pindex != -1)						// sanity check for debugging
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("bind(): Bug detected in implementation of port / rbm element array\r\n"));
	}
	
	// error checking complete. Begin processing 				
	size = DEFAULT_RX_QUEUE_SIZE;
	// at this point, we know that
	// 1. port 'port' is available 
	// 2. at least 'size' receive buffers are available
	// 3. bind() / setReceiveQueueSize() was not called earlier
	
	// retrieve the indices of the elements
	port_index = get_index_unassigned_port_element();
	rbm_index = get_index_unassigned_rbm_element();
	
	if(port_index == NRK_ERROR || rbm_index == NRK_ERROR)	// sanity check for debugging 
	{	
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("bind(): Bug detected in implementation of port / rbm element array\r\n"));
	}
	
	// fill up the members of the port element
	assign_port(port_index, (uint8_t)port);
	
	// fill up the members of the socket descriptor
	sock[sock_num].pindex = port_index;
	sock[sock_num].rbmindex = rbm_index;
	
	// fill up the members of the rbm element
	rx_buf_mgr[rbm_index].pindex = port_index;
	rx_buf_mgr[rbm_index].pid = nrk_get_pid();
		
	// allocate 'size' buffers 
	for(i = 1; i <= size; i++)
	{
		buf_index = get_index_unallocated_rx_buf();	// look for the unallocated receive buffer 
		if(buf_index == NRK_ERROR)	//	 this should not happen 
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("bind(): Bug found in implementation of num_bufs_free\r\n"));
		}
		
		insert_rx_fq(&rx_buf_udp[buf_index], rbm_index, EMPTY);	// insert the buffer in the free queue of the port
		rx_buf_mgr[rbm_index].countTotal++; 						// increment the countTotal for this port 
		num_bufs_free--;		
	}
	
	leave_cr(tl_sem, 8);	
	leave_cr(bm_sem, 8);
	return NRK_OK; 
}
/*****************************************************************************************/
int8_t get_rx_queue_size(int8_t sock_num)
{
	int8_t count;	// to store the count of total rx buffers set aside for the given socket 
	
	enter_cr(bm_sem, 9);	
	enter_cr(tl_sem, 9);
	
	// ERROR checking
	if(sock_num < 0 || sock_num >= NUM_PORTS)	// bad input
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 9);	
		leave_cr(bm_sem, 9);
		return NRK_ERROR;
	}
		
	if(sock[sock_num].pid != nrk_get_pid())	// wrong socket number is passed
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 9);	
		leave_cr(bm_sem, 9);
		return NRK_ERROR;
	}
	
	if(sock[sock_num].type == SOCK_RAW)
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 9);	
		leave_cr(bm_sem, 9);
		return NRK_ERROR;
	}	
	
	if(sock[sock_num].pindex == -1)	// no socket operations were performed yet
	{
		leave_cr(tl_sem, 9);	
		leave_cr(bm_sem, 9);	
		return 0;							// hence return 0 
	}
	
	if(sock[sock_num].rbmindex == -1) // sanity check for debugging
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("get_rx_queue_size(): Bug detected in implementation of port/rbm element array\r\n"));
	}	
		
	// at this point we know that some socket operation has been performed on the socket prior to this call
	count = rx_buf_mgr[sock[sock_num].rbmindex].countTotal;	
	
	leave_cr(tl_sem, 9);	
	leave_cr(bm_sem, 9);
	return count;
}	
	
/******************************************************************************************/
int8_t set_rx_queue_size(int8_t sock_num, int8_t size)
{
	uint8_t port;		// to store the port number associated with given socket number
	int8_t i;			// loop index
	int8_t flag;		// to mark whether bind() was called prior to this call or not 
	
	int8_t buf_index;	 // stores the index of next unallocated receive buffer
	int8_t port_index; // stores the index of the next unassigned port element
	int8_t rbm_index;	 // stores the index of the next unassigned rbm element  
		
	enter_cr(bm_sem, 10);
	enter_cr(tl_sem, 10);
		
	if(tlayer_init_done != 1)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("set_rx_queue_size(): Transport layer not initialised\r\n"));
	}
		
	flag = 0;			// assume bind() was called prior to this call 	
	// ERROR CHECKING 
	if(sock_num < 0 || sock_num >= NUM_PORTS || size <= 0)	// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 10);
		leave_cr(bm_sem, 10);
		return NRK_ERROR;
	}
			
	if(sock[sock_num].pid != nrk_get_pid())	// if a wrong sock_num is passed 
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 10);
		leave_cr(bm_sem, 10);
		return NRK_ERROR;
	}
	
	if(sock[sock_num].type == SOCK_RAW)
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 10);	
		leave_cr(bm_sem, 10);
		return NRK_ERROR;
	}	
	
	if(sock[sock_num].pindex == -1)				// there is no port associated with this socket yet
	{
		flag = 1;										// bind() was not called earlier 
		port = get_next_available_port();		// search for a ephemeral port number 
		if(port == INVALID_PORT)					// no port available 
		{
			leave_cr(tl_sem, 10);
			leave_cr(bm_sem, 10);
			return NRK_ERROR;							// errorNo is already set to NO_PORTS_AVAILABLE
		} 
		if(get_num_bufs_free() < DEFAULT_RX_QUEUE_SIZE) // port available but no buffers available 
		{
			_nrk_errno_set(NO_RX_BUFFERS_AVAILABLE);
			
			leave_cr(tl_sem, 10);
			leave_cr(bm_sem, 10);
			return NRK_ERROR;
	   }
		// port available and default num of buffers available, 
		// retrieve the indices of the elements
		port_index = get_index_unassigned_port_element();
		rbm_index = get_index_unassigned_rbm_element();
		
		if(port_index == NRK_ERROR || rbm_index == NRK_ERROR)	// sanity check for debugging 
		{	
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("set_rx_queue_size(): Bug detected in implementation of port / rbm element array\r\n"));
		}
		
		// fill up the members of the port element
		assign_port(port_index, port);
		
		// fill up the members of the socket descriptor
		sock[sock_num].pindex = port_index;
		sock[sock_num].rbmindex = rbm_index;
		
		// fill up the members of the rbm element
		rx_buf_mgr[rbm_index].pindex = port_index;
		rx_buf_mgr[rbm_index].pid = nrk_get_pid();
	
	} // end if(bind was called earlier)
	else // bind was called earlier. DEFAULT_RX_QUEUE_SIZE buffers already allocated
	{
		if(size == DEFAULT_RX_QUEUE_SIZE)	// nothing to do, buffers already allocated
		{
			leave_cr(tl_sem, 10);
			leave_cr(bm_sem, 10);
			return size;
		}
		else // size > DEFAULT_RX_QUEUE_SIZE
			size -= DEFAULT_RX_QUEUE_SIZE;
	}
	
	// check to see if there are 'size' buffers available in the system	
	if(get_num_bufs_free() < size)
		size = get_num_bufs_free();
		
	// at this point, the socket is associated with a valid port number
	// and at least one receive buffer is free
	// allocate 'size' buffers 
	for(i = 1; i <= size; i++)
	{
		buf_index = get_index_unallocated_rx_buf();	// look for the next unallocated receive buffer 
		if(buf_index == NRK_ERROR)	//	 this should not happen 
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("set_rx_queue_size(): Bug found in implementation of num_bufs_free\r\n"));
		}		
		insert_rx_fq(&rx_buf_udp[buf_index], rbm_index, EMPTY);	// insert the buffer in the free queue of the port
		rx_buf_mgr[rbm_index].countTotal++; 
		num_bufs_free--;		
	}
	
	leave_cr(tl_sem, 10);
	leave_cr(bm_sem, 10);
	
	if(flag == 0)	// bind was called earlier
		return size + DEFAULT_RX_QUEUE_SIZE;	
	
	return size;
}
/**************************************************************************************************/
int8_t release_buffer(int8_t sock_num, uint8_t *ptr)
{
	ReceiveBufferUDP *buf;			// pointer to traverse the buffer list of the free queue 
	
	enter_cr(bm_sem, 11);	
	enter_cr(tl_sem, 11);	
	
	// ERROR CHECKING 
	if(sock_num < 0 || sock_num >= NUM_PORTS || ptr == NULL)	// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 11);	
		leave_cr(bm_sem, 11);
		return NRK_ERROR;
	}
	if(sock[sock_num].pid != nrk_get_pid())	// if a wrong sock_num is passed 
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 11);	
		leave_cr(bm_sem, 11);
		return NRK_ERROR;
	}
	
	if(sock[sock_num].type == SOCK_RAW)
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 11);	
		leave_cr(bm_sem, 11);
		return NRK_ERROR;
	}	
	
	if(sock[sock_num].pindex == -1)	// if no socket operation was performed earlier 
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 11);	
		leave_cr(bm_sem, 11);
		return NRK_ERROR;
	}
	
	if(sock[sock_num].rbmindex == -1)	// sanity check for debugging 
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("release_buffer(): Bug discovered in implementation of port / rbm element array\r\n"));
	}
	
	// check to see if the pointer passed is valid
	buf = rx_buf_mgr[sock[sock_num].rbmindex].head_fq;
	while(buf != NULL)
	{
		if( (ptr == (buf -> seg).data) && (buf -> status == FULL) )	
			break;
		buf = buf -> next;
	}
	if(buf == NULL)		// the buffer was not found
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 11);	
		leave_cr(bm_sem, 11);
		return NRK_ERROR;
	}
		
	buf -> status = EMPTY;		// mark the buffer as available now
	rx_buf_mgr[sock[sock_num].rbmindex].countFree++; 
	
	leave_cr(tl_sem, 11);	
	leave_cr(bm_sem, 11);
	return NRK_OK;
}
/*************************************************************************************************/
int8_t close_socket(int8_t sock_num)
{
	ReceiveBufferUDP *ptr;	// temp variable 
		
	enter_cr(bm_sem, 12);
	enter_cr(tl_sem, 12);
	
	// ERROR checking 
	if(sock_num < 0 || sock_num >= NUM_PORTS)	// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 12);
		leave_cr(bm_sem, 12);
		return NRK_ERROR;
	}	
	if(sock[sock_num].pid != nrk_get_pid() )	// wrong socket number passed 
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 12);
		leave_cr(bm_sem, 12);
		return NRK_ERROR;
	}
	
	if(sock[sock_num].type == SOCK_RAW)
	{
		sock[sock_num].pid = INVALID_PID;
		sock[sock_num].timeout.secs = 0;
		sock[sock_num].timeout.nano_secs = 0;
			
		leave_cr(tl_sem, 12);
		leave_cr(bm_sem, 12);
		return NRK_OK;
	}		
		
	// Free up resources associated with this socket descriptor 
	if(sock[sock_num].pindex != -1)	// some operation was performed earlier with this socket 
	{
		if(sock[sock_num].rbmindex == -1)	// sanity check
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("close_socket(): Bug discovered in implementation of port /rbm element array\r\n"));
		}		
		// remove all the FULL buffers from the port queue 			
		while( (ptr = remove_rx_pq(sock[sock_num].rbmindex)) != NULL )		
			ptr -> status = UNALLOCATED;		
			
		// remove all the EMPTY buffers from the free queue 		
		while( (ptr = remove_rx_fq(sock[sock_num].rbmindex, EMPTY)) != NULL ) 
			ptr -> status = UNALLOCATED;
			
		// remove all the FULL buffers from the free queue 		
		while( (ptr = remove_rx_fq(sock[sock_num].rbmindex,FULL)) != NULL )
			ptr -> status = UNALLOCATED;
			
		rx_buf_mgr[sock[sock_num].rbmindex].pid = INVALID_PID;
		rx_buf_mgr[sock[sock_num].rbmindex].pindex = -1;
		rx_buf_mgr[sock[sock_num].rbmindex].countTotal = 0;
		rx_buf_mgr[sock[sock_num].rbmindex].countFree = 0;
		
		release_port(sock[sock_num].pindex);
	}  
	
	sock[sock_num].pindex = -1;
	sock[sock_num].rbmindex = -1;
	sock[sock_num].pid = INVALID_PID;
	sock[sock_num].timeout.secs = 0;
	sock[sock_num].timeout.nano_secs = 0;
	
	leave_cr(tl_sem, 12);
	leave_cr(bm_sem, 12);	
	return NRK_OK;
}
/************************************************************************************************/
int8_t is_port_associated(int16_t port)
{
	// ERROR checking
	if(port <= 0 || port > MAX_PORT_NUM)
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		return NRK_ERROR;
	}
		
	enter_cr(tl_sem, 13);
		
	if(check_port_available((uint8_t)port) == NRK_OK)
	{
		leave_cr(tl_sem, 13);
		return FALSE;
	}
	leave_cr(tl_sem, 13);
	return TRUE;
}
/************************************************************************************************/ 
int8_t send(int8_t sock_num, int8_t *ptr, int8_t len, int32_t dest_addr, int16_t dest_port, int8_t prio)
{
	int8_t result;			// to hold the return type of various functions 
	
	enter_cr(bm_sem, 14);
	enter_cr(tl_sem, 14);
	
	// ERROR checking
	if(sock_num < 0 || sock_num >= NUM_PORTS)								// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
				
		leave_cr(tl_sem, 14);
		leave_cr(bm_sem, 14);		
		return NRK_ERROR;
	}
		
	if(ptr == NULL || len <= 0 || len > MAX_APP_PAYLOAD)				// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
						
		leave_cr(tl_sem, 14);
		leave_cr(bm_sem, 14);
		return NRK_ERROR;
	}
	
	if(sock[sock_num].pid != nrk_get_pid())								// wrong socket number 
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 14);
		leave_cr(bm_sem, 14);
		return NRK_ERROR;
	}

	// first handle the case of SOCK_RAW 
	if(sock[sock_num].type == SOCK_RAW)
	{
		nrk_sig_t ll_tx_done_signal = bmac_get_tx_done_signal();
		if(nrk_signal_register(ll_tx_done_signal) == NRK_ERROR)
  		{
  			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("send(): Error in registering for ll_tx_done_signal\r\n"));
		}
		
		// check if the user specified a timeout value 		
		/*
		if(sock[sock_num].timeout.secs != 0 || sock[sock_num].timeout.nano_secs != 0)
		{
			nrk_sig_mask_t result;
						
			if( nrk_signal_register(nrk_wakeup_signal) == NRK_ERROR)
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("send(): Error in registering for nrk_wakeup_signal\r\n"));
			}
			if( nrk_set_next_wakeup(sock[sock_num].timeout) == NRK_ERROR)
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("send(): Error returned by nrk_set_next_wakeup\r\n"));
			}
			while(bmac_tx_pkt(ptr, len) == -1)
			{
				leave_cr(tl_sem, 14);
				leave_cr(bm_sem, 14);
				
				result = nrk_event_wait( SIG(ll_tx_done_signal) | SIG(nrk_wakeup_signal) );
				
				if(result & SIG(nrk_wakeup_signal))		// timed out 
				{
					_nrk_errno_set(SOCKET_TIMEOUT);
					return NRK_ERROR;
				}
				if(result)
				{;
				}	
  		*/
  		
  		while( bmac_tx_pkt(ptr, len) == -1)
  		{
   		result = nrk_event_wait( SIG(ll_tx_done_signal) );
   		}
   
  		leave_cr(tl_sem, 14);
		leave_cr(bm_sem, 14);
		return NRK_OK;
	}
			
	if(dest_addr < 0 || dest_port <= 0 || dest_port >= MAX_PORT_NUM)	// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 14);
		leave_cr(bm_sem, 14);
		return NRK_ERROR;
	}
	
	if(prio <= 0 || prio > MAX_PRIORITY)									// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 14);
		leave_cr(bm_sem, 14);
		return NRK_ERROR;
	}
		
	/* Error checking is complete. Begin processing */
	if(sock[sock_num].pindex == -1)	// this means that bind() / set_rx_queue_size() was 
												// not called earlier
	{
		if(DEBUG_TL == 2)
		{
			printf("%d ", NODE_ADDR);		
			nrk_kprintf(PSTR("called send() without a mapping\r\n"));
		}		
		int8_t buf_index, port_index, rbm_index; // indices for the various arrays
		uint8_t port; 
		
		if(get_num_bufs_free() < DEFAULT_RX_QUEUE_SIZE)	// no rx buffers left for this socket 
		{
			leave_cr(tl_sem, 14);
			leave_cr(bm_sem, 14);
			_nrk_errno_set(NO_RX_BUFFERS_AVAILABLE);
			return NRK_ERROR;
		}
		// retrieve the indices of the elements
		port_index = get_index_unassigned_port_element();
		rbm_index = get_index_unassigned_rbm_element();
		buf_index = get_index_unallocated_rx_buf();
		
		if(port_index == NRK_ERROR || rbm_index == NRK_ERROR)	// sanity check for debugging 
		{	
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("send(): Bug detected in implementation of port / rbm element array\r\n"));
		}
		
		if(buf_index == NRK_ERROR)	// sanity check for debugging
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("send(): Bug detected in implementation of num_bufs_free\r\n"));
		}			
		// retrieve the next ephemeral port number
		port = get_next_available_port();
				
		// fill up the members of the port element
		assign_port(port_index, port);
		
		// fill up the members of the socket descriptor
		sock[sock_num].pindex = port_index;
		sock[sock_num].rbmindex = rbm_index;
		
		// fill up the members of the rbm element
		rx_buf_mgr[rbm_index].pindex = port_index;
		rx_buf_mgr[rbm_index].pid = nrk_get_pid();
		
		// insert a rx buffer in the free queue 
		insert_rx_fq(&rx_buf_udp[buf_index], rbm_index, EMPTY);
		rx_buf_mgr[rbm_index].countTotal++; 	// increment the countTotal for this port 
		num_bufs_free--;
	
	} //end if(bind was called earlier)
	
	// prepare a UDP segment 
	udp_seg.srcPort = ports[sock[sock_num].pindex].pno;
	udp_seg.destPort = (uint8_t)dest_port;
	udp_seg.length = len;
	memcpy(udp_seg.data, ptr, len);	// application payload already packed
	
	if(DEBUG_TL == 2)
	{
		printf("%d: ", NODE_ADDR);	
		nrk_kprintf(PSTR("sent segment = "));
		print_seg(&udp_seg);
	}	 
	
	switch(sock[sock_num].type)
	{
		case SOCK_DGRAM:
			pkt.src = (uint16_t)NODE_ADDR;			// prepare a network packet 
			pkt.dest = (uint16_t)dest_addr;
			pkt.nextHop = route_addr(dest_addr);
			pkt.prevHop = NODE_ADDR;
			
			pkt.ttl = MAX_NETWORK_DIAMETER;
			pkt.type = UDP;
			pkt.length = SIZE_TRANSPORT_UDP_HEADER + len;
			pkt.prio = prio;
			pack_TL_UDP_header(pkt.data, &udp_seg);	// pack the UDP header 
			memcpy(pkt.data + SIZE_TRANSPORT_UDP_HEADER, udp_seg.data, MAX_APP_PAYLOAD); // copy the application payload
			
			if(DEBUG_TL == 2)
			{
				printf("%d: ",NODE_ADDR);
				nrk_kprintf(PSTR("sent packet = "));
				print_pkt(&pkt);
			}
		
			
			// insert the packet into the active transmit queue of the system
		 	result = insert_tx_aq(&pkt);
		 	if(DEBUG_TL == 2)
		 	{
		 		nrk_kprintf("TL: send(): Inserted packet.");
		 		print_tx_buffer();
		 	}
			
			if(result == NRK_ERROR)
				_nrk_errno_set(NO_TX_BUFFERS_AVAILABLE);
				
			leave_cr(tl_sem, 14);// This structure represents a packet sent by a node to the gateway over a serial connection
			leave_cr(bm_sem, 14);	
			return result;
			break;
			
		case SOCK_IPC:
			insert_rx_pq(&udp_seg, prio, NODE_ADDR, INVALID_RSSI);
			leave_cr(tl_sem, 14);
			leave_cr(bm_sem, 14);
			break;
			
		default:		// this should never happen
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
			{
				nrk_kprintf(PSTR("TL: send(): Bug discovered in implementation of socket type\r\n"));
			}
	} // end switch(socket type)
	
	return NRK_OK;
} // end send() 
/************************************************************************************************/
int8_t set_timeout(int8_t sock_num, int8_t secs, int8_t nano_secs)
{
	enter_cr(tl_sem, 15);
	//nrk_kprintf(PSTR("TL: set_timeout(): Successfully got the semaphore "));
	//printf("%d\r\n", *tl_sem);
	
	
	// ERROR checking
	if(sock_num < 0 || sock_num >= NUM_PORTS || secs < 0 || nano_secs < 0)	// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 15);
		return NRK_ERROR;
	}
	
	if(secs == 0 && nano_secs == 0)				// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 15);
		return NRK_ERROR;
	} 
	
	if(sock[sock_num].pid != nrk_get_pid()) 	// wrong socket number is passed 
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 15);
		return NRK_ERROR;
	}
	
	// error checking completed
	// assign the timeout values to the socket  
	sock[sock_num].timeout.secs = secs;
	sock[sock_num].timeout.nano_secs = nano_secs;
	
	leave_cr(tl_sem, 15);
	//nrk_kprintf(PSTR("TL: set_timeout(): Successfully released the sempahore "));
	//printf("%d\r\n", *tl_sem);
		
	return NRK_OK;
}
/*************************************************************************************************/
uint8_t* receive(int8_t sock_num, int8_t *len, uint16_t *srcAddr, uint8_t *srcPort, int8_t *rssi)
{
	nrk_sig_mask_t my_sigs; 	// to hold the return value of network layer signals 
	ReceiveBufferUDP *buf;		// temporary variable 
	Transport_Segment_UDP *seg;// pointer to received segment from the network layer 
	int8_t rbm_index;				// to store the index of the corresponding receive buffer manager
	int8_t port_index;			// to store the index of the port element  
	
	enter_cr(bm_sem, 16);
	enter_cr(tl_sem, 16);
	
	// ERROR checking
	if(sock_num < 0 || sock_num >= NUM_PORTS || len == NULL)
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 16);
		leave_cr(bm_sem, 16);
		return NULL;
	}

	if( sock[sock_num].pid != nrk_get_pid()) 
	{
		_nrk_errno_set(INVALID_SOCKET);	// socket description not complete
		
		leave_cr(tl_sem, 16);
		leave_cr(bm_sem, 16);
		return NULL;
	}
	
	if(sock[sock_num].pindex == -1)
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 16);
		leave_cr(bm_sem, 16);
		return NULL;
	}
	
	// check for SOCK_RAW first
	if(sock[sock_num].type == SOCK_RAW)
	{
		;
	}
	
	if(sock[sock_num].rbmindex == -1)	// sanity check for debugging
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("receive(): Bug detected in implementation of port/rbm element array\r\n"));
	}
	
	// error checking complete. Begin processing 
	rbm_index = sock[sock_num].rbmindex;	
	port_index = sock[sock_num].pindex;
	if(sock[sock_num].timeout.secs == 0 && sock[sock_num].timeout.nano_secs == 0)
	{
		// the user has not specified any maximum wait period 
		if(DEBUG_TL == 2)
			nrk_kprintf(PSTR("receive(): Inside the section that relates to 'without timeout' receive\r\n"));
		
		// check the receive buffer manager for this port to see if there are any queued segments
		if(rx_buf_mgr[rbm_index].countFree == rx_buf_mgr[rbm_index].countTotal)	// no queued segments 
		{
			if(DEBUG_TL == 2)
			{
				nrk_kprintf(PSTR("receive(): No segments in receive queue of port "));
				printf("%d\n", ports[port_index].pno);
			}
			while(1)
			{
				leave_cr(tl_sem, 16);
				leave_cr(bm_sem, 16);
				my_sigs = nrk_event_wait(SIG(ports[port_index].data_arrived_signal));	// block
				enter_cr(bm_sem, 16);
				enter_cr(tl_sem, 16);  
		
				// check what signal we got 
   			if(my_sigs == 0)                       
   			{
   				nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
						nrk_kprintf(PSTR("receive(): Error calling nrk_event_wait (without timeout)\r\n"));
				}   	
  				if( my_sigs & SIG(ports[port_index].data_arrived_signal) )	// got a segment 
  				{
  					if(DEBUG_TL == 2)
   					nrk_kprintf(PSTR("receive(): Received the data arrived signal\r\n"));
   				break;
   			}
				else 	// some other signal was received by the task. Go back and wait for 'seg arrived'
				{
					nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
						nrk_kprintf(PSTR("receive(): Unknown signal received (without timeout)\r\n"));
				} 
			} // end while(1) 
		} // end if
	} // end if(timeout == 0)
	else // the user has specified a timeout value 
	{
		if(DEBUG_TL == 2)
			nrk_kprintf(PSTR("receive(): Inside the section that relates to 'with timeout' receive\r\n"));
			
		if( nrk_signal_register(nrk_wakeup_signal) == NRK_ERROR)
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("receive(): Error in registering for nrk_wakeup_signal\r\n"));
		}
		if( nrk_set_next_wakeup(sock[sock_num].timeout) == NRK_ERROR)
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("receive(): Error returned by nrk_set_next_wakeup\r\n"));
		}		
		// check the receive manager for this port to see if there are any queued segments
		if(rx_buf_mgr[rbm_index].countFree == rx_buf_mgr[rbm_index].countTotal)	// no queued segments 
		{
			if(DEBUG_TL == 2)
			{
				nrk_kprintf(PSTR("receive(): No segments in receive queue of port "));
				printf("%d\n", ports[port_index].pno);
			}
			
			
			while(1)
			{
				leave_cr(tl_sem, 16);
				leave_cr(bm_sem, 16);
				my_sigs = nrk_event_wait(SIG(ports[port_index].data_arrived_signal) | SIG(nrk_wakeup_signal));
				enter_cr(bm_sem, 16);
				enter_cr(tl_sem, 16); 					 
		
				// check what signal we got 
   			if(my_sigs == 0)                       
   			{
   				nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
						nrk_kprintf(PSTR("receive(): Error calling nrk_event_wait() (with timeout)\r\n"));
				}   	
   			if( my_sigs & SIG(ports[port_index].data_arrived_signal) )	// got a segment 
   			{
   				sock[sock_num].timeout.secs = 0;		// invalidate the timeout 
   				sock[sock_num].timeout.nano_secs = 0;
   				if(DEBUG_TL == 2)
   					nrk_kprintf(PSTR("Received the data arrived signal\r\n"));
   				break;
   			}
   	
   			if( my_sigs & SIG(nrk_wakeup_signal) )		// timed out, no segment received so far 
   			{
   				sock[sock_num].timeout.secs = 0;			// invalidate the timeout value of the socket 
   				sock[sock_num].timeout.nano_secs = 0;
   				 
   				_nrk_errno_set(SOCKET_TIMEOUT);
   				
   				leave_cr(tl_sem, 16);
					leave_cr(bm_sem, 16);
	  				return NULL;
   			}
   			else
   			{
   				nrk_int_disable();
					nrk_led_set(RED_LED);
					while(1)
						nrk_kprintf(PSTR("receive(): Unknown signal received (with timeout)\r\n"));
				}
   		} // end while(1)
   	} // end if 
   } // end else
   
   // if the code reaches here, there are two possibilties
   // 1. The 'seg_arrived_signal' woke the task up
   // 2. The rx queue was not empty and no wait() was done
   // In either case, retrieve the segment and return back to user task 
   
   buf = remove_rx_pq(rbm_index);	// remove the buffer from the port queue 
   if(buf == NULL)	// this should not happen 
   {
   	nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("receive(): Bug found in implementation of data_arrived_signal / rx buffer mgmt\r\n"));
	}
   insert_rx_fq(buf, rbm_index, FULL);	// insert the buffer into the free queue with status = FULL 
   seg = &(buf -> seg);
   
   // fill up the values to be returned to the calling task 
	*len = seg -> length;
	if(srcAddr != NULL)
		*srcAddr = buf -> srcAddr;
	if(srcPort != NULL)
		*srcPort = seg -> srcPort;
	if(rssi != NULL)
		*rssi = buf -> rssi;

	leave_cr(tl_sem, 16);
	leave_cr(bm_sem, 16);
	return seg -> data;
}

/***************************************************************************************************/
int8_t check_receive_queue(int8_t sock_num)
{
	int8_t rbm_index;			// to store the index of rbm element associated with the socket	
	int8_t count1, count2, count3;
	
	enter_cr(bm_sem, 17);
	enter_cr(tl_sem, 17);
	
	// ERROR checking
	if(sock_num < 0 || sock_num >= NUM_PORTS)		// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 17);
		leave_cr(bm_sem, 17);
		return NRK_ERROR;
	}

	if( (sock[sock_num].pid != nrk_get_pid()) || (sock[sock_num].pindex == -1) ) // wrong socket number 
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 17);
		leave_cr(bm_sem, 17);
		return NRK_ERROR;
	}
	
	rbm_index = sock[sock_num].rbmindex;
	count1 = rx_buf_mgr[rbm_index].countTotal;
	count2 = rx_buf_mgr[rbm_index].countFree;
	count3 = get_in_process_buf_count(rbm_index);
	
	leave_cr(tl_sem, 17);
	leave_cr(bm_sem, 17);
	return count1 - count2 - count3;
}
/*****************************************************************************************************/
int8_t wait_until_send_done(int8_t sock_num)
{
	int8_t port_index;		// to store the index of the port element associated with the socket 	
	nrk_sig_mask_t my_sigs; // temp variable 
	
	enter_cr(bm_sem, 18);
	enter_cr(tl_sem, 18);
	
	// ERROR checking
	if(sock_num < 0 || sock_num >= NUM_PORTS)		// bad input 
	{
		_nrk_errno_set(INVALID_ARGUMENT);
		
		leave_cr(tl_sem, 18);
		leave_cr(bm_sem, 18);
		return NRK_ERROR;
	}
	
	if( (sock[sock_num].pid != nrk_get_pid()) || (sock[sock_num].pindex == -1) ) // bad socket 
	{
		_nrk_errno_set(INVALID_SOCKET);
		
		leave_cr(tl_sem, 18);
		leave_cr(bm_sem, 18);
		return NRK_ERROR;
	}
	
	// error checking complete. Begin processing 
	
	port_index = sock[sock_num].pindex;
	
	if(sock[sock_num].timeout.secs == 0 && sock[sock_num].timeout.nano_secs == 0)
	{
		// user has not specified any timeout value 
		while(1)
		{
			leave_cr(tl_sem, 18);
			leave_cr(bm_sem, 18);			
			my_sigs = nrk_event_wait( SIG(ports[port_index].send_done_signal) );
			enter_cr(bm_sem, 18);
			enter_cr(tl_sem, 18);
		
			if(my_sigs == 0)
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("wait_until_send_done(): Error returned by nrk_event_wait() (without timeout)\r\n"));
			}
			
			if( my_sigs & SIG(ports[port_index].send_done_signal) )	// data sent 
			{
				leave_cr(tl_sem, 18);
				leave_cr(bm_sem, 18);			
				return NRK_OK;
			}
				
			else	// unknown signal
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("wait_until_send_done(): Unknown signal received (without timeout)\r\n"));
			}
		} // end while(1)
	}
	else // user has specified some timeout value 
	{
		if( nrk_signal_register(nrk_wakeup_signal) == NRK_ERROR )
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("wait_until_send_done(): Error registering for nrk_wakeup_signal\r\n"));
		}
		if( nrk_set_next_wakeup(sock[sock_num].timeout) == NRK_ERROR)
		{
			nrk_int_disable();
			nrk_led_set(RED_LED);
			while(1)
				nrk_kprintf(PSTR("wait_until_send_done(): Error returned by nrk_set_next_wakeup()\r\n"));
		}		 
		while(1)
		{
			leave_cr(tl_sem, 18);
			leave_cr(bm_sem, 18);			
			my_sigs = nrk_event_wait( SIG(ports[port_index].send_done_signal) | SIG(nrk_wakeup_signal) );
			enter_cr(bm_sem, 18);
			enter_cr(tl_sem, 18);
			
			if(my_sigs == 0)
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("wait_until_send_done(): Error returned by nrk_event_wait()\r\n"));
			}
				
			if( my_sigs & SIG(ports[port_index].send_done_signal) )
			{
				sock[sock_num].timeout.secs = 0;			// disable the timeout 
				sock[sock_num].timeout.nano_secs = 0; 
				
				leave_cr(tl_sem, 18);
				leave_cr(bm_sem, 18);
				return NRK_OK;
			}
			
			if( my_sigs & SIG(nrk_wakeup_signal) )
			{
				sock[sock_num].timeout.secs = 0;			// disable the timeout 
				sock[sock_num].timeout.nano_secs = 0; 
				_nrk_errno_set(SOCKET_TIMEOUT);
				
				leave_cr(tl_sem, 18);
				leave_cr(bm_sem, 18);
				return NRK_ERROR;
			}
			else	// unknown signal caught
			{
				nrk_int_disable();
				nrk_led_set(RED_LED);
				while(1)
					nrk_kprintf(PSTR("wait_until_send_done(): Unknown signal received (with timeout)\r\n"));
			}
		} //end while(1)
	} // end else 
	
	leave_cr(tl_sem, 18);
	leave_cr(bm_sem, 18);
	return NRK_ERROR;
}
/**********************************************************************************************/
void print_seg_header(Transport_Segment_UDP *seg)
{
	printf("[%d %d %d] ", seg -> srcPort, seg -> destPort, seg -> length);
	return;
}
/*************************************************************************************************/
void print_seg(Transport_Segment_UDP *seg)
{
	int8_t i;
	
	print_seg_header(seg);
	for(i = 0; i < seg -> length; i++)
		printf("%d ", (seg -> data)[i]);
		
	printf("\r\n");
}
/*************************************************************************************************/
	
