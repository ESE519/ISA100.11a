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
*  Anthony Rowe
*******************************************************************************/


#ifndef HAL_FIREFLY2_H
#define HAL_FIREFLY2_H

#define FIREFLY2_PLATFORM 

#define ORANGE_LED	0
#define BLUE_LED	1
#define GREEN_LED	2
#define RED_LED		3

#define NRK_DEFAULT_UART 1



//-------------------------------------------------------------------------------------------------------
// External RAM interface:
//     PA and PC - Multiplexed address/data
//     PG.0 - Output: Write enable: WR_N
//     PG.1 - Output: Read enable: RD_N
//     PG.2 - Output: Address Latch Enable: ALE
//-------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
// Port setup macros

// Port initialization
// Disables pull-up on all inputs!!!
#define PORT_INIT() \
    do { \
        SFIOR |= BM(PUD); \
        DDRB  = BM(MOSI) | BM(SCK) | BM(SPI_SS);  \
        PORTB = BM(MOSI) | BM(SCK) | BM(SPI_SS);  \
        DDRC  = BM(CSN); \
        PORTC = BM(CSN); \
        DDRE  = BM(LED_0) | BM(LED_1) | BM(LED_2) | BM(LED_3);  \
        DDRA  = BM(VREG_EN) | BM(RESET_N) | BM(DEBUG_0) | BM(DEBUG_1); \
        PORTA = BM(RESET_N); \
    } while (0)

// Enables the external SRAM
//#define ENABLE_EXT_RAM() (MCUCR |= BM(SRE))

// Enables/disables the SPI interface
#define SPI_ENABLE()                (PORTC &= ~BM(CSN))
#define SPI_DISABLE()               (PORTC |= BM(CSN))
//-------------------------------------------------------------------------------------------------------




/*******************************************************************************************************
 *******************************************************************************************************
 **************************                 CC2420 PIN ACCESS                 **************************
 *******************************************************************************************************
 *******************************************************************************************************/
//-------------------------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------------------------



/*******************************************************************************************************
 *******************************************************************************************************
 **************************               EXTERNAL INTERRUPTS                 **************************
 *******************************************************************************************************
 *******************************************************************************************************/


//-------------------------------------------------------------------------------------------------------
// Rising edge trigger for external interrupt 0 (FIFOP)
#define FIFOP_INT_INIT()            do { EICRA |= 0x03; CLEAR_FIFOP_INT(); } while (0)

// FIFOP on external interrupt 0
#define ENABLE_FIFOP_INT()          do { EIMSK |= 0x01; } while (0)
#define DISABLE_FIFOP_INT()         do { EIMSK &= ~0x01; } while (0)
#define CLEAR_FIFOP_INT()           do { EIFR = 0x01; } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// SFD interrupt on timer 1 capture pin
#define ENABLE_SFD_CAPTURE_INT()    do { TIMSK |= BM(TICIE1); } while (0)
#define DISABLE_SFD_CAPTURE_INT()   do { TIMSK &= ~BM(TICIE1); } while (0)
#define CLEAR_SFD_CAPTURE_INT()     do { TIFR = BM(ICF1); } while (0)
//-------------------------------------------------------------------------------------------------------


#endif
