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
*  Zane Starr
*  Anand Eswaren
*******************************************************************************/



#ifndef _RT_LINK_H
#define _RT_LINK_H
#include <include.h>
#include <basic_rf.h>
#include <nrk.h>
#include <nrk_cfg.h>

#define RT_LINK_STACK_SIZE 128 

#define MAX_RTL_PKT_SIZE	RF_MAX_PAYLOAD_SIZE

#define RTL_TOKEN_NOT_SENT  	 0
#define RTL_TOKEN_SENT   	 1
// token timeout in ms
#define RTL_TOKEN_TIMEOUT	 10000
// token timeout multiplier for battery save
#define RTL_BATTERY_SAVE_TIMEOUT 5 


// max slots
#define MAX_SLOTS 1024

#ifndef MAX_ABS_WAKEUP
#define MAX_ABS_WAKEUP	4
#endif

#define TDMA_FRAME_SLOTS   32
// flag to define when contention slots are being accessed
#define RTL_CONTENTION 33

// Sync wakeup slot of 1000 gives 24*5ms worth of warmup time
#define _RTL_SYNC_WAKEUP_SLOT (MAX_SLOTS-10)


nrk_task_type rtl_task;
NRK_STK rtl_task_stack[RT_LINK_STACK_SIZE];

// Fill this in with all packet fields.
typedef enum {
	// Global slot requires 10 bits to hold 1024 slots
	// hence it uses 2 bytes.
	GLOBAL_SLOT=0,
	// This token is used to ensure that a node never
	// synchronizes off of a node that has an older sync 
	// value.
	TIME_SYNC_TOKEN=2,
	PKT_DATA_START=3
} rtl_pkt_field_t;


typedef enum {
	RTL_MOBILE,
	RTL_FIXED,
	RTL_COORDINATOR
} rtl_node_mode_t;

rtl_node_mode_t rtl_node_mode;


// some handy book keeping values
uint16_t last_slot;
nrk_time_t last_slot_time;


void rtl_task_config ();

// Status API
uint8_t rtl_sync_status();

// Configure API
void rtl_init(rtl_node_mode_t mode);
void rtl_start();
void rtl_set_tx_power(uint8_t pwr);
void rtl_set_channel(uint8_t chan);


// Transmit API
int8_t rtl_tx_pkt_check(uint8_t slot);
int8_t rtl_tx_pkt (uint8_t *buf, uint8_t len, uint8_t slot);
int8_t rtl_tx_abs_pkt (uint8_t *buf, uint8_t len, uint16_t abs_slot);
int8_t rtl_wait_until_tx_done(uint8_t slot);


// Receive API
int8_t rtl_rx_pkt_check();
uint8_t* rtl_rx_pkt_get (uint8_t *len, int8_t *rssi, uint8_t *slot);
void rtl_rx_pkt_release();
int8_t rtl_wait_until_rx_pkt();
int8_t rtl_rx_pkt_set_buffer(uint8_t *buf, uint8_t size);


// Signal Functions
int8_t rtl_get_tx_done_signal();
int8_t rtl_get_rx_pkt_signal();


// RTL only functions
uint16_t rtl_get_global_slot();
uint8_t _rtl_sync_ok;
volatile uint8_t _rtl_ready;


void rtl_nw_task();
int8_t rtl_ready();


uint8_t _rtl_time_token;
uint8_t _rtl_time_token_status;


RF_TX_INFO rtl_tsync_tx;
uint8_t rtl_tsync_buf[PKT_DATA_START];

uint8_t _rtl_contention_pending;
uint8_t _rtl_contention_slots;


RF_RX_INFO rtl_rfRxInfo;
uint8_t rtl_rx_data_ready;
volatile uint8_t rtl_rx_slot;
// This is the link layer's only buffer
volatile uint8_t rtl_rx_buf[RF_MAX_PAYLOAD_SIZE];  

// Extra slot at end for abs slot buffer pointer  
volatile RF_TX_INFO rtl_rfTxInfo;

typedef struct {
	int8_t length;
	uint8_t *pPayload;
} RTL_TX_INFO;


RTL_TX_INFO	rtl_tx_info[TDMA_FRAME_SLOTS+1]; 

uint16_t rtl_abs_tx_slot;
uint8_t rtl_abs_tx_ready;
uint32_t rtl_tx_data_ready;


uint16_t rtl_get_slot();
// the core tx function called by the scheduler
void _rtl_tx (uint8_t slot);      
// the core rx funtion called by the scheduler
void _rtl_rx (uint8_t slot);      
// the core rx funtion called by the scheduler
uint8_t _rtl_rx_sync ();          

typedef enum {
    RTL_RX,
    RTL_TX,
} rtl_rx_tx_t;

typedef struct {
    uint16_t mac_addr;
    uint8_t channel;
    uint8_t power;
    uint16_t tx_guard_time;
    uint16_t rx_timeout;
    uint8_t mobile_sync_timeout;
} rtl_param_t;

rtl_param_t rtl_param;

// Each set bit in the mask designates that the slot is used.
// The actual schedule of which frames use the slot are computed
// from the schedule array.  The frame scheduling options are log
// valued (i.e. 1,2,4,8,16,32, [64,128] ) and selected using 4 bits
// of the sched value.  sched[0] sets the schedule for slots 0 and 1.
// slot 0 from the LSB and slot 1 from the MSB.  The log mappings are:
// 0 -> ERROR not in mask
// 1 -> 1  (every frame)
// 2 -> 2  (ever 2nd frame)
// 3 -> 4  (ever 4th frame)
// 4 -> 8  ( etc... )
// 5 -> 16
// 6 -> 32
uint32_t rtl_tdma_rx_mask;
uint32_t rtl_tdma_tx_mask;        


// Event Callback functions
void (*rx_callback)(uint8_t slot);
void (*tx_callback)(uint8_t slot);
void (*abs_callback)(uint16_t global_slot);
void (*slot_callback)(uint16_t global_slot);
void (*cycle_callback)(uint16_t global_cycle); // cycles since the epoch

void rtl_set_abs_callback(void *fp);
void rtl_set_rx_callback(void *fp);
void rtl_set_tx_callback(void *fp);
void rtl_set_cycle_callback(void *fp);
void rtl_set_slot_callback(void *fp);


/********************* rtl_scheduler.c ***********************************/

int8_t rtl_set_abs_wakeup (uint16_t slot, uint8_t repeat);
void rtl_clr_abs_wakeup (uint16_t slot);
void rtl_set_contention(uint8_t slot, uint8_t rate);
int8_t rtl_set_schedule (rtl_rx_tx_t rx_tx, uint8_t slot, uint8_t schedule);
int8_t rtl_clr_schedule (rtl_rx_tx_t rx_tx, uint8_t slot);
int8_t rtl_get_schedule (uint8_t slot);
uint16_t rtl_get_slots_until_next_wakeup (uint16_t current_slot);





#endif
