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
*  Nuno Pereira
*  Anthony Rowe
*******************************************************************************/


#ifndef NRK_PIN_DEFINE_H
#define NRK_PIN_DEFINE_H

/*******************************************************************************************************
 *******************************************************************************************************
 **************************                   AVR I/O PORTS                   **************************
 *******************************************************************************************************
 *******************************************************************************************************/

//-------------------------------------------------------------------------------------------------------
// Port A
#define LED_0            0  // PA.0 - Output: Yellow LED
#define LED_1            1  // PA.1 - Output: Green LED
#define LED_2            2  // PA.2 - Output: Red LED
#define VREG_EN         5  // PA.5 - Output: VREG_EN to CC2420
#define RESET_N         6  // PA.6 - Output: RESET_N to CC2420

//-------------------------------------------------------------------------------------------------------
// Port B
#define CSN             0  // PB.0 - Output: SPI Chip Select (CS_N)
#define SCK             1  // PB.1 - Output: SPI Serial Clock (SCLK)
#define MOSI            2  // PB.2 - Output: SPI Master out - slave in (MOSI)
#define MISO            3  // PB.3 - Input:  SPI Master in - slave out (MISO)
#define FIFO            7  // PB.7 - Input:  FIFO from CC2420
//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------
// Port C
#define DEBUG_0         1
#define DEBUG_1         2
#define DEBUG_2         0   // placeholder not implemented
#define DEBUG_3         0   // placeholder not implemented

// Set LED
#define CLR_LED_0()                  (PORTA |= BM(YLED))
#define CLR_LED_1()                  (PORTA |= BM(GLED))
#define CLR_LED_2()                  (PORTA |= BM(RLED))

// Clears LED
#define SET_LED_0()                  (PORTA &= ~BM(YLED))
#define SET_LED_1()                  (PORTA &= ~BM(GLED))
#define SET_LED_2()                  (PORTA &= ~BM(RLED))
//-------------------------------------------------------------------------------------------------------
// Port D
#define UART1_RXD       2 // PD.2 - Input:  UART1 RXD
#define UART1_TXD       3 // PD.3 - Output: UART1 TXD
#define SFD             4 // PD.4 - Input:  SFD from CC2420
#define UART1_RTS       5 // PD.5 - Output: UART HW handshaking: RTS
#define CCA             6 // PD.6 - Input:  CCA from CC2420
#define UART1_CTS       7 // PD.7 - Input:  UART HW handshaking: CTS

//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Port E
#define UART0_RXD       0 // PE.0 - Input:  UART0 RXD
#define UART0_TXD       1 // PE.1 - Output: UART0 TXD
#define BUTTON          5 // PE.5 - Input : Push button S2
#define FIFOP           6 // PE.6 - Input:  FIFOP from CC2420
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Port F
// ADC inputs
#define ADC_INPUT_0     0
#define ADC_INPUT_1     1 // PF.1 - ADC1
#define ADC_INPUT_2     2 // PF.2 - ADC2
#define ADC_INPUT_3     3 // PF.3 - ADC3
#define ADC_INPUT_4     4 // PF.3 - ADC3
#define ADC_INPUT_5     5 // PF.3 - ADC3
#define ADC_INPUT_6     6 // PF.3 - ADC3
#define ADC_INPUT_7     7 // PF.3 - ADC3

// JTAG interface:
//     PF.4 - Input : JTAG TCK serial clock
//     PF.5 - Input : JTAG TMS strobe enable
//     PF.6 - Output: JTAG TDO serial data output
//     PF.7 - Input : JTAG TDI serial data input
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Port G
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// External RAM interface:
//     PA and PC - Multiplexed address/data
//     PG.0 - Output: Write enable: WR_N
//     PG.1 - Output: Read enable: RD_N
//     PG.2 - Output: Address Latch Enable: ALE
//-------------------------------------------------------------------------------------------------------

/*******************************************************************************************************
 *******************************************************************************************************
 **************************                        GPIO                       **************************
 *******************************************************************************************************
 *******************************************************************************************************/

//-------------------------------
// GPIO handling functions
// these macros perform raw hw access
// ports and pins are acctual hw ports and pins

// use pin_port, and pin; ie: nkr_gpio_raw_set( PORTB, DEBUG_0 )
#define nrk_gpio_raw_set( _port, _pin ) {do { _port |= BM(_pin); } while(0);}
// use pin_port, and pin; ie: nkr_gpio_raw_clr( PORTB, DEBUG_0 )
#define nrk_gpio_raw_clr( _port, _pin ) {do { _port &= ~BM(_pin); } while(0);}
// use pin_port, and pin; ie: nkr_gpio_raw_get( PINB, DEBUG_0 )
#define nrk_gpio_raw_get( _pin_port, _pin ) (_pin_port & BM(_pin))
// use pin_port, port and pin; ie: nkr_gpio_raw_toggle( PINB, PORTB, DEBUG_0 )
#define nrk_gpio_raw_toggle( _pin_port, _port, _pin ) { \
        if ((_pin_port & BM(_pin))) do{ _port &= ~BM(_pin); } while(0); \
        else do { _port |= BM(_pin); }while(0);  \
}
// use direction; ie: nkr_gpio_raw_direction( DDRB, DEBUG_0 )
#define nrk_gpio_raw_direction( _direction_port_name, _pin, _pin_direction ) { \
        if (_pin_direction == NRK_PIN_INPUT) { \
                _direction_port_name &= ~BM( _pin ); \
        } else { \
                _direction_port_name |= BM( _pin ); \
        } \
}

// when a platform does not support one
// of the NRK_<pin name> declared below, it
// must define it has an invalid pin in the
// platform ulib.c (e.g. a platform that does not
// support NRK_DEBUG_0 should have the following in
// ulib.c NRK_INVALID_PIN( NRK_DEBUG_0 ) )
#define NRK_INVALID_PIN_VAL 0xFF

// nrk ports NRK_<hw port> used for the mapping
// to the real hw. (3 bits reserved for ports)
#define NRK_PORTA 0
#define NRK_PORTB 1
#define NRK_PORTC 2
#define NRK_PORTD 3
#define NRK_PORTE 4
#define NRK_PORTF 5

// define pin directions
#define NRK_PIN_INPUT 0
#define NRK_PIN_OUTPUT 1


//---------------------------------------------------------------------------------------------
// GPIO related definitions

// macros to define a pin as used by higher level programs.
// higher level programs refer to pin as NRK_<pin name>
// these functions declare these NRK_<pin name> pins and provide
// the mappings to the hardware
#define DECLARE_NRK_PIN( _pin_name ) extern const uint8_t NRK_ ## _pin_name;
#define NRK_PIN( _pin_name, _pin , _port ) const uint8_t NRK_ ## _pin_name = (_pin << 3) + (_port & 0x07);
#define NRK_INVALID_PIN( _pin_name ) const uint8_t NRK_ ## _pin_name = NRK_INVALID_PIN_VAL;

// declare pins as used by higher level programs
// mapping to the hardware is done by ulib.c
DECLARE_NRK_PIN( VREG_EN ) 			// declare pin named NRK_VREG_EN
DECLARE_NRK_PIN( RESET_N ) 			// declare pin named NRK_RESET_N
DECLARE_NRK_PIN( DEBUG_0 ) 			// declare pin named NRK_DEBUG_0
DECLARE_NRK_PIN( DEBUG_1 ) 			// declare pin named NRK_DEBUG_1
DECLARE_NRK_PIN( DEBUG_2 ) 			// declare pin named NRK_DEBUG_2
DECLARE_NRK_PIN( DEBUG_3 ) 			// declare pin named NRK_DEBUG_3
DECLARE_NRK_PIN( BUTTON ) 			// declare pin named NRK_BUTTON

DECLARE_NRK_PIN( SPI_SS ) 			// declare pin named NRK_SPI_SS
DECLARE_NRK_PIN( SCK ) 				// declare pin named NRK_SCK
DECLARE_NRK_PIN( MOSI ) 			// declare pin named NRK_MOSI
DECLARE_NRK_PIN( MISO ) 			// declare pin named NRK_MISO

DECLARE_NRK_PIN( GPIO28 ) 			// declare pin named NRK_GPIO28
DECLARE_NRK_PIN( GPIO26 ) 			// declare pin named NRK_GPIO26

DECLARE_NRK_PIN( CSN ) 				// declare pin named NRK_CSN
DECLARE_NRK_PIN( FIFO ) 			// declare pin named NRK_FIFO

DECLARE_NRK_PIN( UART1_RXD ) 			// declare pin named NRK_UART1_RXD
DECLARE_NRK_PIN( UART1_TXD ) 			// declare pin named NRK_UART1_TXD
DECLARE_NRK_PIN( SFD ) 				// declare pin named NRK_SFD
DECLARE_NRK_PIN( UART1_RTS ) 			// declare pin named NRK_UART1_RTS
DECLARE_NRK_PIN( CCA ) 				// declare pin named NRK_CCA
DECLARE_NRK_PIN( UART1_CTS ) 			// declare pin named NRK_UART1_CTS

DECLARE_NRK_PIN( UART0_RXD ) 			// declare pin named NRK_UART0_RXD
DECLARE_NRK_PIN( UART0_TXD ) 			// declare pin named NRK_UART0_TXD
DECLARE_NRK_PIN( FIFOP ) 			// declare pin named NRK_FIFOP
DECLARE_NRK_PIN( LED_0 ) 			// declare pin named NRK_LED_0
DECLARE_NRK_PIN( LED_1 ) 			// declare pin named NRK_LED_1
DECLARE_NRK_PIN( LED_2 ) 			// declare pin named NRK_LED_2
DECLARE_NRK_PIN( LED_3 ) 			// declare pin named NRK_LED_3

DECLARE_NRK_PIN( GPIO34 )			// declare pin named NRK_GPIO34

DECLARE_NRK_PIN( ADC_INPUT_0) 	// declare pin named NRK_ADC_INPUT_0_POT_METER
DECLARE_NRK_PIN( ADC_INPUT_1 ) 			// declare pin named NRK_ADC_INPUT_1
DECLARE_NRK_PIN( ADC_INPUT_2 ) 			// declare pin named NRK_ADC_INPUT_2
DECLARE_NRK_PIN( ADC_INPUT_3) 	// declare pin named NRK_ADC_INPUT_3_TEMP_SENSOR

DECLARE_NRK_PIN( ADC_INPUT_6 ) 			// declare pin named NRK_ADC_INPUT_6
DECLARE_NRK_PIN( ADC_INPUT_7 ) 			// declare pin named NRK_ADC_INPUT_7

#endif
