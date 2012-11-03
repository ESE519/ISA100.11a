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
		