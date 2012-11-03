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
*******************************************************************************/

#include <nrk_cfg.h>

#ifdef NRK_MAX_RESERVES

#include <nrk_time.h>
#include <nrk.h>
#include <nrk_error.h>
#include <nrk_reserve.h>

nrk_reserve _nrk_reserve[NRK_MAX_RESERVES];
//experimental


// This function is called by the kernel at startup
// to clear the list of reserves
void _nrk_reserve_init ()
{
  uint8_t i;

  for (i = 0; i < NRK_MAX_RESERVES; i++)
    _nrk_reserve[i].active = -1;
}

// This function returns the id of a free reserve
// This returns NRK_ERROR if there are no free reserves
int8_t nrk_reserve_create ()
{
  int8_t i;
  for (i = 0; i < NRK_MAX_RESERVES; i++) {
    if (_nrk_reserve[i].active == -1) {
      // Check and Accept
      _nrk_reserve[i].active = 1;
      return i;
    }
  }

  return NRK_ERROR;
}

// This function frees a current reserve
// This returns NRK_ERROR if the reserve does not exist
int8_t nrk_reserve_delete (uint8_t reserve_id)
{
  int8_t i;
  if (reserve_id >= 0 && reserve_id < NRK_MAX_RESERVES) {
    if (_nrk_reserve[i].active == 1) {
      _nrk_reserve[i].active = 0;
      return NRK_OK;
    }
  }
  return NRK_ERROR;
}




uint8_t nrk_reserve_get (uint8_t reserve_id)
{

  if (reserve_id >= NRK_MAX_RESERVES) {
    _nrk_errno_set (1);
    return 0;
  }
  if (_nrk_reserve[reserve_id].active == -1) {
    // Reserve isn't active 
    _nrk_errno_set (2);
    return 0;
  }

  _nrk_reserve_update (reserve_id);

  if (_nrk_reserve[reserve_id].cur_access >
      _nrk_reserve[reserve_id].set_access)
    return 0;
  return (_nrk_reserve[reserve_id].set_access -
          _nrk_reserve[reserve_id].cur_access);
}

void _nrk_reserve_update (uint8_t reserve_id)
{
  nrk_time_t t;

  nrk_int_disable ();
  nrk_time_get (&t);

  _nrk_reserve[reserve_id].cur_time = (int32_t) _nrk_time_to_ticks_long (t);
  if (_nrk_reserve[reserve_id].cur_time >= _nrk_reserve[reserve_id].set_time) {
    // If the reserve is passed its period then replenish it
    _nrk_reserve[reserve_id].set_time =
      _nrk_reserve[reserve_id].cur_time +
      _nrk_reserve[reserve_id].period_ticks;
    _nrk_reserve[reserve_id].cur_access = 0;
  }
  nrk_int_enable ();

}

int8_t nrk_reserve_consume (uint8_t reserve_id)
{

  if (reserve_id >= NRK_MAX_RESERVES) {
    _nrk_errno_set (1);
    return NRK_ERROR;
  }
  if (_nrk_reserve[reserve_id].active == -1) {
    _nrk_errno_set (2);
    return NRK_ERROR;
  }

  _nrk_reserve_update (reserve_id);

  if ((_nrk_reserve[reserve_id].set_access <=
       _nrk_reserve[reserve_id].cur_access)) {
    // You violated your resource (like MJ after a little boy)
    nrk_int_enable ();
    if (_nrk_reserve[reserve_id].error != NULL)
      _nrk_reserve[reserve_id].error ();
    return NRK_ERROR;
  }
  else {
    // Reserve is fine. Take some of it.
    _nrk_reserve[reserve_id].cur_access++;
  }


  return NRK_OK;
}

int8_t nrk_reserve_set (uint8_t id, nrk_time_t * period, int16_t access_count,
                        void *errhandler)
{
  nrk_time_t tmp_time;

  if (id >= NRK_MAX_RESERVES)
    return NRK_ERROR;
  if (_nrk_reserve[id].active == -1)
    return NRK_ERROR;

  tmp_time.secs = period->secs;
  tmp_time.nano_secs = period->nano_secs;
  _nrk_reserve[id].period_ticks = _nrk_time_to_ticks_long (tmp_time);
  _nrk_reserve[id].set_access = access_count;
  _nrk_reserve[id].cur_access = 0;

  nrk_time_get (&tmp_time);
  _nrk_reserve[id].cur_time = (uint32_t) _nrk_time_to_ticks_long (tmp_time);
  _nrk_reserve[id].set_time =
    _nrk_reserve[id].cur_time + _nrk_reserve[id].period_ticks;
  _nrk_reserve[id].error = (void *) errhandler;

  return NRK_OK;
}

#endif
