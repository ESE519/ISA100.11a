/******************************************************************************
*  Nano-RK, a real-time operating system for sensor networks.
*  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
*  All rights reserved.
*
*  This is the Open Source Version of Nano-RK included as part of a Dual
*  Licensing Model. If you are unsure which license to use please refer to:
*  http://www.nanork.org/nano-RK/wiki/Licensing
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, version 2.0 of the License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Contributing Authors (specific to this file):
*  Zane Starr
*  Anthony Rowe
*  Mark Hamilton
*******************************************************************************/


#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <adc_driver.h>
#include <include.h>
#include <stdio.h>
#include <ulib.h>
#include <nrk_error.h>
#include <nrk.h>
#include <stdint.h>

#define ADC_SETUP_DELAY  500

uint8_t adc_channel;

#define ADC_INIT() \
  do { \
    ADC12CTL1 = 0; \
    ADC12MCTL0 = SREF_1; \
  } while (0)

#define ADC_SET_CHANNEL(channel) \
	do { \
		ADC12MCTL0 = (ADC12MCTL0 & 0xf0) | ((channel) & 0x0f); \
	} while (0)

#define ADC_ENABLE() \
	do { \
  	ADC12CTL0 = ADC12ON|REFON|REF2_5V; \
	} while (0)

#define ADC_DISABLE() \
	do { \
  	ADC12CTL0 &= ~ENC; \
  	ADC12CTL0 = 0; \
	} while (0)

#define ADC_SAMPLE_SINGLE() \
	do { \
		ADC12CTL0 |= ADC12SC + ENC; \
		delay(); \
		ADC12CTL0 &= ~ADC12SC; \
		while (ADC12CTL1 & ADC12BUSY); \
		ADC12CTL0 &= ~ENC; \
	} while(0)

// Macros for obtaining the latest sample value
#define ADC_GET_SAMPLE_12(x) \
	do { \
		(x) = ADC12MEM0; \
	} while (0)

uint8_t dev_manager_adc(uint8_t action,uint8_t opt,uint8_t *buffer,uint8_t size)
{
  uint8_t count=0;
  // key and value get passed as opt and size
  uint8_t key=opt;
  uint8_t value=size;
  uint16_t val;

  switch(action)
  {
    case INIT: 
      init_adc();  
      return NRK_OK;

    case OPEN:   
      if(opt&READ_FLAG)
      {
        return NRK_OK; 
      }
      if(opt&WRITE_FLAG)
      {
        return NRK_ERROR; 
      }
      if(opt&APPEND_FLAG)
      {
        return NRK_ERROR; 
      }
      if(opt&(READ_FLAG|WRITE_FLAG|APPEND_FLAG)==0)
        return NRK_ERROR;
      else return NRK_OK;

    case READ:
      /* Conversion to 8-bit value*/
      val=get_adc_val();
      buffer[count]=val & 0xFF;
      count++;
      buffer[count]=(val>>8)  & 0xFF;
      count++;
      return count;

    case CLOSE:
      return NRK_OK;

    case GET_STATUS:
      // use "key" here 
      if(key==ADC_CHAN) return adc_channel;
      return NRK_ERROR;

    case SET_STATUS:
      // use "key" and "value" here
      if(key==ADC_CHAN) 
      {
        adc_channel=value;
        ADC_SET_CHANNEL (adc_channel);
        return NRK_OK;
      }
      return NRK_ERROR;
    default:
      nrk_kernel_error_add(NRK_DEVICE_DRIVER,0);
      return 0;
  }
}

void init_adc()
{
  ADC_INIT ();
	adc_channel = 0;
  ADC_SET_CHANNEL(adc_channel);
  ADC_ENABLE ();
}

uint16_t get_adc_val()
{                         
	uint16_t adc_val;
	ADC_SAMPLE_SINGLE();
	delay();
	ADC_GET_SAMPLE_12(adc_val);
	return adc_val;
}
void delay()
{
  nrk_spin_wait_us(ADC_SETUP_DELAY);
}
