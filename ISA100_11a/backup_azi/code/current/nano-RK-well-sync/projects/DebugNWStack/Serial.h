/* This file contains the constants and data structures required for the implementation
   of the serial communication component of the network stack
   
   Authors:
   Aditya Bhave 
*/

// CHECKED
#ifndef _SERIAL_H
#define _SERIAL_H

#include "NWStackConfig.h"
#include <stdint.h>
#include <slip.h>

#define DEBUG_SR 0 					// debug flag for Serial.c 

/********************************** CONSTANTS ********************************************/
#define SIZE_GATEWAYTONODESERIAL_PACKET_HEADER 2
#define SIZE_GATEWAYTONODESERIAL_PACKET (SIZE_GATEWAYTONODESERIAL_PACKET_HEADER + MAX_GATEWAY_PAYLOAD)

#define SIZE_SLIP_TX_BUF 128

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
void create_serial_task();
/*
This function creates the serial task

	PARAMS:		None
	
	Returns:	None
	Comments: 	private function
*/

void initialise_serial_communication();
/*
This function initialises the serial communication component of the network stack. 

	PARAMS:		None
	
	Returns:	None
	Comments:	private function. This is simply a wrapper around create_serial_task()
*/
	
	


#endif
