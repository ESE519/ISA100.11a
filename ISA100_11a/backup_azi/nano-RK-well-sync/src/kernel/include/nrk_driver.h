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


#ifndef NRK_DRIVER_H
#define NRK_DRIVER_H

#include <stdint.h>

// Driver Open/Close 
#define WAS_OPEN  1
#define WAS_NOT_OPEN 0


// Driver Actions 
#define INIT		0
#define OPEN		1
#define CLOSE		2
#define GET_STATUS	3
#define SET_STATUS	4
#define READ		5
#define WRITE		6

// Access Privileges Bits
#define READ_FLAG 	1
#define WRITE_FLAG 	2
#define APPEND_FLAG	4


typedef struct nrk_driver
{
	int8_t dev_id; 
	// for enabling and disabling device drivers 
	int8_t access_priv;
	// Access privileges (read, write, append etc) 
	int8_t (*devicemanager)(uint8_t state,uint8_t opt,uint8_t*buf,uint8_t size); 
	// function pointer to the driver call

} NRKDriver;


int8_t nrk_register_driver(void *devicemanager,uint8_t driver_name);
int8_t nrk_open(uint8_t dev_id,uint8_t opt); // options provide the ablility to set_status
int8_t nrk_read(uint8_t dev_fd,uint8_t *buffer,uint8_t size);
int8_t nrk_write(uint8_t dev_fd,uint8_t *buffer, uint8_t size);
int8_t nrk_close(uint8_t dev_fd); // options provide the ablility to set_status
int8_t nrk_set_status(uint8_t dev_fd,uint8_t key,uint8_t value);
int8_t nrk_get_status(uint8_t dev_fd,uint8_t key);

#endif
