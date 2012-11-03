#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <hal_tmote.h>
//#include <avr/interrupt.h>
#include <nrk_pin_define.h>
#include <nrk_error.h>


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
int8_t uart_rx_signal;

SIGNAL(USART1_RX_vect)
{
char c;
uint8_t sig;
DISABLE_UART1_RX_INT();
   UART1_WAIT_AND_RECEIVE(c);
   uart_rx_buf[uart_rx_buf_end]=c;
   //if(uart_rx_buf_end==uart_rx_buf_start) sig=1; else sig=0;
   uart_rx_buf_end++;
   if(uart_rx_buf_end==MAX_RX_UART_BUF) uart_rx_buf_end=0;
   nrk_event_signal(uart_rx_signal);
CLEAR_UART1_RX_INT();
ENABLE_UART1_RX_INT();
}

char getc1()
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
UART1_WAIT_AND_RECEIVE(tmp);
return tmp;
}

uint8_t nrk_uart_data_ready(uint8_t uart_num)
{
if(uart_rx_buf_start!=uart_rx_buf_end) return 1;
return 0;
}

int8_t nrk_uart_rx_signal_get()
{
   if(uart_rx_signal==0) nrk_error_add(NRK_SIGNAL_CREATE_ERROR);
   return uart_rx_signal;
}

#else

int8_t nrk_uart_rx_signal_get()
{
   return 0;
}

uint8_t nrk_uart_data_ready(uint8_t uart_num)
{
if(uart_num==1)
        {
        if( IFG2 & BM(URXIFG1) ) return 1;
        }
return 0;
}
/*
uint8_t nrk_uart_data_ready(uint8_t uart_num)
{
if(uart_num==1)
        {
        if( UCSR1A & BM(RXC1) ) return 1;
        }
return 0;
}
*/

char getc1(void){
        unsigned char tmp;
        UART1_WAIT_AND_RECEIVE(tmp);
        return tmp;
}

#endif

//TODO: fix for msp
void nrk_kprintf( const char *addr)
{
 char c;
   //while((c=pgm_read_byte(addr++)))
        //putchar(c);
}

//TODO: fix for msp
void nrk_setup_ports()
{
	nrk_gpio_direction(NRK_LED_0, NRK_PIN_OUTPUT);
	nrk_gpio_direction(NRK_LED_1, NRK_PIN_OUTPUT);
	nrk_gpio_direction(NRK_LED_2, NRK_PIN_OUTPUT);
	//PORT_INIT();
	//SPI_INIT();
}

//---------------------------------------------------------------------------------------------
// GPIO related definitions
//---------------------------------------------------------------------------------------------
// Define high-level nrk pins mappings to hardware pins and ports
// This is used for nrk_gpio_... functions.
// Raw GPIO mapping can be found in the nrk_pin_define.h file.
//---------------------------------------------------------------------------------------------

//TODO: fix these for msp
//-------------------------------
// Port A
NRK_PIN( DEBUG_0,DEBUG_0, NRK_PORTA )
NRK_PIN( DEBUG_1,DEBUG_1, NRK_PORTA )
NRK_INVALID_PIN( DEBUG_2 )
NRK_INVALID_PIN( DEBUG_3 )
NRK_PIN( BUTTON,BUTTON, NRK_PORTA )
//-------------------------------
// Port B
//NRK_PIN( SPI_SS,SPI_SS, NRK_PORTB )
NRK_PIN( SCK,SCK, NRK_PORTB )
NRK_PIN( MOSI,MOSI, NRK_PORTB )
NRK_PIN( MISO,MISO, NRK_PORTB )
NRK_PIN( GPIO26,4, NRK_PORTB )
NRK_PIN( GPIO28,5, NRK_PORTB )

//-------------------------------
// Port D
NRK_PIN( UART1_RXD,UART1_RXD, NRK_PORTD )
NRK_PIN( UART1_TXD,UART1_TXD, NRK_PORTD )
NRK_PIN( CCA,CCA, NRK_PORTD )
NRK_PIN( SFD,SFD, NRK_PORTD )

//-------------------------------
// Port E
NRK_PIN( UART0_RXD,UART0_RXD, NRK_PORTE )
NRK_PIN( UART0_TXD,UART0_TXD, NRK_PORTE )
NRK_PIN( LED_0,LED_0, NRK_PORTE )
NRK_PIN( LED_1,LED_1, NRK_PORTE )
NRK_PIN( LED_2,LED_2, NRK_PORTE )
NRK_INVALID_PIN( LED_3 )
NRK_PIN( GPIO34,GPIO34, NRK_PORTE )
NRK_PIN( FIFOP, FIFOP, NRK_PORTE )


//-------------------------------
// Port F
NRK_PIN( ADC_INPUT_0, ADC_INPUT_0, NRK_PORTF )
NRK_PIN( ADC_INPUT_1, ADC_INPUT_1, NRK_PORTF )
NRK_PIN( ADC_INPUT_2, ADC_INPUT_2, NRK_PORTF )
NRK_PIN( ADC_INPUT_3, ADC_INPUT_3, NRK_PORTF )
NRK_PIN( ADC_INPUT_4, ADC_INPUT_4, NRK_PORTF )
NRK_PIN( ADC_INPUT_5, ADC_INPUT_5, NRK_PORTF )
NRK_PIN( ADC_INPUT_6, ADC_INPUT_6, NRK_PORTF )
NRK_PIN( ADC_INPUT_7, ADC_INPUT_7, NRK_PORTF )

//-------------------------------
// GPIO handling functions

int8_t nrk_gpio_set(uint8_t pin)
{
        if (pin == NRK_INVALID_PIN_VAL) return -1;
        switch (pin & 0x07) {
                case NRK_PORTA:
                        do { P1OUT |= BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTB:
                        do { P2OUT |= BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTC:
                        do { P3OUT |= BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTD:
                        do { P4OUT |= BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTE:
                        do { P5OUT |= BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTF:
                        do { P6OUT |= BM((pin & 0xF8) >> 3); } while(0); break; 
                default: return -1;
        }
        return 1;
}

int8_t nrk_gpio_clr(uint8_t pin)
{
        if (pin == NRK_INVALID_PIN_VAL) return -1;
        switch (pin & 0x07) {
                case NRK_PORTA:
                        do { P1OUT &= ~BM((pin & 0xF8) >> 3); } while(0); break;
                case NRK_PORTB:
                        do { P2OUT &= ~BM((pin & 0xF8) >> 3); } while(0); break;
                case NRK_PORTC:
                        do { P3OUT &= ~BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTD:
                        do { P4OUT &= ~BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTE:
                        do { P5OUT &= ~BM((pin & 0xF8) >> 3); } while(0); break; 
                case NRK_PORTF:
                        do { P6OUT &= ~BM((pin & 0xF8) >> 3); } while(0); break; 
                default: return -1;
        }
        return 1;
}

int8_t nrk_gpio_get(uint8_t pin)
{
    if (pin == NRK_INVALID_PIN_VAL) return -1;
        switch (pin & 0x07) {
                case NRK_PORTA:
                        return (P1IN & BM((pin & 0xF8) >> 3));
                case NRK_PORTB:
                        return (P2IN & BM((pin & 0xF8) >> 3));
                case NRK_PORTC:
                        return (P3IN & BM((pin & 0xF8) >> 3));
                case NRK_PORTD:
                        return (P4IN & BM((pin & 0xF8) >> 3));
                case NRK_PORTE:
                        return (P5IN & BM((pin & 0xF8) >> 3));
                case NRK_PORTF:
                        return (P6IN & BM((pin & 0xF8) >> 3));
                default: return -1;
        }
        return 1;
}

int8_t nrk_gpio_toggle(uint8_t pin)
{
    if (pin == NRK_INVALID_PIN_VAL) return -1;
        switch (pin & 0x07) {
                case NRK_PORTA:
                        if ((P1IN & BM((pin & 0xF8) >> 3)) == 0) {
                                do { P1OUT |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { P1OUT &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                case NRK_PORTB:
                        if ((P2IN & BM((pin & 0xF8) >> 3)) == 0) {
                                do { P2OUT |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { P2OUT &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                case NRK_PORTC:
                        if ((P3IN & BM((pin & 0xF8) >> 3)) == 0) {
                                do { P3OUT |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { P3OUT &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                case NRK_PORTD:
                        if ((P4IN & BM((pin & 0xF8) >> 3)) == 0) {
                                do { P4OUT |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { P4OUT &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                case NRK_PORTE:
                        if ((P5IN & BM((pin & 0xF8) >> 3)) == 0) {
                                do { P5OUT |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { P5OUT &= ~BM((pin & 0xF8) >> 3); } while(0);
                        }
                        break;
                case NRK_PORTF:
                        if ((P6IN & BM((pin & 0xF8) >> 3)) == 0) {
                                do { P6OUT |= BM((pin & 0xF8) >> 3); } while(0);
                        } else {
                                do { P6OUT &= ~BM((pin & 0xF8) >> 3); } while(0);
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
                                P1DIR &= ~BM((pin & 0xF8) >> 3); 
                                P1OUT |= BM((pin & 0xF8) >> 3); 
                                break;
                        case NRK_PORTB:
                                P2DIR &= ~BM((pin & 0xF8) >> 3); 
                                P2OUT |= BM((pin & 0xF8) >> 3); 
                                break;
                        case NRK_PORTC:
                                P3DIR &= ~BM((pin & 0xF8) >> 3); 
                                P3OUT |= BM((pin & 0xF8) >> 3); 
                                break;
                        case NRK_PORTD:
                                P4DIR &= ~BM((pin & 0xF8) >> 3); 
                                P4OUT |= BM((pin & 0xF8) >> 3); 
                                break;
                        case NRK_PORTE:
                                P5DIR &= ~BM((pin & 0xF8) >> 3); 
                                P5OUT |= BM((pin & 0xF8) >> 3); 
                                break;
                        case NRK_PORTF:
                                P6DIR &= ~BM((pin & 0xF8) >> 3);
                                P6OUT |= BM((pin & 0xF8) >> 3); 
                                break;
                        default: return -1;
                }
        } else {
                switch (pin & 0x07) {
                        case NRK_PORTA:
                                P1DIR |= BM((pin & 0xF8) >> 3); break;
                        case NRK_PORTB:
                                P2DIR |= BM((pin & 0xF8) >> 3); break;
                        case NRK_PORTC:
                                P3DIR |= BM((pin & 0xF8) >> 3); break;
                        case NRK_PORTD:
                                P4DIR |= BM((pin & 0xF8) >> 3); break;
                        case NRK_PORTE:
                                P5DIR |= BM((pin & 0xF8) >> 3); break;
                        case NRK_PORTF:
                                P6DIR |= BM((pin & 0xF8) >> 3); break;
                        default: return -1;
                }
        }
        return 1;
}

int8_t nrk_gpio_pullups(uint8_t enable)
{
return NRK_ERROR;
}

int8_t nrk_get_button(uint8_t b)
{
if(b==0) {
	 return( !(P2IN & BM(BUTTON))); 
	} 
return -1;
}
/*
int8_t nrk_get_button(uint8_t b)
{
if(b==0) {
	 return( !(PINA & BM(BUTTON))); 
	} 
return -1;
}
*/

int8_t nrk_led_toggle( int led )
{
if(led==0) { nrk_gpio_toggle(NRK_LED_0); return 1; }
if(led==1) { nrk_gpio_toggle(NRK_LED_1); return 1; }
if(led==2) { nrk_gpio_toggle(NRK_LED_2); return 1; }
//if(led==3) { nrk_gpio_toggle(NRK_LED_3); return 1; }
return -1;
}

int8_t nrk_led_clr( int led )
{
if(led==0) { nrk_gpio_set(NRK_LED_0); return 1; }
if(led==1) { nrk_gpio_set(NRK_LED_1); return 1; }
if(led==2) { nrk_gpio_set(NRK_LED_2); return 1; }
//if(led==3) { nrk_gpio_set(NRK_LED_3); return 1; }
return -1;
}

int8_t nrk_led_set( int led )
{
if(led==0) { nrk_gpio_clr(NRK_LED_0); return 1; }
if(led==1) { nrk_gpio_clr(NRK_LED_1); return 1; }
if(led==2) { nrk_gpio_clr(NRK_LED_2); return 1; }
//if(led==3) { nrk_gpio_clr(NRK_LED_3); return 1; }
return -1;
}


void IO_SET_E(uint8_t pin)
{
P5OUT |= BM(pin);
}

void IO_CLR_E(uint8_t pin)
{
P5OUT &= ~BM(pin);
}

void IO_SET_F(uint8_t pin)
{
P6OUT |= BM(pin);
}

void IO_CLR_F(uint8_t pin)
{
P6OUT &= ~BM(pin);
}

void putc0(char x)
{
     UART0_WAIT_AND_SEND(x);
}

//void putc1(char x)
int putchar(int x)
{
     UART1_WAIT_AND_SEND(x);
     return 1;
}

//TODO: fix for msp
void setup_uart0(uint16_t baudrate)
{
//INIT_UART1( UART_BAUDRATE_115K2, (UART_OPT_NO_PARITY|UART_OPT_8_BITS_PER_CHAR|UART_OPT_ONE_STOP_BIT));
//INIT_UART0( baudrate, (UART_OPT_NO_PARITY|UART_OPT_8_BITS_PER_CHAR|UART_OPT_ONE_STOP_BIT));
//ENABLE_UART0();
}

//TODO: fix for msp
void setup_uart1(uint16_t baudrate)
{
//INIT_UART1( UART_BAUDRATE_115K2, (UART_OPT_NO_PARITY|UART_OPT_8_BITS_PER_CHAR|UART_OPT_ONE_STOP_BIT));
//INIT_UART1( baudrate, (UART_OPT_NO_PARITY|UART_OPT_8_BITS_PER_CHAR|UART_OPT_ONE_STOP_BIT));
//ENABLE_UART1();
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
//TODO: fix for msp
void nrk_setup_uart(uint16_t baudrate)
{

  setup_uart1(baudrate);
  //setup_uart0(baudrate);

  //stdout = fdevopen( putc1, getc1);
  //stdin = fdevopen( putc1, getc1);
#ifdef NRK_UART_BUF
   uart_rx_signal=nrk_signal_create();
   if(uart_rx_signal==0) nrk_error_add(NRK_SIGNAL_CREATE_ERROR);
   uart_rx_buf_start=0;
   uart_rx_buf_end=0;
   ENABLE_UART1_RX_INT();
#endif

}



/* get one char from uart */
char getc0(void){
	unsigned char tmp;
	UART0_WAIT_AND_RECEIVE(tmp);
	return tmp;
}

void PORT_INIT()
{
/*
        WDTCTL = WDTPW + WDTHOLD;
	P1DIR |= BM(LED_0) | BM(BTN_SUPPLY);
	P2DIR |= BIT0+BIT1;
	P1OUT = BM(LED_0) | BM(BTN_SUPPLY);
*/
}


