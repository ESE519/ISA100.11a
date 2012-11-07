/* This file contains the start-up routines for the network stack */

#include "NWStackConfig.h"
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

//From bmac.c
extern void bmac_task_config();

// From Serial.c
void initialise_serial_communication();

/********************************* FUNCTION DEFINITIONS ****************************/
void nrk_init_nw_stack()
{
	// initialise a random number generator 
	//srand(NODE_ADDR);
	
	initialise_serial_communication();
	if(DEBUG_NWSC == 2)
		nrk_kprintf(PSTR("Serial communications initalized\r\n"));
		
	
	//bmac_task_config();
	if(DEBUG_NWSC == 2)
		nrk_kprintf(PSTR("Link layer initialised\r\n"));
	
	return;
}
