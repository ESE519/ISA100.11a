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



#ifndef _ISA_H
#define _ISA_H
#include <include.h>
#include <isa_scheduler.h>
#include <basic_rf.h>
#include <nrk.h>
#include <nrk_cfg.h>

/*For isa link define */
#define MAX_ISA_GLOBAL_SLOTS 100
#define ISA_STACK_SIZE 128 
#define ISA_TOKEN_TIMEOUT 10000

/* For isa link*/

typedef enum{
    DHDR_INDEX=0,
    DHR_INDEX=0,
    DMXHR_INDEX=1,
    DAUX_INDEX=5,
    DROUT_INDEX=34,
    DADDR_INDEX=37,
    //SLOT_INDEX=6,
    SLOT_INDEX=1,
    SRC_INDEX=2,//change
    OFFSET_HIGH=1,
    OFFSET_LOW=2,
    //PKT_DATA_START=3
    PKT_DATA_START=42
} isa_pkt_field_t;

uint8_t isa_id;
uint8_t isa_clk_src_id;//change
uint8_t tx_slot_from_join[4]; //change
uint32_t isa_rx_data_ready;
uint32_t isa_tx_data_ready;
uint8_t DHDR;
uint16_t last_slot;

typedef struct {
    int8_t length;
    int8_t DHDR;
    uint8_t *pPayload;
} ISA_TX_INFO;

ISA_TX_INFO	isa_tx_info[ISA_SLOTS_PER_FRAME];

typedef struct {
    uint16_t mac_addr;
    uint8_t channel;
    uint8_t power;
    uint16_t tx_guard_time;
    uint16_t rx_timeout;
    uint8_t mobile_sync_timeout;
} isa_param_t;

isa_param_t isa_param;

typedef enum {
	ISA_RECIPIENT,
	ISA_REPEATER,
	ISA_GATEWAY,//change
	ISA_ROUTER
} isa_node_mode_t;

isa_node_mode_t isa_node_mode;

nrk_task_type isa_task;



/* declare task stack for storing isa tasks */
NRK_STK isa_task_stack[ISA_STACK_SIZE];

/* rx or tx structure */
RF_RX_INFO isa_rfRxInfo;
RF_TX_INFO isa_ack_tx;
uint8_t isa_ack_buf[4];

volatile RF_TX_INFO isa_rfTxInfo;

uint8_t _isa_ready; //flag indicating isa protocol is ready
uint8_t _isa_join_ok; //flag indicating join process success

/********************* config function ***********************************/
void isa_start();
void isa_task_config ();
uint8_t isa_init(isa_node_mode_t mode,uint8_t id, uint8_t src_id);
void isa_nw_task();
void isa_set_channel (uint8_t chan);
int8_t isa_ready();
int8_t configDHDR();

/********************* waiting function ***********************************/
int8_t isa_wait_until_rx_pkt ();
int8_t isa_wait_until_rx_or_tx ();

/********************* rx and tx function ***********************************/
int8_t isa_rx_pkt_set_buffer(uint8_t *buf, uint8_t size);
void _isa_rx(uint8_t slot);
int8_t isa_rx_pkt_check();
void isa_rx_pkt_release();
void _isa_tx (uint8_t slot);
int8_t isa_tx_pkt (uint8_t *tx_buf, uint8_t len, uint8_t DHDR, uint8_t slot);
uint8_t* isa_rx_pkt_get (uint8_t *len, int8_t *rssi);
int8_t isa_tx_pkt_check(uint8_t slot);


/********************* isa_scheduler.c ***********************************/
int8_t isa_set_schedule (isa_node_mode_t isa_node_mode, uint8_t clk_src_id);
int8_t isa_get_schedule (uint8_t slot);

#endif
