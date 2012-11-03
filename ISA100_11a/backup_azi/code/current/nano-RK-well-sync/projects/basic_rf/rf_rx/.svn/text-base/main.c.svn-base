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
*******************************************************************************/


#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <basic_rf.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>



RF_TX_INFO rfTxInfo;
RF_RX_INFO rfRxInfo;
uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
//------------------------------------------------------------------------------
//      void main (void)
//
//      DESCRIPTION:
//              Startup routine and main loop
//------------------------------------------------------------------------------
int main (void)
{
    uint8_t cnt,i,length,n;

    nrk_setup_ports(); 
    nrk_setup_uart (UART_BAUDRATE_115K2);
 
    printf( "Receiver\r\n" ); 
    nrk_led_set(0); 
    nrk_led_clr(1); 
    nrk_led_clr(2); 
    nrk_led_clr(3); 

    rfRxInfo.pPayload = rx_buf;
    rfRxInfo.max_length = RF_MAX_PAYLOAD_SIZE;
    rfRxInfo.ackRequest= 0;
    rf_init (&rfRxInfo, 25, 0x2420, 0x1215);
    printf( "Waiting for packet...\r\n" );
    
    while(1)
	{
	nrk_led_set(2);
        rf_init (&rfRxInfo, 25, 0x2420, 0x1215);
        rf_set_rx (&rfRxInfo, 25); 	
	rf_polling_rx_on ();
        while ((n = rf_rx_check ()) == 0); 
	nrk_led_clr(2);
 	if (n != 0) {
        	n = 0;
        	// Packet on its way
    		cnt=0;
        	while ((n = rf_polling_rx_packet ()) == 0) {
		if (cnt > 50) {
                	printf( "PKT Timeout\r\n" );
			break;          // huge timeout as failsafe
			}
        	halWait(10000);
		cnt++;
		}
    	}
	//rf_rx_off();
    	if (n == 1) {
        	// CRC and checksum passed
		printf( "SNR: %d [",rfRxInfo.rssi );
        	for(i=0; i<rfRxInfo.length; i++ )
			printf( "%c", rfRxInfo.pPayload[i]);
		printf( "]\r\n" );
		nrk_led_set(1);	
    	} else { printf( "CRC failed!\r\n" ); nrk_led_clr(1); }
	
	}


}




