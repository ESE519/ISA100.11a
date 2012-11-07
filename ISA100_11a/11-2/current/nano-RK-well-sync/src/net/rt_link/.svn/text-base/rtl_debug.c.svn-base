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


#include <rtl_debug.h>
#include <nrk.h>
#include <include.h>

void rtl_debug_time_update(uint16_t offset)
{
    rtl_drift_rate=offset-prev_offset;
    prev_offset=offset;	
}

int16_t rtl_debug_time_get_drift()
{
return rtl_drift_rate;
}

void rtl_debug_init()
{
uint8_t i;
for(i=0; i<MAX_PKT_LOG/8; i++ )
        pkt_log[i]=0xFF;
for(i=0; i<MAX_TSYNC_LOG/8; i++ )
        tsync_log[i]=0xFF;
_dbg_index=0;
prev_offset=0;
}

uint8_t bit_count(uint8_t n)
{
n = (n & 0x55) + ((n>>1) & 0x55);
n = (n & 0x33) + ((n>>2) & 0x33);
n = (n & 0x0F) + ((n>>4) & 0x0F);
return n;
}

uint8_t rtl_debug_get_pkt_loss()
{
uint8_t i,j,cnt,tmp;
cnt=0;
for(i=0; i<MAX_PKT_LOG/8; i++ )
        cnt+=bit_count(pkt_log[i]);
return cnt;
}

uint8_t rtl_debug_get_tsync_loss()
{
uint8_t i,j,cnt,tmp;
cnt=0;
for(i=0; i<MAX_TSYNC_LOG/8; i++ )
        cnt+=bit_count(tsync_log[i]);
return cnt;
}



void rtl_debug_rx_pkt()
{
uint8_t offset;
offset=_dbg_index/8;
pkt_log[offset]|=(1<<(7-(_dbg_index%8)));
_dbg_index++;
if(_dbg_index>MAX_PKT_LOG) _dbg_index=0;
}

void rtl_debug_dropped_pkt()
{
uint8_t offset;
offset=_dbg_index/8;
pkt_log[offset]&=~(1<<(7-(_dbg_index%8)));
_dbg_index++;
if(_dbg_index>MAX_PKT_LOG) _dbg_index=0;
}

uint8_t rtl_debug_get_tsync_delay(uint8_t index)
{

if(index<MAX_TSYNC_LOG) return tsync_delay[index];
return 0;
}


void rtl_debug_rx_tsync()
{
uint8_t offset;
offset=_tsync_index/8;
tsync_log[offset]|=(1<<(7-(_tsync_index%8)));
tsync_delay[_tsync_index]=0;
_tsync_index++;
if(_tsync_index>MAX_TSYNC_LOG) _tsync_index=0;
}

void rtl_debug_dropped_tsync(uint8_t delay)
{
uint8_t offset;
offset=_tsync_index/8;
tsync_log[offset]&=~(1<<(7-(_tsync_index%8)));
tsync_delay[_tsync_index]=delay;
_tsync_index++;
if(_tsync_index>MAX_TSYNC_LOG) _tsync_index=0;
}

