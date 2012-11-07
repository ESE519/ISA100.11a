/* This file contains the constants and data structures required for the implementation
   of the serial communication component of the network stack
   
   Authors:
   Aditya Bhave 
*/

// CHECKED
#ifndef _SERIAL_H
#define _SERIAL_H

#include "NWStackConfig.h"
#include "NetworkLayer.h"
#include <stdint.h>
#include <slip.h>

#define DEBUG_SR 0 					// debug flag for Serial.c 

/********************************** CONSTANTS ********************************************/
// possible bit mask values of the packet type (bit 7) 
#define SERIAL_APPLICATION  0x00  // 0  
#define SERIAL_NW_CONTROL	0x80  // 1

// possible bit mask values of the packet sub type for SERIAL_NW_CONTROL (bits 4,3,2,1)
#define SERIAL_NGB_LIST 0x80				// 100 0 000 0 (sent from node to gateway)
#define SERIAL_ROUTE_REQUEST 0x82			// 100 0 001 0 (sent from node to gateway)
#define SERIAL_ROUTE_CONFIG 0x84			// 100 0 010 0 (sent from gateway to the node)
#define SERIAL_ROUTE_REPLY 0x86				// 100 0 011 0 (sent from gateway to node)
#define SERIAL_NW_INFO_ACQUIRED 0x88		// 100 0 100 0 (sent from gateway to node)
#define SERIAL_SEND_NW_INFO 0x8A			// 100 0 101 0 (sent from gateway to node)
#define SERIAL_SEND_NODE_INFO 0x8C			// 100 0 110 0 (sent from gateway to node)
#define SERIAL_NODE_INFO 0x8E				// 100 0 111 0 (sent from node to gateway)
#define SERIAL_NODE_INFO_ACQUIRED 0x90		// 100 1 000 0 (sent from gateway to node)

#define SERIAL_SEND_NW_INFO_ACK	0x92		// 100 1 001 0 (sent from node to gateway)
#define SERIAL_NW_INFO_ACQUIRED_ACK	0x94	// 100 1 010 0 (sent from node to gateway)
#define SERIAL_INVALID 0x96					// 100 1 011 0 (invalid packet type)
#define SERIAL_ACK 0x98						

#define SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER 2  
#define SIZE_NODETOGATEWAYSERIAL_PACKET (SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER + MAX_SERIAL_PAYLOAD)

#define SIZE_GATEWAYTONODESERIAL_PACKET_HEADER 2
#define SIZE_GATEWAYTONODESERIAL_PACKET (SIZE_GATEWAYTONODESERIAL_PACKET_HEADER + MAX_GATEWAY_PAYLOAD)

#define SIZE_SLIP_TX_BUF 128

// This structure represents a packet sent by a node to the gateway over a serial connection
typedef struct
{
	uint8_t type;						// bit mask for packet type and subtype
											// bit 7:	SERIAL_APPLICATION OR SERIAL_NW_CONTROL 
											// bit 6,5: subtype for SERIAL_APPLICATION; set to 00 for now 
											// bit 4,3,2,1: subtype of SERIAL_NW_CONTROL;
											// bit 0: unused, always set to 0 
	
	int8_t length;					   // actual length of the serial payload 
	uint32_t seq_no;				   // seq_no copied 
	uint8_t data[MAX_SERIAL_PAYLOAD];  // serial payload 

}NodeToGatewaySerial_Packet;

typedef struct
{
	uint8_t type;						// bit mask for packet type and subtype
											// bit 7:	SERIAL_APPLICATION OR SERIAL_NW_CONTROL 
											// bit 6,5: subtype for SERIAL_APPLICATION; set to 00 for now 
											// bit 4,3,2,1: subtype of SERIAL_NW_CONTROL;
											// bit 0: unused, always set to 0 
	
	int8_t length;					   // actual length of the serial payload 
	uint8_t data[MAX_GATEWAY_PAYLOAD]; // serial payload 

}GatewayToNodeSerial_Packet;

/************************************ FUNCTION DEFINITIONS **********************************/
int8_t sendToSerial(uint8_t *buf, uint8_t length);
/*
This function implements the SLIP protocol to transfer a buffer of bytes to the gateway

	PARAMS:		buf:	pointer to the buffer
				length: length of the buffer (CANNOT be more than 255 bytes)
					
	RETURNS:	NRK_ERROR if an error is encountered
				NRK_OK if transmission is successful
	Comments:	private function.
*/ 

void printBuffer(uint8_t *buf, uint8_t len);
/*
This function sends an unformatted stream of characters to the serial port 

	PARAMS:		buf: pointer to the buffer
				len: length of the buffer (CANNOT be more than 255 bytes)
					
	RETURNS:	None
	Comments:	private function for debugging
*/ 

void create_serial_task();
/*
This function creates the serial task

	PARAMS:		None
	
	Returns:	None
	Comments: 	private function
*/

uint8_t serial_pkt_type(GatewayToNodeSerial_Packet *pkt);
/*
This function evaluates the type of the serial packet

	PARAMS:		pkt: Pointer to the packet
	
	Returns:	the type of the packet or INVALID if a wrong packet is received
	Comments:	private function
*/

void process_serial_app_pkt(GatewayToNodeSerial_Packet *pkt);
/*
This function processes the packet if it is of type SERIAL_APPLICATION

	PARAMS:		pkt: pointer to the packet
	
	Returns:	None
	Comments:	private function
*/

void process_serial_nw_ctrl_pkt(GatewayToNodeSerial_Packet *pkt);
/*
This function processes the packet if it is of type SERIAL_NW_CONTROL

	PARAMS:		pkt: pointer to the packet
	
	RETURNS:	None
	Comments: 	private function
*/

void print_gtn_pkt(GatewayToNodeSerial_Packet*);
void print_gtn_pkt_header(GatewayToNodeSerial_Packet *pkt);
/*
These functions print out the contents of the corresponding packet

	PARAMS:		pkt: pointer to the packet
	
	Returns:	None
	Comments:	private function for debugging
*/
void initialise_serial_communication();
/*
This function initialises the serial communication component of the network stack. 

	PARAMS:		None
	
	Returns:	None
	Comments:	private function. This is simply a wrapper around create_serial_task()
*/
void send_ACK();
	
	


#endif
