#ifndef _NRK_EEPROM_H_
#define _NRK_EEPROM_H_
#include <stdint.h>

// EEPROM Address List
#define EE_MAC_ADDR_0 		    0
#define EE_MAC_ADDR_1 		    1
#define EE_MAC_ADDR_2 		    2
#define EE_MAC_ADDR_3 		    3
#define EE_MAC_ADDR_CHKSUM 	    4
#define EE_CHANNEL		    5
#define EE_LOAD_IMG_PAGES           6
#define EE_CURRENT_IMAGE_CHECKSUM   7
#define EE_AES_KEY		    8

int8_t read_eeprom_load_img_pages(uint8_t *load_pages);
int8_t write_eeprom_load_img_pages(uint8_t *load_pages);
int8_t read_eeprom_aes_key(uint8_t *aes_key);
int8_t write_eeprom_aes_key(uint8_t *aes_key);
int8_t read_eeprom_mac_address(uint32_t *mac_addr);
int8_t read_eeprom_channel(uint8_t *chan);
uint8_t nrk_eeprom_read_byte( uint16_t addr );
int8_t nrk_eeprom_write_byte( uint16_t addr, uint8_t value );

#endif
