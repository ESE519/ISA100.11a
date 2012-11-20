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


#include <isa.h>
#include <isa_scheduler.h>
#include <include.h>
#include <nrk_error.h>

/* For ISA */
/* This method is only for demo 1. Need to be updated!! */
int8_t isa_set_schedule (isa_node_mode_t isa_node_mode, uint8_t clk_src_id)
{
    char i =0;
    isa_clk_src_id = clk_src_id;//change
    if (isa_node_mode==ISA_GATEWAY){
        isa_tdma_tx_mask |= ((uint32_t) 1) << 2;
	isa_tdma_rx_mask |= ((uint32_t) 1) << 3;//change for test
	//isa_tdma_tx_mask |= ((uint32_t) 1) << 1;
	//isa_tdma_rx_mask |= ((uint32_t) 1) << 4;

	//isa_sched[1] = 1;
	isa_sched[2] = 1;//change for test
	isa_sched[3] = 1;
	//isa_sched[4] = 1;
    }
    else if (isa_node_mode==ISA_REPEATER){ //change
        isa_tdma_rx_mask |= ((uint32_t) 1) << 5;
	isa_tdma_tx_mask |= ((uint32_t) 1) << 7;//change for test
	isa_tdma_tx_mask |= ((uint32_t) 1) << 8;
	isa_tdma_rx_mask |= ((uint32_t) 1) << 9;

	isa_sched[7] = 1;
	isa_sched[8] = 1;//change for test
	isa_sched[5] = 1;
	isa_sched[9] = 1;

    }
    else if(isa_node_mode==ISA_RECIPIENT){
	isa_tdma_tx_mask |= ((uint32_t) 1) << 1;
	//isa_tdma_rx_mask |= ((uint32_t) 1) << 1;//change for test
	isa_tdma_rx_mask |= ((uint32_t) 1) << 4;
	//isa_tdma_rx_mask |= ((uint32_t) 1) << 0;

//	isa_sched[0] = 1;
	//isa_sched[1] = 1;//change for test
	isa_sched[1] = 1;
	isa_sched[4] = 1;
    }


    /*printf("isa_scheduler.h, isa_set_schedule():\n\r");
    for(i=0;i<25;i++)
	printf("%d,",isa_sched[i]);
    printf("\n\r");*/
    
return NRK_OK;
}

/**
 * isa_get_schedule()
 *
 * This function returns the stored schedule for a particular slot.
 * 
 * Return: schedule value
 */
int8_t isa_get_schedule (uint8_t slot)
{
    if (slot > ISA_SLOTS_PER_FRAME)
        return NRK_ERROR;

    return isa_sched[slot];
}

/**
 * _isa_clear_sched_cache()
 *
 * This function is called by the timer interrupt at the
 * start of each ISA cycle to remove any cached scheduling
 * values.  Only call this if you are reseting the ISA frames.
 */
void _isa_clear_sched_cache ()
{
    uint8_t i;
// FIXME compress this shit later...
    for (i = 0; i < ISA_SLOTS_PER_FRAME; i++) {
        isa_sched[i] = 0;
    }
}



