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

#include <nrk_cfg.h>

#ifdef  NRK_MAX_DRIVER_CNT

#include <nrk.h>
#include <nrk_events.h>
#include <nrk_task.h>
#include <nrk_error.h>
#include <nrk_scheduler.h>
#include <include.h>
#include <ulib.h>
#include <nrk_timer.h>
#include <nrk_time.h>
#include <nrk_cpu.h>
#include <nrk_driver.h>
#include <nrk_driver_list.h>


NRKDriver nrk_drivers[NRK_MAX_DRIVER_CNT];
uint8_t nrk_driver_init[NRK_MAX_DRIVER_CNT];

int8_t _nrk_driver_count;

/*
 * nrk_register_driver()
 *
 * This function takes a pointer to the entrance of a driver function
 * and returns a device handler for the driver.
 *
 *
 */

int8_t nrk_register_driver(void *devicemanager,uint8_t dev_index)
{
		if(_nrk_driver_count<NRK_MAX_DRIVER_CNT)
		{
                 nrk_drivers[_nrk_driver_count].dev_id=dev_index; 
                 nrk_drivers[_nrk_driver_count].devicemanager=(void*)devicemanager;
		 _nrk_driver_count++;
		 return NRK_OK;
	        }
		 else
			 return NRK_ERROR;
}


int8_t nrk_open(uint8_t dev_index,uint8_t opt) 
{
uint8_t cnt;


		for(cnt=0;cnt<_nrk_driver_count;cnt++)
		{
		      if(nrk_drivers[cnt].dev_id==dev_index)  
		      {
				if(nrk_driver_init[cnt]==WAS_NOT_OPEN)
				      {
					// Only call init the first time driver is opened
                                        nrk_driver_init[cnt]=WAS_OPEN;
        				nrk_drivers[cnt].devicemanager(INIT,opt,NULL,0);
				      }
        		nrk_drivers[cnt].devicemanager(OPEN,opt,NULL,0);
		        return cnt;
		      }
		}

 return NRK_ERROR;
}
 
int8_t nrk_close(uint8_t dev_fd)
{
uint8_t error;

	if(dev_fd>_nrk_driver_count)
		{
		_nrk_errno_set(1);  // invalid device
		return NRK_ERROR;
		}

error=nrk_drivers[dev_fd].devicemanager(CLOSE,0,NULL,0);

// Do we need to do init on a second reopen?
//if(!error)
//	driver_record_init[dev_fd]=0;

return error;
} 


int8_t nrk_write(uint8_t dev_fd,uint8_t *buffer, uint8_t size)
{
	if(dev_fd>_nrk_driver_count)
		{
		_nrk_errno_set(1);  // invalid device
		return NRK_ERROR;
		}

         return nrk_drivers[dev_fd].devicemanager(WRITE,0,buffer,size);

}

int8_t nrk_read(uint8_t dev_fd,uint8_t *buffer,uint8_t size)
{
	if(dev_fd>_nrk_driver_count)
		{
		_nrk_errno_set(1);  // invalid device
		return NRK_ERROR;
		}

         return nrk_drivers[dev_fd].devicemanager(READ,0,buffer,size);

}
/*if key is 0 then assumed to create a frequency setting*/
int8_t nrk_set_status(uint8_t dev_fd,uint8_t key,uint8_t value)
{
	if(dev_fd<0 || dev_fd>_nrk_driver_count)
		{
		_nrk_errno_set(1);  // invalid device
		return NRK_ERROR;
		}
         return nrk_drivers[dev_fd].devicemanager(SET_STATUS,key,NULL,value);
} 

int8_t nrk_get_status(uint8_t dev_fd,uint8_t key)
{
	if(dev_fd>_nrk_driver_count)
		{
		_nrk_errno_set(1);  // invalid device
		return NRK_ERROR;
		}

   	return nrk_drivers[dev_fd].devicemanager(GET_STATUS,key,NULL,0);
}

#endif
