/* This file serves as the header file for the link layer */

/* Authors:
  Aditya Bhave
*/

#include<stdint.h>

/***************************** CONSTANTS *********************************************/\
#define TX_CHANNEL 25 
#define TX_RADIO_POWER 10	// transmit power of the radio
#define TX_WAIT_PERIOD 10	

/***************************** FUNCTION PROTOTYPES ***********************************/

int8_t initialise_link_layer(uint8_t channel);
/* 
	This function initialises the link layer presently used by the network stack 
	
	PARAMS: channel: The RF channel on which the radio should communicate
	RETURNS: None
	Comments: Internal function
*/
