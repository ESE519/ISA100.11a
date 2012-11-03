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
*******************************************************************************/

//#include <rtl_debug.h>
#include <include.h>
#include <ulib.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <nrk.h>
#include <nrk_events.h>
#include <nrk_timer.h>
#include <nrk_error.h>
//#include <rtl_defs.h>
#include <stdlib.h>
#include <isa_scheduler.h>
#include <isa.h>
#include <isa_defs.h>

#define CHANNEL_HOPPING 
//#define CORRECTION
//#define INIT_SYNC
#define LED_SLOT_DEBUG
//#define HIGH_TIMER_DEBUG
#define TX_RX_DEBUG
#define ACK_DEBUG
#define RX_DEBUG
#define TX_DEBUG
//#define JOIN_PROCESS

#ifdef JOIN_PROCESS
 uint8_t join_pkt_buf[RF_MAX_PAYLOAD_SIZE];
#endif

/* slot related declaration */
volatile uint16_t global_slot;
volatile uint16_t current_global_slot;
volatile uint16_t global_cycle;
uint16_t last_sync_slot;

/* channel hopping related declaration */
uint8_t slottedPattern[16];
uint8_t slowPattern[3];
uint8_t currentChannel;
uint8_t channelIndex;
uint8_t slowIndex;

/*used for calculating offset*/
uint16_t slot_start_time;
uint16_t tx_start_time; // actual transmission starting time
uint16_t rx_start_time; 
uint16_t offsetY;
uint16_t offsetX;

/* SYNC related declaration */
uint8_t _isa_sync_ok;
uint8_t AFTER_FIRST_SYNC;
uint16_t EXPIRATION = 200;// each slot lasts for 10ms, so 100 slots last for 1s
uint16_t slot_expired;
uint8_t previous_tx_slot;

/* signal related declaration */
int8_t isa_tx_done_signal;
int8_t isa_rx_pkt_signal;

/* header type */
uint8_t DHDR;  // Data link layer header sub-header, currently used as ACK control
//uint8_t DMXHR[4]; //Data link layer media access control extension sub header, mainly used for security control
uint8_t DAUX[29]; //Data link layer auxiliary sub-header, currently used for join process
//uint8_t DROUT[3]; //Routing sub-header, compressed variant
//uint8_t DADDR[5]; //Addressing sub-header
uint8_t DHR;   // ACK's data link layer header

/* Test variable */
uint8_t tmp_curSec;
uint8_t tmp_offsetSec;
int16_t tmp_offsetNanoSec;
uint16_t tmp_count=0;



void config_child_list (uint8_t node_id)
{
    child_list |= ((uint32_t) 1) << node_id;
}

/**
 * isa_set_channel()
 *
 * This function set channel and is used for channel hopping.
 *
 */
void isa_set_channel (uint8_t chan)
{
    isa_param.channel = chan;
    rf_set_channel (chan);
}

uint8_t isa_get_channel()
{
    return isa_param.channel;
}

void isa_set_channel_pattern(uint8_t pattern)
{
    switch (pattern)
    {
	case 1:
	    slottedPattern[0] = 19;
	    slottedPattern[1] = 12;
	    slottedPattern[2] = 20;
	    slottedPattern[3] = 24;
	    slottedPattern[4] = 16;
	    slottedPattern[5] = 23;
	    slottedPattern[6] = 18;
	    slottedPattern[7] = 25;
	    slottedPattern[8] = 14;
	    slottedPattern[9] = 21;
	    slottedPattern[10] = 11;
	    slottedPattern[11] = 15;
	    slottedPattern[12] = 22;
	    slottedPattern[13] = 17;
	    slottedPattern[14] = 13;
	    slottedPattern[15] = 26;
	    break;
	case 3:
	    slowPattern[0]=15;
	    slowPattern[1]=20;
	    slowPattern[2]=25;
	    break;
	default:
	    break;
    }
}

int8_t isa_ready()
{
    if (_isa_ready ==  1)
        return NRK_OK;
    else
        return NRK_ERROR;
}

int8_t isa_rx_pkt_set_buffer(uint8_t *buf, uint8_t size)
{

    if(size==0 || buf==NULL) return NRK_ERROR;
    isa_rfRxInfo.pPayload = buf;
    isa_rfRxInfo.max_length = size;

return NRK_OK;
}

int8_t isa_wait_until_rx_pkt()
{
    nrk_signal_register(isa_rx_pkt_signal);
    if (isa_rx_pkt_check() != 0)
        return NRK_OK;
    nrk_event_wait (SIG(isa_rx_pkt_signal));
    return NRK_OK;
}

int8_t isa_wait_until_rx_or_tx ()
{
    nrk_signal_register(isa_rx_pkt_signal);
    nrk_signal_register(isa_tx_done_signal);
    nrk_event_wait (SIG(isa_rx_pkt_signal) | SIG(isa_tx_done_signal));
    return NRK_OK;
}

/**
 * isa_init()
 *
 * This function sets up the low level link layer parameters.
 * This starts the main timer routine that will then automatically
 * trigger whenever a packet might be sent or received.
 * This should be called before ANY scheduling information is set
 * since it will clear some default values.
 *
 */
uint8_t isa_init (isa_node_mode_t mode, uint8_t id, uint8_t src_id)
{
    uint8_t i;

    /* Generate signals */
    isa_rx_pkt_signal=nrk_signal_create();
    if(isa_rx_pkt_signal==NRK_ERROR){
	nrk_kprintf(PSTR("ISA ERROR: creating rx signal failed\r\n"));
	nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
	return NRK_ERROR;
    }
    isa_tx_done_signal=nrk_signal_create();
    if(isa_tx_done_signal==NRK_ERROR){
	nrk_kprintf(PSTR("ISA ERROR: creating tx signal failed\r\n"));
	nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
	return NRK_ERROR;
    }

    // No buffer to start with
    isa_rfRxInfo.pPayload = NULL;
    isa_rfRxInfo.max_length = 0;

    /*FIXME Actually we dont need to always run the high speed timer */
    _nrk_high_speed_timer_start();  

    /* clear everything out */
    global_cycle = 0;
    global_slot = MAX_ISA_GLOBAL_SLOTS;
    _isa_sync_ok = 0;
    _isa_join_ok = 0;
    slot_expired = 0;
    isa_node_mode = mode;
    isa_id = id;//change
    isa_clk_src_id=src_id; //change
    isa_rx_data_ready = 0;
    isa_tx_data_ready = 0;


    isa_param.mobile_sync_timeout = 100;
    isa_param.rx_timeout = 8000;   // 8000 *.125us = 1ms
    isa_param.tx_guard_time = TX_GUARD_TIME;
    isa_param.channel = 15;
    isa_param.mac_addr = 0x1981;

for (i = 0; i < ISA_SLOTS_PER_FRAME; i++) {
        isa_sched[i] = 0;
    }
    isa_tdma_rx_mask = 0;
    isa_tdma_tx_mask = 0;

    /* Setup the cc2420 chip */
    rf_init (&isa_rfRxInfo, isa_param.channel, 0x2421, isa_param.mac_addr);

    AFTER_FIRST_SYNC = 1;

    /* Setup fisrt hopping channel */
    #ifdef CHANNEL_HOPPING
	slowIndex=0; 
	if(id!=0){
	    channelIndex = src_id;
	    currentChannel = slottedPattern[channelIndex];   
	}else{
	    channelIndex = 0;
	    currentChannel = slottedPattern[channelIndex];
	}
	isa_set_channel(currentChannel);
    #endif

    #ifdef JOIN_PROCESS
	if(mode==ISA_GATEWAY){
	   for(i=22;i<=24;i++){
		isa_tx_info[i].pPayload = join_pkt_buf;
    		isa_tx_info[i].length = PKT_DATA_START+1;    // pass le pointer
    		isa_tx_info[i].DHDR = configDHDR();
    		isa_tx_data_ready |= ((uint32_t) 1 << i);        // set the flag
	   }
        }
    #endif

    resync_times=0;

    return NRK_OK;
}

void isa_start ()
{
    //_isa_clear_sched_cache ();
    _isa_ready = 2;
}

/**
 * configDHDR()
 *
 * Gateway could config the DHDR by informing the nodes.
 * DHDR contains control information that should be loaded
 * into the PDU header.
 *
 */
int8_t configDHDR()
{
    int8_t DHDR = 1;
    if(1){//request ACK
	DHDR |= 1<<7;
    }
    if(1){//request signal quality in ACK
	DHDR |= 1<<6;
    }
    if(1){//request EUI
	DHDR |= 1<<5;
    }
    if(0){//include DAUX
	DHDR |= 1<<4;
    }
    if(0){//include slow hopping offset
	DHDR |= 1<<3;
    }
    if(isa_node_mode == ISA_RECIPIENT){//is clock recipient
	DHDR |= 1<<2;
    }
    return DHDR;
}

/**
 * configDHR()
 *
 * Gateway could config the DHR by informing the nodes.
 * DHR contains control information that should be loaded
 * into the ACK reply header.
 *
 */
int8_t configDHR()
{
    int8_t DHR = 3;
    if(isa_node_mode != ISA_RECIPIENT){//include clock correction change
	DHR |= 1<<7;
    }
    if(0){//including slow-hopping timeslot offset
	DHR |= 1<<6;
    }
    if(0){//request EUI
	DHR |= 1<<5;
    }
    if(0){//include DAUX
	DHR |= 1<<4;
    }
    if(0){//include slow hopping offset
	DHR |= 1<<3;
    }
   
    return DHR;
}

/**
 * configAdvDAUX()
 *
 * Gateway could config the DAUX 
 * DAUX contains superframe and join information 
 *
 */
void configAdvDAUX(uint8_t chPattern, uint8_t chIndex, uint8_t sp_period, uint8_t tx_slot1, uint8_t tx_slot2, uint8_t tx_slot3, uint8_t tx_slot4, uint8_t rx_slot1, uint8_t rx_slot2, uint8_t rx_slot3, uint8_t rx_slot4)
{	
    /*ADVERTISEMENT SELECTION*/
    DAUX[0]=0; // Advertisement selection, 0 indicates advertisement DAUX
    
    /*TIME SYNCHRONIZATION*/
    /* 1-6 bytes are reserved for time synchronization */ 
  
    /*SUPERFRAME INFORMATION*/
    DAUX[7]=10; // timeslot duration, currently not used.
    DAUX[8]=0; // reserved for long timeslot duration
    DAUX[9]=chPattern; // channel pattern selection
    DAUX[10]=chIndex; // channel index selection
    DAUX[11]=sp_period; // superframe period
    DAUX[12]=0; // reserved for long period situation  
    /* 13 reserved for superframe cycle starting point
       14 reserved of the length of slow hopping period
       15 and 16 reserved for channel hopping channel map for spectrum management
    */

    /*JOIN INFORMATION*/
    /* 17 reserved for Join backoff and timeout
       18 reserved for Indicates fields that are transmitted
    */
    DAUX[19]=tx_slot1;  //TX slot 1
    DAUX[20]=tx_slot2;  //TX slot 2
    DAUX[21]=tx_slot3;  //TX slot 3
    DAUX[22]=tx_slot4;  //TX slot 4
    DAUX[23]=rx_slot1;  //RX slot 1
    DAUX[24]=rx_slot2;  //RX slot 2
    DAUX[25]=rx_slot3;  //RX slot 3
    DAUX[26]=rx_slot4;  //RX slot 4

    /*INTEGRETY CHECK*/
    /* 27 and 28 are reserved for Integrety check*/
}

/**
 * isa_check_rx_status()
 *
 * This function returns if there is a packet in the link layer
 * rx buffer.  Once a packet has been received, it should be quickly
 * processed or moved and then rtl_release_rx_packet() should be called. 
 * rtl_release_rx_packet() then resets the value of rtl_check_rx_status()
 *
 * Returns: 1 if a new packet was received, 0 otherwise
 */
int8_t isa_rx_pkt_check()
{
    return isa_rx_data_ready;
}

/**
 * isa_rx_pkt_get()
 *
 * This function returns the rx buffer point. It should be called
 * once a packet is received and must be followed by isa_release_rx_packet().
 * isa_release_rx_packet() then resets the value of isa_check_rx_status().
 * 
 * Returns: rx buffer point
 */
uint8_t* isa_rx_pkt_get (uint8_t *len, int8_t *rssi)
{
    if(isa_rx_pkt_check()==0){
	*len=0;
	*rssi=0;
	return NULL;
    }
    *len=isa_rfRxInfo.length;
    *rssi=isa_rfRxInfo.rssi;

    return isa_rfRxInfo.pPayload;
}

/**
 * _isa_rx()
 *
 * This is the low level RX packet function.  It will read in
 * a packet and buffer it in the link layer's single RX buffer.
 * This buffer can be checked with rtl_check_rx_status() and 
 * released with rtl_release_rx_packet().  If the buffer has not
 * been released and a new packet arrives, the packet will be lost.
 * This function is only called from the timer interrupt routine.
 *
 * Arguments: slot is the current slot that is actively in RX mode.
 */

void _isa_rx (uint8_t slot)
{
    uint8_t n;
    uint32_t node_mask;
    volatile uint8_t timeout;

    #ifdef LED_DEBUG
	nrk_led_set(1);
    #endif
    rf_set_rx (&isa_rfRxInfo, isa_param.channel);       // sets rx buffer and channel 
    rf_polling_rx_on ();
    
    // Timing for waiting for sfd
    timeout = _nrk_os_timer_get();
    timeout+=4;  // 4ms
    n = 0;
    //nrk_gpio_set(NRK_DEBUG_3);
    while ((n = rf_rx_check_sfd()) == 0) {
        if (_nrk_os_timer_get() > timeout) {
	    //spend too much time on waiting for a pkt's arrival
	    rf_rx_off ();
	    #ifdef LED_DEBUG
		nrk_led_clr(1);
	    #endif
	    #ifdef RX_DEBUG
		//printf("sfd times out.\n\r");
	    #endif
	    return;
        }
    }
//printf("%d\n\r",_nrk_high_speed_timer_get());
    // sfd received, start receiving packet and record start time
    rx_start_time = _nrk_high_speed_timer_get();

    // Timing for waiting for finishing packet receiving
    timeout = _nrk_os_timer_get(); 
    timeout += 5;               // 5ms
    if (n != 0) {
        n = 0;
       // printf("Packet on its way\n\r");
        while ((n = rf_polling_rx_packet (false,128)) == 0) {
	    //printf("%d\n\r",_nrk_os_timer_get());
            if (_nrk_os_timer_get () > timeout) {
		#ifdef RX_DEBUG
		    printf("packet is too long, times out.\n\r");
		#endif
		// spend too much time on receiving pkt.
                return;          // huge timeout as fail safe
            }
        }
    }
    rf_rx_off ();
    if (n == 1) {// successfully received packet
    	nrk_led_toggle(BLUE_LED);
    	putchar ('r');
	isa_rx_data_ready = 1;
	//potential problem: if repeater or recipient receives noise, the DHDR would be changed. And it is highly possible that the highest bit of DHDR would be set to 0
/*	if(isa_node_mode != ISA_GATEWAY)
	    DHDR = isa_rfRxInfo.pPayload[DHDR_INDEX];
*/
	#ifdef RX_DEBUG
	   // printf("Repeater slot = %d, local slot is %d.\n\r", isa_rfRxInfo.pPayload[SLOT_INDEX],global_slot);
	#endif RX_DEBUG	
	nrk_event_signal(isa_rx_pkt_signal);	

	//_nrk_high_speed_timer_reset();
	//nrk_high_speed_timer_wait(0,CPU_PROCESS_TIME);
	//nrk_gpio_set(NRK_DEBUG_3);

	node_mask = ((uint32_t) 1) << isa_rfRxInfo.pPayload[SRC_INDEX];
	

	if( !(node_mask & child_list))
	   return;  //FIXME change
	// ACK required
	if(DHDR & (1<<7)){

	    // Transmit ACK packet
	    DHR = configDHR();
	    isa_ack_buf[DHR_INDEX]= DHR;
	    #ifdef ACK_DEBUG
		//printf("DHR is %d.\n\r",DHR);
	    #endif
	    isa_ack_tx.pPayload = isa_ack_buf;
	    if (DHDR & (1<<2)){ // recipient , only reply explicit ACK
	        //isa_ack_tx.length = PKT_DATA_START-1;
		isa_ack_tx.length = 2;
            }
	    else { //reply ACK with time offsetX
		offsetX = rx_start_time - slot_start_time;
		//printf("slot_start_time is %d,rx_start_time is %d.\n\r",slot_start_time,rx_start_time);
		uint8_t temp1,temp2;
		temp1 = (offsetX & 0xFF00)>>8;
		isa_ack_buf[OFFSET_HIGH]=temp1;
		temp2 = (offsetX & 0x00FF);
 		isa_ack_buf[OFFSET_LOW]=temp2;
		#ifdef ACK_DEBUG
		  //  printf("offsetX is %d\n\r", offsetX);
		#endif		
		//isa_ack_tx.length = PKT_DATA_START + 1;	
		isa_ack_tx.length = 4;
	    }
	    rf_tx_tdma_packet (&isa_ack_tx,slot_start_time,isa_param.tx_guard_time,&tx_start_time);	
	}	
//nrk_gpio_clr(NRK_DEBUG_3);
    }        
    #ifdef LED_DEBUG
	nrk_led_clr (1);
    #endif
}

/**
 * isa_release_rx_packet()
 *
 * This function releases the link layer's hold on the rx buffer.
 * This must be called after a packet is received before a new
 * packet can be buffered!  This should ideally be done by the
 * network layer.
 *
 */

void isa_rx_pkt_release()
{
    isa_rx_data_ready = 0;
}

/**
 * rtl_tx_packet()
 *
 * This function associates a slot with a particular packet that needs
 * to be sent. 
 * 
 * Arguments: RF_TX_INFO *tx is a pointer to a transmit structure, this structure
 *            must have a valid pPayload pointer to the real packet.
 *            uint8_t slot is the value of the tx slot (starting from 0)
 *
 * Return:  currently always returns 1
 */
int8_t isa_tx_pkt (uint8_t *tx_buf, uint8_t len, uint8_t DHDR, uint8_t slot)
{
    isa_tx_info[slot].pPayload = tx_buf; 
    isa_tx_info[slot].length = len;    // pass le pointer
    isa_tx_info[slot].DHDR = DHDR;
    isa_tx_data_ready |= ((uint32_t) 1 << slot);        // set the flag
    return 1;
}

/**
 * isa_tx_pkt_check()
 *
 * This function allows an upper layer to check if a TX packet
 * has been sent.  For instance, you would first send a packet 
 * and then you could sleep or do more work, and eventually check
 * done = isa_tx_pkt_check(8);
 * to see if the packet had been sent by the interrupt routine.
 *
 * Returns: 1 if the packet was sent, 0 otherwise
 */
int8_t isa_tx_pkt_check(uint8_t slot)
{
    if ((isa_tx_data_ready & ((uint32_t) 1 << slot)) != 0)
        return 1;
    return 0;
}

/**
 * _isa_tx()
 *
 * This function is the low level TX function.
 * It is only called from the timer interrupt and fetches any
 * packets that were set for a particular slot by rtl_tx_packet().
 *
 * Arguments: slot is the active slot set by the interrupt timer.
 */
void _isa_tx (uint8_t slot)
{
    uint8_t n;
    uint8_t i;
    int8_t tmp;
    volatile uint8_t timeout;
    uint8_t offsetSec, curSec;
    uint16_t offsetNanoSec;
    int16_t time_correction;
    uint8_t tmp_nrk_prev_timer_val;
    // load header
    isa_rfTxInfo.cca = true;
    isa_rfTxInfo.pPayload=isa_tx_info[slot].pPayload;
    #ifdef TX_DEBUG
	//printf("TX Payload is: %s.\n\r", isa_rfTxInfo.pPayload);
    #endif
    isa_rfTxInfo.length=isa_tx_info[slot].length;

    isa_rfTxInfo.pPayload[DHDR_INDEX] = isa_tx_info[slot].DHDR;

    isa_rfTxInfo.pPayload[SLOT_INDEX] = (global_slot & 0xFF); 
    isa_rfTxInfo.pPayload[SRC_INDEX] = isa_id;//change

    #ifdef JOIN_PROCESS
    if(slot>=22 && isa_node_mode == ISA_GATEWAY){
	for(i=0;i<29;i++){
	 isa_rfTxInfo.pPayload[DAUX_INDEX+i]=DAUX[i];
	 //printf("DAUX[%d]=%d\r\n",i,isa_rfTxInfo.pPayload[DAUX_INDEX+i]);
        }
    } 
    #endif
    // FIXME a small bug. should not happen and should be fixed in _isa_init_sync()
    //if(AFTER_FIRST_SYNC == 1){    
	_nrk_high_speed_timer_reset();
	nrk_high_speed_timer_wait(0,WAIT_TIME_BEFORE_TX);
	//AFTER_FIRST_SYNC = 0;
    //}
	    #ifdef TX_RX_DEBUG
		
		//nrk_gpio_set(NRK_DEBUG_1);
   		//printf("T\r\n");
	    #endif
    if(rf_tx_tdma_packet (&isa_rfTxInfo,slot_start_time,isa_param.tx_guard_time,&tx_start_time))
    {
    	nrk_led_toggle(RED_LED);
    	putchar ('t');
	//("rx_start_time is %d.\n\r",_nrk_high_speed_timer_get());
	offsetY = tx_start_time - slot_start_time;
	//printf("offset Y is %d.\n\r",offsetY);
	#ifdef HIGH_TIMER_DEBUG
	    //printf("In isa.c _isa_tx(): offsetY is %d, tx_start_time is %d\n\r",offsetY,tx_start_time);
	#endif
    }
    nrk_event_signal (isa_tx_done_signal);
    isa_tx_data_ready &= ~((uint32_t) 1 << slot);       // clear the flag

    // ACK required
    if(DHDR & (1<<7) && isa_node_mode!=ISA_GATEWAY){ //Azriel
    	rf_polling_rx_on ();
	
	_nrk_high_speed_timer_reset();
	nrk_high_speed_timer_wait(0,CPU_PROCESS_TIME);
//nrk_gpio_set(NRK_DEBUG_1);
	// Timing for waiting for receiving ACK
	timeout = _nrk_os_timer_get();
	timeout+=2;  // 2ms
	n = 0;
	while ((n = rf_rx_check_sfd()) == 0) {
	    if (_nrk_os_timer_get() > timeout) {
		tmp = slot - previous_tx_slot;
		if(slot == previous_tx_slot)
		    slot_expired += 25;
		else{
		    tmp = slot - previous_tx_slot;
		    if(tmp>0)
			slot_expired += tmp;
		    else
			slot_expired += 25+tmp;
		}
		//printf("%d,%d,%d,%d,%d\n\r",slot_expired,tmp_curSec,tmp_offsetSec,tmp_offsetNanoSec,++tmp_count);
		//printf("%d,%d\n\r",slot_expired,isa_param.channel);
		//printf("%d,%d,%d\n\r",slot_expired,slot,previous_tx_slot);
	    //spend too much time on waiting for a pkt's arrival
		rf_rx_off ();
		#ifdef LED_DEBUG
		    nrk_led_clr(1);
		#endif
		#ifdef RX_DEBUG
		  //  printf("sfd times out.\n\r");
		#endif
		//nrk_gpio_clr(NRK_DEBUG_1);
		return;
	    }
	}
	nrk_gpio_clr(NRK_DEBUG_1);
	timeout = _nrk_os_timer_get(); 
	timeout += 2;               // 5ms
	if (n != 0) {
	    n = 0;
	    //printf("Packet on its way\n\r");
	    while ((n = rf_polling_rx_packet (true,4)) == 0) {
		//printf("%d\n\r",_nrk_os_timer_get());		
		if (_nrk_os_timer_get () > timeout) {
		#ifdef RX_DEBUG
		    printf("packet is too long, times out.\n\r");
		#endif	
		    tmp_curSec = _nrk_os_timer_get();
		    // spend too much time on receiving pkt.
                    return;          // huge timeout as fail safe
		}
		//if(n==-6)		
		  //  printf("%d\n\r",_nrk_os_timer_get());	
	    }
	}
	rf_rx_off ();
	if (n == 1) {// successfully received ACK
	    slot_expired = 0;
	    //isa_rx_data_ready = 1;
	    DHR = isa_rfRxInfo.pPayload[DHR_INDEX];
	    #ifdef ACK_DEBUG
		//printf("DHR = %d.\n\r", isa_rfRxInfo.pPayload[DHR_INDEX]);
	    #endif ACK_DEBUG

	    if((DHDR & (1<<7))&&isa_node_mode!=ISA_GATEWAY){

		if(DHR & (1<<7)){
		    offsetX = ((0x0000|isa_rfRxInfo.pPayload[OFFSET_HIGH])<<8)&0xff00 + 0x0000|isa_rfRxInfo.pPayload[OFFSET_LOW];
		    #ifdef ACK_DEBUG
		//	printf("offset X is %d.\n\r", offsetX);
		//	printf("offset Y is %d.\n\r", offsetY);
		    nrk_led_toggle(ORANGE_LED);
		    putchar('a');
		    #endif ACK_DEBUG
//printf("%d,%d\n\r",offsetX,offsetY);
		    time_correction = offsetX - offsetY;
		    #ifdef HIGH_TIMER_DEBUG
			printf("time correction is %d.\n\r", time_correction);
		    #endif
		    //printf("time correction is %d.\n\r", time_correction);
		    // SYNC as a by-product of communication.
		    //tmp_nrk_prev_timer_val = _nrk_prev_timer_val;
//printf("%d\n\r",time_correction);
//printf("%d\n\r",HIGH_TIMER_TICKES_PER_MILISEC-time_correction%HIGH_TIMER_TICKES_PER_MILISEC);
		    timeout=50;
		    curSec = _nrk_os_timer_get();
		    if(time_correction >= 0&&curSec<7){
			tmp_curSec = curSec;
			/*do{
			    curSec = _nrk_os_timer_get();
			}while(curSec<=7);*/
			_nrk_os_timer_stop();
			//_nrk_os_timer_reset();
			offsetSec = time_correction/HIGH_TIMER_TICKES_PER_MILISEC+1;
			tmp_offsetSec = offsetSec;
			offsetNanoSec = HIGH_TIMER_TICKES_PER_MILISEC-time_correction%HIGH_TIMER_TICKES_PER_MILISEC;
			tmp_offsetNanoSec = offsetNanoSec;
//printf("%d,%d\n\r",curSec,offsetSec);
			    //if(curSec+offsetSec>=10)
			    	//printf("%d,%d\n\r",curSec,offsetSec);
			/*if(curSec+offsetSec>=9){
			    _nrk_set_next_wakeup(curSec+offsetSec+2);
			    if(curSec+offsetSec>=10)
			    	printf("%d,%d\n\r",curSec,offsetSec);
			}
			else
			    _nrk_set_next_wakeup(10);*/
			_nrk_high_speed_timer_reset();
			nrk_spin_wait_us(50);
			//printf("%d,%d,%d\n\r",curSec,offsetSec,offsetNanoSec);
			nrk_gpio_set(NRK_DEBUG_2);
			do{}while(_nrk_high_speed_timer_get()<offsetNanoSec);
			nrk_gpio_clr(NRK_DEBUG_2);
			//nrk_high_speed_timer_wait(0,offsetNanoSec);
			//nrk_high_speed_timer_wait(0,4912);
			_nrk_os_timer_set(curSec+offsetSec);
			nrk_spin_wait_us(100);
			#ifdef CORRECTION
			    nrk_gpio_set(NRK_DEBUG_1);
			    nrk_gpio_clr(NRK_DEBUG_1);
			#endif
//printf("p\n\r");	
			_nrk_os_timer_start();
		    }else if(-time_correction>8000&&curSec<7){
			_nrk_os_timer_stop();
			//_nrk_os_timer_reset();
			_nrk_high_speed_timer_reset();
			do{
			    NOP();
			    NOP();
			    NOP();
			    NOP();
			}while(timeout--);
			//_nrk_set_next_wakeup(10);
			#ifdef CORRECTION
			nrk_gpio_set(NRK_DEBUG_2);
			//nrk_high_speed_timer_wait(0,22800); // for test
			nrk_gpio_clr(NRK_DEBUG_2);
			#endif
			//printf("%d\n\r",-time_correction);
			nrk_high_speed_timer_wait(0,-time_correction);
			_nrk_os_timer_set(curSec);
			timeout = 100;
			do{
			    NOP();
			    NOP();
			    NOP();
			    NOP();
			}while(timeout--);
			_nrk_os_timer_start();
		    }
		    //_nrk_prev_timer_val=tmp_nrk_prev_timer_val;
//nrk_cur_task_TCB->next_wakeup = 10;
		    _nrk_high_speed_timer_reset();
		    nrk_spin_wait_us(50);
		}
	    }		
        }    	
    }//wait for ACK 
}


uint8_t _isa_join_process ()
{
    int8_t n;
    uint16_t timeout;
    uint16_t timer;
    uint8_t tdma_start_tick;
    uint8_t battery_save_cnt;
    uint8_t last_nrk_tick;
    uint8_t i;

    timer=0;
    battery_save_cnt=0;

    while(1)
    {
	
	rf_set_rx (&isa_rfRxInfo, isa_param.channel);       // sets rx buffer and channel 
	rf_polling_rx_on ();
	n = 0;
	_isa_sync_ok = 0;
	last_nrk_tick=0;  // should be 0 going in
	//_nrk_prev_timer_val=250;
	//_nrk_set_next_wakeup(250);
	//_nrk_os_timer_set(0);
	//timeout=200;
	while ((n = rf_rx_check_sfd()) == 0) {
	    // every OS tick 
	    if(last_nrk_tick!=_nrk_os_timer_get()) {
		last_nrk_tick=_nrk_os_timer_get();
		timer++;
		if(timer>ISA_TOKEN_TIMEOUT){
		    timer=0;
		    break;
		}
	    }
	}
	//_nrk_high_speed_timer_reset();

	tdma_start_tick=_nrk_os_timer_get();
	timeout = tdma_start_tick+4;
	// an interrupt could happen in here and mess things up
	if (n != 0) {
	    n = 0;
	// Packet on its way
	    while ((n = rf_polling_rx_packet (false,128)) == 0) {
		if (_nrk_os_timer_get () > timeout)
		{
		    //nrk_kprintf( PSTR("Pkt timed out\r\n") );
		    break;          // huge timeout as failsafe
		}
	    }
	}
	rf_rx_off (); 
	if (n == 1 /*&& isa_rfRxInfo.length>0*/) {
	   // if(isa_rfRxInfo.pPayload[SRC_INDEX]==isa_clk_src_id){//change
		// CRC and checksum passed
		if(isa_rfRxInfo.pPayload[DAUX_INDEX+7]==10){ // DAUX packet
			isa_rx_data_ready = 1;	
			//global_slot = (volatile)isa_rfRxInfo.pPayload[SLOT_INDEX];

			isa_set_channel_pattern(isa_rfRxInfo.pPayload[DAUX_INDEX+9]); //set channel hopping pattern
			channelIndex=isa_rfRxInfo.pPayload[DAUX_INDEX+10];
			currentChannel = slottedPattern[channelIndex];
			isa_set_channel(currentChannel);
			for(i=0;i<4;i++){  // set tx slots
			    if(isa_rfRxInfo.pPayload[DAUX_INDEX+19+i]==0)
				break;
			    else{
				isa_tdma_tx_mask |= ((uint32_t) 1) << isa_rfRxInfo.pPayload[DAUX_INDEX+19+i];
				isa_sched[isa_rfRxInfo.pPayload[DAUX_INDEX+19+i]] = 1;
				tx_slot_from_join[i]=isa_rfRxInfo.pPayload[DAUX_INDEX+19+i];
			//	printf("TX:%d\r\n",tx_slot_from_join[i]);
                            }
			}			
			
			for(i=0;i<4;i++){  // set rx slots
			    if(isa_rfRxInfo.pPayload[DAUX_INDEX+23+i]==0)
				break;
			    else{
				isa_tdma_rx_mask |= ((uint32_t) 1) << isa_rfRxInfo.pPayload[DAUX_INDEX+23+i];
				isa_sched[isa_rfRxInfo.pPayload[DAUX_INDEX+23+i]] = 1;
			    }
			}
			nrk_event_signal(SIG(isa_rx_pkt_signal));
		break;
 		}
	   // }
	}
    }



    _isa_join_ok=1;
    isa_rx_pkt_release();
    return _isa_join_ok;
}

int8_t isa_join_ready()
{
    if (_isa_join_ok ==  1)
        return NRK_OK;
    else
        return NRK_ERROR;
}


/** FIXME this is only a temperary function need to be more specified
 * _isa_init_sync()
 *
 * This function is used for join process.
 * A node that wants to join the network would keep listening first
 * and set up first sync.
 * 
 * Return: _isa_sync_ok.
 */
uint8_t _isa_init_sync ()
{
    int8_t n;
    uint16_t timeout;
    uint16_t timer;
    uint8_t tdma_start_tick;
    uint8_t battery_save_cnt;
    uint8_t last_nrk_tick;
    uint8_t tmp_nrk_prev_timer_val;
    //volatile uint16_t sfd_start_time;

	//printf("%d,%d\n\r",isa_param.channel,global_slot);
  //  DISABLE_GLOBAL_INT ();
    timer=0;
    battery_save_cnt=0;


    while(1)
    {

    	//printf("Init sync \r\n");
	isa_rfRxInfo.pPayload[DHDR_INDEX]=configDHDR();
	//isa_rfRxInfo.pPayload[SLOT_INDEX]=global_slot;
	
	#ifdef LED_DEBUG
	    nrk_led_set(1);
	#endif
	rf_set_rx (&isa_rfRxInfo, isa_param.channel);       // sets rx buffer and channel 
	rf_polling_rx_on ();
	n = 0;
	_isa_sync_ok = 0;
	last_nrk_tick=0;  // should be 0 going in
	//_nrk_prev_timer_val=250;
	_nrk_set_next_wakeup(250);
	_nrk_os_timer_set(0);
	//timeout=200;
	while ((n = rf_rx_check_sfd()) == 0) {

	    // every OS tick 
	    if(last_nrk_tick!=_nrk_os_timer_get()) {
		last_nrk_tick=_nrk_os_timer_get();
		timer++;
		if(timer>ISA_TOKEN_TIMEOUT){
		    timer=0;
		    break;
		}
	    }
	}
	//printf("3 \n");
	_nrk_high_speed_timer_reset();
	// capture SFD transition with high speed timer
	//sfd_start_time=_nrk_high_speed_timer_get();
	tdma_start_tick=_nrk_os_timer_get();

	timeout = tdma_start_tick+4;
	// an interrupt could happen in here and mess things up
	if (n != 0) {
	    n = 0;
	// Packet on its way
	    while ((n = rf_polling_rx_packet (false,128)) == 0) {
	  //  	printf("4 \n");
		if (_nrk_os_timer_get () > timeout)
		{
		    //nrk_kprintf( PSTR("Pkt timed out\r\n") );
		    break;          // huge timeout as failsafe
		}
	    }
	}
	rf_rx_off (); 
	//printf("5 \n");
	if (n == 1 /*&& isa_rfRxInfo.length>0*/) {
	    if(isa_rfRxInfo.pPayload[SRC_INDEX]==isa_clk_src_id){//change
		// CRC and checksum passed
		isa_rx_data_ready = 1;
		//rtl_rx_slot = 0;
		//DHDR = (volatile)isa_rfRxInfo.pPayload[DHDR_INDEX];		
		global_slot += (volatile)isa_rfRxInfo.pPayload[SLOT_INDEX];
		nrk_led_toggle(GREEN_LED);
		putchar ('i');

		nrk_event_signal(SIG(isa_rx_pkt_signal));
		break;
		//
	    }
	}
    }


#ifdef LED_DEBUG
    nrk_led_clr(1);
#endif
     //printf("os_timer=%d\r\n",_nrk_os_itimer_get());
     #ifdef INIT_SYNC
	nrk_gpio_set(NRK_DEBUG_1);
     #endif
    _isa_sync_ok = 1;
    isa_rx_pkt_release();
    tmp_nrk_prev_timer_val=_nrk_prev_timer_val;
    _nrk_os_timer_stop();
    //_nrk_os_timer_reset();
    _nrk_set_next_wakeup(10);
    _nrk_os_timer_set(7);
    nrk_high_speed_timer_wait(0,SFD_TO_NEXT_SLOT_TIME);
    //_nrk_os_timer_reset();
    _nrk_os_timer_start();
    //_nrk_prev_timer_val=9;
    //printf("%d\n\r", _nrk_os_timer_get());
nrk_cur_task_TCB->next_wakeup = 10;
    //printf("%d\n\r",_nrk_prev_timer_val);
   // _nrk_high_speed_timer_reset();
   // slot_start_time=_nrk_high_speed_timer_get();
    #ifdef INIT_SYNC
	nrk_gpio_clr(NRK_DEBUG_1);
     #endif

	return _isa_sync_ok;

}


void isa_nw_task ()
{
    uint8_t slot;
    uint32_t slot_mask;
    uint16_t next_slot_offset = 0; 
    uint8_t FIRST = 1;
    _isa_ready = 0;
    
    // wait for isa ready 
    do {
        nrk_wait_until_next_period ();
    }while (_isa_ready == 0);

    _isa_ready = 1;
    nrk_gpio_clr(NRK_DEBUG_0);
    //nrk_time_get (&last_slot_time);// dont know if it is useful
    while (1) {

	//nrk_gpio_set(NRK_DEBUG_2);
	
	// reset high speed timer and then record the timer value used for calculating offsets
	_nrk_high_speed_timer_reset();
        slot_start_time = _nrk_high_speed_timer_get();
	//nrk_time_get (&last_slot_time);// dont know if it is useful
        last_slot = global_slot; //global_slot has been initialized to MAX_ISA_GLOBAL_SLOTS in isa_init()
        if (last_slot > MAX_ISA_GLOBAL_SLOTS)
            last_slot -= (MAX_ISA_GLOBAL_SLOTS+1);
	
	current_global_slot = global_slot;
	/* global_slot should be wrapped */
        if (global_slot > MAX_ISA_GLOBAL_SLOTS) {
            global_slot -= MAX_ISA_GLOBAL_SLOTS;
            global_cycle++;
        }
	
	slot = global_slot % ISA_SLOTS_PER_FRAME;
        slot_mask = ((uint32_t) 1) << slot;
	if(_isa_sync_ok == 1){
	    #ifdef CHANNEL_HOPPING
		channelIndex += next_slot_offset;
		currentChannel = slottedPattern[(channelIndex)&0x0F];
		isa_set_channel(currentChannel);
		if(slot>=22 && isa_node_mode == ISA_GATEWAY){
		   slowIndex = slowIndex % 3;
		   currentChannel = slowPattern[slowIndex];
		   isa_set_channel(currentChannel);
		   if(slot>=24)
		      slowIndex++;
		}
	//    printf("CH:%d SL: %d\r\n",currentChannel,slot);
	    #endif
	    //printf("%d,%d\n\r",currentChannel,(channelIndex)&0x0F);
	    //printf("isa_rx_data_ready:%d\r\n",isa_rx_data_ready);
	    // if TX slot mask and tx ready, send a packet
	    #ifdef JOIN_PROCESS
		if(slot>=22 && isa_node_mode == ISA_GATEWAY)
		   isa_tx_data_ready |= ((uint32_t) 1 << slot);
	    #endif
	    #ifdef TX_RX_DEBUG
		//printf("R\r\n");
		nrk_gpio_set(NRK_DEBUG_0);
	    #endif
            if (slot_mask & isa_tx_data_ready & isa_tdma_tx_mask){
		//printf("isa tx slot %d.\n\r",slot);
	//	printf("TX %d,%d,%d\n\r",currentChannel,(channelIndex)&0x0F,slot);
		//printf("tx\n\r");
		_isa_tx (slot);
		previous_tx_slot = slot; 
		#ifdef HIGH_TIMER_DEBUG
	    	    //printf("TX later, high speed timer value is %d.\n\r", _nrk_high_speed_timer_get());
		#endif		
	    } else if ((slot_mask & isa_tdma_rx_mask) && (isa_rx_data_ready == 0)){// if RX slot mask and rx not ready, send a packet
	    #ifdef TX_RX_DEBUG
		//printf("R\r\n");
		//nrk_gpio_set(NRK_DEBUG_0);
	    #endif
		//printf("isa rx slot %d.\n\r",slot);
	//	printf("RX %d,%d %d\n\r",currentChannel,(channelIndex)&0x0F, slot);
		//printf("rx\n\r");
		_isa_rx (slot);
	    } 
	    #ifdef TX_RX_DEBUG
		    nrk_gpio_clr(NRK_DEBUG_0);
		    //nrk_gpio_clr(NRK_DEBUG_1);
	    #endif
	    // if RX slot mask and RX buffer free, try to receive a packet
            /*else if ((slot_mask & rtl_tdma_rx_mask) && (rtl_rx_data_ready == 0)){ 
		_rtl_rx (slot);
	    }*/ 
	} else	{

	    ///do joining or sync request here
	    DHDR = configDHDR();
	    if(isa_node_mode != ISA_GATEWAY){//change
		#ifdef JOIN_PROCESS
		if(!_isa_join_ok){
		    _isa_join_process();
		}
		#endif

		_isa_sync_ok = _isa_init_sync();

		//printf("waiting for sync...isa_sync_ok is %d.\n\r",_isa_sync_ok);
	    }else if (isa_node_mode == ISA_GATEWAY){
		_isa_sync_ok = 1;
	    }	
	}
	
	//nrk_gpio_clr(NRK_DEBUG_2); 
	//printf("next_slot_offset %d\n\r",next_slot_offset);
	// To make sure that initial sync is correct
        /*if(FIRST){
		//printf("%d\n\r",_nrk_os_timer_get ());
		next_slot_offset = 1;
		FIRST = 0;
	}*/
	if(slot_expired >= EXPIRATION && isa_node_mode != ISA_GATEWAY){
	    //printf("re-sync\n\r");
	    _isa_sync_ok = 0;
	    slot_expired = 0;    
	    global_slot = 100;
	    next_slot_offset = 0;
	    resync_times++;
	    if(isa_id!=0){
	        channelIndex = isa_clk_src_id;
	        currentChannel = slottedPattern[channelIndex];    
	    }else{
	        channelIndex = 0;
	        currentChannel = slottedPattern[channelIndex];
	    }
	    isa_set_channel(currentChannel);


	}else{
	    //printf("global_slot is %d. global cycle is %d.\n\r",global_slot,global_cycle);
	    next_slot_offset = isa_get_slots_until_next_wakeup (global_slot);
	    //printf("NOS:%d\n\r",next_slot_offset);
	    //printf("%d,%d,%d,%d\n\r",_nrk_os_timer_get (),_nrk_get_next_wakeup (),global_slot,next_slot_offset);
	    global_slot += next_slot_offset;
            //nrk_clr_led (1);
	    #ifdef LED_SLOT_DEBUG
	    nrk_led_clr(0);
	    #endif

	    offsetY = 0;
	    //printf("%d\n\r",next_slot_offset);

            nrk_wait_until_next_n_periods (next_slot_offset);


	    #ifdef LED_SLOT_DEBUG
	    nrk_led_set(0);
	    #endif
	//}
        //nrk_set_led (1);
        // Set last_slot_time to the time of the start of the slot
	}
    }
}

void isa_task_config ()
{
    isa_task.task = isa_nw_task;
    nrk_task_set_stk( &isa_task, isa_task_stack, ISA_STACK_SIZE);
    isa_task.prio = 20;
    isa_task.FirstActivation = TRUE;
    isa_task.Type = BASIC_TASK;
    isa_task.SchType = PREEMPTIVE;
    isa_task.period.secs = 0;
    isa_task.period.nano_secs = 10*NANOS_PER_MS;
    isa_task.cpu_reserve.secs = 0;      
    isa_task.cpu_reserve.nano_secs = 0;
    isa_task.offset.secs = 0;
    isa_task.offset.nano_secs = 0;
    nrk_activate_task (&isa_task);
}
