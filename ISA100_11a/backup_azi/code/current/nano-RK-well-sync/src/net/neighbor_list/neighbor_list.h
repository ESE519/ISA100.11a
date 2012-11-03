#ifndef _NEIGHBOR_LIST_H_
#define _NEIGHBOR_LIST_H_

#include <nrk.h>

#ifndef NEIGHBOR_LIST_SIZE
// MAX Table size is 127
#define NEIGHBOR_LIST_SIZE   5
#endif

#define VALID_MASK  		0x01


typedef struct neighbor_type {
uint16_t mac;
uint16_t ttl; 
int8_t rssi; 
int8_t flags; 
} neighbor_list_t;

neighbor_list_t neighbor_table[NEIGHBOR_LIST_SIZE];

int8_t neighbor_list_init();
void neighbor_list_ttl_update();
int8_t neighbor_list_add( uint16_t mac, int8_t rssi, uint16_t ttl);

// Use these functions to help do neighbor list tabulation
int8_t neighbor_list_get_next_valid_index(uint8_t start_index);
uint16_t neighbor_list_get_mac(int8_t index);
int8_t neighbor_list_get_rssi(int8_t index);
uint16_t neighbor_list_get_ttl(int8_t index);

#endif
