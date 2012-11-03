#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <hal_imec.h>
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
        //TODO: fix
        if( IFG2 & BM(URXIFG1) ) return 1;
        }
return 0;
}

char getc1(void){
        unsigned char tmp;
        UART1_WAIT_AND_RECEIVE(tmp);
        return tmp;
}

#endif

void nrk_kprintf( const char *addr)
{
	char* c = addr;
	while(*c) {
		putchar(*c);
		c++;
	}
}

void nrk_setup_ports()
{
	//TODO: set unused pins as output to minimize current consumption
  //TODO: fix nrk_gpio_raw_direction functions, currently not working
  P3DIR |= 0x01;

  //Misc pins
	nrk_gpio_raw_direction(P6DIR,MISC1,NRK_PIN_OUTPUT);
	nrk_gpio_raw_direction(P6DIR,MISC2,NRK_PIN_OUTPUT);
	nrk_gpio_raw_direction(P6DIR,MISC3,NRK_PIN_OUTPUT);
	nrk_gpio_raw_direction(P6DIR,MISC4,NRK_PIN_OUTPUT);

  //ASIC pins
	nrk_gpio_raw_direction(P1DIR,ASIC_CLK,NRK_PIN_OUTPUT);
	nrk_gpio_raw_direction(P2DIR,UP_RESET,NRK_PIN_OUTPUT);
	nrk_gpio_raw_direction(P2DIR,ENABLE1,NRK_PIN_INPUT);

	nrk_gpio_raw_direction(P1DIR,UP_A0_ECG,NRK_PIN_OUTPUT);
	nrk_gpio_raw_direction(P1DIR,UP_A1_ECG,NRK_PIN_OUTPUT);
	nrk_gpio_raw_direction(P1DIR,UP_A2_ECG,NRK_PIN_OUTPUT);

	nrk_gpio_raw_direction(P2DIR,UP_A0_EEG,NRK_PIN_OUTPUT);
	nrk_gpio_raw_direction(P2DIR,UP_A1_EEG,NRK_PIN_OUTPUT);
	nrk_gpio_raw_direction(P2DIR,UP_A2_EEG,NRK_PIN_OUTPUT);
}

void nrk_init_hardware() {
	//XT2 Oscillator On
	BCSCTL1 &= ~XT2OFF;
	//Clear OSCFault flag and wait for it to set
	do 
  {
	  IFG1 &= ~OFIFG;
  }
  while ((IFG1 & OFIFG) != 0);
  BCSCTL2 = SELM1|SELS;	// MCLK = SMCLK = LFXT2 / 1

}

/*
 * GPIO related definitions
 * Define high-level nrk pins mappings to hardware pins and ports This is used
 * for nrk_gpio_... functions.  Raw GPIO mapping can be found in the
 * nrk_pin_define.h file.
 */

NRK_PIN(IMP_CS,IMP_CS,NRK_PORTA)
NRK_PIN(IMP_WR,IMP_WR,NRK_PORTA)
NRK_PIN(UP_A0_ECG,UP_A0_ECG,NRK_PORTA)
NRK_PIN(UP_A1_ECG,UP_A1_ECG,NRK_PORTA)
NRK_PIN(UP_A2_ECG,UP_A2_ECG,NRK_PORTA)
NRK_PIN(ASIC_CLK,ASIC_CLK,NRK_PORTA)

NRK_PIN(ENABLE1,ENABLE1,NRK_PORTB)
NRK_PIN(START_UP,START_UP,NRK_PORTB)
NRK_PIN(OLD_IMP_STIM_NO_LONGER_USED,OLD_IMP_STIM_NO_LONGER_USED,NRK_PORTB)
NRK_PIN(UP_RESET,UP_RESET,NRK_PORTB)
NRK_PIN(UP_A0_EEG,UP_A0_EEG,NRK_PORTB)
NRK_PIN(UP_A1_EEG,UP_A1_EEG,NRK_PORTB)
NRK_PIN(UP_A2_EEG,UP_A2_EEG,NRK_PORTB)
NRK_PIN(DR1_RF,DR1_RF,NRK_PORTB)

NRK_PIN(LED,LED,NRK_PORTC)
NRK_PIN(DATA_RF,DATA_RF,NRK_PORTC)
NRK_PIN(CLK1_RF,CLK1_RF,NRK_PORTC)
NRK_PIN(TXD,TXD,NRK_PORTC)
NRK_PIN(RXD,RXD,NRK_PORTC)

NRK_PIN(CS_RF,CS_RF,NRK_PORTD)
NRK_PIN(DOUT2_RF,DOUT2_RF,NRK_PORTD)
NRK_PIN(CLK2_RF,CLK2_RF,NRK_PORTD)
NRK_PIN(DR2_RF,DR2_RF,NRK_PORTD)
NRK_PIN(CE_RF,CE_RF,NRK_PORTD)
NRK_PIN(PWR_UP_RF,PWR_UP_RF,NRK_PORTD)

NRK_PIN(MUXA0,MUXA0,NRK_PORTE)
NRK_PIN(MUXA1,MUXA1,NRK_PORTE)
NRK_PIN(MUXA2,MUXA2,NRK_PORTE)
NRK_PIN(MUXA3,MUXA3,NRK_PORTE)
NRK_PIN(MUXA4,MUXA4,NRK_PORTE)
NRK_PIN(IMP_STIM,IMP_STIM,NRK_PORTE)

NRK_PIN(ASIC_OUT,ASIC_OUT,NRK_PORTF)
NRK_PIN(IMP_OUT,IMP_OUT,NRK_PORTF)
NRK_PIN(MISC1,MISC1,NRK_PORTF)
NRK_PIN(MISC2,MISC2,NRK_PORTF)
NRK_PIN(MISC3,MISC3,NRK_PORTF)
NRK_PIN(MISC4,MISC4,NRK_PORTF)

/*
 * GPIO handling functions
 */
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
	//No buttons on IMEC node.
	return NRK_ERROR;
}

int8_t nrk_led_toggle( int led )
{
	//Only one LED on IMEC node.
	if(led==0) { nrk_gpio_toggle(NRK_LED); return 1; }
	return NRK_ERROR;
}

int8_t nrk_led_clr( int led )
{
	if(led==0) { nrk_gpio_set(NRK_LED); return 1; }
	return NRK_ERROR;
}

int8_t nrk_led_set( int led )
{
	if(led==0) { nrk_gpio_clr(NRK_LED); return 1; }
	return NRK_ERROR;
}

void putc0(char x)
{
     UART0_WAIT_AND_SEND(x);
}

int putchar(int x)
{
	UART1_WAIT_AND_SEND(x);
	return 1;
}

/*
 * UART0 not connected on IMEC node
 */
void setup_uart0(uint16_t baudrate)
{
}

/*
 * Sets up UART1
 * TODO: perhaps use macros in hal.h like other platforms?
 */
void setup_uart1(uint16_t baudrate)
{
	UCTL1 = CHAR; //8-bit character
	UTCTL1 = SSEL1; //Clock source SMCLK=8Mhz
	UBR01=baudrate&0xff; //Clock divider LSB
	UBR11=(baudrate>>8)&0xff; //Clock divider MSB
	UMCTL1=0x00; //Modulation disabled
	ME2 |= UTXE1; //Enable USART1 TX
	//IE2 |= URXIE1; //Enable USART1 RX interrupt
	P3SEL |= (1<<6); //Port 3.6 USART1 TXD
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


