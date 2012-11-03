#include <nrk.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <nrk_eeprom.h>
#include <avr/wdt.h>
#include "bootloader.h"
#include "globals.h"

#define DELETE		0
#define INSERT		1
#define SUBSTITUTE	2

#define NEWFILE		0
#define OLDFILE		1
#define PATCHFILE	2

#define START_OF_GROUP 	1
#define IN_GROUP 	2
#define NO_GROUP 	3

#define SHORT_BYTE 	4
#define LONG_BYTE	3


static int16_t i;
static int16_t j; 
static uint16_t offset;
static int16_t pat = 0;
static int16_t plen = 0;
static uint16_t newfile_page_size;
static uint8_t pagepos = 0;

struct
{
  uint16_t addr;
  uint8_t cmd;
  uint8_t data;
}
patchEntry;

struct FilePointers
{
  uint32_t read_addr;
  uint32_t filepos;
}
filePointer[3];


inline int8_t decode_patch(void);
inline void ffread(uint8_t *data, uint8_t fileid);
inline void ffwrite_newfile(uint8_t *data);
inline void ffseek(uint16_t infilepos, uint8_t fileid);

void NanoPatch(uint16_t ee_update_section_byte_size, uint16_t ee_load_section_byte_size, uint8_t newChecksum);

/*$PAGE*/
/*
**********************************************************************
*                           PATCH SECTION
* 
**********************************************************************
*/

void NanoPatch(uint16_t ee_update_section_byte_size, uint16_t ee_load_section_byte_size, uint8_t newChecksum)
{

  uint8_t data;

  uint16_t old=0, pat_wait=0;

  offset = 0;

  patchEntry.addr = 255;
  patchEntry.cmd = 0;
  patchEntry.data = '0';

  filePointer[NEWFILE].filepos = 0;
  filePointer[OLDFILE].filepos = 0;
  filePointer[PATCHFILE].filepos = 0;

  plen = ee_update_section_byte_size;

  cli();
  wdt_disable();

  ffseek(0, NEWFILE);
  ffseek(0, OLDFILE);
  ffseek(0, PATCHFILE);

  while((old < ee_load_section_byte_size) || (pat <= ee_update_section_byte_size))
  {
    // For boundary condition

    if((pat <= ee_update_section_byte_size) && (pat_wait == 0))
    {
      if(decode_patch())
        pat_wait = 1;
      else
        pat++;
    }
    
    if (patchEntry.addr == old && pat_wait == 1)
    {
      pat_wait = 0;
      switch(patchEntry.cmd)
      {
        case SUBSTITUTE	://printf("SUB %x\r\n", old);
        ffread(&data, OLDFILE);
        old++;
        ffwrite_newfile(&patchEntry.data);
        break;
        case INSERT	://printf("INS %x\r\n", old);
        ffwrite_newfile(&patchEntry.data);
        break;
        case DELETE	://printf("DEL %x\r\n", old);
        ffread(&data, OLDFILE);
        old++;
        break;
      }
      
    }
    else
    {
      if(old < ee_load_section_byte_size)
      {
        ffread(&data, OLDFILE);
        old++;
        ffwrite_newfile(&data);
      }	
    }

  }

  if(pagepos != 0)
  {
    commit_page(filePointer[NEWFILE].read_addr, ph_buf);
    filePointer[NEWFILE].read_addr += PAGESIZE;
  }

  newfile_page_size = filePointer[NEWFILE].filepos / 256;
  if((filePointer[NEWFILE].filepos % PAGESIZE) > 0) newfile_page_size ++;

  // Write the new image size in pages
  write_eeprom_load_img_pages(&newfile_page_size);

  // Write the new checksum
  write_eeprom_current_image_checksum(&newChecksum);

  nrk_kprintf(PSTR("NOW COPYING...\r\n"));
  printf("New Load Pages: %X\r\n", newfile_page_size);
  
  copy_section(SCRATCH_SECTION, LOAD_SECTION, newfile_page_size, ph_buf);

  while(1);

  return;
}



/*$PAGE*/
/*
**********************************************************************
*                      DECODE PATCH
*		reads and decodes from patch file
**********************************************************************
*/

inline int8_t decode_patch(void)
{
  uint8_t control_byte, addr_byte, grp_count;
  uint16_t addr_2byte;

  static uint8_t encode_mode = NO_GROUP;
  static uint8_t grp_cmd;
  static int group_mem=0;

  if((pat == plen) && (group_mem == 0)) return 0;

  if(encode_mode == NO_GROUP && (pat < plen))
  {
    ffread(&control_byte, PATCHFILE);
    pat++;

    if((uint8_t)(control_byte >> 4) == 0x0F)
    {
      encode_mode = START_OF_GROUP;
      control_byte = (uint8_t)(control_byte << 4);
    }
		
    patchEntry.cmd = (uint8_t)(control_byte >> 6);
		
    if (patchEntry.cmd == LONG_BYTE)
    {
      //addr offset in 2 bytes
      patchEntry.cmd  = (uint8_t)(control_byte << 2) >> 6;
      //endian independent

      if(encode_mode == START_OF_GROUP)
      {
        encode_mode = IN_GROUP;
        grp_cmd = patchEntry.cmd;
        ffread(&grp_count, PATCHFILE);
        pat++;
        group_mem = grp_count - 1;
      }
      
      ffread(&addr_byte, PATCHFILE);
      pat++;

      addr_2byte = (uint16_t)(addr_byte << 8);

      ffread(&addr_byte, PATCHFILE);
      pat++;

      addr_2byte |= (uint16_t)addr_byte;
      patchEntry.addr = addr_2byte + offset;

    }
    else
    {
      if(encode_mode == START_OF_GROUP)
      {
        encode_mode = IN_GROUP;
        grp_cmd = patchEntry.cmd;
        ffread(&grp_count, PATCHFILE);
        pat++;

        group_mem = grp_count - 1;
        ffread(&addr_byte, PATCHFILE);
        pat++;
        addr_2byte = (uint16_t)addr_byte;

      }
      else
      {
        //addr offset in 6 bits
        addr_2byte = (uint8_t)(control_byte << 2) >> 2;
      }
      patchEntry.addr = addr_2byte + offset;
    }
  }
  else if(encode_mode == IN_GROUP)
  {

    group_mem --;

    if(group_mem == 0)
      encode_mode = NO_GROUP;

    patchEntry.cmd = grp_cmd;

    switch(patchEntry.cmd)
    {
      case SUBSTITUTE:
        patchEntry.addr = offset + 1;
        break;
      case INSERT:
        patchEntry.addr = offset;
        break;
      case DELETE:
        patchEntry.addr = offset + 1;
        break;
    }

  }

  if(pat < plen){
    switch(patchEntry.cmd)
    {
      case SUBSTITUTE:
        ffread(&patchEntry.data, PATCHFILE);
        pat++;
        break;
      case INSERT:
        ffread(&patchEntry.data, PATCHFILE);
        pat++;
        break;
      case DELETE:
        break;
    }
  }

  offset = patchEntry.addr;

  return 1;
}

/*$PAGE*/
/*
**********************************************************************
*         FUNC to read/write/seek Flash Memory like a file
*	
**********************************************************************
*/


inline void ffread(uint8_t *data, uint8_t fileid)
{
  ws_flash_read_byte(filePointer[fileid].read_addr, data);

  filePointer[fileid].filepos++;
  filePointer[fileid].read_addr++;

  return;

}


inline void ffwrite_newfile(uint8_t *data)
{

  if(pagepos < PAGESIZE - 1)
  {
    ph_buf[pagepos] = *data;
    pagepos++;
  }
  else if(pagepos == PAGESIZE -1)
  {

    ph_buf[pagepos] = *data;

    commit_page(filePointer[NEWFILE].read_addr, ph_buf);
    filePointer[NEWFILE].read_addr += PAGESIZE;

    pagepos = 0;
  }
  else
  {
    nrk_kprintf(PSTR("PAGE BUFFER OUT OF BOUNDS\r\n"));
  }
  filePointer[NEWFILE].filepos++;
}



inline void ffseek(uint16_t infilepos, uint8_t fileid)
{
  switch(fileid)
  {
    case NEWFILE: filePointer[fileid].read_addr = (uint32_t)SCRATCH_SECTION + (uint32_t)infilepos;
      break;
    case OLDFILE: filePointer[fileid].read_addr = (uint32_t)LOAD_SECTION + (uint32_t)infilepos;
      break;
    case PATCHFILE: filePointer[fileid].read_addr = (uint32_t)UPDATE_SECTION + (uint32_t)infilepos;
      break;
    default:
      break;
  }

  return;
}
