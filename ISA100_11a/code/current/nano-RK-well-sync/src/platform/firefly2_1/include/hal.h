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
*  Chipcon Development Team (mbr)
*  Anthony Rowe
*******************************************************************************/

#ifndef HAL_H
#define HAL_H

typedef uint8_t   NRK_STK;                   // Each stack entry is 8-bit wide

/*******************************************************************************************************
 *******************************************************************************************************
 **************************            AVR<->CC2420 SPI INTERFACE             **************************
 *******************************************************************************************************
 *******************************************************************************************************/


//-------------------------------------------------------------------------------------------------------
// Initialization

// Enables SPI, selects "master", clock rate FCK / 2, and SPI mode 0
#define SPI_INIT() \
    do { \
        SPCR = BM(SPE) | BM(MSTR); \
        SPSR = BM(SPI2X); \
    } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// FAST SPI: Low level functions
//      x = value (uint8_t or uint16_t)
//      p = pointer to the byte array to operate on
//      c = the byte count
//
// SPI_ENABLE() and SPI_DISABLE() are located in the devboard header file (CS_N uses GPIO)

#define FASTSPI_WAIT() \
    do { \
        while (!(SPSR & BM(SPIF))); \
    } while (0) 

#define FASTSPI_TX(x) \
    do { \
        SPDR = x; \
        FASTSPI_WAIT(); \
    } while (0)

#define FASTSPI_RX(x) \
    do { \
        SPDR = 0; \
        FASTSPI_WAIT(); \
        x = SPDR; \
    } while (0)

#define FASTSPI_RX_GARBAGE() \
    do { \
        SPDR = 0; \
        FASTSPI_WAIT(); \
    } while (0)

#define FASTSPI_TX_WORD_LE(x) \
    do { \
        FASTSPI_TX(x); \
        FASTSPI_TX((x) >> 8); \
    } while (0)
    
#define FASTSPI_TX_WORD(x) \
    do { \
        FASTSPI_TX(((uint16_t)(x)) >> 8); \
        FASTSPI_TX((uint8_t)(x)); \
    } while (0)
    
#define FASTSPI_TX_MANY(p,c) \
    do { \
        for (uint8_t spiCnt = 0; spiCnt < (c); spiCnt++) { \
            FASTSPI_TX(((uint8_t*)(p))[spiCnt]); \
        } \
    } while (0)
        
#define FASTSPI_RX_WORD_LE(x) \
    do { \
        SPDR = 0; \
        FASTSPI_WAIT(); \
        x = SPDR; \
        SPDR = 0; \
        FASTSPI_WAIT(); \
        x |= SPDR << 8; \
    } while (0)

#define FASTSPI_RX_WORD(x) \
    do { \
        SPDR = 0; \
        FASTSPI_WAIT(); \
        x = SPDR << 8; \
        SPDR = 0; \
        FASTSPI_WAIT(); \
        x |= SPDR; \
    } while (0)
    
#define FASTSPI_RX_MANY(p,c) \
    do { \
        for (uint8_t spiCnt = 0; spiCnt < (c); spiCnt++) { \
            FASTSPI_RX((p)[spiCnt]); \
        } \
    } while (0)
        
// Register address:
#define FASTSPI_TX_ADDR(a) \
    do { \
        SPDR = a; \
        FASTSPI_WAIT(); \
    } while (0)

// Register address:
#define FASTSPI_RX_ADDR(a) \
    do { \
        SPDR = (a) | 0x40; \
        FASTSPI_WAIT(); \
    } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
//  FAST SPI: Register access
//      s = command strobe
//      a = register address
//      v = register value

#define FASTSPI_STROBE(s) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX_ADDR(s); \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_SETREG(a,v) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX_ADDR(a); \
        FASTSPI_TX((uint8_t) ((v) >> 8)); \
        FASTSPI_TX((uint8_t) (v)); \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_GETREG(a,v) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_RX_ADDR(a); \
        FASTSPI_RX_WORD(v); \
        SPI_DISABLE(); \
    } while (0)

// Updates the SPI status byte
#define FASTSPI_UPD_STATUS(s) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX_ADDR(CC2420_SNOP); \
        s = SPDR; \
        SPI_DISABLE(); \
    } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
//  FAST SPI: FIFO access
//      p = pointer to the byte array to be read/written
//      c = the number of bytes to read/write
//      b = single data byte

#define FASTSPI_WRITE_FIFO(p,c) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX_ADDR(CC2420_TXFIFO); \
        for (uint8_t spiCnt = 0; spiCnt < (c); spiCnt++) { \
            FASTSPI_TX(((uint8_t*)(p))[spiCnt]); \
        } \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_FIFO(p,c) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_RX_ADDR(CC2420_RXFIFO); \
        for (uint8_t spiCnt = 0; spiCnt < (c); spiCnt++) { \
            while (!FIFO_IS_1); \
            FASTSPI_RX(((uint8_t*)(p))[spiCnt]); \
        } \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_FIFO_BYTE(b) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_RX_ADDR(CC2420_RXFIFO); \
        FASTSPI_RX(b); \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_FIFO_NO_WAIT(p,c) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_RX_ADDR(CC2420_RXFIFO); \
        for (uint8_t spiCnt = 0; spiCnt < (c); spiCnt++) { \
            FASTSPI_RX(((uint8_t*)(p))[spiCnt]); \
        } \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_FIFO_GARBAGE(c) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_RX_ADDR(CC2420_RXFIFO); \
        for (uint8_t spiCnt = 0; ((spiCnt < (c)) && (FIFO_IS_1)); spiCnt++) { \
            FASTSPI_RX_GARBAGE(); \
        } \
        SPI_DISABLE(); \
    } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
//  FAST SPI: CC2420 RAM access (big or little-endian order)
//      p = pointer to the variable to be written
//      a = the CC2420 RAM address
//      c = the number of bytes to write
//      n = counter variable which is used in for/while loops (uint8_t)
//
//  Example of usage:
//      uint8_t n;
//      uint16_t shortAddress = 0xBEEF;
//      FASTSPI_WRITE_RAM_LE(&shortAddress, CC2420RAM_SHORTADDR, 2);

#define FASTSPI_WRITE_RAM_LE(p,a,c,n) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX(0x80 | (a & 0x7F)); \
        FASTSPI_TX((a >> 1) & 0xC0); \
        for (n = 0; n < (c); n++) { \
            FASTSPI_TX(((uint8_t*)(p))[n]); \
        } \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_RAM_LE(p,a,c,n) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX(0x80 | (a & 0x7F)); \
        FASTSPI_TX(((a >> 1) & 0xC0) | 0x20); \
        for (n = 0; n < (c); n++) { \
            FASTSPI_RX(((uint8_t*)(p))[n]); \
        } \
        SPI_DISABLE(); \
    } while (0)
    
#define FASTSPI_WRITE_RAM(p,a,c,n) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX(0x80 | (a & 0x7F)); \
        FASTSPI_TX((a >> 1) & 0xC0); \
        n = c; \
        do { \
            FASTSPI_TX(((uint8_t*)(p))[--n]); \
        } while (n); \
        SPI_DISABLE(); \
    } while (0)

#define FASTSPI_READ_RAM(p,a,c,n) \
    do { \
        SPI_ENABLE(); \
        FASTSPI_TX(0x80 | (a & 0x7F)); \
        FASTSPI_TX(((a >> 1) & 0xC0) | 0x20); \
        n = c; \
        do { \
            FASTSPI_RX(((uint8_t*)(p))[--n]); \
        } while (n); \
        SPI_DISABLE(); \
    } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Other useful SPI macros
#define FASTSPI_RESET_CC2420() \
    do { \
        FASTSPI_SETREG(CC2420_MAIN, 0x0000); \
        FASTSPI_SETREG(CC2420_MAIN, 0xF800); \
    } while (0)
//-------------------------------------------------------------------------------------------------------




/*******************************************************************************************************
 *******************************************************************************************************
 **************************                    INTERRUPTS                     **************************
 *******************************************************************************************************
 *******************************************************************************************************/


//-------------------------------------------------------------------------------------------------------
// General
#define ENABLE_GLOBAL_INT()         do { asm ("sei\n\t" ::); } while (0)
#define DISABLE_GLOBAL_INT()        do { asm ("cli\n\t" ::); } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// UART1 interrupts
#define ENABLE_UART1_INT()          do { UCSR1B |= (BM(UDRIE1) | BM(RXCIE1)); } while (0)
#define DISABLE_UART1_INT()         do { UCSR1B &= ~(BM(UDRIE1) | BM(RXCIE1)); } while (0) 

#define ENABLE_UART1_TX_INT()       do { UCSR1B |= BM(UDRIE1); } while (0)
#define DISABLE_UART1_TX_INT()      do { UCSR1B &= ~BM(UDRIE1); } while (0) 
#define CLEAR_UART1_TX_INT()        do { UCSR1A &= ~BM(UDRE1); } while (0)
#define SET_UART1_TX_INT()          do { UCSR1A |= BM(UDRE1); } while (0)

#define ENABLE_UART1_RX_INT()       do { UCSR1B |= BM(RXCIE1); } while (0)
#define DISABLE_UART1_RX_INT()      do { UCSR1B &= ~BM(RXCIE1); } while (0) 
#define CLEAR_UART1_RX_INT()        do { UCSR1A &= ~BM(RXC1); } while (0)







#define ENABLE_UART0_TX_INT()       do { UCSR0B |= BM(UDRIE0); } while (0)
#define DISABLE_UART0_TX_INT()      do { UCSR0B &= ~BM(UDRIE0); } while (0) 
#define CLEAR_UART0_TX_INT()        do { UCSR0A &= ~BM(UDRE0); } while (0)
#define SET_UART0_TX_INT()          do { UCSR0A |= BM(UDRE0); } while (0)

#define ENABLE_UART0_RX_INT()       do { UCSR0B |= BM(RXCIE0); } while (0)
#define DISABLE_UART0_RX_INT()      do { UCSR0B &= ~BM(RXCIE0); } while (0) 
#define CLEAR_UART0_RX_INT()        do { UCSR0A &= ~BM(RXC0); } while (0)


//-------------------------------------------------------------------------------------------------------




/*******************************************************************************************************
 *******************************************************************************************************
 **************************                         ADC                       **************************
 *******************************************************************************************************
 *******************************************************************************************************/


//-------------------------------------------------------------------------------------------------------
// ADC initialization
#define ADC_INIT() \
    do { \
        ADCSRA = BM(ADPS0) | BM(ADPS1) | BM(ADFR); \
        ADMUX = BM(REFS0); \
    } while (0)

/*
#define ADC_INIT() \
    do { \
        ADCSRA = BM(ADPS0) | BM(ADPS1) | BM(ADIF); \
        ADMUX = BM(REFS0); \
    } while (0)
*/

// Selects which ADC channel to use. The channels (0-3) are defined in the development board definition
// files, e.g. hal_cc2420db.h, as ADC_INPUT_...
#define ADC_SET_CHANNEL(channel) do { ADMUX = (ADMUX & ~0x1F) | (channel); } while (0)

// Enables/disables the ADC
#define ADC_ENABLE() do { ADCSRA |= BM(ADEN); } while (0)
#define ADC_DISABLE() do { ADCSRA &= ~BM(ADEN); } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// ADC sampling

// Macro for taking a single sample in single-conversion mode (not required in continuous mode)
#define ADC_SAMPLE_SINGLE() \
    do { \
        ADCSRA |= BM(ADSC); \
        while (!(ADCSRA & 0x10)); \
    } while(0)

// Macros for obtaining the latest sample value
#define ADC_GET_SAMPLE_10(x) \
    do { \
        x =  ADCL; \
        x |= ADCH << 8; \
    } while (0)

#define ADC_GET_SAMPLE_8(x) \
    do { \
        x = ((uint8_t) ADCL) >> 2; \
        x |= ((int8_t) ADCH) << 6; \
    } while (0)
//-------------------------------------------------------------------------------------------------------




/*******************************************************************************************************
 *******************************************************************************************************
 **************************           Timer / Pulse Width Modulator           **************************
 *******************************************************************************************************
 *******************************************************************************************************/


//-------------------------------------------------------------------------------------------------------
// Pulse width modulator (PWM) using timer 0 (8 bits)
//
// Example of usage:
//     // Initialize
//     PWM_INIT(TIMER_CLK_DIV1);
//
//     // Increase level gradually (over approx 2.5 seconds)
//     for (uint8_t n; n < 255; n++) {
//         PWM0_SET_DUTY_CYCLE(n);
//         halWait(10000);
//     }

// Initialization
#define PWM0_INIT(period) \
    do { \
        OCR0 = 0; \
        TCCR0 = BM(WGM00) | BM(COM01) | BM(COM00); \
        PWM0_SET_PERIOD(period); \
    } while(0)

// Sets the PWM period
#define PWM0_SET_PERIOD(period) do { TCCR0 = ((TCCR0 & ~0x07) | (period)); } while (0)

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
#define PWM0_SET_DUTY_CYCLE(dutyCycle) do { OCR0 = (dutyCycle); } while (0)
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Timer/Counter0 interrupts

// Compare match interrupt
#define ENABLE_T0_COMPARE_INT()     do { TIMSK |= BM(OCIE0); } while (0)
#define DISABLE_T0_COMPARE_INT()    do { TIMSK &= ~BM(OCIE0); } while (0)
#define CLEAR_T0_COMPARE_INT()      do { TIFR  &= ~BM(TOV0); } while (0)
// Overflow interrupt
#define ENABLE_T0_OVERFLOW_INT()    do { TIMSK |= BM(TOIE1); } while (0)
#define DISABLE_T0_OVERFLOW_INT()   do { TIMSK &= ~BM(TOIE1); } while (0)
#define CLEAR_T0_OVERFLOW_INT()     do { TIFR  &= ~BM(OCF0)); } while (0)
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
#define ENABLE_UART0()              (UCSR0B |= (BM(RXEN0) | BM(TXEN0))) 
#define DISABLE_UART0()             (UCSR0B &= ~(BM(RXEN0) | BM(TXEN0)))
#define ENABLE_UART1()              (UCSR1B |= (BM(RXEN1) | BM(TXEN1))) 
#define DISABLE_UART1()             (UCSR1B &= ~(BM(RXEN1) | BM(TXEN1)))
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
// Macros which are helful when transmitting and receiving data over the serial interface.
//
// Example of usage:
//      UART1_SEND(pData[0]);
//      for (i = 1; i < len; i++) {
//          UART1_WAIT_AND_SEND(pData[i]);
//      }

#define UART1_WAIT()                do { while (!(UCSR1A & BM(UDRE1))); /*CLEAR_UART1_TX_INT();*/ } while (0)

#define UART1_WAIT_RX()             do { while (!(UCSR1A & BM(RXC1))); CLEAR_UART1_RX_INT(); } while (0)

#define UART1_SEND(x)               do { UDR1 = (x); } while (0)

#define UART1_WAIT_AND_SEND(x)      do { UART1_WAIT(); UART1_SEND(x); } while (0)

#define UART1_RECEIVE(x)            do { (x) = UDR1; } while (0)

#define UART1_WAIT_AND_RECEIVE(x)   do { /*UDR1 = 0;*/ UART1_WAIT_RX(); UART1_RECEIVE(x); } while (0)

 
#define UART0_WAIT()                do { while (!(UCSR0A & BM(UDRE0))); CLEAR_UART0_TX_INT(); } while (0)

#define UART0_WAIT_RX()             do { while (!(UCSR0A & BM(RXC0))); CLEAR_UART0_RX_INT(); } while (0)

#define UART0_SEND(x)               do { UDR0 = (x); } while (0)

#define UART0_WAIT_AND_SEND(x)      do { UART0_WAIT(); UART0_SEND(x); } while (0)

#define UART0_RECEIVE(x)            do { (x) = UDR0; } while (0)

#define UART0_WAIT_AND_RECEIVE(x)   do { /*UDR1 = 0;*/ UART0_WAIT_RX(); UART0_RECEIVE(x); } while (0)

            

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

#define TIMER0_SET_CONTROL(x)           do {TCCR0 = (x); } while (0)

#define TIMER0_SET_VALUE(x)             do {TCNT0 = (x); } while (0)
#define TIMER0_READ_VALUE(x)            do {(x) = TCNT0; } while (0)

#define TIMER0_SET_COMPARE_VALUE(x)     do {OCR0 = (x); } while (0)
#define TIMER0_READ_COMPARE_VALUE(x)    do {(x) = OCR0; } while (0)

/*******************************************************************************************************
 *******************************************************************************************************
 **************************                   USEFUL STUFF                    **************************
 *******************************************************************************************************
 *******************************************************************************************************/


//-------------------------------------------------------------------------------------------------------
// Useful stuff
#define NOP() asm volatile ("nop\n\t" ::)
//-------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------
//  void halWait(uint16_t timeout)
//
//  DESCRIPTION:
//      Runs an idle loop for [timeout] microseconds.
//
//  ARGUMENTS:
//      uint16_t timeout
//          The timeout in microseconds
//-------------------------------------------------------------------------------------------------------
void halWait(uint16_t timeout);




/*******************************************************************************************************
 *******************************************************************************************************
 **************************              SIMPLE CC2420 FUNCTIONS              **************************
 *******************************************************************************************************
 *******************************************************************************************************/

//-------------------------------------------------------------------------------------------------------
//  Example of usage: Starts RX on channel 14 after reset
//      FASTSPI_RESET_CC2420();
//      FASTSPI_STROBE(CC2420_SXOSCON);
//      halRfSetChannel(14);
//      ... other registers can for instance be initialized here ...
//      halRfWaitForCrystalOscillator();
//      ... RAM access can be done here, since the crystal oscillator must be on and stable ...
//      FASTSPI_STROBE(CC2420_SRXON);
//-------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------
//  void rfWaitForCrystalOscillator(void)
//
//  DESCRIPTION:
//      Waits for the crystal oscillator to become stable. The flag is polled via the SPI status byte.
//
//      Note that this function will lock up if the SXOSCON command strobe has not been given before the
//      function call. Also note that global interrupts will always be enabled when this function
//      returns.
//-------------------------------------------------------------------------------------------------------
void halRfWaitForCrystalOscillator(void);


//-------------------------------------------------------------------------------------------------------
//  void halRfSetChannel(uint8_t Channel)
//
//  DESCRIPTION:
//      Programs CC2420 for a given IEEE 802.15.4 channel.
//      Note that SRXON, STXON or STXONCCA must be run for the new channel selection to take full effect.
//
//  PARAMETERS:
//      uint8_t channel
//          The channel number (11-26)
//-------------------------------------------------------------------------------------------------------
void halRfSetChannel(uint8_t channel);

#endif
