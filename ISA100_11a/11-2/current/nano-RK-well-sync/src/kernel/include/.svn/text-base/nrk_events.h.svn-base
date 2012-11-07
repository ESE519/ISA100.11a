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
*  Anthony Rowe
*******************************************************************************/


#ifndef NRK_EVENTS_H
#define NRK_EVENTS_H

#include <nrk_time.h>

#define SIG(x)  ((uint32_t)1)<<x 

typedef int8_t nrk_sig_t;
typedef uint32_t nrk_sig_mask_t;

//typedef uint8_t nrk_sem_t;
typedef struct semaphore_type {
int8_t count;
int8_t resource_ceiling;
int8_t value;
} nrk_sem_t;


uint32_t _nrk_signal_list;

uint32_t nrk_signal_get_registered_mask();
int8_t nrk_signal_delete(nrk_sig_t sig_id); //removes any tasks association with signal including unsuspends tasks that wer waiting on signal removed
int8_t nrk_signal_unregister(int8_t sig_id);
int8_t nrk_signal_register(int8_t sig_id);
int8_t nrk_signal_create(); // returns signal

int8_t nrk_event_signal(int8_t event_num);
uint32_t nrk_event_wait(uint32_t event_num);

nrk_sem_t* nrk_sem_create(uint8_t count,uint8_t ceiling_prio);

int8_t  nrk_sem_delete(nrk_sem_t *resrc);
int8_t nrk_get_resource_index(nrk_sem_t *resrc);
int8_t nrk_sem_post(nrk_sem_t *rsrc);
int8_t nrk_sem_pend(nrk_sem_t *rsrc );
int8_t nrk_sem_query(nrk_sem_t *rsrc );


#endif
