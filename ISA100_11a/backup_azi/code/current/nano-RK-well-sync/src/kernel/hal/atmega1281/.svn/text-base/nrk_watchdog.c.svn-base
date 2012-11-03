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
#include <nrk_watchdog.h>
#include <nrk_error.h>
#include <nrk.h>
#include <avr/wdt.h>

void nrk_watchdog_disable()
{
nrk_int_disable();
nrk_watchdog_reset();
MCUSR &= ~(1<<WDRF);
WDTCSR |= (1<<WDCE) | (1<<WDE);
WDTCSR = 0;
nrk_int_enable();
}

void nrk_watchdog_enable()
{
// Enable watchdog with 1024K cycle timeout
// No Interrupt Trigger
nrk_int_disable();
MCUSR &= ~(1<<WDRF);
nrk_watchdog_reset();
WDTCSR |= (1<<WDCE) | (1<<WDE);
WDTCSR = (1<<WDE) | (1<<WDP2) | (1<<WDP0);
nrk_int_enable();

}

int8_t nrk_watchdog_check()
{

if((MCUSR & (1<<WDRF))==0) return NRK_OK;
return NRK_ERROR;
}

inline void nrk_watchdog_reset()
{
wdt_reset();

}
