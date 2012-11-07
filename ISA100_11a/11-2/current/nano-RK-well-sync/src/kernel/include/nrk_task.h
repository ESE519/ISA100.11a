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
*  Anand Eswaren
*  Zane Starr
*******************************************************************************/


#ifndef _NRK_TASK_h	/* Only include stuff once */
#define _NRK_TASK_h


#include <nrk_cpu.h> /* New add on */
#include <nrk_cfg.h>
#include <nrk.h>
#include <nrk_time.h>
//#include <stdbool.h>

#define TIME_PAD  2

typedef	uint16_t		EventMaskType;
typedef EventMaskType*	EventMaskRefType;
/*
 **********************************************************
 *                      TASK CONTROL BLOCK                *
 **********************************************************
 */

typedef struct os_tcb {
	NRK_STK        *OSTaskStkPtr;        /* Pointer to current top of stack */
	NRK_STK        *OSTCBStkBottom;     /* Pointer to bottom of stack    */


	bool      elevated_prio_flag;
	bool      suspend_flag;
	bool      nw_flag;                // allows user to wake up on event or nw;
	uint8_t   event_suspend;          // event 0 = no event ; 1-255 event type;
	int8_t	  task_ID;                // For quick reference later, -1 means not active 	
	uint8_t   task_state;             // Task status    
	uint8_t   task_prio;              // Task priority (0 == highest, 63 == lowest) 
	uint8_t   task_prio_ceil;         // Task priority (0 == highest, 63 == lowest)    
	uint8_t   errno;                  // 0 no error 1-255 error code 
	uint32_t  registered_signal_mask; // List of events that are registered 
	uint32_t  active_signal_mask;     // List of events currently waiting on

	// Inside TCB, all timer values stored in tick multiples to save memory
	uint16_t  next_wakeup;
	uint16_t  next_period;
	uint16_t  cpu_remaining;	
	uint16_t  period;
	uint16_t  cpu_reserve;
	uint16_t  num_periods;	


} NRK_TCB;



/*
 **********************************************************
 *                   TASK MANAGEMENT                      *
 **********************************************************
 */

/*-------------- DataTypes - --------------*/

//typedef	uint8_t			nrk_task_type;	
typedef struct task_type {

	int8_t	task_ID;
	void *Ptos;
	void *Pbos;
	void (*task)();	
	bool	FirstActivation;
	uint8_t	prio;
	uint8_t	Type;
	uint8_t	SchType;
	nrk_time_t period;
	nrk_time_t cpu_reserve;
	nrk_time_t offset;

} nrk_task_type;



/*
 **********************************************************
 *                   ERROR CODES                          *
 **********************************************************
 */
typedef  int8_t	nrk_status_t;





/*------------- System Services -------------*/

nrk_status_t nrk_terminate_task();
nrk_status_t nrk_activate_task(nrk_task_type *);
nrk_status_t Task(void);
nrk_status_t Schedule(void);

void _nrk_scheduler(void);

uint8_t nrk_get_pid();
int8_t nrk_wait_until_next_period();
int8_t nrk_wait_until_next_n_periods(uint16_t p);
int8_t nrk_wait_until(nrk_time_t t);
int8_t nrk_wait(nrk_time_t t);
int8_t nrk_wait_until_ticks(uint16_t ticks);
int8_t nrk_wait_ticks(uint16_t ticks);
int8_t nrk_wait_until_nw();
int8_t nrk_set_next_wakeup(nrk_time_t t);

uint16_t _nrk_time_to_ticks(nrk_time_t t);
uint32_t _nrk_time_to_ticks_long(nrk_time_t t); 
nrk_time_t _nrk_ticks_to_time(uint32_t ticks);
/*---------- Constants ----------*/

#define	RUNNING   		0
#define	WAITING   		1
#define	READY     		2
#define	SUSPENDED 		3
#define FINISHED        	4
#define EVENT_SUSPENDED	        5

#define SIG_EVENT_SUSPENDED       1
#define RSRC_EVENT_SUSPENDED      2

#define NONPREEMPTIVE	0
#define PREEMPTIVE    	1

#define	INVALID_TASK 	0
#define	BASIC_TASK    	1
#define	IDLE_TASK      	2

#define TCB_EMPTY_PRIO	99
#define RES_FREE		99



/*
 **************************************************************
 *                   QUEUE  MANAGEMENT			     *
 **************************************************************
 */

typedef struct node {
	uint8_t	task_ID;
	struct node *Prev;
	struct node *Next;
} nrk_queue;

void nrk_rem_from_readyQ(int8_t task_ID);
uint8_t nrk_get_high_ready_task_ID(void);
void nrk_add_to_readyQ(int8_t task_ID);
void nrk_add_to_readyQ_Before(int8_t task_ID);
void nrk_print_readyQ(void);


//************************************************************





#endif

