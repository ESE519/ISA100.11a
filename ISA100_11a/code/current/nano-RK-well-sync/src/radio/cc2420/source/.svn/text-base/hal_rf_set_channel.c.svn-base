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
//	void halRfSetChannel(UINT8 Channel)
//
//	DESCRIPTION:
//		Programs CC2420 for a given IEEE 802.15.4 channel. 
//		Note that SRXON, STXON or STXONCCA must be run for the new channel selection to take full effect.
//
//	PARAMETERS:
//		UINT8 channel
//			The channel number (11-26)
//-------------------------------------------------------------------------------------------------------
void halRfSetChannel(uint8_t channel) {
	uint16_t f;
	
	// Derive frequency programming from the given channel number
	f = (uint16_t) (channel - 11); // Subtract the base channel 
	f = f + (f << 2);    		 // Multiply with 5, which is the channel spacing
	f = f + 357 + 0x4000;		 // 357 is 2405-2048, 0x4000 is LOCK_THR = 1
	
    // Write it to the CC2420
	DISABLE_GLOBAL_INT();
	FASTSPI_SETREG(CC2420_FSCTRL, f);
	ENABLE_GLOBAL_INT();

} // rfSetChannel


