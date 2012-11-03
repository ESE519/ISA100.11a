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
if(pin==NRK_EXT_INT_0) { EIMSK |= BM(INT0); return NRK_OK; }
if(pin==NRK_EXT_INT_1) { EIMSK |= BM(INT1); return NRK_OK; }
if(pin==NRK_EXT_INT_2) { EIMSK |= BM(INT2); return NRK_OK; }
if(pin==NRK_PC_INT_0 ) { PCMSK0 |= BM(PCINT0); return NRK_OK; }
if(pin==NRK_PC_INT_1 ) { PCMSK0 |= BM(PCINT1); return NRK_OK; }
if(pin==NRK_PC_INT_2 ) { PCMSK0 |= BM(PCINT2); return NRK_OK; }
if(pin==NRK_PC_INT_3 ) { PCMSK0 |= BM(PCINT3); return NRK_OK; }
if(pin==NRK_PC_INT_4 ) { PCMSK0 |= BM(PCINT4); return NRK_OK; }
if(pin==NRK_PC_INT_5 ) { PCMSK0 |= BM(PCINT5); return NRK_OK; }
if(pin==NRK_PC_INT_6 ) { PCMSK0 |= BM(PCINT6); return NRK_OK; }
if(pin==NRK_PC_INT_7 ) { PCMSK0 |= BM(PCINT7); return NRK_OK; }
return NRK_ERROR;
}

int8_t  nrk_ext_int_disable(uint8_t pin )
{
if(pin==NRK_EXT_INT_0) { EIMSK &= ~BM(INT0); return NRK_OK; }
if(pin==NRK_EXT_INT_1) { EIMSK &= ~BM(INT1); return NRK_OK; }
if(pin==NRK_EXT_INT_2) { EIMSK &= ~BM(INT1); return NRK_OK; }
if(pin==NRK_PC_INT_0 ) { PCMSK0 &= ~BM(PCINT0); return NRK_OK; }
if(pin==NRK_PC_INT_1 ) { PCMSK0 &= ~BM(PCINT1); return NRK_OK; }
if(pin==NRK_PC_INT_2 ) { PCMSK0 &= ~BM(PCINT2); return NRK_OK; }
if(pin==NRK_PC_INT_3 ) { PCMSK0 &= ~BM(PCINT3); return NRK_OK; }
if(pin==NRK_PC_INT_4 ) { PCMSK0 &= ~BM(PCINT4); return NRK_OK; }
if(pin==NRK_PC_INT_5 ) { PCMSK0 &= ~BM(PCINT5); return NRK_OK; }
if(pin==NRK_PC_INT_6 ) { PCMSK0 &= ~BM(PCINT6); return NRK_OK; }
if(pin==NRK_PC_INT_7 ) { PCMSK0 &= ~BM(PCINT7); return NRK_OK; }
return NRK_ERROR;
}



int8_t  nrk_ext_int_configure(uint8_t pin, uint8_t mode, void *callback_func)
{
if(pin==NRK_EXT_INT_0)
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
if(pin==NRK_EXT_INT_1)
	{
	ext_int1_callback=callback_func;
	if(mode==NRK_LOW_TRIGGER) EICRA &= ~(BM(ISC11) | BM(ISC10));
	if(mode==NRK_LEVEL_TRIGGER) 
		{ EICRA &= (~BM(ISC11)); EICRA |= BM(ISC10); }
	if(mode==NRK_FALLING_EDGE) 
		{ EICRA |= BM(ISC11); EICRA &= (~BM(ISC10)); }
	if(mode==NRK_RISING_EDGE) EICRA |= BM(ISC11) | BM(ISC10);
	return NRK_OK;
	}
if(pin==NRK_EXT_INT_2)
	{
	ext_int1_callback=callback_func;
	if(mode==NRK_LOW_TRIGGER) EICRA &= ~(BM(ISC21) | BM(ISC20));
	if(mode==NRK_LEVEL_TRIGGER) 
		{ EICRA &= (~BM(ISC21)); EICRA |= BM(ISC20); }
	if(mode==NRK_FALLING_EDGE) 
		{ EICRA |= BM(ISC21); EICRA &= (~BM(ISC20)); }
	if(mode==NRK_RISING_EDGE) EICRA |= BM(ISC21) | BM(ISC20);
	return NRK_OK;
	}



if(pin==NRK_PC_INT_0 || pin==NRK_PC_INT_1 || pin==NRK_PC_INT_2 || pin==NRK_PC_INT_3 || pin==NRK_PC_INT_4 || pin==NRK_PC_INT_5 || pin==NRK_PC_INT_6 || pin==NRK_PC_INT_7){
	PCICR |= BM(PCIE0);	
	pc_int0_callback=callback_func;
	return NRK_OK;
	}
return NRK_ERROR;
}


SIGNAL(PCINT0_vect) {
	if(pc_int0_callback!=NULL) pc_int0_callback();
	else
	nrk_kernel_error_add(NRK_SEG_FAULT,0);
	return;  	
}


SIGNAL(INT0_vect) {
	if(ext_int0_callback!=NULL) ext_int0_callback();
	else
	nrk_kernel_error_add(NRK_SEG_FAULT,0);
	return;  	
}

