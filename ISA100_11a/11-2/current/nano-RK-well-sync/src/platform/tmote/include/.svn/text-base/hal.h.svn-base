/******************************************************************************************************
*  TEXAS INSTRUMENTS INC.,                                                                            *
*  MSP430 APPLICATIONS.                                                                               *
*  Copyright Texas Instruments Inc, 2004                                                              *
 *****************************************************************************************************
*  This header file is for the MSP430_RF application.                                                 *
*  Compiler: IAR Workbench 3.0 or MSP430-GCC                                                                       *
 *****************************************************************************************************/

#ifndef HAL_H
#define HAL_H

// Enables SPI, selects "master", clock rate FCK / 2, and SPI mode 0
//TODO: implement for msp

typedef uint16_t NRK_STK;

#define SPI_INIT() \
	do { \
	} while (0)

/***********************************************************
	FAST SPI: Low level functions
***********************************************************/

#define	SPI_WAITFOREOTx()	while ((U1TCTL & TXEPT) == 0)  // USART1 Tx buffer ready? 
#define	SPI_WAITFOREORx()	while ((IFG2 & URXIFG1) == 0)  // USART1 Rx buffer ready?


#define FASTSPI_TX(x)\
	do {\
		U1TXBUF = x;\
		SPI_WAITFOREOTx();\
	} while(0)

#define FASTSPI_RX(x)\
    do {\
        U1TXBUF = 0;\
	    SPI_WAITFOREORx();\
		x = U1RXBUF;\
    } while(0)

#define FASTSPI_RX_GARBAGE()\
	do {\
    	U1TXBUF = 0;\
		SPI_WAITFOREORx();\
		U1RXBUF;\
	} while(0)

#define FASTSPI_TX_MANY(p,c)\
	do {\
		for (uint8_t spiCnt = 0; spiCnt < (c); spiCnt++) {\
			FASTSPI_TX(((uint8_t*)(p))[spiCnt]);\
		}\
	} while(0)


#define FASTSPI_RX_WORD(x)\
	 do {\
	    U1TXBUF = 0;\
        SPI_WAITFOREORx();\
		x = U1RXBUF << 8;\
	    U1TXBUF = 0;\
		SPI_WAITFOREORx();\
		x |= U1RXBUF;\
    } while (0)

#define FASTSPI_TX_ADDR(a)\
	 do {\
		  U1TXBUF = a;\
		  SPI_WAITFOREOTx();\
	 } while (0)

#define FASTSPI_RX_ADDR(a)\
	 do {\
		  U1TXBUF = (a) | 0x40;\
		  SPI_WAITFOREOTx();\
	 } while (0)



void halWait(uint16_t timeout);

#define ENABLE_GLOBAL_INT()         do { eint(); } while (0)
#define DISABLE_GLOBAL_INT()        do { dint(); } while (0)



//data register empty interrupt, receive complete interrupt
#define ENABLE_UART1_INT()          do { IE2 |= (BM(UTXIE1) | BM(URXIE1)); } while (0)
#define DISABLE_UART1_INT()         do { IE2 &= ~(BM(UTXIE1) | BM(URXIE1)); } while (0)
/*
#define ENABLE_UART1_INT()          do { UCSR1B |= (BM(UDRIE1) | BM(RXCIE1)); } while (0)
#define DISABLE_UART1_INT()         do { UCSR1B &= ~(BM(UDRIE1) | BM(RXCIE1)); } while (0)
*/

#define ENABLE_UART1_TX_INT()       do { IE2 |= BM(UTXIE1); } while (0)
#define DISABLE_UART1_TX_INT()      do { IE2 &= ~BM(UTXIE1); } while (0)
#define CLEAR_UART1_TX_INT()        do { IFG2 &= ~BM(UTXIFG1); } while (0)
#define SET_UART1_TX_INT()          do { IFG2 |= BM(UTXIFG1); } while (0)
/*
#define ENABLE_UART1_TX_INT()       do { UCSR1B |= BM(UDRIE1); } while (0)
#define DISABLE_UART1_TX_INT()      do { UCSR1B &= ~BM(UDRIE1); } while (0)
#define CLEAR_UART1_TX_INT()        do { UCSR1A &= ~BM(UDRE1); } while (0)
#define SET_UART1_TX_INT()          do { UCSR1A |= BM(UDRE1); } while (0)
*/

#define ENABLE_UART1_RX_INT()       do { IE2 |= BM(URXIE1); } while (0)
#define DISABLE_UART1_RX_INT()      do { IE2 &= ~BM(URXIE1); } while (0)
#define CLEAR_UART1_RX_INT()        do { IFG2 &= ~BM(URXIFG1); } while (0)
/*
#define ENABLE_UART1_RX_INT()       do { UCSR1B |= BM(RXCIE1); } while (0)
#define DISABLE_UART1_RX_INT()      do { UCSR1B &= ~BM(RXCIE1); } while (0)
#define CLEAR_UART1_RX_INT()        do { UCSR1A &= ~BM(RXC1); } while (0)
*/

#define ENABLE_UART0_TX_INT()       do { IE1 |= BM(UTXIE0); } while (0)
#define DISABLE_UART0_TX_INT()      do { IE1 &= ~BM(UTXIE0); } while (0)
#define CLEAR_UART0_TX_INT()        do { IFG1 &= ~BM(UTXIFG0); } while (0)
#define SET_UART0_TX_INT()          do { IFG1 |= BM(UTXIFG0); } while (0)
/*
#define ENABLE_UART0_TX_INT()       do { UCSR0B |= BM(UDRIE0); } while (0)
#define DISABLE_UART0_TX_INT()      do { UCSR0B &= ~BM(UDRIE0); } while (0)
#define CLEAR_UART0_TX_INT()        do { UCSR0A &= ~BM(UDRE0); } while (0)
#define SET_UART0_TX_INT()          do { UCSR0A |= BM(UDRE0); } while (0)
*/

#define ENABLE_UART0_RX_INT()       do { IE1 |= BM(URXIE0); } while (0)
#define DISABLE_UART0_RX_INT()      do { IE1 &= ~BM(URXIE0); } while (0)
#define CLEAR_UART0_RX_INT()        do { IFG1 &= ~BM(URXIFG0); } while (0)
/*
#define ENABLE_UART0_RX_INT()       do { UCSR0B |= BM(RXCIE0); } while (0)
#define DISABLE_UART0_RX_INT()      do { UCSR0B &= ~BM(RXCIE0); } while (0)
#define CLEAR_UART0_RX_INT()        do { UCSR0A &= ~BM(RXC0); } while (0)
*/

//TODO: implement for MSP
#define PWM0_INIT(period) \
    do { \
    } while(0)
/*
#define PWM0_INIT(period) \
    do { \
        OCR0 = 0; \
        TCCR0 = BM(WGM00) | BM(COM01) | BM(COM00); \
        PWM0_SET_PERIOD(period); \
    } while(0)
*/

// Sets the PWM period
//TODO: implement for MSP
#define PWM0_SET_PERIOD(period) do {  } while (0)
/*
#define PWM0_SET_PERIOD(period) do { TCCR0 = ((TCCR0 & ~0x07) | (period)); } while (0)
*/

// Period definitions for use with the PWM0_INIT and PWM0_SET_PERIOD macros
#define TIMER_CLK_STOP          0x00 /* Stop mode (the timer is not counting)     */
#define TIMER_CLK_DIV1          0x01 /* Total period = Clock freq / 256           */
#define TIMER_CLK_DIV8          0x02 /* Total period = Clock freq / (256 * 8)     */
#define TIMER_CLK_DIV64         0x03 /* Total period = Clock freq / (256 * 64)    */
#define TIMER_CLK_DIV256        0x04 /* Total period = Clock freq / (256 * 256)   */
#define TIMER_CLK_DIV1024       0x05 /* Total period = Clock freq / (256 * 1024)  */
#define TIMER_CLK_T_FALL        0x06 /* External Clock on T(x) pin (falling edge) */
#define TIMER_CLK_T_RISE        0x07 /* External Clock on T(x) pin (rising edge)  */

// Sets the PWM duty cycle
//TODO: implement for MSP
#define PWM0_SET_DUTY_CYCLE(dutyCycle) do {  } while (0)
/*
#define PWM0_SET_DUTY_CYCLE(dutyCycle) do { OCR0 = (dutyCycle); } while (0)
*/
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Timer/Counter0 interrupts

// Compare match interrupt
//TODO: implement for MSP
#define ENABLE_T0_COMPARE_INT()     do {  } while (0)
#define DISABLE_T0_COMPARE_INT()    do {  } while (0)
#define CLEAR_T0_COMPARE_INT()      do {  } while (0)
/*
#define ENABLE_T0_COMPARE_INT()     do { TIMSK |= BM(OCIE0); } while (0)
#define DISABLE_T0_COMPARE_INT()    do { TIMSK &= ~BM(OCIE0); } while (0)
#define CLEAR_T0_COMPARE_INT()      do { TIFR  &= ~BM(TOV0); } while (0)
*/
// Overflow interrupt
//TODO: implement for MSP
#define ENABLE_T0_OVERFLOW_INT()    do {  } while (0)
#define DISABLE_T0_OVERFLOW_INT()   do {  } while (0)
#define CLEAR_T0_OVERFLOW_INT()     do {  } while (0)
/*
#define ENABLE_T0_OVERFLOW_INT()    do { TIMSK |= BM(TOIE1); } while (0)
#define DISABLE_T0_OVERFLOW_INT()   do { TIMSK &= ~BM(TOIE1); } while (0)
#define CLEAR_T0_OVERFLOW_INT()     do { TIFR  &= ~BM(OCF0); } while (0)
*/
//-------------------------------------------------------------------------------------------------------




/*******************************************************************************************************
 *******************************************************************************************************
 **************************               SERIAL PORT (UART1)                 **************************
 *******************************************************************************************************
 *******************************************************************************************************/


//-------------------------------------------------------------------------------------------------------
//  INIT_UART1(baudRate,options)
//
//  DESCRIPTION:
//      A macro which does all the initialization necessary to communicate on UART 1. The UART is
//      configured according to options (defined below). Note that this macro does not call
//      ENABLE_UART1().
//
//  ARGUMENTS:
//      baudRate
//          One of the UART_BAUDRATE_... constants defined below
//      options
//          One or more of the UART_OPT constants defined below. The value 0 gives one stop bit, no
//          parity and 5 bits per char.
//-------------------------------------------------------------------------------------------------------
//TODO: implement for msp
#define INIT_UART1(baudRate,options) \
    do { \
    } while (0)

/*
#define INIT_UART1(baudRate,options) \
    do { \
        UBRR1H = (baudRate) >> 8; \
        UBRR1L = (baudRate); \
        UCSR1C = (uint8_t) options; \
        if (options > 0xFF) { \
            UCSR1B |= 0x04; \
        } else { \
            UCSR1B &= ~0x04; \
        } \
        UCSR1A |= BM(U2X1); \
    } while (0)
*/

//TODO: implement for msp
#define INIT_UART0(baudRate,options) \
    do { \
    } while (0)
/*
#define INIT_UART0(baudRate,options) \
    do { \
        UBRR0H = (baudRate) >> 8; \
        UBRR0L = (baudRate); \
        UCSR0C = (uint8_t) options; \
        if (options > 0xFF) { \
            UCSR0B |= 0x04; \
        } else { \
            UCSR0B &= ~0x04; \
        } \
        UCSR0A |= BM(U2X0); \
    } while (0)
*/


// Baud rate codes for use with the INIT_UART1 macro
#define UART_BAUDRATE_2K4           416 
#define UART_BAUDRATE_4K8           207 
#define UART_BAUDRATE_9K6           103 
#define UART_BAUDRATE_14K4          68   
#define UART_BAUDRATE_19K2          51 
#define UART_BAUDRATE_28K8          34   
#define UART_BAUDRATE_38K4          25 
#define UART_BAUDRATE_57K6          16 
#define UART_BAUDRATE_115K2         8
#define UART_BAUDRATE_230K4         3
#define UART_BAUDRATE_250K          3
//#define UART_BAUDRATE_500K          1   
//#define UART_BAUDRATE_1M            0
    
// Options for use with the INIT_UART1 macro
#define UART_OPT_ONE_STOP_BIT       0
#define UART_OPT_TWO_STOP_BITS      0x08
#define UART_OPT_NO_PARITY          0
#define UART_OPT_EVEN_PARITY        0x20
#define UART_OPT_ODD_PARITY         0x30
#define UART_OPT_5_BITS_PER_CHAR    0
#define UART_OPT_6_BITS_PER_CHAR    0x02
#define UART_OPT_7_BITS_PER_CHAR    0x04
#define UART_OPT_8_BITS_PER_CHAR    0x06
#define UART_OPT_9_BITS_PER_CHAR    0x0406
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Enable/disable macros

// Enable/disable UART1
//TODO: implement for msp
#define ENABLE_UART0()              () 
#define DISABLE_UART0()             ()
#define ENABLE_UART1()              () 
#define DISABLE_UART1()             ()
/*
#define ENABLE_UART0()              (UCSR0B |= (BM(RXEN0) | BM(TXEN0))) 
#define DISABLE_UART0()             (UCSR0B &= ~(BM(RXEN0) | BM(TXEN0)))
#define ENABLE_UART1()              (UCSR1B |= (BM(RXEN1) | BM(TXEN1))) 
#define DISABLE_UART1()             (UCSR1B &= ~(BM(RXEN1) | BM(TXEN1)))
*/
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Macros which are helful when transmitting and receiving data over the serial interface.
//
// Example of usage:
//      UART1_SEND(pData[0]);
//      for (i = 1; i < len; i++) {
//          UART1_WAIT_AND_SEND(pData[i]);
//      }

//TODO: implement for msp
#define UART1_WAIT()                do {  } while (0)

#define UART1_WAIT_RX()             do {  } while (0)

#define UART1_SEND(x)               do {  } while (0)

#define UART1_WAIT_AND_SEND(x)      do {  } while (0)

#define UART1_RECEIVE(x)            do {  } while (0)

#define UART1_WAIT_AND_RECEIVE(x)   do {  } while (0)

 
#define UART0_WAIT()                do {  } while (0)

#define UART0_WAIT_RX()             do {  } while (0)

#define UART0_SEND(x)               do {  } while (0)

#define UART0_WAIT_AND_SEND(x)      do {  } while (0)

#define UART0_RECEIVE(x)            do {  } while (0)

#define UART0_WAIT_AND_RECEIVE(x)   do {  } while (0)

            

/*
#define UART1_WAIT()                do { while (!(UCSR1A & BM(UDRE1))); } while (0)

#define UART1_WAIT_RX()             do { while (!(UCSR1A & BM(RXC1))); CLEAR_UART1_RX_INT(); } while (0)

#define UART1_SEND(x)               do { UDR1 = (x); } while (0)

#define UART1_WAIT_AND_SEND(x)      do { UART1_WAIT(); UART1_SEND(x); } while (0)

#define UART1_RECEIVE(x)            do { (x) = UDR1; } while (0)

#define UART1_WAIT_AND_RECEIVE(x)   do { UART1_WAIT_RX(); UART1_RECEIVE(x); } while (0)

 
#define UART0_WAIT()                do { while (!(UCSR0A & BM(UDRE0))); CLEAR_UART0_TX_INT(); } while (0)

#define UART0_WAIT_RX()             do { while (!(UCSR0A & BM(RXC0))); CLEAR_UART0_RX_INT(); } while (0)

#define UART0_SEND(x)               do { UDR0 = (x); } while (0)

#define UART0_WAIT_AND_SEND(x)      do { UART0_WAIT(); UART0_SEND(x); } while (0)

#define UART0_RECEIVE(x)            do { (x) = UDR0; } while (0)

#define UART0_WAIT_AND_RECEIVE(x)   do { UART0_WAIT_RX(); UART0_RECEIVE(x); } while (0)
*/


/*******************************************************************************************************
 *******************************************************************************************************
 **************************                   Timter / Counter 0              **************************
 *******************************************************************************************************
 *******************************************************************************************************/

#define TIMER0_OFF              0
#define TIMER0_PRESCALE_1       1
#define TIMER0_PRESCALE_8       2
#define TIMER0_PRESCALE_32      3
#define TIMER0_PRESCALE_64      4
#define TIMER0_PRESCALE_128     5
#define TIMER0_PRESCALE_256     6
#define TIMER0_PRESCALE_1024    7

#define TIMER0_WGM_0            0x00
#define TIMER0_WGM_1            0x40
#define TIMER0_WGM_2            0x08
#define TIMER0_WGM_3            0x48

#define TIMER0_COM_0            0x00
#define TIMER0_COM_1            0x08
#define TIMER0_COM_2            0x10
#define TIMER0_COM_3            0x18
#define TIMER0_COM_4            0x20
#define TIMER0_COM_5            0x28
#define TIMER0_COM_6            0x30
#define TIMER0_COM_7            0x38

//TODO: implement for msp
#define TIMER0_SET_CONTROL(x)           do {  } while (0)

#define TIMER0_SET_VALUE(x)             do {  } while (0)
#define TIMER0_READ_VALUE(x)            do {  } while (0)

#define TIMER0_SET_COMPARE_VALUE(x)     do {  } while (0)
#define TIMER0_READ_COMPARE_VALUE(x)    do {  } while (0)

/*
#define TIMER0_SET_CONTROL(x)           do {TCCR0 = (x); } while (0)

#define TIMER0_SET_VALUE(x)             do {TCNT0 = (x); } while (0)
#define TIMER0_READ_VALUE(x)            do {(x) = TCNT0; } while (0)

#define TIMER0_SET_COMPARE_VALUE(x)     do {OCR0 = (x); } while (0)
#define TIMER0_READ_COMPARE_VALUE(x)    do {(x) = OCR0; } while (0)
*/

/*******************************************************************************************************
 *******************************************************************************************************
 **************************                   USEFUL STUFF                    **************************
 *******************************************************************************************************
 *******************************************************************************************************/


//-------------------------------------------------------------------------------------------------------
// Useful stuff
#define NOP() asm volatile ("nop\n\t" ::)
//-------------------------------------------------------------------------------------------------------

#endif
