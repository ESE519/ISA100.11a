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


#include <rt_link.h>
#include <rtl_scheduler.h>
#include <include.h>
#include <nrk_error.h>


/**
 * rtl_set_schedule ( rx_tx, slot, sched )
 *
 * Arguments: rtl_rx_tx_t rx_tx takes either RTL_RX or RTL_TX to set if you
 *            are setting an RX or a TX schedule.
 *            uint8_t slot sets which RX or TX slot you wish to schedule
 *            uint8_t sched takes in the log based schedule you wish to set
 *            		sched	counts
 *            		0	reserved
 *            		1	1
 *            		2	2
 *            		3	4
 *            		4	8
 *            		5	16
 *            		6	32
 *            		7	reserve
 *            		8	reserve
 *
 * example: rtl_set_schedule( RTL_RX, 5, 3 );
 *
 * Return: NRK_OK if good, NRK_ERROR if failed
 */
int8_t rtl_set_schedule (rtl_rx_tx_t rx_tx, uint8_t slot, uint8_t sched)
{
    uint32_t t_mask;
    uint8_t dslot;
    if (sched > 15 || slot > 31)
        return NRK_ERROR;
    t_mask = 0;
    t_mask = ((uint32_t) 1) << slot;
    if (rx_tx == RTL_RX)
        rtl_tdma_rx_mask |= t_mask;
    else
        rtl_tdma_tx_mask |= t_mask;
    dslot = slot >> 1;
    if (slot % 2 == 0) {
        rtl_sched[dslot] = rtl_sched[dslot] & 0xF0;
        rtl_sched[dslot] = rtl_sched[dslot] | sched;
    }
    else {
        rtl_sched[dslot] = rtl_sched[dslot] & 0x0F;
        rtl_sched[dslot] = rtl_sched[dslot] | ((sched << 4) & 0xF0);
    }
return NRK_OK;
/*
   printf( "slot = %d sched = %d\n", slot,sched );
   printf( "index = %d\n", dslot);
   printf( "value = %d\n", rtl_rx_sched[dslot]);
   printf( "mask = %x\n", rtl_tdma_rx_mask );
   printf( "--------------------------\n");
*/
}





/**
 * rtl_get_schedule()
 *
 * This function returns the stored schedule for a particular slot.
 * Since you can't have RX and TX on the same slot, it assumes you know
 * if it is an RX or TX slot.
 *
 * Return: schedule value in its original form 1-6
 */
int8_t rtl_get_schedule (uint8_t slot)
{
    uint8_t dslot;

    if (slot > 31)
        return NRK_ERROR;

    dslot = slot >> 1;
    if (slot % 2 == 0)
        return (rtl_sched[dslot] & 0x0F);
    else
        return (rtl_sched[dslot] >> 4);
}

/**
 * rtl_clr_schedule()
 *
 * This function clears an already scheduled slot so that it
 * is not called by the scheduler anymore.  Use this to delete
 * slots when they are not wanted anymore.
 *
 * Arguments: rtl_rx_tx_t rx_tx is either RTL_RX or RTL_TX depending on if it
 *            is a TX or RX slot
 *            uint8_t slot is the slot number starting from 0
 *
 * Return: NRK_OK upon success, NRK_ERROR on failure
 */
int8_t rtl_clr_schedule (rtl_rx_tx_t rx_tx, uint8_t slot)
{
    uint32_t t_mask;
    uint8_t dslot;
    if (slot > 31)
        return NRK_ERROR;
    t_mask = 0;
    t_mask = 1 << slot;
    if (rx_tx == RTL_RX)
        rtl_tdma_rx_mask &= ~t_mask;
    else
        rtl_tdma_tx_mask &= ~t_mask;
    dslot = slot >> 1;
    if (slot % 2 == 0)
        rtl_sched[dslot] = rtl_sched[dslot] & 0xF0;
    else
        rtl_sched[dslot] = rtl_sched[dslot] & 0x0F;
}

/**
 * _rtl_clr_abs_all_wakeup()
 *
 * This function clears all absolute wakeups. 
 */
void _rtl_clr_abs_all_wakeup ()
{
    uint8_t i;
    for (i = 0; i < MAX_ABS_WAKEUP; i++)
        rtl_abs_wakeup[i] = MAX_SLOTS + 1;
}

/**
 * rtl_set_abs_wakeup()
 *
 * This function sets an absolute wakeup.  An absolute wakeup
 * is a 16 bit slot value 0-1024 that signifies that the interrupt
 * will be called at this point of each cycle.  This function does
 * not check if a duplicate wakeup exists, so try to avoid them.  
 * There is only room for MAX_ABS_WAKEUP number of scheduled events.
 *
 * Argument: uint16_t slot is the value of a slot between 0 and 1024
 * 	     uint8_t repeat is 1 if this should always trigger each cycle
 * 	           if repeat is 0, it is cleared after it triggers once.
 *
 * Return: 1 on success, 0 if no slots are available
 *
 */
int8_t rtl_set_abs_wakeup (uint16_t slot, uint8_t repeat)
{
    uint8_t i;
    uint16_t tmp;
    for (i = 0; i < MAX_ABS_WAKEUP; i++) {
	tmp=rtl_abs_wakeup[i]&0x7FFF;
        if (tmp > MAX_SLOTS) {
	    rtl_abs_wakeup[i] = slot;
	    if(repeat) rtl_abs_wakeup[i]|=0x8000;
            return NRK_OK;
        }
    }
return NRK_ERROR;
}

/**
 * _rtl_clr_abs_wakeup()
 *
 * This function clears an already set absolute wakeup.
 */
void _rtl_clr_abs_wakeup (uint16_t slot)
{
    uint8_t i;
    
    for (i = 0; i < MAX_ABS_WAKEUP; i++) {
    	uint16_t tmp;
	tmp=rtl_abs_wakeup[i]&0x7FFF;
        if (tmp == slot) {
            rtl_abs_wakeup[i] = MAX_SLOTS + 1;
            return;
        }
    }
}

/**
 * _rtl_match_abs_wakeup()
 *
 * This function is called by the interrupt timer to check to
 * see if there is a scheduled slot at the current time.
 *
 * Return: 1 if this is a scheduled slot, 0 otherwise
 */
uint8_t _rtl_match_abs_wakeup (uint16_t global_slot)
{
    uint8_t i;
    for (i = 0; i < MAX_ABS_WAKEUP; i++) {
    	uint16_t tmp;
	tmp=rtl_abs_wakeup[i]&0x7FFF;
        if (tmp == global_slot)
	{
	    //if( (rtl_abs_wakeup[i]&0x8000)==0 )
	    if( rtl_abs_wakeup[i]<=0x8000 )
	    	{
		// If it is not a repeat slot, clear it
		_rtl_clr_abs_wakeup(global_slot);
		}
            return 1;
	}
    }
    return 0;
}

/**
 * _rtl_get_next_abs_wakeup()
 *
 * This function returns the number of slots between the given
 * global_slot and the next already scheduled absolute wakeup.
 * This is used by the timer interrupt to help schedule when it needs 
 * to wakeup again.
 * 
 * Arguments: global_slot is the current slot
 *
 * Return: uint16_t with the offset until the next absolute wakeup.  If the
 * 	   next wakeup is greater than 1024, then 0 is returned.
 */
uint16_t _rtl_get_next_abs_wakeup (uint16_t global_slot)
{
    uint8_t i;
    int16_t min;
    int16_t tmp;
    min = MAX_SLOTS +1;

    for (i = 0; i < MAX_ABS_WAKEUP; i++) {
	if((rtl_abs_wakeup[i]&0x7FFF)<MAX_SLOTS)
	{
        tmp = (rtl_abs_wakeup[i]&0x7FFF) - global_slot;
        if (tmp > 0 && tmp < min)
            min = tmp;
	}
    }
    if (min == MAX_SLOTS +1)
        min = 0;
    return min;
}

/**
 * _rtl_clear_sched_cache()
 *
 * This function is called by the timer interrupt at the
 * start of each TDMA cycle to remove any cached scheduling
 * values.  Only call this if you are reseting the TDMA frames.
 */
void _rtl_clear_sched_cache ()
{
    uint8_t i;
// FIXME compress this shit later...
    for (i = 0; i < 32; i++) {
        rtl_sched_cache[i] = 0;
    }
}


/**
 * rtl_get_slots_until_next_wakeup()
 *
 * This function returns the absolute number of slots between the current_slot
 * and the next RX/TX related wakeup.  It uses an internal cache to allow for
 * faster computation.
 *
 * Argument: current_slot is the current slot
 * Return: uint16_t number of slots until the next wakeup
 */
uint16_t rtl_get_slots_until_next_wakeup (uint16_t current_slot)
{
    uint16_t abs_slot;
    uint16_t min_slot;
    uint8_t test_slot;
    uint8_t test_frame;
    uint8_t frame_inc;

//total_slot = (((uint16_t)current_frame)<<5) + current_slot; 
    min_slot = MAX_SLOTS + 1;
    for (test_slot = 0; test_slot < 32; test_slot++) {
        uint8_t s;
        s = rtl_get_schedule (test_slot);
        if (s == 0)
            continue;
        s--;
        //test_frame=0;   
        test_frame = rtl_sched_cache[test_slot];
        frame_inc = _rtl_pow (2, s);
        do {
            abs_slot = (((uint16_t) test_frame) << 5) + test_slot;
            if (abs_slot <= current_slot)
                test_frame += frame_inc;
            //printf_u( "." );
        } while (abs_slot <= current_slot);
        rtl_sched_cache[test_slot] = test_frame;
         //printf( "current_slot = %d,  test_slot = %d, abs_slot=%d\n",current_slot, test_slot, abs_slot );
        if (abs_slot - current_slot < min_slot && abs_slot < MAX_SLOTS + 1)
            min_slot = abs_slot - current_slot;
    }
// If next slot is in the next TDMA cycle, return 0 to wakeup at the start if the frame.
    if (min_slot > MAX_SLOTS)
        return 0;
    return min_slot;
}

/**
 * _rtl_pow()
 *
 * This is a little helper function to do powers in order to 
 * resolve the schedules.  This is all 8 bit and should not
 * be forced to overflow.
 *
 * Arguments: x and y to compute x^y
 * Return: x^y
 */
uint8_t _rtl_pow (uint8_t x, uint8_t y)
{
    uint8_t acc, i;
    if (y == 0)
        return 1;
    acc = x;
    for (i = 0; i < y - 1; i++)
        acc = acc * x;
    return acc;
}
