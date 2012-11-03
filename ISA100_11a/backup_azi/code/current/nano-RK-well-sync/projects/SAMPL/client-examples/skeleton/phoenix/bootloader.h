/* bootloader.h */
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#define BOOTLOADER_SECTION_ws_flash_write_page	__attribute__ ((section (".bootloader_ws_flash_write_page")))
#define BOOTLOADER_SECTION_ws_flash_read_page   __attribute__ ((section (".bootloader_ws_flash_read_page")))
#define BOOTLOADER_SECTION_ws_flash_read_byte   __attribute__ ((section (".bootloader_ws_flash_read_byte")))
#define BOOTLOADER_SECTION_copy_section   	__attribute__ ((section (".bootloader_copy_section")))
#define BOOTLOADER_SECTION_add_packet_to_page   __attribute__ ((section (".bootloader_add_packet_to_page")))
#define BOOTLOADER_SECTION_commit_page   	__attribute__ ((section (".bootloader_commit_page")))

// Flash Addresses
#define LOAD_SECTION    0x00000L
#define UPDATE_SECTION	0xC000L
#define PAGESIZE 	256L
#define FLASH_END	0x1F000L
#define SCRATCH_SECTION	0x12000L

#define DATA_PAYLOAD    64

extern void ws_flash_write_page(uint32_t address, uint8_t *buf);
extern void ws_flash_read_page(uint32_t addr, uint8_t *buf);
extern void ws_flash_read_byte(uint32_t addr, uint8_t *buf);
extern uint8_t copy_section(uint32_t src_start_addr, uint32_t dest_start_addr, uint8_t num_pages, uint8_t *temp_buf);
extern void add_packet_to_page(uint8_t *packet, uint8_t offset, uint8_t *fill_page);
extern uint8_t commit_page(uint32_t page_start_address, uint8_t *fill_page);

#endif
