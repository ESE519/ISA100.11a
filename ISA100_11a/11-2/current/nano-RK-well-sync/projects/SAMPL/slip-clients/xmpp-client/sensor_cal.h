#ifndef _SENSOR_CAL_H_
#define _SENSOR_CAL_H_

#define MAX_CAL_SIZE	64

#include <stdint.h>


typedef struct cal_struct
{

   uint32_t mac;
   float temp_offset;
   int16_t light_offset;


} cal_struct_t;

int cal_elements;
cal_struct_t cal_params[MAX_CAL_SIZE];

void cal_load_params(char *file_name);
float cal_get_temp(uint16_t adc_in );
uint16_t cal_get_light(uint16_t adc_in, float voltage);
float cal_get_temp_offset( uint32_t mac );
int16_t cal_get_light_offset( uint32_t mac );


#endif
