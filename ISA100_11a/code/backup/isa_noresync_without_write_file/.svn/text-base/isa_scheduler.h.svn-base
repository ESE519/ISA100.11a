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

/* For ISA */
#ifndef _ISA_SCHEDULER_H_
#define _ISA_SCHEDULER_H_

#define ISA_SLOTS_PER_FRAME 25

uint8_t isa_sched[ISA_SLOTS_PER_FRAME];

uint32_t isa_tdma_rx_mask;
uint32_t isa_tdma_tx_mask; 

void _isa_clear_sched_cache ();
uint16_t isa_get_slots_until_next_wakeup (uint16_t current_slot);

#endif
