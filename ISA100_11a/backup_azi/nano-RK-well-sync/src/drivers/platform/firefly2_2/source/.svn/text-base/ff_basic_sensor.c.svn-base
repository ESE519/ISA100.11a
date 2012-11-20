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
*  Anthony Rowe
*  Zane Starr
*******************************************************************************/


#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <ff_basic_sensor.h>
#include <include.h>
#include <stdio.h>
#include <ulib.h>
#include <nrk_error.h>
#include <nrk.h>
#include <stdint.h>
#include <basic_rf.h>
#include <nrk_timer.h>

#define ADC_STARTUP_DELAY  1000
#define ADC_SETUP_DELAY  200

uint8_t channel;
uint8_t is_open;

// VREF is set to VCC by default
#define ADC_INIT() \
    do { \
	ADCSRA = BM(ADPS0) | BM(ADPS1); \
	ADMUX = BM(REFS0);  \
} while (0)

#define ADC_VREF_VCC() \
   do { \
	ADMUX &= ~(BM(REFS1));  \
	ADMUX |= BM(REFS0);  \
} while(0)


#define ADC_VREF_1_1() \
   do { \
	ADMUX &= ~(BM(REFS0));  \
	ADMUX |= BM(REFS1);  \
} while(0)


#define ADC_VREF_2_56() \
   do { \
	ADMUX |= BM(REFS1) | BM(REFS0);  \
} while(0)

#define ADC_SET_CHANNEL(channel) do { ADMUX &= ~0x1F; ADMUX |= (ADMUX & ~0x1F) | (channel); } while (0)

// Enables/disables the ADC
#define ADC_ENABLE() do { ADCSRA |= BM(ADEN); } while (0)
#define ADC_DISABLE() do { ADCSRA &= ~BM(ADEN); } while (0)

#define ADC_SAMPLE_SINGLE() \
    do { \
ADCSRA |= BM(ADSC); \
while (!(ADCSRA & 0x10)); \
} while(0)

// Macros for obtaining the latest sample value
#define ADC_GET_SAMPLE_10(x) \
do { \
x =  ADCL; \
x |= ADCH << 8; \
} while (0)

#define ADC_GET_SAMPLE_8(x) \
do { \
x = ((uint8_t) ADCL) >> 2; \
x |= ((int8_t) ADCH) << 6; \
} while (0)

uint16_t read_voltage_status();
uint8_t dev_manager_ff_sensors(uint8_t action,uint8_t opt,uint8_t *buffer,uint8_t size)
{
uint8_t count=0;
// key and value get passed as opt and size
uint8_t key=opt;
uint8_t value=size;

     switch(action)
     {
            case INIT: 
			// Set the pwr ctrl pin as output
  			DDRF = PWR_CTRL_MASK;
			PORTF |= PWR_CTRL_MASK;
	     		init_adc();  
			is_open=0;
		      return 1;
	     
	    case OPEN:  
		    if(is_open==1) return NRK_ERROR;
		    is_open=1; 
		    if(opt&READ_FLAG)
		    {
		   	// Turn on Sensor Node Power
			PORTF &= ~(PWR_CTRL_MASK);
  			channel=0;
  			ADC_SET_CHANNEL (0);
			nrk_spin_wait_us(ADC_STARTUP_DELAY);
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
		    if(((opt)&(READ_FLAG|WRITE_FLAG|APPEND_FLAG))==0)
		    	return NRK_ERROR;
		    else return NRK_OK;
		
	    

             case READ:
	     	      count=0;
		      if(size!=1 && size!=2) return 0;
			if(channel!=BAT && channel<7)
			{
			      /* Conversion to 8-bit value*/
			      uint16_t val=get_adc_val();

				if(size==2)
			      	{
					buffer[count]=val  & 0xFF;
			      		count++;
			      		buffer[count]=(val>>8)  & 0xFF;
				}

				if(size==1)
			      	{
			      		buffer[count]=(val>>2)  & 0xFF;
				}


			} else if(channel==BAT)
			{
			uint16_t tmp;
			tmp=read_voltage_status();
			if(size==2)
				{
					buffer[count]=tmp & 0xFF;
					count++;
					buffer[count]=(tmp>>8) & 0xFF;
				}
			if(size==1)
				{
					buffer[count]=(tmp>>2) & 0xFF;
				}
			}else if(channel==AUDIO_P2P)
			{
  			/* Conversion to 8-bit value*/
			      //uint16_t val=get_adc_val();
			      uint16_t val,min, max;
			      uint8_t i;
				max=0;
			        min=1025;
			      for(i=0; i<64; i++ )
				{
				val=get_adc_val();
				if(val<min)min=val;
				if(val>max)max=val;
				// 8 Khz
				nrk_spin_wait_us(125);	
				}
			 	val=max-min;	
				if(size==2)
			      	{
					buffer[count]=val  & 0xFF;
			      		count++;
			      		buffer[count]=(val>>8)  & 0xFF;
				}

				if(size==1)
			      	{
			      		buffer[count]=(val>>2)  & 0xFF;
				}

			}
			
		      count++;
                      return count;

             case CLOSE:
			// Turn off sensor power
			PORTF |= PWR_CTRL_MASK;
		    	is_open=0; 
                        return NRK_OK;
             
	     case GET_STATUS:
	     		// use "key" here 
			if(key==SENSOR_SELECT) return channel;
	     		return NRK_ERROR;
			
             case SET_STATUS:
	     		// use "key" and "value" here
  			if(key==SENSOR_SELECT) 
			{
			// Set to audio channel if it is an average value
			if(value==AUDIO_P2P) 
			  {
				channel=value;
				   //ADC_VREF_2_56();	
				ADC_VREF_VCC();	
				ADC_SET_CHANNEL (AUDIO);
				nrk_spin_wait_us(ADC_SETUP_DELAY);
				return NRK_OK;

			  } else
			  {
				if(value>7) 
				{
					_nrk_errno_set(1);
					return NRK_ERROR;
				}
				channel=value;
				if(channel==LIGHT)
				   ADC_VREF_VCC();	
				else
				   ADC_VREF_2_56();	
				ADC_SET_CHANNEL (channel);
				nrk_spin_wait_us(ADC_SETUP_DELAY);
				return NRK_OK;
			  }
			}
			return NRK_ERROR;
	     default:
		nrk_kernel_error_add(NRK_DEVICE_DRIVER,0);
		 return 0;
	}
}


// read_voltage_status()
//
// This function sets different voltage threshold levels on
// the cc2420 chip to search for the voltage.
// If the voltage is above 3.3 volts, then the ADC reads
// the external voltage value going through a voltage divider.
// This function will return VOLTS*100
uint16_t read_voltage_status()
{
volatile uint16_t val;
uint8_t check,level;
nrk_sem_t *radio_sem;

radio_sem= rf_get_sem();

// if semaphore not created, then assume you own the radio 
if(radio_sem!=NULL)
 nrk_sem_pend (radio_sem);

    // activate cc2420 vreg
    SET_VREG_ACTIVE();
    // FIXME: Check at end if VREG needs to be disabled again...

level=0;
while(level<0x1F)
{
val=0x0020 | level;
FASTSPI_SETREG(CC2420_BATTMON, val);
nrk_spin_wait_us(2);
FASTSPI_GETREG(CC2420_BATTMON, val);
if(val&0x0040) break; 
level++;
}
if(radio_sem!=NULL)
 nrk_sem_post(radio_sem);
if(level==0)
  {
  val=get_adc_val();
  // FIXME:  This probably isn't correct...
  if(val>174) val-=174;
  if(val<330) val=330;
  }
else val=(9000-(level*125)) / 27;

return val;
}



void init_adc()
{
// Initialize values here
  ADC_INIT ();
  ADC_ENABLE ();
  channel=0;
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
  nrk_spin_wait_us(ADC_SETUP_DELAY);
}
