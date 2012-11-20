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


#include<stdio.h>

// SET/GET STATUS options
#define ADC_CHAN 1

// ADC channels
#define CHAN_0 0
#define CHAN_1 1
#define CHAN_2 2
#define CHAN_3 3
#define CHAN_4 4
#define CHAN_5 5
#define CHAN_6 6
#define CHAN_7 7

void delay();
uint8_t dev_manager_adc(uint8_t state,uint8_t opt,uint8_t * buffer,uint8_t size);
uint16_t get_adc_val();

// Functions for initializing and updating sensor values
void init_adc();
