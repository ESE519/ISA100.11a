/* This file contains the constants and data structures required for the implementation
   of the serial line protocol connecting the gateway to the node 
*/

#ifndef _SERIAL_H
#define _SERIAL_H

#include "NWStackConfig.h"
#include "NetworkLayer.h"
#include <stdint.h>
#include <slip.h>

#define DEBUG_SR 0 					// debug flag for Serial.c 

/* SLIP character codes 
#define END             0300     // indicates end of packet
#define ESC             0333     // indicates byte stuffing
#define ESC_END         0334     // ESC ESC_END means END data byte
#define ESC_ESC         0335     // ESC ESC_ESC means ESC data byte
*/

// possible bit mask values of the packet type 
#define SERIAL_APPLICATION 0x00	// 0 
#define SERIAL_NW_CONTROL	0x80  // 1 

// possible bit mask values of the packet sub type for SERIAL_NW_CONTROL
#define SERIAL_NGB_LIST 0x80		// 000  
#define SERIAL_ROUTE_CONFIG 0x80	// 000
#define INVALID	0xFF				// 111 if the type is invalid

#define SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER 2  
#define SIZE_NODETOGATEWAYSERIAL_PACKET (SIZE_NODETOGATEWAYSERIAL_PACKET_HEADER + MAX_SERIAL_PAYLOAD)

#define SIZE_GATEWAYTONODESERIAL_PACKET_HEADER 2
#define SIZE_GATEWAYTONODESERIAL_PACKET (SIZE_GATEWAYTONODESERIAL_PACKET_HEADER + MAX_GATEWAY_PAYLOAD)

// This structure represents a packet sent by a node to the gateway over a serial connection
typedef struct
{
	uint8_t type;						// bit mask for packet type and subtype
											// bit 7:	SERIAL_APPLICATION OR SERIAL_NW_CONTROL 
											// bit 6,5,4:  subtype for SERIAL_APPLICATION; set to 000 for now 
											// bit 3,2,1:  subtype of SERIAL_NW_CONTROL;
											//             SERIAL_NGB_LIST is the only option for now 
											// bit 0: unused, always set to 0 
	
	int8_t length;						// actual length of the serial payload 
	uint8_t data[MAX_SERIAL_PAYLOAD]; // serial payload 

}NodeToGatewaySerial_Packet;

typedef struct
{
	uint8_t type;						// bit mask for packet type and subtype
										// bit 7:	SERIAL_APPLICATION OR SERIAL_NW_CONTROL 
										// bit 6,5,4:  subtype for SERIAL_APPLICATION; set to 000 for now 
										// bit 3,2,1:  subtype of SERIAL_NW_CONTROL;
										//             SERIAL_ROUTE_CONFIG is the only option for now 
										// bit 0: unused, always set to 0 
	int8_t length;
	uint8_t data[MAX_GATEWAY_PAYLOAD];
}GatewayToNodeSerial_Packet;

/************************************ FUNCTION DEFINITIONS **********************************/
void sendToSerial(uint8_t *buf, int8_t length);
/*
This function implements the SLIP protocol to transfer a buffer of bytes to the gateway

	PARAMS:		buf:	pointer to the buffer
					length: length of the buffer
					
	RETURNS:		None
	Comments:	private function.
*/ 

void printBuffer(uint8_t *buf, int8_t len);
/*
This function simply sends an unformatted stream of characters to the serial port 

	PARAMS:		buf: pointer to the buffer
					len: length of the buffer
					
	RETURNS:		None
	Comments:	None
*/ 

void create_serial_task();
uint8_t serial_pkt_type(GatewayToNodeSerial_Packet *pkt);
uint8_t serial_nw_ctrl_type(GatewayToNodeSerial_Packet *pkt);
void process_serial_app_pkt(GatewayToNodeSerial_Packet *pkt);
void process_serial_nw_ctrl_pkt(GatewayToNodeSerial_Packet *pkt);
void print_gtn_pkt(GatewayToNodeSerial_Packet*);
void print_gtn_pkt_header(GatewayToNodeSerial_Packet *pkt);
void print_RoutingTable(Msg_RoutingTable *mrtbl);
void initialise_serial_communication();


#endif
