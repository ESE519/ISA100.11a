/******************************************************************************
*  Nano-RK, a real-time operating system for sensor networks.
*  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
*  All rights reserved.
*
*  This is the Open Source Version of Nano-RK included as part of a Dual
*  Licensing Model. If you are unsure which license to use please refer to:
*  http://www.nanork.org/nano-RK/wiki/Licensing
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, version 2.0 of the License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Contributing Authors (specific to this file):
*  Aditya Bhave
*******************************************************************************/


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
extern go_into_panic(char*);

//From bmac.c
extern void bmac_task_config();

// From Serial.c
void initialise_serial_communication();

/********************************* FUNCTION DEFINITIONS ****************************/
void nrk_init_nw_stack()
{
	if(endianness() == ERROR_ENDIAN)
	{
		nrk_int_disable();
		nrk_led_set(RED_LED);
		while(1)
			nrk_kprintf(PSTR("Error in calculating endianness in init_nw_stack()"));
	}
	
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
		
	return;
}
