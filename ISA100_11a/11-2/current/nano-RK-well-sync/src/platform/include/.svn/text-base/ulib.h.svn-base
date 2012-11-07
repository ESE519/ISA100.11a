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
*******************************************************************************/


#ifndef ULIB_H
#define ULIB_H

#include <nrk_pin_define.h>
#include <nrk_events.h>
 
char getc0();
char getc1();
//void putc0(char);
void putc1(char);

uint8_t nrk_uart_data_ready(uint8_t uart_num);

// FIXME: should be using included file
int8_t nrk_uart_rx_signal_get();
//nrk_sig_t nrk_uart_rx_signal_get();
void nrk_kprintf(const char *addr);
void nrk_setup_ports();
void nrk_init_hardware();
void nrk_setup_uart(uint16_t baudrate);


//---------------------------------------------------------------------------------------------
// LED related definitions
int8_t nrk_led_set(int led);
int8_t nrk_led_clr(int led);
int8_t nrk_led_toggle(int led);

//---------------------------------------------------------------------------------------------
// Button related definitions
int8_t nrk_get_button(uint8_t b);

//---------------------------------------------------------------------------------------------
// GPIO related definitions  (see also nrk_gpio_raw functions for platform specific access)
// these macros provide simplified gpio access for
// higher level programs. Pins are defined has NRK_<pin name>; 
// mapping to the hardware is done in ulib.c
int8_t nrk_gpio_set( uint8_t pin );
int8_t nrk_gpio_clr( uint8_t pin );
int8_t nrk_gpio_get( uint8_t pin );
int8_t nrk_gpio_toggle( uint8_t pin );
int8_t nrk_gpio_direction(uint8_t pin, uint8_t pin_direction);
int8_t nrk_gpio_pullups( uint8_t enable );

#endif
