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
*  Zane Starr
*******************************************************************************/


#ifndef __DSR_H_
#define __DSR_H_
#include<stdio.h>
#include<nrk_cfg.h>
#include<nrk_time.h>
#include<nrk_events.h>
#include<bmac.h> 
#include<include.h> 

#define EVENT_DSR_PKT_RX 14
#define EVENT_DSR_PKT_TX 13
#define DSR_PATH_FOUND_EVENT 12

#define DSR_PACKET_TIMEOUT  11
#define DSR_MAX_HOP_COUNT 10
#define MAX_ROUTE  16
#define MAX_PATH_LEN 16
#define MAX_MAC_ADDR 128
#define TABLE_CLEAN_TIME 10
#define ROUTE_REQUEST 1
#define ROUTE_RETURN  2
#define DATA_PACKET   3
#define DSR_STACK_SIZE 128 
nrk_task_type dsr_task;
NRK_STK dsr_task_stack[DSR_STACK_SIZE];

void dsr_network_task();
void dsr_task_config();
uint8_t dsr_add_path(uint8_t *route);
uint8_t* dsr_get_path(uint8_t mac);
//This is where the magic happens prototype doesn't link properly
void dsr_rate_adjust(uint8_t increase);//increase rate decrease timeout rate

void _dsr_tx_packet(uint8_t dest,uint8_t* payload,uint8_t len);
void dsr_tx_packet(uint8_t dest,uint8_t* payload,uint8_t len);
uint8_t dsr_request_path();

typedef struct  DSR_PKT
{
  uint8_t *src_route;
  uint8_t src_cnt;//your theo pos in the route
  uint8_t dest_cnt;// how far in the pos is final destination
  uint8_t type; // type of packet
  uint8_t len;//length of your packet
  uint8_t *payload;
}DSR_PKT_INFO;
DSR_PKT_INFO *  dsr_rx_pkt();

#endif
