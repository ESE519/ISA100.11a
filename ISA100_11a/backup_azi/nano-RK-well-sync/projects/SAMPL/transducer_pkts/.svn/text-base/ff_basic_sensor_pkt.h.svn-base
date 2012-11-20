#include <globals.h>
#include <sampl.h>
#include "transducer_registry.h"
#include <transducer_pkt.h>

typedef struct ff_sensor_short_pkt 
{
	uint8_t battery;        // Byte 2
	uint8_t light;          // Byte 3
	uint8_t temperature;    // Byte 4
	uint8_t acceleration;   // Byte 5
	uint8_t sound_level;   // Byte 6
} FF_SENSOR_SHORT_PKT_T;

typedef struct ff_sensor_long_pkt 
{
	uint16_t battery;        // Byte 2,3
	uint16_t light;          // Byte 4,5
	uint16_t temperature;    // Byte 6,7
	uint16_t mic;   	 // Byte 8,9
	uint16_t adxl_x;   	 // Byte 10,11
	uint16_t adxl_y;   	 // Byte 12,13
	uint16_t adxl_z;  	 // Byte 14,15
	uint16_t sound_level;  	 // Byte 16,17
} FF_SENSOR_LONG_PKT_T;



uint8_t ff_basic_sensor_short_pack( TRANSDUCER_REPLY_PKT_T *t, FF_SENSOR_SHORT_PKT_T *s );
uint8_t ff_basic_sensor_short_unpack( TRANSDUCER_REPLY_PKT_T *t, FF_SENSOR_SHORT_PKT_T *s );
