/* This file contains the data structures and constants needed for the implementation
	of the transport layer of the network stack. This layer provides UDP-like functionality 
*/

#ifndef _TRANSPORT_LAYER_UDP_H
#define _TRANSPORT_LAYER_UDP_H

#include "NWStackConfig.h"
#include <nrk.h>
#include <include.h>
#include <stdint.h>
/********************************* CONSTANTS **********************************************/

#define DEBUG_TL 0   					// debug flag for the transport layer 

// Types of sockets supported by the network stack 
#define SOCK_DGRAM 1 					// UDP-like 
#define SOCK_REL 2 						// reliable UDP (stop and wait) 
#define SOCK_RAW 3 						// if the application wants to bypass the transport and network layers
#define SOCK_IPC 4 						// for IPC within a node 

#define EPHEMERAL_PORT_NUM_START 11		// 11 is the smallest port number available to user tasks
#define MAX_PORT_NUM 255				// maximum port number that can be opened in the system
#define MAX_PORTS 127			 		// maximum number of ports that can be opened in the system 


// limits on various object types 
#define SIZE_TRANSPORT_UDP_HEADER 3
#define MAX_TRANSPORT_UDP_SEG (MAX_APP_PAYLOAD + SIZE_TRANSPORT_UDP_HEADER)
#define SIZE_TRANSPORT_UDP_RELIABLE_HEADER 4
#define SIZE_TRANSPORT_UDP_RELIABLE_SEG (MAX_APP_PAYLOAD + SIZE_TRANSPORT_UDP_RELIABLE_HEADER)

#define INVALID_PORT		0   		// indicates an invalid port number 
#define INVALID_PID			0   		// indicates an invalid pid (reserved for the kernel)
#define INVALID_RSSI		(-120)		// indicates an invalid RSSI	

/********************************* DATA STRUCTURES ****************************************/
typedef struct
{
	uint8_t srcPort;						// source port of the application
	uint8_t destPort;						// destination port of the application
	int8_t length;							// actual length of the data payload
	uint8_t data[MAX_APP_PAYLOAD];			// application layer data 
		
}Transport_Segment_UDP;

typedef struct
{
	uint8_t srcPort;
	uint8_t destPort;
	int8_t length;
	int8_t ack;	// -1 indicates a data packet. Otherwise indicates the sequence no being ACKed
	uint8_t data[MAX_APP_PAYLOAD];
}Transport_Segment_UDP_Reliable;
	

typedef struct 
{
	uint8_t pno;					// the actual port number 
	nrk_sig_t data_arrived_signal;	// indicates that data has arrived for this port in the receive queue 
	nrk_sig_t send_done_signal;		// indicates that the highest priority message from this port has been sent
	  									 
}Port;

typedef struct
{
	int8_t pindex;				// index of the port element for this socket
	int8_t rbmindex;			// index of the rbm element for this socket	 
	int8_t pid;					// pid of the process holding this socket descriptor
	int8_t type;				// type of socket
	nrk_time_t timeout;			// to store a timeout value for the socket
}Socket;

/********************************** FUNCTION PROTOTYPES ***********************************/

// Function prototypes defined in TransportLayerUDP.c
void initialise_transport_layer_udp();
/*
This function initialises the variables used by the UDP-transport layer

	PARAMS:		None
	RETURNS:		None
	COMMENTS:	private function, called during initialization of the network stack 
*/

int8_t get_next_available_socket();
/*
This function returns the index of the next available socket descriptor, if available 

	PARAMS:		None
	RETURNS:		index of the next available socket descriptor if available, NRK_ERROR otherwise
	ERROR NOS:	NO_SOCKET_DESC_AVAILABLE when no descriptors are available 
	COMMENTS:	private function 
*/

int8_t create_socket(int8_t type);
/*
This function creates a socket of a given type and returns it's descriptor

	PARAMS:		type: The type of the socket. Currently only SOCK_DGRAM is supported
	RETURNS:		socket descriptor or NRK_ERROR otherwise
	ERROR NOS:	UNSUPPORTED_SOCK_TYPE if an invalid socket type is passed
					NO_SOCKET_DESC_AVAILABLE if no descriptor is available
	COMMENTS:	User API
*/

uint8_t get_next_available_port();
/*
This function returns the next available port number if present

	PARAMS:		None
	RETURNS:		port number if available, INVALID_PORT otherwise
	ERROR NOS:	NO_PORTS_AVAILABLE if no ephemeral port numbers are available
	COMMENTS:	private function 
*/

int8_t check_port_available(uint8_t pt);
/* 
This function checks whether port 'pt' is available

	PARAMS:		pt: port number to be checked
	RETURNS:		NRK_OK if 'pt' is available. NRK_ERROR otherwise
	ERROR NOS:	PORT_UNAVAILABLE if 'pt' is found to be unavailable
	COMMENTS:	private function 
*/ 

void assign_port(int8_t pindex, uint8_t pt);
/*
This functions records the fact that port 'pt' is allocated

	PARAMS:		pindex: index of the relevant port element
					pt: port number to be allocated
	RETURNS:		None
	COMMENTS:	private function
	
*/

void release_port(int8_t pindex);
/*
This functions records the fact that a given port is now available

	PARAMS:		pindex: port element holding information about the port
	RETURNS:		None
	COMMENTS:	private function
*/

int8_t get_unassigned_port_element();
/*
This function returns the index an unassigned port element or NRK_ERROR if none are available

	PARAMS:		None
	RETURNS:		index of an unassigned port element or NRK_ERROR if none are available
	
	ERROR NOS:	NO_PORT_ELEMENT_AVAILABLE
	Comments:	private function
*/

int8_t get_unassigned_rbm_element();
/*
This function returns the index an unassigned rbm element or NRK_ERROR if none are available

	PARAMS:		None
	RETURNS:		index of an unassigned rbm element or NRK_ERROR if none are available
	
	ERROR NOS:	NO_RBM_ELEMENT_AVAILABLE
	Comments:	private function
*/

uint8_t get_port_num(int8_t sock_num);
/*
This function can be used by an application to find the port number mapped to a given socket descriptor

	PARAMS:		sock_num: The socket descriptor
	RETURNS:		the corresponding port number or INVALID_PORT if none exists / error
	
	ERROR NOS:	INVALID_ARGUMENT if the passed parameter is faulty
					INVALID_SOCKET if a wrong socket number is passed
					UNMAPPED_SOCKET if no socket/port mapping exists
					
	Comments:	User API
					A return value of INVALID_PORT could mean either an error has occurred or there 
					does not exist any socket/port mapping. Check the error number to find out.
*/ 

int8_t bind(int8_t sock_num, int16_t port);
/*
This function binds a given socket descriptor to a given port number

	PARAMS:		sock_num:	The socket number
					port:			The port number
	
	RETURNS:		NRK_OK if the bind was successful
					NRK_ERROR if not
					
	ERROR NOS:	INVALID_ARGUMENT when the passed paramters are faulty
					INVALID_SOCKET if a wrong socket number is passed
					PORT_UNAVAILABLE if the requested port number is already taken
					NO_RX_BUFFERS_AVAILABLE if the system has run out of receive buffers
					INVALID_CALL if bind() or set_rx_queue_size() was called earlier
						
	COMMENTS:	User API
*/

int8_t get_rx_queue_size(int8_t sock_num);
/*
This function returns the size of the rx queue associated with a given socket descriptor

	PARAMS:		sock_num: The socket descriptor
	RETURNS:		size of the rx queue if no error is found, NRK_ERROR otherwise
	
	ERROR NOS:	INVALID_ARGUMENT when the passed paramters are faulty
					INVALID_SOCKET if a wrong socket number is passed
					
	COMMENTS:	User API. 
					If no socket operation is performed on the passed socket descriptor prior
					to this call, no rx buffers are set aside and hence 0 is returned
*/	 

int8_t set_rx_queue_size(int8_t sock_num, int8_t size);
/*
This function sets the size of the receive queue for a given socket descriptor

	PARAMS:		size:	Size of the queue in units of number of transport layer segments
					sock_num: socket descriptor corresponding to the port for which the buffers
								 are being requested
	
	RETURNS:		actual number of rx buffers allocated. Usually this will equal 'size' but it 
					may be less than 'size' if not enough buffers are available in the pool.
					
	ERROR NOS: 	INVALID_ARGUMENT when the passed parameters are faulty
					INVALID_SOCKET if a wrong socket number is passed
					NO_PORTS_AVAILABLE if bind() was not called previously 
											 and no ephemeral ports are available
					NO_RX_BUFFERS_AVAILABLE if bind() was not called previously 
											   and no buffers are available
	
	COMMENTS:	User API
					This function can be called independently of bind(). Remember however that
					3 things are essential for a socket description to be complete
					1. a valid pid
					2. a valid port
					3. a receive queue of size at least 1
					
*/ 

int8_t release_buffer(int8_t sock_num, uint8_t* ptr);
/* 
This function allows an application to return a receive buffer back to the buffer manager

	PARAMS:		ptr: pointer to the buffer
					sock_num: socket descriptor corresponding to the port number to which the 
								 receive buffer belongs 
					
	RETURNS:		NRK_OK if the release is successful, NRK_ERROR otherwise
		
	ERROR NOS:	INVALID_ARGUMENT when the passed parameters are faulty
					INVALID_SOCKET if the wrong socket number is passed
					
	COMMENTS:	User API
					This function should be called when the application has finished processing
					a received message or has copied the message to a application-local storage 
					area
*/ 

int8_t close_socket(int8_t sock_num);
/*
This function allows an application to close an existing opened socket

		PARAMS: 		sock_num: The socket descriptor
		RETURNS:		NRK_OK if the close operation is performed successfully, NRK_ERROR otherwise
		
		ERROR NOS:	INVALID_ARGUMENT when the passed parameters are faulty
						INVALID_SOCKET if the wrong socket number is passed
		
		COMMENTS: User API
*/

int8_t is_port_associated(int16_t port);
/*
This function checks whether a given port number has been assigned to a socket or not 
		
		PARAMS:		port: The port number
		RETURNS: 	TRUE if a mapping exists, FALSE if a mapping does not exist, NRK_ERROR on error
		
		ERROR NOS:	INVALID_ARGUMENT if the passed paramter is faulty		
		COMMENTS: 	User API 
*/

int8_t send(int8_t sock_num, int8_t *ptr, int8_t len, int32_t dest_addr, int16_t dest_port, int8_t prio);
/*
This function can be used by the application task to send data to other nodes on the network

	PARAMS: 		ptr:  		pointer to application task buffer
					len:			size of the message to be transmitted
					dest_addr:	address of destination node
					dest_port:	receiving port number of destination task
					prio:			priority of the message to be sent 
					
	RETURNS: 	NRK_OK if the message is queued successfully in the TX queue, 
					NRK_ERROR otherwise
					
	ERROR NOS:	INVALID_ARGUMENT if the passed paramters are faulty
					INVALID_SOCKET if the wrong socket number is passed
					NO_PORTS_AVAILABLE if this is the first operation to be performed on the socket
											 and no ephemeral port number is present
					NO_RX_BUFFERS_AVAILABLE if this is the first operation to be performed on the socket
											 and no receive buffers are available
					NO_TX_BUFFERS_AVAILABLE if the transmit queue of the system is full
					
	COMMENTS: 	User API
					non-blocking, returns status code immediately. A return value of NRK_OK only means
					that the message has been queued in the transmit queue. Look for the send_done signal
					to find out when the message actually leaves the radio.
*/

int8_t set_timeout(int8_t sock_num, int8_t secs, int8_t nano_secs);
/*
This functions allows an application task to set a timeout value on a given socket 

	PARAMS:		sock_num: The socket descriptor
					secs, nano_secs: The timeout value
					
	RETURNS:		NRK_OK if the timeout was regsitered correctly
					NRK_ERROR otherwise
					
	ERROR NOS:	INVALID_ARGUMENT if the passed paramters are faulty
					INVALID_SOCKET if the wrong socket number is passed 
					
	Comments:	User API
					You cannot pass a value of (0,0) as the timeout value. One or both of the 
					paramters have to be positive integers. Use check_receive_queue() for a 
					non-blocking receive()
*/ 

uint8_t* receive(int8_t sock_num, int8_t *len, uint16_t *srcAddr, uint8_t *srcPort, int8_t *rssi);
/*
This function allows an application to receive a message along with network control information
from its socket

	PARAMS:		sock_num: The socket descriptor
					len:		 The network stack will put the length of the received message in this variable
					srcAddr:	 The network stack will put the source address of the message in this variable
					srcPort:	 The network stack will put the source port of the message in this variable 
					rssi:		 The network stack will put the signal strength with which the underlying
								 packet was received in this variable
								 
	RETURNS:		pointer to the application payload
					NULL if some error is encountered
					
	ERROR NOS:	INVALID_ARGUMENT if the passed paramters are faulty
					INVALID_SOCKET if a wrong socket number is passed
					SOCKET_TIMEOUT if a timeout value is specified prior to this call and 
					the timeout value expires
	
	Comments:	User API
					This function will block the task until a message arrives in its receive buffer
					If you want to specify a maximum wait period, use set_timeout() prior to every such
					receive() call.
*/ 

int8_t check_receive_queue(int8_t sock_num);
/*
This function allows an application to check whether there is any message queued for it 

	PARAMS:		sock_num: The socket descriptor
	
	RETURNS:		number of queued messages (0 if the queue is empty)
					NRK_ERROR if some error is encountered during processing
					
	ERROR NOS:	INVALID_ARGUMENT if the passed paramter is faulty
					INVALID_SOCKET if the wrong socket number is passed
	
	Comments:	User API
					Use this function if you want a non-blocking receive(). Call this first and if
					this returns a positive integer, a subsequent receive() call will return the
					queued message	immediately. 
					
					NOTE: The number returned by this function corresponds to the number of queued
					messages NOT yet read by the application.	If the application has not
					released a buffer it receive()'d previously, that message is not included in
					the count.
*/ 

int8_t wait_until_send_done(int8_t sock_num);
/*
This function will block the task until a message enqueued by it previously is actually sent over 
the radio

	PARAMS:		sock_num: The socket descriptor
	RETURNS:		NRK_OK if the 'send_done' signal wakes the task up
					NRK_ERROR if there is some error in processing OR timeout value when specified 
					expires. In this case, the error number is set to SOCKET_TIMEOUT
	
	ERROR NOS:	INVALID_ARGUMENT if the passed parameter is faulty
					INVALID_SOCKET if an incomplete socket descriptor is passed
					SOCKET_TIMEOUT if a timeout value is specified prior to this call and 
					the timeout value expires
	Comments:	User API
					If you want to specify a maximum wait period, use set_timeout() prior to every such
					wait_until_send_done() call. The return value of NRK_ERROR could mean either that an 
					error occured during processing or that the timeout expired. Check the error number
					to find out
*/ 

void print_seg_header(Transport_Segment_UDP* seg);
void print_seg(Transport_Segment_UDP *seg);

#endif












