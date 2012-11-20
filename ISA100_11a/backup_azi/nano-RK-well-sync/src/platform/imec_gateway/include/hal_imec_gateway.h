/*******************************************************************************************************
 *                                                                                                     *
 *        **********                                                                                   *
 *       ************                                                                                  *
 *      ***        ***                                                                                 *
 *      ***   +++   ***                                                                                *
 *      ***   + +   ***                                                                                *
 *      ***   +                        CHIPCON HARDWARE ABSTRACTION LIBRARY FOR THE CC2420             *
 *      ***   + +   ***                             MSP430FET defintion file                           *
 *      ***   +++   ***                                                                                *
 *      ***        ***                                                                                 *
 *       ************                                                                                  *
 *        **********                                                                                   *
 *                                                                                                     *
 *******************************************************************************************************
 * The Chipcon Hardware Abstraction Library is a collection of functions, macros and constants, which  *
 * can be used to ease access to the hardware on the CC2420 and the target microcontroller.            *
 *                                                                                                     *
 * This file contains all definitions that are specific for the MSP-FET430P140 development board.      *
 *******************************************************************************************************
 * Compiler: MSP430-GCC and IAR Embedded Workbench                                                     *
 * Target platform: MSP430FET                                                                          *
 *******************************************************************************************************
 * Revision history:                                                                                   *
 *  
 *
 *
 *
 *******************************************************************************************************/
#ifndef HAL_IMEC_GATEWAY_H
#define HAL_IMEC_GATEWAY_H

#define IMEC_GATEWAY_PLATFORM

#define RED_LED 0
#define GREEN_LED 1
#define BLUE_LED 2
#define ORANGE_LED 3

void PORT_INIT(void);

#define SPI_ENABLE()    ( P5OUT &= ~BM(CSN) ) // ENABLE CSn (active low)
#define SPI_DISABLE()	( P5OUT |=  BM(CSN) ) // DISABLE CSn (active low)

// Rising edge trigger for external interrupt 0 (FIFOP)
#define FIFOP_INT_INIT()            do { P2IES &= ~BM(FIFO_P); CLEAR_FIFOP_INT(); } while (0)

// FIFOP on external interrupt 0
#define ENABLE_FIFOP_INT()          do { P2IE |= BM(FIFO_P); } while (0)
#define DISABLE_FIFOP_INT()         do { P2IE &= ~BM(FIFO_P); } while (0)
#define CLEAR_FIFOP_INT()           do { P2IFG &= ~BM(FIFO_P); } while (0)

// SFD interrupt on timer 1 capture pin
#define ENABLE_SFD_CAPTURE_INT()    do { TIMSK |= BM(TICIE1); } while (0)
#define DISABLE_SFD_CAPTURE_INT()   do { TIMSK &= ~BM(TICIE1); } while (0)
#define CLEAR_SFD_CAPTURE_INT()     do { TIFR = BM(ICF1); } while (0)

#endif
