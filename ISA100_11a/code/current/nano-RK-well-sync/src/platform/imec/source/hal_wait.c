/*******************************************************************************************************
 *                                                                                                     *
 *        **********                                                                                   *
 *       ************                                                                                  *
 *      ***        ***                                                                                 *
 *      ***   +++   ***                                                                                *
 *      ***   + +   ***                                                                                *
 *      ***   +                        CHIPCON HARDWARE ABSTRACTION LIBRARY FOR THE CC2420             *
 *      ***   + +   ***                                   Idle looping                                 *
 *      ***   +++   ***                                                                                *
 *      ***        ***                                                                                 *
 *       ************                                                                                  *
 *        **********                                                                                   *
 *                                                                                                     *
 *******************************************************************************************************
 * The Chipcon Hardware Abstraction Library is a collection of functions, macros and constants, which  *
 * can be used to ease access to the hardware on the CC2420 and the target microcontroller.            *
 *                                                                                                     *
 * This function contains a function for idle looping with millisecond resolution.                     *
 *******************************************************************************************************
 * Compiler: AVR-GCC                                                                                   *
 * Target platform: CC2420DB, CC2420 + any ATMEGA MCU                                                  *
 *******************************************************************************************************
 * Revision history:                                                                                   *
 * $Log: hal_wait.c,v $
 * Revision 1.1  2006/09/03 03:02:10  agr
 * added firefly2 platform dir
 *
 * Revision 1.2  2006/08/17 21:19:48  agr
 * Refactored Code... This was huge... MAJOR Problems could happen...
 *
 * Revision 1.1.1.1  2006/06/29 16:16:15  agr
 * Import from Source
 *
 * Revision 1.1.1.1  2006/02/02 04:41:48  agr
 * Import From Source
 *
 * Revision 1.1  2005/03/25 07:32:22  rtml
 * Added updated rf library with sample app.
 *
 * Revision 1.1.1.1  2005/03/23 18:50:51  agr
 * Import wispernet
 *
 * Revision 1.2  2004/03/30 14:59:50  mbr
 * Release for web
 *  
 *
 *
 *******************************************************************************************************/
#include <include.h>


//-------------------------------------------------------------------------------------------------------
//	void halWait(uint16_t timeout)
//
//	DESCRIPTION:
//		Runs an idle loop for [timeout] microseconds.
//
//  ARGUMENTS:
//      uint16_t timeout
//          The timeout in microseconds
//-------------------------------------------------------------------------------------------------------
void halWait(uint16_t timeout) {

    // This sequence uses exactly 8 clock cycle for each round
    do {
        NOP();
        NOP();
        NOP();
        NOP();
    } while (--timeout);

} // halWait
