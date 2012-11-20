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
*******************************************************************************/

#include <include.h>
#include <avr/interrupt.h>
#include <ulib.h>
#include <nrk_ext_int.h>
#include <nrk_error.h>
#include <nrk_cfg.h>


int8_t  nrk_ext_int_enable(uint8_t pin )
{
return NRK_ERROR;
}

int8_t  nrk_ext_int_disable(uint8_t pin )
{
return NRK_ERROR;
}



int8_t  nrk_ext_int_configure(uint8_t pin, uint8_t mode, void *callback_func)
{
/*if(pin==NRK_EXT_INT_0)
	{
	ext_int0_callback=callback_func;
	if(mode==NRK_LOW_TRIGGER) EICRA &= ~(BM(ISC01) | BM(ISC00));
	if(mode==NRK_LEVEL_TRIGGER) 
		{ EICRA &= (~BM(ISC01)); EICRA |= BM(ISC00); }
	if(mode==NRK_FALLING_EDGE) 
		{ EICRA |= BM(ISC01); EICRA &= (~BM(ISC00)); }
	if(mode==NRK_RISING_EDGE) EICRA |= BM(ISC01) | BM(ISC00);
	return NRK_OK;
	}
*/
return NRK_ERROR;
}


/*
SIGNAL(INT0_vect) {
//	if(ext_int0_callback!=NULL) ext_int0_callback();
//	else
	nrk_kernel_error_add(NRK_SEG_FAULT,0);
	return;  	
}
*/
