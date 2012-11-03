/* This file contains the data structures and function prototypes required for the 
   generation of the sensor network topology
*/

#ifndef _TOPOLOGY_GENERATION_H
#define _TOPOLOGY_GENERATION_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/************************************* CONSTANTS *************************************/
#define ADDR_LENGTH 5					 // length of a node address formatted as a character string 
#define DEBUG_TG 0 



/********************************** FUNCTION PROTOTYPES ******************************/
void toString(char* str, uint16_t addr);
void add_link_to_topology_file(FILE *fp, uint16_t f, uint16_t t, int8_t rssi);
void begin_topology_file(FILE *fp);
void end_topology_file(FILE *fp);
void generate_TopGraph();

#endif 
