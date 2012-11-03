#include <stdio.h>
#include <stdlib.h>
#include <sensor_cal.h>
#include <math.h>


void cal_load_params(char *file_name)
{
int i,v;
FILE *fp;
char buf[1024];
uint32_t t_mac;
float t_temp;
int t_light;

cal_elements=0;
for(i=0; i<MAX_CAL_SIZE; i++ ) cal_params[i].mac=0;
fp=fopen( file_name, "r" );
if( fp==NULL ) {
	printf( "Could not open calibration file: %s",file_name );
	return;
}

do {
v=fscanf( fp, "%[^\n]\n", buf);
if(v!=-1 && buf[0]!='#')
  {
	v=sscanf(buf,"%x %f %d",&t_mac,&t_temp, &t_light);
	if(v==3) {
		cal_params[cal_elements].mac=t_mac;
		cal_params[cal_elements].temp_offset=t_temp;
		cal_params[cal_elements].light_offset=(int16_t)t_light;
	printf( "cal mac 0x%08x temp=%f light=%d\n",cal_params[cal_elements].mac,
		cal_params[cal_elements].temp_offset,
		cal_params[cal_elements].light_offset );
		if(cal_elements<MAX_CAL_SIZE ) cal_elements++;
	}
  }
} while(v!=-1);


}

float cal_get_temp_offset( uint32_t mac )
{
int i;
for(i=0; i<MAX_CAL_SIZE; i++ )
	if(cal_params[i].mac==mac ) return cal_params[i].temp_offset;
return 0;
}

int16_t cal_get_light_offset( uint32_t mac )
{
int i;
for(i=0; i<MAX_CAL_SIZE; i++ )
	if(cal_params[i].mac==mac ) return cal_params[i].light_offset;
return 0;
}

float cal_get_temp(uint16_t adc_in)
{
float t;
float vo;

if(adc_in==0) adc_in=1;
vo=((float)adc_in/1023.0)*2.56;  
// 2.56 is fixed due to internal vref on atmega1281
t=2196200.0+(((1.8639-vo)*100000)/(.388));
//printf( "adc_in=%d voltage=%f vo=%f t=%f\n",adc_in, voltage, vo,t );
t=-1481.96+sqrt(t);
return t;
}

uint16_t cal_get_light(uint16_t adc_in, float voltage)
{
float vo;
if( voltage>3.3 ) voltage=3.3;
if(adc_in==0) adc_in=1;
vo=((float)adc_in/255.0)*voltage;
vo=vo*(255.0/3.3);
return ((uint16_t)vo);
}
