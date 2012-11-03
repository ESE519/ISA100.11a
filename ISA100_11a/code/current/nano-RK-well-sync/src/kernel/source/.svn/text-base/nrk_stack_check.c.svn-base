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

#include <nrk.h>
#include <nrk_error.h>
#include <nrk_stack_check.h>
#include <stdio.h>

void dump_stack_info()
{
	unsigned int *stk;
	unsigned char *stkc;
	uint8_t i;

	nrk_kprintf( PSTR("\r\nSTACK DUMP\r\n"));

	printf( "cur: %d ",nrk_cur_task_TCB->task_ID);
	stk= (unsigned int *)nrk_cur_task_TCB->OSTCBStkBottom;
	stkc = (unsigned char*)stk;
	printf( "bottom = %x ",(uint16_t)stkc );
	printf( "canary = %x ",*stkc );
	stk= (unsigned int *)nrk_cur_task_TCB->OSTaskStkPtr;
	stkc = (unsigned char*)stk;
	printf( "stk = %x ",(uint16_t)stkc );
	printf( "tcb addr = %x\r\n",(uint16_t)nrk_cur_task_TCB);

	for(i=0; i<NRK_MAX_TASKS; i++ )
	{
		stk= (unsigned int *)nrk_task_TCB[i].OSTCBStkBottom;
		stkc = (unsigned char*)stk;
		printf( "%d: bottom = %x ",i,(uint16_t)stkc );
		printf( "canary = %x ",*stkc );
		stk= (unsigned int *)nrk_task_TCB[i].OSTaskStkPtr;
		stkc = (unsigned char*)stk;
		printf( "stk = %x ",(uint16_t)stkc );
		printf( "tcb addr = %x\r\n",(uint16_t)&nrk_task_TCB[i]);

	}

}


/*
 * Simple Canary value stack overflow check.
 * If the end of the stack was overwritten, then flag an error.
 *
 * */
//inline void nrk_stack_check()
void nrk_stack_check()
{
#ifdef NRK_STACK_CHECK

unsigned int *stk ;  // 2 bytes
unsigned char *stkc; // 1 byte
    
    stk  = (unsigned int *)nrk_cur_task_TCB->OSTCBStkBottom;          /* Load stack pointer */ 
    stkc = (unsigned char*)stk;
    if(*stkc != STK_CANARY_VAL) {
	    	#ifdef NRK_REPORT_ERRORS
	    	 dump_stack_info();
		#endif
	   	 nrk_error_add( NRK_STACK_OVERFLOW ); 
		 *stkc=STK_CANARY_VAL; 
    		  } 
 
    stk  = (unsigned int *)nrk_cur_task_TCB->OSTaskStkPtr;          /* Load stack pointer */ 
    stkc = (unsigned char*)stk;
    if(stkc > (unsigned char *)RAMEND ) {
	    	#ifdef NRK_REPORT_ERRORS
	    	 dump_stack_info();
		#endif
	   	 nrk_error_add( NRK_INVALID_STACK_POINTER); 
    		 } 




#endif
}

int8_t nrk_stack_check_pid(int8_t pid)
{
#ifdef NRK_STACK_CHECK

unsigned int *stk ;  // 2 bytes
unsigned char *stkc; // 1 byte
    
    stk  = (unsigned int *)nrk_task_TCB[pid].OSTCBStkBottom;          /* Load stack pointer */ 
    stkc = (unsigned char*)stk;
    if(*stkc != STK_CANARY_VAL) {
		 *stkc=STK_CANARY_VAL; 
		 return NRK_ERROR;
    		  }  
    stk  = (unsigned int *)nrk_task_TCB[pid].OSTaskStkPtr;          /* Load stack pointer */ 
    stkc = (unsigned char*)stk;
    if(stkc > (unsigned char *)RAMEND ) {
	   	 nrk_error_add( NRK_INVALID_STACK_POINTER); 
		 return NRK_ERROR;
    		}
#endif
return NRK_OK;
}

