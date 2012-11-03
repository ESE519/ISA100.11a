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
*  Anthony Rowe
*  Nuno Pereira
*  Chipcon Development Team 
*******************************************************************************/


#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <hal_micaZ.h>
#include <avr/interrupt.h>
#include <nrk_pin_define.h>
#include <nrk_error.h>
#include <nrk_events.h>

#ifdef NANORK
#include <nrk_cfg.h>
#endif

#ifdef NRK_UART_BUF
#include <nrk_events.h>

#ifndef MAX_RX_UART_BUF
#define MAX_RX_UART_BUF    16
#endif


uint8_t uart_rx_buf_start,uart_rx_buf_end;
char uart_rx_buf[MAX_RX_UART_BUF];
nrk_sig_t uart_rx_signal;

SIGNAL(USART0_RX_vect)
{
char c;
uint8_t sig;
   DISABLE_UART0_RX_INT();
   UART0_WAIT_AND_RECEIVE(c);
   uart_rx_buf[uart_rx_buf_end]=c;
   //if(uart_rx_buf_end==uart_rx_buf_start) sig=1; else sig=0;
   uart_rx_buf_end++;
   if(uart_rx_buf_end==MAX_RX_UART_BUF) uart_rx_buf_end=0;
   nrk_event_signal(uart_rx_signal);
CLEAR_UART0_RX_INT();
ENABLE_UART0_RX_INT();
}

char getc0()
{
char tmp;

if(uart_rx_buf_start!=uart_rx_buf_end)
   {
   tmp=uart_rx_buf[uart_rx_buf_start];
   uart_rx_buf_start++;
   if(uart_rx_buf_start==MAX_RX_UART_BUF) uart_rx_buf_start=0;
   return(tmp);
   }
// if buffer empty, then we have to block for it
UART0_WAIT_AND_RECEIVE(tmp);
return tmp;
}

uint8_t nrk_uart_data_ready(uint8_t uart_num)
{
if(uart_num==0)
	{
	if(uart_rx_buf_start!=uart_rx_buf_end) return 1;
	}
if(uart_num==1)
        {
        if( UCSR1A & BM(RXC1) ) return 1;
        }
return 0;
}

nrk_sig_t nrk_uart_rx_signal_get()
{
   if(uart_rx_signal==NRK_ERROR) nrk_error_add(NRK_SIGNAL_CREATE_ERROR);
   return uart_rx_signal;
}

#else

nrk_sig_t nrk_uart_rx_signal_get()
{
   return NRK_ERROR;
}


uint8_t nrk_uart_data_ready(uint8_t uart_num)
{
if(uart_num==0)
        {
        if( UCSR0A & BM(RXC0) ) return 1;
        }
if(uart_num==1)
        {
        if( UCSR1A & BM(RXC1) ) return 1;
        }
return 0;
}

char getc0(void){
        unsigned char tmp;
        UART0_WAIT_AND_RECEIVE(tmp);
        return tmp;
}

#endif

void nrk_kprintf( const char *addr)
{
 char c;
   while((c=pgm_read_byte(addr++)))
        putchar(c);
}


void nrk_setup_ports()
{
PORT_INIT();
SPI_INIT();
}




//---------------------------------------------------------------------------------------------
// GPIO related definitions

//-------------------------------
// Define nrk pins mappings to hardware pins and ports


//-------------------------------
// Port A
NRK_PIN( LED_0, LED_0, NRK_PORTA )
NRK_PIN( LED_1, LED_1, NRK_PORTA )
NRK_PIN( LED_2, LED_2, NRK_PORTA )
NRK_PIN( VREG_EN, VREG_EN, NRK_PORTA )
NRK_PIN( RESET_N, RESET_N, NRK_PORTA )

//-------------------------------
// Port B
NRK_PIN( CSN, CSN, NRK_PORTB )
NRK_PIN( SCK, SCK, NRK_PORTB )
NRK_PIN( MOSI, MOSI, NRK_PORTB )
NRK_PIN( MISO, MISO, NRK_PORTB )
NRK_PIN( FIFO, FIFO, NRK_PORTB )

//-------------------------------
// Port D
NRK_PIN( UART1_RXD, UART1_RXD, NRK_PORTD )
NRK_PIN( UART1_TXD, UART1_TXD, NRK_PORTD )
NRK_PIN( SFD, SFD, NRK_PORTD )
NRK_PIN( UART1_RTS, UART1_RTS, NRK_PORTD )
NRK_PIN( CCA, CCA, NRK_PORTD )
NRK_PIN( UART1_CTS, UART1_CTS, NRK_PORTD )
//-------------------------------
// Port E
NRK_PIN( UART0_RXD, UART0_RXD, NRK_PORTE ) 
NRK_PIN( UART0_TXD, UART0_TXD, NRK_PORTE )  
NRK_PIN( FIFOP, FIFOP, NRK_PORTE ) 
//-------------------------------
// Port F
// ADC inputs
NRK_PIN( ADC_INPUT_0, ADC_INPUT_0, NRK_PORTF ) 
NRK_PIN( ADC_INPUT_1, ADC_INPUT_1, NRK_PORTF ) 
NRK_PIN( ADC_INPUT_2, ADC_INPUT_2, NRK_PORTF ) 
NRK_PIN( ADC_INPUT_3, ADC_INPUT_3, NRK_PORTF ) 

NRK_PIN( ADC_INPUT_6, 6, NRK_PORTF ) 
NRK_PIN( ADC_INPUT_7, 7, NRK_PORTF ) 

NRK_PIN( DEBUG_0, DEBUG_0, NRK_PORTC )
NRK_PIN( DEBUG_1, DEBUG_1, NRK_PORTC )

NRK_INVALID_PIN( DEBUG_2 )
NRK_INVALID_PIN( DEBUG_3 )

// pins that do not exist in this platform
//NRK_INVALID_PIN( DEBUG_0 )
//NRK_INVALID_PIN( DEBUG_1 )
NRK_INVALID_PIN( LED_3 )
NRK_INVALID_PIN( BLED )
NRK_INVALID_PIN( SPI_SS )
NRK_INVALID_PIN( GPIO26 )
NRK_INVALID_PIN( GPIO28 )
NRK_INVALID_PIN( GPIO34 )

//-------------------------------
// GPIO handling functions
int8_t nrk_gpio_set(uint8_t pin)
{
        if (pin == NRK_INVALID_PIN_VAL) return -1;
        switch (pin & 0x07) {
                case NRK_PORTA:
                        do { PORTA |= BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTB:
                        do { PORTB |= BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTC:
                        do { PORTC |= BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTD:
                        do { PORTD |= BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTE:
                        do { PORTE |= BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTF:
                        do { PORTF |= BM((pin & 0xF8) >> 3); } while(0); break; 
                default: return -1;
        }
        return 1;
}

int8_t nrk_gpio_clr(uint8_t pin)
{
        if (pin == NRK_INVALID_PIN_VAL) return -1;
        switch (pin & 0x07) {
                case NRK_PORTA:
                        do { PORTA &= ~BM((pin & 0xF8) >> 3); } while(0); break;
                case NRK_PORTB:
                        do { PORTB &= ~BM((pin & 0xF8) >> 3); } while(0); break;
                case NRK_PORTC:
                        do { PORTC &= ~BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTD:
                        do { PORTD &= ~BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTE:
                        do { PORTE &= ~BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTF:
                        do { PORTF &= ~BM((pin & 0xF8) >> 3); } while(0); break; 
                default: return -1;
        }
        return 1;
}

int8_t nrk_gpio_get(uint8_t pin)
{
    if (pin == NRK_INVALID_PIN_VAL) return -1;
        switch (pin & 0x07) {
                case NRK_PORTA:
                        return !!(PINA & BM((pin & 0xF8) >> 3));
                case NRK_PORTB:
                        return !!(PINB & BM((pin & 0xF8) >> 3));
                case NRK_PORTC:
                        return !!(PINC & BM((pin & 0xF8) >> 3));
                case NRK_PORTD:
                        return !!(PIND & BM((pin & 0xF8) >> 3));
                case NRK_PORTE:
                        return !!(PINE & BM((pin & 0xF8) >> 3));
                case NRK_PORTF:
                        return !!(PINF & BM((pin & 0xF8) >> 3));
                default: return -1;
        }
        return 1;
}

int8_t nrk_gpio_pullups(uint8_t enable)
{
if(enable) SFIOR &= ~BM(PUD);
else SFIOR |= BM(PUD);
return NRK_OK;
}

int8_t nrk_gpio_toggle(uint8_t pin)
{
    if (pin == NRK_INVALID_PIN_VAL) return -1;
        switch (pin & 0x07) {
                case NRK_PORTA:
                        if ((PINA & BM((pin & 0xF8) >> 3)) == 0) {
                                do { PORTA |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { PORTA &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                case NRK_PORTB:
                        if ((PINB & BM((pin & 0xF8) >> 3)) == 0) {
                                do { PORTB |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { PORTB &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                case NRK_PORTC:
                        if ((PINC & BM((pin & 0xF8) >> 3)) == 0) {
                                do { PORTC |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { PORTC &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                case NRK_PORTD:
                        if ((PIND & BM((pin & 0xF8) >> 3)) == 0) {
                                do { PORTD |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { PORTD &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                case NRK_PORTE:
                        if ((PINE & BM((pin & 0xF8) >> 3)) == 0) {
                                do { PORTE |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { PORTE &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                case NRK_PORTF:
                        if ((PINF & BM((pin & 0xF8) >> 3)) == 0) {
                                do { PORTF |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { PORTF &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                default: return -1;
        }
        return 1;
}

int8_t nrk_gpio_direction(uint8_t pin, uint8_t pin_direction)
{
        if (pin == NRK_INVALID_PIN_VAL) return -1;
        if (pin_direction == NRK_PIN_INPUT) {
                switch (pin & 0x07) {
                        case NRK_PORTA:
                                DDRA &= ~BM((pin & 0xF8) >> 3); 
                                PORTA |= BM((pin & 0xF8) >> 3); 
                                break;
                        case NRK_PORTB:
                                DDRB &= ~BM((pin & 0xF8) >> 3); 
                                PORTB |= BM((pin & 0xF8) >> 3); 
                                break;
                        case NRK_PORTC:
                                DDRC &= ~BM((pin & 0xF8) >> 3); 
                                PORTB |= BM((pin & 0xF8) >> 3); 
                                break;
                        case NRK_PORTD:
                                DDRD &= ~BM((pin & 0xF8) >> 3); 
                                PORTB |= BM((pin & 0xF8) >> 3); 
                                break;
                        case NRK_PORTE:
                                DDRE &= ~BM((pin & 0xF8) >> 3); 
                                PORTB |= BM((pin & 0xF8) >> 3); 
                                break;
                        case NRK_PORTF:
                                DDRF &= ~BM((pin & 0xF8) >> 3);
                                PORTB |= BM((pin & 0xF8) >> 3); 
                                break;
                        default: return -1;
                }
        } else {
                switch (pin & 0x07) {
                        case NRK_PORTA:
                                DDRA |= BM((pin & 0xF8) >> 3); break;
                        case NRK_PORTB:
                                DDRB |= BM((pin & 0xF8) >> 3); break;
                        case NRK_PORTC:
                                DDRC |= BM((pin & 0xF8) >> 3); break;
                        case NRK_PORTD:
                                DDRD |= BM((pin & 0xF8) >> 3); break;
                        case NRK_PORTE:
                                DDRE |= BM((pin & 0xF8) >> 3); break;
                        case NRK_PORTF:
                                DDRF |= BM((pin & 0xF8) >> 3); break;
                        default: return -1;
                }
        }
        return 1;
}


int8_t nrk_get_button(uint8_t b)
{
return NRK_ERROR;
}



int8_t nrk_led_toggle( int led )
{
if(led==0) { nrk_gpio_toggle(NRK_LED_0); return 1; }
if(led==1) { nrk_gpio_toggle(NRK_LED_1); return 1; }
if(led==2) { nrk_gpio_toggle(NRK_LED_2); return 1; }
return -1;
}

int8_t nrk_led_clr( int led )
{
if(led==0) { nrk_gpio_set(NRK_LED_0); return 1; }
if(led==1) { nrk_gpio_set(NRK_LED_1); return 1; }
if(led==2) { nrk_gpio_set(NRK_LED_2); return 1; }
return -1;
}

int8_t nrk_led_set( int led )
{
if(led==0) { nrk_gpio_clr(NRK_LED_0); return 1; }
if(led==1) { nrk_gpio_clr(NRK_LED_1); return 1; }
if(led==2) { nrk_gpio_clr(NRK_LED_2); return 1; }
return -1;
}
/*
void IO_SET_E(uint8_t pin)
{
PORTE |= BM(pin);
}

void IO_CLR_E(uint8_t pin)
{
PORTE &= ~BM(pin);
}

void IO_SET_F(uint8_t pin)
{
PORTF |= BM(pin);
}

void IO_CLR_F(uint8_t pin)
{
PORTF &= ~BM(pin);
}
*/

void putc0(char x)
{
     UART0_WAIT_AND_SEND(x);
}

void putc1(char x)
{
     UART1_WAIT_AND_SEND(x);
}

/* print uart string */
/*
void printus1(char *x) 
{
     int i=0;
     while(x[i]!='\0'){
     UART1_WAIT_AND_SEND(x[i]);
     i++;     
     }
}

void printus0(char *x) 
{
     int i=0;
     while(x[i]!='\0'){
     UART0_WAIT_AND_SEND(x[i]);
     i++;     
     }
}
*/
void setup_uart0(uint16_t baudrate)
{
//INIT_UART1( UART_BAUDRATE_115K2, (UART_OPT_NO_PARITY|UART_OPT_8_BITS_PER_CHAR|UART_OPT_ONE_STOP_BIT));
INIT_UART0( baudrate, (UART_OPT_NO_PARITY|UART_OPT_8_BITS_PER_CHAR|UART_OPT_ONE_STOP_BIT));
ENABLE_UART0();
}


/**
 * nrk_setup_uart()
 *
 * Sets a default uart for a given platform and
 * direct stdin and stdout to that port.
 *
 * More advanced UART usage will require manually
 * setting parameters.
 */
void nrk_setup_uart(uint16_t baudrate)
{

  //setup_uart1(baudrate);
  setup_uart0(baudrate);

  stdout = fdevopen( putc0, getc0);
  stdin = fdevopen( putc0, getc0);

#ifdef NRK_UART_BUF
   uart_rx_signal=nrk_signal_create();
   if(uart_rx_signal==NRK_ERROR) nrk_error_add(NRK_SIGNAL_CREATE_ERROR);
   uart_rx_buf_start=0;
   uart_rx_buf_end=0;
   ENABLE_UART0_RX_INT();
#endif



}

void setup_uart1(uint16_t baudrate)
{
//INIT_UART1( UART_BAUDRATE_115K2, (UART_OPT_NO_PARITY|UART_OPT_8_BITS_PER_CHAR|UART_OPT_ONE_STOP_BIT));
INIT_UART1( baudrate, (UART_OPT_NO_PARITY|UART_OPT_8_BITS_PER_CHAR|UART_OPT_ONE_STOP_BIT));
ENABLE_UART1();
}



char getc1(void){
	unsigned char tmp;
	UART1_WAIT_AND_RECEIVE(tmp);
	return tmp;
}

