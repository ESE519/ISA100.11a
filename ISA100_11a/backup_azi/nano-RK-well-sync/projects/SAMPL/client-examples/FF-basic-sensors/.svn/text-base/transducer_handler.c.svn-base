#include <globals.h>
#include <nrk.h>
#include <sampl.h>
#include <transducer_handler.h>
#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <ff_basic_sensor.h>
#include "../../transducer_pkts/ff_basic_sensor_pkt.h"
#include "../../transducer_pkts/transducer_registry.h"
#include <debug.h>

#define TEMPERATURE_OFFSET  400

uint16_t adxl_prev_x;
uint16_t adxl_prev_y;
uint16_t adxl_prev_z;
uint16_t max_delta;
uint16_t tmp;
uint16_t buf;

int8_t transducer_handler(uint8_t key, uint8_t value, TRANSDUCER_REPLY_PKT_T *pkt)
{
int8_t fd,val,i;
pkt->type=TRAN_NCK;
pkt->len=0;
if(key==TRAN_FF_BASIC_SHORT)
	{
	FF_SENSOR_SHORT_PKT_T s;
  	debug_stats.sensor_samples++;
  	// Open ADC device as read 
  	fd=nrk_open(FIREFLY_SENSOR_BASIC,READ);
  	if(fd==NRK_ERROR) nrk_kprintf(PSTR("Failed to open sensor driver\r\n"));
  	val=nrk_set_status(fd,SENSOR_SELECT,BAT);
  	val=nrk_read(fd,&tmp,2);
  	s.battery=(uint8_t)(tmp-100);  // subtract 1 volt to fit in 8 bits

  	// Wait for Audio sensor to stabalize
  	nrk_wait_ticks(100);
  	val=nrk_set_status(fd,SENSOR_SELECT,LIGHT);
  	val=nrk_read(fd,&(s.light),1);

  	val=nrk_set_status(fd,SENSOR_SELECT,TEMP);
  	val=nrk_read(fd,&tmp,2);
  	if(tmp<TEMPERATURE_OFFSET) tmp=0;
  	else tmp-=TEMPERATURE_OFFSET;
  	// subtract offset and divide by 2
  	tmp=tmp>>1;
  	if(tmp>255) tmp=255;
  	s.temperature=tmp;
  	max_delta=0;
 
  	val=nrk_set_status(fd,SENSOR_SELECT,ACC_X);
  	val=nrk_read(fd,&buf,2);
  	if(buf> adxl_prev_x )
		tmp=buf-adxl_prev_x;	
  	else tmp=adxl_prev_x-buf;
  		adxl_prev_x=buf;
  	if(tmp>max_delta) max_delta=tmp;

  	val=nrk_set_status(fd,SENSOR_SELECT,ACC_Y);
  	val=nrk_read(fd,&buf,2);
  	if(buf> adxl_prev_y )
		tmp=buf-adxl_prev_y;	
  	else tmp=adxl_prev_y-buf;
  		adxl_prev_y=buf;
  	if(tmp>max_delta) max_delta=tmp;

  	val=nrk_set_status(fd,SENSOR_SELECT,ACC_Z);
  	val=nrk_read(fd,&buf,2);
  	if(buf> adxl_prev_z )
		tmp=buf-adxl_prev_z;	
  	else tmp=adxl_prev_z-buf;
  	adxl_prev_z=buf;
  	if(tmp>max_delta) max_delta=tmp;
  	if(max_delta>255)
    		s.acceleration=255;
  	else
    		s.acceleration=(uint8_t)(max_delta & 0xFF);

  	val=nrk_set_status(fd,SENSOR_SELECT,AUDIO_P2P);
  	val=nrk_read(fd,&tmp,2);
  	if(tmp>255) tmp=255;
  	s.sound_level=tmp;
	nrk_close(fd);

  	// Pack up the data 
	ff_basic_sensor_short_pack( pkt, &s );

	}
else if(key==TRAN_FF_BASIC_LONG)
	{
	FF_SENSOR_LONG_PKT_T s;

  	debug_stats.sensor_samples++;
  	// Open ADC device as read 
  	fd=nrk_open(FIREFLY_SENSOR_BASIC,READ);
  	if(fd==NRK_ERROR) nrk_kprintf(PSTR("Failed to open sensor driver\r\n"));
  	val=nrk_set_status(fd,SENSOR_SELECT,BAT);
  	val=nrk_read(fd,&(s.battery),2);
  	
	// Wait for Audio sensor to stabalize
  	nrk_wait_ticks(100);
  	val=nrk_set_status(fd,SENSOR_SELECT,AUDIO_P2P);
  	val=nrk_read(fd,&(s.sound_level),2);
  	val=nrk_set_status(fd,SENSOR_SELECT,LIGHT);
  	val=nrk_read(fd,&(s.light),2);
  	val=nrk_set_status(fd,SENSOR_SELECT,TEMP);
  	val=nrk_read(fd,&(s.temperature),2);
  	val=nrk_set_status(fd,SENSOR_SELECT,ACC_X);
  	val=nrk_read(fd,&(s.adxl_x),2);
  	val=nrk_set_status(fd,SENSOR_SELECT,ACC_Y);
  	val=nrk_read(fd,&(s.adxl_y),2);
  	val=nrk_set_status(fd,SENSOR_SELECT,ACC_Z);
  	val=nrk_read(fd,&(s.adxl_z),2);
  	nrk_close(fd);

  	// Pack up the data 
	pkt->type=TRAN_FF_BASIC_LONG;
	pkt->payload[0]=s.battery>>8;
	pkt->payload[1]=s.battery&0xff;
	pkt->payload[2]=s.light>>8;
	pkt->payload[3]=s.light&0xff;
	pkt->payload[4]=s.temperature>>8;
	pkt->payload[5]=s.temperature&0xff;
	pkt->payload[6]=s.adxl_x>>8;
	pkt->payload[7]=s.adxl_x&0xff8;
	pkt->payload[8]=s.adxl_y>>8;
	pkt->payload[9]=s.adxl_y&0xff8;
	pkt->payload[10]=s.adxl_z>>8;
	pkt->payload[11]=s.adxl_z&0xff8;
	pkt->payload[12]=s.sound_level>>8;
	pkt->payload[13]=s.sound_level&0xff;
	pkt->len=sizeof(FF_SENSOR_LONG_PKT_T);

	}
else if(key==TRAN_RAW_ADC)
	{



	}



return 0;
}
