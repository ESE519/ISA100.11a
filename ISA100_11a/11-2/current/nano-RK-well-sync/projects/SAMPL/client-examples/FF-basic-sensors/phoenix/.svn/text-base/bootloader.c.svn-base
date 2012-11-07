#include <include.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <nrk_timer.h>
#include <avr/wdt.h>
#include "flash.h"
#include <avr/eeprom.h>
#include "bootloader.h"

/*
void ws_flash_write_page(uint32_t address, uint8_t *buf)
    BOOTLOADER_SECTION_ws_flash_write_page;
void ws_flash_read_page(uint32_t addr, uint8_t *buf)
    BOOTLOADER_SECTION_ws_flash_read_page;
void ws_flash_read_byte(uint32_t addr, uint8_t *buf)
    BOOTLOADER_SECTION_ws_flash_read_byte;
uint8_t copy_section(uint32_t src_start_addr, uint32_t dest_start_addr, uint8_t num_pages, uint8_t *temp_buf)
    BOOTLOADER_SECTION_copy_section;
void add_packet_to_page(uint8_t *packet, uint8_t offset, uint8_t *fill_page)
    BOOTLOADER_SECTION_add_packet_to_page;
uint8_t commit_page(uint32_t page_start_address, uint8_t *fill_page)
    BOOTLOADER_SECTION_commit_page;
*/

void ws_flash_write_page(uint32_t address, uint8_t *buf) BOOTLOADER_SECTION;
void ws_flash_read_page(uint32_t addr, uint8_t *buf) BOOTLOADER_SECTION;
void ws_flash_read_byte(uint32_t addr, uint8_t *buf) BOOTLOADER_SECTION;
uint8_t copy_section(uint32_t src_start_addr, uint32_t dest_start_addr, uint8_t num_pages, uint8_t *temp_buf) BOOTLOADER_SECTION;
void clear_page() BOOTLOADER_SECTION;
void add_packet_to_page(uint8_t *packet, uint8_t offset, uint8_t *fill_page) BOOTLOADER_SECTION;
uint8_t commit_page(uint32_t page_start_address, uint8_t *fill_page) BOOTLOADER_SECTION;


void ws_flash_read_page(uint32_t addr, uint8_t *buf)
{
  uint8_t sreg;
  uint32_t i;

  sreg = SREG;
  cli();
  eeprom_busy_wait();

  for (i=0;i<PAGESIZE;i++)
  {
    _WAIT_FOR_SPM();
    eeprom_busy_wait();
    boot_spm_busy_wait();
    _ENABLE_RWW_SECTION();
    buf[i] = pgm_read_byte_far(addr+i);
  }

  boot_rww_enable();
  SREG = sreg;
  return;
}

void ws_flash_read_byte(uint32_t addr, uint8_t *buf)
{
  uint8_t sreg;

  sreg = SREG;
  cli();
  eeprom_busy_wait();
  _WAIT_FOR_SPM();
  eeprom_busy_wait();
  boot_spm_busy_wait();
  _ENABLE_RWW_SECTION();
  *buf = pgm_read_byte_far(addr);

  boot_rww_enable();
  SREG = sreg;
  return;
}

void ws_flash_write_page(uint32_t address, uint8_t *buf)
{
  uint32_t i;
  uint8_t sreg;
  uint8_t *buffer;

  buffer = buf;

  // Disable interrupts
  sreg = SREG;
  cli();

  // Start by erasing the page at given address
  boot_page_erase_safe (address);
  boot_spm_busy_wait ();	// Wait until the memory is erased

  for(i=0; i<PAGESIZE; i+=2){
    uint16_t w = *buffer++;
    w += (*buffer++) << 8;
    boot_page_fill_safe(address+i, w);
  }
  boot_page_write_safe (address);
  boot_spm_busy_wait ();

  while(boot_rww_busy()){ boot_rww_enable_safe();}

  boot_rww_enable();
  SREG = sreg;
}

uint8_t copy_section(uint32_t src_start_addr, uint32_t dest_start_addr, uint8_t num_pages, uint8_t *temp_buf)
{

  uint32_t i, j;
  uint32_t curr_src_address, curr_dest_address;

  curr_src_address = src_start_addr;
  curr_dest_address = dest_start_addr;
  if (((src_start_addr & 0x000FF) > 0)  ||((dest_start_addr & 0x000FF) > 0) )
  {
    //printf("PAGE_ALIGNMENT ERROR !!\r\n");
    return 1 ; //not page aligned
  }

  //clear interrupt
  cli();

  for(i=0;i<num_pages * PAGESIZE; i+=PAGESIZE)
  {
    for(j=0; j<PAGESIZE; j++) temp_buf[j] = 0;

    //copy pages from source to destination
    ws_flash_read_page(curr_src_address, temp_buf);
    if(temp_buf[0] == 0xff) ws_flash_read_page(curr_src_address, temp_buf);
    ws_flash_write_page(curr_dest_address, temp_buf);
    curr_src_address += PAGESIZE;
    curr_dest_address += PAGESIZE;
  }

  wdt_enable(20);

  while(1);

  return 0;

}

void add_packet_to_page(uint8_t *packet, uint8_t offset, uint8_t *fill_page)
{
  uint8_t i;
  for(i = 0;i < DATA_PAYLOAD; i++)
    fill_page[offset+i] = packet[i];
}

uint8_t commit_page(uint32_t page_start_address, uint8_t *fill_page)
{
  uint16_t i;
  if ((page_start_address & 0x000FF) > 0 )
  {
    //printf("PAGE_ALIGNMENT ERROR !!\r\n");
    return -1 ; //not page aligned
  }
  ws_flash_write_page(page_start_address, fill_page);
  return 0;
}

