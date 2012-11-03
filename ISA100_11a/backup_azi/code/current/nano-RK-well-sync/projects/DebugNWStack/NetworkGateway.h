// This file contains the data structures needed for the implementation of the network gateway 

#ifndef _NETWORK_GATEWAY_H
#define _NETWORK_GATEWAY_H 

#include <stdio.h>
#include <stdint.h>
#include <slipstream.h>

/************************************** CONSTANTS *******************************************/
#define MAX_GATEWAY_PAYLOAD 48
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

#define SIZE_GATEWAYTONODESERIAL_PACKET_HEADER 2
#define SIZE_GATEWAYTONODESERIAL_PACKET (SIZE_GATEWAYTONODESERIAL_PACKET_HEADER + MAX_GATEWAY_PAYLOAD)

#define SIZE_IP_ADDRESS	16			// size of a string capable of holding an IP address
#define GATEWAY_ADDRESS ("127.0.0.1")
#define GATEWAY_PORT 4000

#ifndef FALSE 
#define FALSE 0 
#endif 

#ifndef TRUE
#define TRUE 1
#endif 
 
#define DEBUG_NG 0  

/********************************** FUNCTION PROTOTYPES *********************************/

#endif 
