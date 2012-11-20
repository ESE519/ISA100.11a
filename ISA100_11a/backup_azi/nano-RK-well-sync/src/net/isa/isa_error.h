#ifndef _ISA_ERROR_H
#define _ISA_ERROR_H

#include <include.h>
#include <stdio.h>

#define LINK_CAPACITY_ERROR 1
#define NEIGHBOR_CAPACITY_ERROR 2
#define TRANSMIT_QUEUE_CAPACITY_ERROR 3
#define  MAX_PAYLOAD_ERROR 4
//********************************Extern functions*******************************************
extern void setIsaError(uint8_t);
extern uint8_t getIsaError ();
extern void printIsaError();
//*******************************************************************************************

#endif
