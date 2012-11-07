/* This file contains the start-up routines for the network stack */

#include "NWStackConfig.h"
#include "NWErrorCodes.h"
#include <stdint.h>
#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <nrk_error.h>
#include <stdlib.h>
#include <math.h>
#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <ff_basic_sensor.h>

#define DEBUG_NWSC 2 


/******************************** EXTERN FUNCTIONS ********************************/

// From TransportLayerUDP.c 
extern void initialise_transport_layer_udp();

// From Pack.c
extern int8_t endianness();

// From BufferManager.c 
extern void initialise_buffer_manager();

// From NetworkLayer.c 
extern void initialise_network_layer();

// From Debug.c
extern void go_into_panic(char*);

//From bmac.c
extern void bmac_task_config();

// From Serial.c
void initialise_serial_communication();

/********************************* FUNCTION DEFINITIONS ****************************/
void nrk_init_nw_stack()
{
	/*if(endianness() == ERROR_ENDIAN)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("Error in calculating endianness in init_nw_stack()"));
	}
	*/
	// initialise a random number generator 
	srand(NODE_ADDR);
	
	initialise_serial_communication();
	if(DEBUG_NWSC == 2)
		nrk_kprintf(PSTR("Serial communications initalized\r\n"));
		
	
	initialise_buffer_manager();
	if(DEBUG_NWSC == 2)
		nrk_kprintf(PSTR("Buffer manager initialised\r\n"));
	
	initialise_transport_layer_udp();
	if(DEBUG_NWSC == 2)
		nrk_kprintf(PSTR("Transport layer initialised\r\n"));
	
	bmac_task_config();
	if(DEBUG_NWSC == 2)
		nrk_kprintf(PSTR("Link layer initialised\r\n"));
	
	initialise_network_layer();
	if(DEBUG_NWSC == 2)
		nrk_kprintf(PSTR("Network layer initialised\r\n"));
		
	nrk_register_drivers();
	
	print_sizes();
	
	return;
}

/*********************************************************************************************/		
void nrk_register_drivers()
{
	int8_t ret;
 	
 	// Register the Basic FireFly Sensor device driver
	ret = nrk_register_driver( &dev_manager_ff_sensors,FIREFLY_SENSOR_BASIC);
 	if(ret == NRK_ERROR) 
 	{
 		nrk_int_disable();
 		nrk_led_set(RED_LED);
 		nrk_kprintf( PSTR("Failed to load my ADC driver\r\n") );
 	}
 		
 	return;
}
/**********************************************************************************************/
void print_sizes()
{
	nrk_kprintf(PSTR("\r\nFrom NWStackConfig\r\n"));
	nrk_kprintf(PSTR("MAX_APP_PAYLOAD: "));
	printf("%u\r\n", MAX_APP_PAYLOAD);
	nrk_kprintf(PSTR("MAX_SERIAL_PAYLOAD: "));
	printf("%u\r\n", MAX_SERIAL_PAYLOAD);
	nrk_kprintf(PSTR("MAX_GATEWAY_PAYLOAD: "));
	printf("%u\r\n", MAX_GATEWAY_PAYLOAD);
	nrk_kprintf(PSTR("MAX_RX_QUEUE_SIZE: "));
	printf("%u\r\n", MAX_SUBNET_SIZE);
	nrk_kprintf(PSTR("MAX_TX_QUEUE_SIZE: "));
	printf("%u\r\n", MAX_TX_QUEUE_SIZE);
	nrk_kprintf(PSTR("NUM_PORTS: "));
	printf("%u\r\n", NUM_PORTS);
	
	return;
}
/*********************************************************************************************************/
