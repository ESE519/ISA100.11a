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


#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <stdint.h>

#include "Debug.h"

/****************************** FUNCTION DEFINITIONS ****************************************/
void go_into_panic(char *str)
// This function is for signalling error conditions 
{
	nrk_int_disable();
	
	nrk_led_set(RED_LED);
	while(1)
		printf("PANIC: %s. This should never happen\n", str);
	return;
}
/**********************************************************************************************/
void print_nw_stack_errno(int8_t n)
{
	switch(n)
	{
		case 1:
			nrk_kprintf(PSTR("Maximum neighbor limit reached\r\n"));
			break;
			
		case 2:
			nrk_kprintf(PSTR("No socket descriptor available\r\n"));
			break;
		
		case 3:
			nrk_kprintf(PSTR("Unsupported socket type\r\n"));
			break;
		
		case 4:
			nrk_kprintf(PSTR("Port is unavailable\r\n"));
			break;

		case 5:
			nrk_kprintf(PSTR("Invalid socket descriptor\r\n"));
			break;

		case 6:
			nrk_kprintf(PSTR("Invalid call made\r\n"));
			break;

		case 7:
			nrk_kprintf(PSTR("No ports available\r\n"));
			break;

		case 8:
			nrk_kprintf(PSTR("Invalid arguments passed\r\n"));
			break;

		case 9:
			nrk_kprintf(PSTR("Error in calculating endianness\r\n"));
			break;

		case 10:
			nrk_kprintf(PSTR("No receive buffers available\r\n"));
			break;

		case 11:
			nrk_kprintf(PSTR("No transmit buffers available\r\n"));
			break;

		case 12:
			nrk_kprintf(PSTR("Socket timeout\r\n"));
			break;
		
		case 13:
			nrk_kprintf(PSTR("Unmapped socket\r\n"));
			break;

		case 14:
			nrk_kprintf(PSTR("No port element available in port array\r\n"));
			break;

		case 15:
			nrk_kprintf(PSTR("No rbm element available in rbm array\r\n"));
			break;

		default:
			nrk_kprintf(PSTR("Unknown error number passed\r\n"));
			
	} // end switch
	
	return;
}		
		
