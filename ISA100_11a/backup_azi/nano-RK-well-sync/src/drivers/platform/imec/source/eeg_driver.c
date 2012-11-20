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
*  Mark Hamilton
*******************************************************************************/


#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <adc_driver.h>
#include <eeg_driver.h>
#include <include.h>
#include <stdio.h>
#include <ulib.h>
#include <nrk_error.h>
#include <nrk.h>
#include <stdint.h>

uint8_t eeg_channel;
uint8_t eeg_gain;
uint8_t ecg_gain;
int8_t eeg_adc_fd;

#define EEG_INIT()\
  do {\
    nrk_gpio_clr(UP_RESET);\
    nrk_gpio_set(UP_RESET);\
    nrk_gpio_clr(UP_RESET);\
  } while(0);

#define EEG_SET_CHANNEL(channel)\
  do {\
  } while(0);

#define EEG_SET_GAIN(gain)\
  do {\
    nrk_gpio_clr(UP_A0_EEG);\
    nrk_gpio_clr(UP_A1_EEG);\
    nrk_gpio_clr(UP_A2_EEG);\
    if(gain&0x01) nrk_gpio_set(UP_A0_EEG);\
    if(gain&0x02) nrk_gpio_set(UP_A1_EEG);\
    if(gain&0x04) nrk_gpio_set(UP_A2_EEG);\
  } while(0)

#define ECG_SET_GAIN(gain)\
  do {\
    nrk_gpio_clr(UP_A0_ECG);\
    nrk_gpio_clr(UP_A1_ECG);\
    nrk_gpio_clr(UP_A2_ECG);\
    if(gain&0x01) nrk_gpio_set(UP_A0_ECG);\
    if(gain&0x02) nrk_gpio_set(UP_A1_ECG);\
    if(gain&0x04) nrk_gpio_set(UP_A2_ECG);\
  } while(0)

#define EEG_ENABLE()\
  do {\
    nrk_gpio_set(ENABLE1);\
  } while(0)

#define EEG_DISABLE()\
  do {\
    nrk_gpio_clr(ENABLE1);\
  } while(0)

uint8_t dev_manager_eeg(uint8_t action,uint8_t opt,uint8_t *buffer,uint8_t size)
{
  uint8_t count=0; //TODO: what is this for?
  // key and value get passed as opt and size
  uint8_t key=opt;
  uint8_t value=size;
  uint16_t val;

  switch(action)
  {
    case INIT: 
      init_eeg();  
      return 1;

    case OPEN:   
      if(opt&READ_FLAG)
      {
        eeg_adc_fd=nrk_open(ADC_DEV_MANAGER,READ);
        if(eeg_adc_fd==NRK_ERROR) {
          nrk_kprintf( "Failed to open ADC driver for EEG\r\n");
          return NRK_ERROR;
        }
        val=nrk_set_status(eeg_adc_fd,ADC_CHAN,0);
        if(val==NRK_ERROR) nrk_kprintf( "Failed to set ADC status\r\n");
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
      if((opt&(READ_FLAG|WRITE_FLAG|APPEND_FLAG))==0)
        return NRK_ERROR;
      else return NRK_OK;

    case READ:
      /* Conversion to 8-bit value*/
      val=get_eeg_val();
      buffer[count]=val & 0xff;
      count++;
      buffer[count]=(val>>8)  & 0xff;
      count++;
      return count;

    case CLOSE:
      EEG_DISABLE();
      return NRK_OK;

    case GET_STATUS:
      // use "key" here 
      if(key==EEG_CHAN)
        return eeg_channel;
      return NRK_ERROR;

    case SET_STATUS:
      // use "key" and "value" here
      switch(key)
      {
        case EEG_CHAN:
        eeg_channel = value;
        EEG_SET_CHANNEL(eeg_channel);
        return NRK_OK;

        case EEG_GAIN:
        eeg_gain = value;
        EEG_SET_GAIN(eeg_gain);
        return NRK_OK;

        case ECG_GAIN:
        ecg_gain = value;
        ECG_SET_GAIN(ecg_gain);
        return NRK_OK;
        
        default:
        return NRK_ERROR;
      }

    default:
      nrk_kernel_error_add(NRK_DEVICE_DRIVER,0);
      return 0;
  }
}

void init_eeg()
{
  EEG_INIT();
	eeg_channel = 0;
  EEG_SET_CHANNEL(eeg_channel);
  EEG_ENABLE();
}

uint16_t get_eeg_val()
{                         
	uint8_t eeg_val[2];
  uint8_t retval;
	retval=nrk_read(eeg_adc_fd,&eeg_val,2); //TODO: ensure correct byte ordering
	if(retval==NRK_ERROR) nrk_kprintf("Failed to read ADC for EEG\r\n");
  printf("get_eeg_val: %d\r\n", eeg_val);
	return eeg_val;
}
