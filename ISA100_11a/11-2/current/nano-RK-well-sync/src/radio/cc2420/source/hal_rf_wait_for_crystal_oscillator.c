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
*  Chipcon Development Team 
*******************************************************************************/

#include <include.h>
#include <ulib.h>

//-------------------------------------------------------------------------------------------------------
//	void rfWaitForCrystalOscillator(void)
//
//	DESCRIPTION:
//		Waits for the crystal oscillator to become stable. The flag is polled via the SPI status byte.
//      
//      Note that this function will lock up if the SXOSCON command strobe has not been given before the
//      function call. Also note that global interrupts will always be enabled when this function 
//      returns.
//-------------------------------------------------------------------------------------------------------
void halRfWaitForCrystalOscillator(void) {
    uint8_t spiStatusByte;

    // Poll the SPI status byte until the crystal oscillator is stable
    do {
	    DISABLE_GLOBAL_INT();
	    FASTSPI_UPD_STATUS(spiStatusByte);
	    ENABLE_GLOBAL_INT();
    } while (!(spiStatusByte & (BM(CC2420_XOSC16M_STABLE))));

} // halRfWaitForCrystalOscillator

