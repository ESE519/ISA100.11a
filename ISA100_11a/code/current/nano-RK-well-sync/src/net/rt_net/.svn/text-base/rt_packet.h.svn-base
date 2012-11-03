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


#ifndef _RT_PACKET_H
#define _RT_PACKET_H


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

#endif
