/* This file implements the network gateway for the sensor network */
/* Authors:
   Aditya Bhave
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "NetworkGateway.h"

/*********************************** External variables and functions ******************************/
/*********************************** Global data structures **************************************/
static GatewayToNodeSerial_Packet gtn_pkt;					// to hold a packet to be sent to the Firefly
static char tx_buf[SIZE_GATEWAYTONODESERIAL_PACKET];    	// transmit buffer to send data to attached Firefly

/*****************************************************************************************************/
void send_dummy_pkt()
{
	int8_t ret;					// to hold the return value of function calls
	static int8_t count1 = 0;	// temporary variable for debugging purposes
	
	
	// send the dummy packet
	do
	{
		// construct the dummy packet
		gtn_pkt.type = 0x00;
		gtn_pkt.length = 1;
		gtn_pkt.data[0] = count1;
	
		tx_buf[0] = gtn_pkt.type;
		tx_buf[1] = gtn_pkt.length;
		memcpy(tx_buf + SIZE_GATEWAYTONODESERIAL_PACKET_HEADER, gtn_pkt.data, MAX_GATEWAY_PAYLOAD);
	
		ret = slipstream_send(tx_buf, (int)SIZE_GATEWAYTONODESERIAL_PACKET); // send an inital message to the server
		if(ret == 0)
		{
			printf("NG: send_dummy_pkt(): Error in sending message to firefly\r\n");
			continue;
		}
		printf("NG: send_dummy_pkt(): Sent dummy message to the firefly: %d\r\n", count1);
		count1 = (count1 + 1) % 120;
		break;
	}while(1);
	
	return;
}
/*********************************** FUNCTION DEFINITIONS ****************************************/
int main()
{
	GatewayToNodeSerial_Packet gtn_pkt;			// to send a packet to the attached Firefly
	int8_t ret;									// holds the return value of function calls
	char gw_addr[SIZE_IP_ADDRESS];				// temporary variable to hold the IP address of the gateway
	uint32_t i;									// loop variable 
	uint32_t j;
	uint32_t count = 10;
	
	// make an UDP socket to connect to the SLIPStream server
	if( slipstream_open(GATEWAY_ADDRESS, GATEWAY_PORT, 0) == 0 )
	{
		printf("NG: main(): Error in connecting to the gateway server at [%s,%d]\r\n", strcpy(gw_addr, GATEWAY_ADDRESS), GATEWAY_PORT);
		exit(1);
	}
	
	// construct and send a dummy packet
	while(1)
	{
		send_dummy_pkt();
		sleep(1);
	}
	
	return;
}
/***************************************************************************************************/

/**********************************************************************************/
