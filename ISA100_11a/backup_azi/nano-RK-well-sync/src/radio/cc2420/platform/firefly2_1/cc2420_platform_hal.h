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

#ifndef _CC2420_PLATFORM_HAL_H_
#define _CC2420_PLATFORM_HAL_H_


// CC2420 pin access

// Pin status
#define FIFO_IS_1       (!!(PINC & BM(FIFO)))
#define CCA_IS_1        (!!(PIND & BM(CCA)))
#define RESET_IS_1      (!!(PINA & BM(RESET_N)))
#define VREG_IS_1       (!!(PINA & BM(VREG_EN)))
#define FIFOP_IS_1      (!!(PINE & BM(FIFOP)))
#define SFD_IS_1        (!!(PIND & BM(SFD)))

// The CC2420 reset pin
#define SET_RESET_ACTIVE()    PORTA &= ~BM(RESET_N)
#define SET_RESET_INACTIVE()  PORTA |= BM(RESET_N)

// CC2420 voltage regulator enable pin
#define SET_VREG_ACTIVE()     PORTA |= BM(VREG_EN)
#define SET_VREG_INACTIVE()   PORTA &= ~BM(VREG_EN)
//---------------------------------------------------------------------------

#endif
