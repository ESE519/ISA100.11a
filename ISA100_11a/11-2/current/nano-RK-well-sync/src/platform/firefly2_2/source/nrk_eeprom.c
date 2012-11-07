#include <nrk_eeprom.h>
#include <avr/eeprom.h>
#include <nrk_error.h>

uint8_t nrk_eeprom_read_byte( uint16_t addr )
{
uint8_t v;
v=eeprom_read_byte((uint8_t*)addr);
return v;
}

int8_t nrk_eeprom_write_byte( uint16_t addr, uint8_t value )
{
eeprom_write_byte( (uint8_t*)addr, value );
}

int8_t read_eeprom_mac_address(uint32_t *mac_addr)
{
uint8_t checksum,ct;
uint8_t *buf;
buf=(uint8_t *)mac_addr;
checksum=buf[0]+buf[1]+buf[2]+buf[3];
buf[3]=eeprom_read_byte ((uint8_t*)EE_MAC_ADDR_0);
buf[2]=eeprom_read_byte ((uint8_t*)EE_MAC_ADDR_1);
buf[1]=eeprom_read_byte ((uint8_t*)EE_MAC_ADDR_2);
buf[0]=eeprom_read_byte ((uint8_t*)EE_MAC_ADDR_3);
checksum=eeprom_read_byte ((uint8_t*)EE_MAC_ADDR_CHKSUM);
ct=buf[0];
ct+=buf[1];
ct+=buf[2];
ct+=buf[3];
if(checksum==ct) return NRK_OK;

return NRK_ERROR;
}

int8_t read_eeprom_channel(uint8_t *channel)
{
  *channel=eeprom_read_byte ((uint8_t*)EE_CHANNEL);
return NRK_OK;
}

int8_t write_eeprom_load_img_pages(uint8_t *load_pages)
{
  eeprom_write_byte ((uint8_t*)EE_LOAD_IMG_PAGES, *load_pages);
  return NRK_OK;
}

int8_t read_eeprom_load_img_pages(uint8_t *load_pages)
{
  *load_pages=eeprom_read_byte ((uint8_t*)EE_LOAD_IMG_PAGES);
  return NRK_OK;
}

int8_t read_eeprom_aes_key(uint8_t *aes_key)
{
uint8_t i;
for(i=0; i<16; i++ )
  aes_key[i]=eeprom_read_byte ((uint8_t*)(EE_AES_KEY+i));
  return NRK_OK;
}

int8_t write_eeprom_aes_key(uint8_t *aes_key)
{
uint8_t i;
for(i=0; i<16; i++ )
  eeprom_write_byte ((uint8_t*)(EE_AES_KEY+i),aes_key[i]);
  return NRK_OK;
}

int8_t read_eeprom_current_image_checksum(uint8_t *image_checksum)
{
  *image_checksum=eeprom_read_byte ((uint8_t*)EE_CURRENT_IMAGE_CHECKSUM);
  return NRK_OK;
}

int8_t write_eeprom_current_image_checksum(uint8_t *image_checksum)
{
  eeprom_write_byte ((uint8_t*)EE_CURRENT_IMAGE_CHECKSUM, *image_checksum);
  return NRK_OK;
}


