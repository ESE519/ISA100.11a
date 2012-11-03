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
*******************************************************************************/

#include <nrk_driver_list.h>
#include <dev_adc.h>
#include <include.h>
#include <stdio.h>
#include <ulib.h>
#include <nrk_error.h>

uint8_t dev_manager_adc(uint8_t state,uint8_t opt,uint8_t *buffer,uint8_t size)
{
     uint8_t count=0;

     switch(state)
     {
             case INIT_STATE: 
	     	init_adc();  
		      return 1;
	     
	    case OPEN_STATE:   {
	    	switch(opt){
                    case   READ_STATE: 
				//sensors_on();
				return 1;
		    case    WRITE_STATE:
				//sensors_on();
				return 2;
		    case    RW_STATE:
				return 3;
		     default:
				//printf("option for device does not exist\n");
				nrk_kernel_error_add(NRK_DEVICE_DRIVER,0);
				return -1;
		}
	    }
             case READ_STATE:
	     	      for(count=0;count<size;count++)
		      {                   
			      /* Conversion to 8-bit value*/
			      uint16_t val=get_adc_val();
			      buffer[count]=val >>2 & 0xFF;
                      }
                      return count;
             case CLOSE_STATE:
	     		//sensors_off();
                        return 1;
			/*SET STATUS AND GET STATUS CALLS DEFINED HERE*/
             case GET_ADC_STATE:
	     		return 1;
			
	     default:
		nrk_kernel_error_add(NRK_DEVICE_DRIVER,0);
		 return 0;
	}
}

void init_adc()
{
// Initialize values here
  DDRA = 0x80;
  ADC_INIT ();
  ADC_ENABLE ();
  ADC_SET_CHANNEL (0);
}

uint16_t get_adc_val()
{                         
	uint16_t adc_val;
	ADC_SAMPLE_SINGLE();
	delay();
	ADC_GET_SAMPLE_10(adc_val);
	return adc_val;
}
void delay()
{
	volatile uint8_t x,a;
	for(x=0;x<100;x++)
		a=x;
}
