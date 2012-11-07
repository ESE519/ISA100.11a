#ifndef _ROUTE_TABLE_H_
#define _ROUTE_TABLE_H_

#include <nrk.h>

#ifndef ROUTE_TABLE_SIZE
// MAX Table size is 127
#define ROUTE_TABLE_SIZE   5
#endif

#ifndef ROUTE_TABLE_VALUES
// MAX Value size is 255
#define ROUTE_TABLE_VALUES	   1
#endif

#define VALID_MASK  		0x01
#define EXTENDED_ADDRESS_MASK   0x02


typedef struct route_type {
uint16_t dst;
uint16_t next_hop;
uint16_t ttl; 
int8_t flags; 
int8_t value[ROUTE_TABLE_VALUES]; 
} route_table_t;

route_table_t route_table[ROUTE_TABLE_SIZE];

int8_t route_table_init();
int8_t route_table_set( uint16_t dst, uint16_t next_hop, uint16_t ttl);
int8_t route_table_clr( uint16_t dst);
uint16_t route_table_get( uint16_t dst);
// Route Find returns 0xFFFF if no route exists
void _route_table_ttl_update();
int8_t route_table_value_set(uint16_t dst, uint8_t value_index, int8_t value);
int8_t route_table_value_get(uint16_t dst, uint8_t value_index);

// Use these functions to help do neighbor list tabulation
int8_t route_table_get_next_valid_index(uint8_t start_index);
uint16_t route_table_get_dst_by_index(int8_t index);

#endif
