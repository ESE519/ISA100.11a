#ifndef INCLUDE_H
#define INCLUDE_H


// FIXME: Need to convert for MSP!

// Just pass the string
#define PSTR(x) x

//-------------------------------------------------------------------------------------------------------
// Common data types
//typedef uint8_t		bool;
/*typedef unsigned char		bool;

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned long		DWORD;
typedef unsigned long long	QWORD;

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned long		uint32_t;
typedef unsigned long long	UINT64;

typedef signed char			int8_t;
typedef signed short		INT16;
typedef signed long			int32_t;
typedef signed long long	INT64;
*/
// Common values
#ifndef FALSE
	#define FALSE 0
#endif
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef NULL
	#define NULL 0
#endif

// Useful stuff
#define BM(n) (1 << (n))
#define BF(x,b,s) (((x) & (b)) >> (s))
#define MIN(n,m) (((n) < (m)) ? (n) : (m))
#define MAX(n,m) (((n) < (m)) ? (m) : (n))
#define ABS(n) ((n < 0) ? -(n) : (n))

// Dynamic function pointer
typedef void (*VFPTR)(void);
//-----------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------
// Standard GCC include files for AVR

//TODO: find msp equivlants of these:
//#include <avr/io.h>
//#include <avr/pgmspace.h>
//#include <avr/signal.h>
#include <io.h>
#include <signal.h>
#include <msp430x16x.h>
#include <string.h>


//TODO: make this the correct value for msp
#define RAMEND 0x09ff


#ifdef STK501
    //#include "hal/atmega128/hal_stk501.h"
#endif

#ifdef CC2420DB
    //#include "hal/atmega128/hal_cc2420db.h"
#endif

// HAL include files
#include <hal.h>
#include <hal_imec.h>
//#include <nrk_pin_define.h>
#include <basic_rf.h>
//-----------------------------------------------------------------------------------------------

#endif
