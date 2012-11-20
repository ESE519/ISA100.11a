/* readflash.h */

#ifndef READFLASH_H
#define READFLASH_H

#define LOAD_SECTION    0x00000
#define UPDATE_SECTION	0x0A000L
#define PAGESIZE 	256L 
#define FLASH_END	0x1F000L

extern void ws_flash_read_page(uint32_t addr, uint8_t *buf);
extern void ws_flash_read_byte(uint32_t addr, uint8_t *buf);

#endif
