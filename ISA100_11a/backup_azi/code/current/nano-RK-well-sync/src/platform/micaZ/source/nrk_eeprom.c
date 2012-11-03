#include <nrk_eeprom.h>
#include <avr/eeprom.h>
#include <nrk_error.h>


int8_t read_eeprom_mac_address(uint32_t *address)
{
uint8_t checksum,ct;
uint8_t *buf;
buf=address;
checksum=buf[0]+buf[1]+buf[2]+buf[3];
buf[3]=eeprom_read_byte ((uint8_t*)0);
buf[2]=eeprom_read_byte ((uint8_t*)1);
buf[1]=eeprom_read_byte ((uint8_t*)2);
buf[0]=eeprom_read_byte ((uint8_t*)3);
checksum=eeprom_read_byte ((uint8_t*)4);
ct=buf[0];
ct+=buf[1];
ct+=buf[2];
ct+=buf[3];
if(checksum==ct) return NRK_OK;

return NRK_ERROR;
}


