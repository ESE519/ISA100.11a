#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <nrk.h>


uint8_t my_subnet_mac[3];
uint8_t my_mac;
uint32_t mac_address;
uint8_t admin_debug_flag;
int8_t mobile_reserve;
uint16_t neighborlist_ttl;


uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t p2p_buf[RF_MAX_PAYLOAD_SIZE];

#ifdef PHOENIX
// flash page buffer
uint8_t ph_buf[256];
#endif

uint8_t reply_buf_len;
uint8_t tx_buf_len;
uint8_t rx_buf_len;
uint8_t p2p_buf_len;



#endif
