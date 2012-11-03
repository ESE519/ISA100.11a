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
*  Nuno Pereira - Polytechnic Institute of Porto; nap@isep.ipp.pt
*******************************************************************************/



/*
//------------------------------------------------------------------------------
/
//      DESCRIPTION:
//          This is the implementation of a very simple sync. protocol
//		somewhat inspired in the Flooding Time Sync. Protocol (SenSys04).
//		Does not handle clock drift; just tries to adjust time when it 
//		receives a packet from the master.
//		The master is dynamically elected based on the ID put on the packet.
//		This ID must be unique in the network, and is currently defined by NODE_ADDR.
//
//		This time sync is used for emulation of a global sync. with 
//		an external device. The idea is having two firefly boards stacked
//		where on board acts like a sync. device that toggles a pin every 
//		sync. period.
//
//------------------------------------------------------------------------------
*/

#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <nrk.h>
#include <hal.h>
#include <basic_rf.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <nrk_error.h>
#include <nrk_timer.h> 
#include <basic_timesync_rf.h>
#include <basic_timesync_timer.h>
#include <widom.h>

#define SYNC_PIN NRK_GPIO26

#define global_time  ((uint32_t)(timesync_timer_get() + ((int32_t)offset)))
#define global2local( the_time) ((int32_t)((int32_t)the_time - ((int32_t)offset)))

#define SYNC_SND_PERIOD_us 1000000
#define SYNC_PIN_TOGGLE_PERIOD_us (((WD_SYNC_PERIOD_us/1000) + 1)*1000)
#define SYNC_PIN_TOGGLE_TIME_BEFORE_us 100

const uint32_t SYNC_SND_PERIOD_clkTks = ( SYNC_SND_PERIOD_us / 1000000.0 ) / CLOCK_TICK_TIME + 1;
const uint32_t SYNC_PIN_TOGGLE_PERIOD_clkTks = ( SYNC_PIN_TOGGLE_PERIOD_us / 1000000.0 ) / CLOCK_TICK_TIME + 1;
const uint32_t SYNC_PIN_TOGGLE_TIME_BEFORE_clkTks = ( SYNC_PIN_TOGGLE_TIME_BEFORE_us / 1000000.0 ) / CLOCK_TICK_TIME + 1;

#define set_toogle_pin_time() next_sync_pin_toogle_global = (((uint32_t)((uint32_t)(global_time)) / ((uint32_t)SYNC_PIN_TOGGLE_PERIOD_clkTks))+1) * SYNC_PIN_TOGGLE_PERIOD_clkTks

#define adjust_toogle_pin_time() next_sync_pin_toogle = ((int32_t)(((int32_t)next_sync_pin_toogle_global-SYNC_PIN_TOGGLE_TIME_BEFORE_clkTks) - ((int32_t)offset)))

/*
//------------------------------------------------------------------------------
//      void main (void)
//
//      DESCRIPTION:
//              Startup routine and main loop
//------------------------------------------------------------------------------
*/
int main (void)
{
    int32_t offset=0;
    uint8_t first_sync=1, seq, fail_sync_cnt=0;
    uint8_t in_sync=0, sync_cnt=0;	
    uint16_t root_id=0xEFFF, id=0;
    uint32_t time, next_time, next_sync_pin_toogle, next_sync_pin_toogle_global, next_sync_snd;	

    nrk_setup_ports(); 
    nrk_setup_uart (UART_BAUDRATE_115K2);

    printf( "Basic time sync. starting up... (ADDR=%u)\r\n", NODE_ADDR);
    printf( "SP=%d ms\r\n", (SYNC_PIN_TOGGLE_PERIOD_us / 1000));

    nrk_init();

	  nrk_led_clr(ORANGE_LED);
	  nrk_led_clr(BLUE_LED);
	  nrk_led_clr(GREEN_LED);
	  nrk_led_clr(RED_LED);
	
    timesync_timer_start();

    btrf_init(TIMESYNC_CHANNEL, 0x2420, NODE_ADDR);
    btrf_set_rcv();

    next_sync_snd = timesync_timer_get() + SYNC_SND_PERIOD_clkTks;	
    next_sync_pin_toogle_global=global2local(timesync_timer_get()+SYNC_PIN_TOGGLE_PERIOD_clkTks);
    adjust_toogle_pin_time();

    nrk_gpio_direction( SYNC_PIN, NRK_PIN_OUTPUT);

    if (NODE_ADDR==0) {
	printf("Became Root (ID=%d)!\r\n", NODE_ADDR);
	in_sync=1;
	root_id = NODE_ADDR;
	nrk_led_toggle(ORANGE_LED);
    }

    do
    {
	time = timesync_timer_get();

	if (in_sync == 1 && (next_sync_pin_toogle_global-SYNC_PIN_TOGGLE_TIME_BEFORE_clkTks) <= global_time) { // toogle sync pin
		while (next_sync_pin_toogle_global > global_time);
		time = timesync_timer_get();
		nrk_gpio_toggle(SYNC_PIN);
		set_toogle_pin_time();
		adjust_toogle_pin_time();
	}

	if (next_sync_snd <= time) { // send sync packet 
		if (root_id == NODE_ADDR) nrk_led_toggle(BLUE_LED);
		if (root_id != NODE_ADDR) {
			nrk_led_toggle(GREEN_LED);
			if (sync_cnt==0) fail_sync_cnt++;
			if (fail_sync_cnt > 4) { // become root
				printf("Became Root (ID=%d)!\r\n", NODE_ADDR);
				offset = 0;
				root_id = NODE_ADDR;
				in_sync=1;
				set_toogle_pin_time();
				fail_sync_cnt=0;
				nrk_led_toggle(ORANGE_LED);
			}
		}
		if (in_sync==1) { 
			//printf("offset=%ld global_time=%ld\r\n", offset, global_time);
			tx_sync_packet(offset, root_id);
		}
		next_sync_snd = ((uint32_t)time) + ((uint32_t)SYNC_SND_PERIOD_clkTks); // TODO: deal with overflow
		sync_cnt=0;
	} 
	
	adjust_toogle_pin_time();

	if (next_sync_pin_toogle < next_sync_snd) {
		next_time = next_sync_pin_toogle;
	} else next_time = next_sync_snd;

	if (wait_rx_sync_pkt_until(next_time, &id, &seq) == 0) { // received a packet
		//printf("Okt ID=%d SEQ=%d\r\n", id, seq);
		if (first_sync == 1 && id < NODE_ADDR) {
			first_sync = 0;
			root_id = id;
			printf("+New root (ID=%d)\r\n", root_id);
			offset = (int32_t) ((uint32_t)timesync_timer_last_pkt_timestamp_get())-((uint32_t)timesync_timer_last_rx_timestamp_get());
		} else if (first_sync == 0 && id <= root_id){
			//printf("ID=%d RID=%d\r\n", id, root_id);
			if (id < root_id) {
				root_id = id;
				printf("New root (ID=%d)\r\n", root_id);
				in_sync = 0;
			}	
			if (root_id != NODE_ADDR) {
				offset = (int32_t) ((uint32_t)timesync_timer_last_pkt_timestamp_get())-((uint32_t)timesync_timer_last_rx_timestamp_get());
				if (in_sync == 0) {
					in_sync=1;
					nrk_led_toggle(ORANGE_LED);
					set_toogle_pin_time();
				}
				adjust_toogle_pin_time();
				sync_cnt++;
			}
			//printf ("Offset=%ld \r\n", offset);
		}
	}
   } while (1);

    return 0;
}
