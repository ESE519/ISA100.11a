/* This file handles concurrency control routines for the network stack */
/*
 Authors:
 Aditya Bhave
*/

#include <stdint.h>
#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_events.h>
#include <nrk_defs.h>


void enter_cr(nrk_sem_t *sem, int8_t fno)
{
	if( nrk_sem_pend(sem) == NRK_ERROR )
	{
		/*
		nrk_kprintf(PSTR("CR: enter_cr(): Address of sem = "));
		printf("%u\r\n", sem);
		for(i = 0; i < NRK_MAX_RESOURCE_CNT; i++)
			printf("SemPend: %u %u\r\n", i, &nrk_resource_value[i]); 
		*/
		if(nrk_errno_get() == 1)
			printf("CR: enter_cr(): Signal not found\r\n");
		else if(nrk_errno_get() == 2)
			printf("CR: enter_cr(): Signal index too large\r\n");
			
		printf("%d: ",fno);
		switch(fno)
		{
			
			/******************************** Network Layer ******************************/
			case 1:
				nrk_kprintf(PSTR("Error sem pending on add_neighbor()\r\n"));
				break;
				
			case 2: 
				nrk_kprintf(PSTR("Error sem pending on update_NgbList()\r\n"));
				break;
				
			case 3: 
				nrk_kprintf(PSTR("Error sem pending on route_addr()\r\n"));
				break;
			
			case 4: 
				nrk_kprintf(PSTR("Error sem pending on route_packet()\r\n"));
				break;
			
			case 5: 
				nrk_kprintf(PSTR("Error sem pending on process_app_pkt()\r\n"));
				break;
			
			case 6: 
				nrk_kprintf(PSTR("Error sem pending on process_app_pkt()\r\n"));
				break;
			
			case 7: 
				nrk_kprintf(PSTR("Error sem pending on process_nw_ctrl_pkt()\r\n"));
				break;
			
			case 8: 
				nrk_kprintf(PSTR("Error sem pending on build_Msg_Hello()\r\n"));
				break;
			
			case 9: 
				nrk_kprintf(PSTR("Error sem pending on build_Msg_NgbList()\r\n"));
				break;
			
			case 10: 
				nrk_kprintf(PSTR("Error sem pending on UNKNOWN()\r\n"));
				break;
			
			case 11: 
				nrk_kprintf(PSTR("Error sem pending on nl_tx_task()\r\n"));
				break;
			
			case 12: 
				nrk_kprintf(PSTR("Error sem pending on nl_tx_task()\r\n"));
				break;
			
			case 13: 
				nrk_kprintf(PSTR("Error sem pending on nl_tx_task()\r\n"));
				break;
			
			case 14: 
				nrk_kprintf(PSTR("Error sem pending on nl_tx_task()\r\n"));
				break;
			
			case 15: 
				nrk_kprintf(PSTR("Error sem pending on nl_tx_task()\r\n"));
				break;
			
			case 16: 
				nrk_kprintf(PSTR("Error sem pending on nl_tx_task()\r\n"));
				break;
				
			case 17: 
				nrk_kprintf(PSTR("Error sem pending on process_Msg_NwInfoAcquired()\r\n"));
				break;
						
			case 18: 
				nrk_kprintf(PSTR("Error sem pending on process_Msg_SendNwInfo()\r\n"));
				break;
				
			case 19: 
				nrk_kprintf(PSTR("Error sem pending on process_Msg_SendNodeInfo()\r\n"));
				break;
			
			case 20: 
				nrk_kprintf(PSTR("Error sem pending on send_nw_pkt_blocking()\r\n"));
				break;
			
			case 21: 
				nrk_kprintf(PSTR("Error sem pending on set_dg()\r\n"));
				break;
				
			case 22:
				nrk_kprintf(PSTR("Error sem pending on get_dg()\r\n"));
			
			case 23: 
				nrk_kprintf(PSTR("Error sem pending on set_continue_sending_ngblist()\r\n"));
				break;
				
			case 24: 
				nrk_kprintf(PSTR("Error sem pending on get_continue_sending_ngblist()\r\n"));
				break;
				
				
			/****************************** BufferManager.c *********************************/
			case 37: 
				nrk_kprintf(PSTR("Error sem pending on initialise_buffer_manager()\r\n"));
				break;
			
			case 38: 
				nrk_kprintf(PSTR("Error sem pending on is_excess_policy_valid()\r\n"));
				break;
			
			case 39: 
				nrk_kprintf(PSTR("Error sem pending on set_excess_policy()\r\n"));
				break;
			
			case 40: 
				nrk_kprintf(PSTR("Error sem pending on get_excess_policy()\r\n"));
				break;
			
			case 41: 
				nrk_kprintf(PSTR("Error sem pending on get_index_unallocated_rx_buf()\r\n"));
				break;
			
			case 42: 
				nrk_kprintf(PSTR("Error sem pending on insert_rx_pq()\r\n"));
				break;
			
			case 43: 
				nrk_kprintf(PSTR("Error sem pending on remove_rx_pq()\r\n"));
				break;
			
			case 44: 
				nrk_kprintf(PSTR("Error sem pending on insert_rx_fq()\r\n"));
				break;
			
			case 45: 
				nrk_kprintf(PSTR("Error sem pending on remove_rx_fq()\r\n"));
				break;
				
			case 46: 
				nrk_kprintf(PSTR("Error sem pending on insert_tx_aq()\r\n"));
				break;
			
			case 47: 
				nrk_kprintf(PSTR("Error sem pending on remove_tx_aq()\r\n"));
				break;
			
			case 48: 
				nrk_kprintf(PSTR("Error sem pending on insert_tx_fq()\r\n"));
				break;
			
			case 49: 
				nrk_kprintf(PSTR("Error sem pending on remove_tx_fq()\r\n"));
				break;
			
			case 50: 
				nrk_kprintf(PSTR("Error sem pending on get_in_process_buf_count()\r\n"));
				break;
			
			/******************************************************************************/
			
			default:
				nrk_kprintf(PSTR("enter_cr(): Unknown function number\r\n"));
				break;
		} // end switch 
	} // end if 
			
	return;
}
/************************************************************************************************/
void leave_cr(nrk_sem_t *sem, int8_t fno)
{
	if( nrk_sem_post(sem) == NRK_ERROR )
	{
		/*
		nrk_kprintf(PSTR("CR: leave_cr(): Address of sem = "));
		printf("%u\r\n", sem);
		for(i = 0; i < NRK_MAX_RESOURCE_CNT; i++)
			printf("SemPost: %u %u\r\n", i, &nrk_resource_value[i]); 
		*/		
		if(nrk_errno_get() == 1)
			printf("CR: leave_cr(): Signal not found\r\n");
		else if(nrk_errno_get() == 2)
			printf("CR: leave_cr(): Signal index too large\r\n");
			
		printf("%d: ", fno);
		switch(fno)
		{
			/******************************** Network Layer ******************************/
			case 1:
				nrk_kprintf(PSTR("Error sem posting on add_neighbor()\r\n"));
				break;
				
			case 2: 
				nrk_kprintf(PSTR("Error sem posting on update_NgbList()\r\n"));
				break;
				
			case 3: 
				nrk_kprintf(PSTR("Error sem posting on route_addr()\r\n"));
				break;
			
			case 4: 
				nrk_kprintf(PSTR("Error sem posting on route_packet()\r\n"));
				break;
			
			case 5: 
				nrk_kprintf(PSTR("Error sem posting on process_app_pkt()\r\n"));
				break;
			
			case 6: 
				nrk_kprintf(PSTR("Error sem posting on process_app_pkt()\r\n"));
				break;
			
			case 7: 
				nrk_kprintf(PSTR("Error sem posting on process_nw_ctrl_pkt()\r\n"));
				break;
			
			case 8: 
				nrk_kprintf(PSTR("Error sem posting on build_Msg_Hello()\r\n"));
				break;
			
			case 9: 
				nrk_kprintf(PSTR("Error sem posting on build_Msg_NgbList()\r\n"));
				break;
			
			case 10: 
				nrk_kprintf(PSTR("Error sem posting on UNKNOWN()\r\n"));
				break;
			
			case 11: 
				nrk_kprintf(PSTR("Error sem posting on nl_tx_task()\r\n"));
				break;
			
			case 12: 
				nrk_kprintf(PSTR("Error sem posting on nl_tx_task()\r\n"));
				break;
			
			case 13: 
				nrk_kprintf(PSTR("Error sem posting on nl_tx_task()\r\n"));
				break;
			
			case 14: 
				nrk_kprintf(PSTR("Error sem posting on nl_tx_task()\r\n"));
				break;
			
			case 15: 
				nrk_kprintf(PSTR("Error sem posting on nl_tx_task()\r\n"));
				break;
			
			case 16: 
				nrk_kprintf(PSTR("Error sem posting on nl_tx_task()\r\n"));
				break;
				
			case 17: 
				nrk_kprintf(PSTR("Error sem posting on process_Msg_NwInfoAcquired()\r\n"));
				break;
						
			case 18: 
				nrk_kprintf(PSTR("Error sem posting on process_Msg_SendNwInfo()\r\n"));
				break;
				
			case 19: 
				nrk_kprintf(PSTR("Error sem posting on process_Msg_SendNodeInfo()\r\n"));
				break;
			
			case 20: 
				nrk_kprintf(PSTR("Error sem posting on send_nw_pkt_blocking()\r\n"));
				break;
			
			case 21: 
				nrk_kprintf(PSTR("Error sem posting on set_dg()\r\n"));
				break;
				
			case 22:
				nrk_kprintf(PSTR("Error sem posting on get_dg()\r\n"));
			
			case 23: 
				nrk_kprintf(PSTR("Error sem posting on set_continue_sending_ngblist()\r\n"));
				break;
				
			case 24: 
				nrk_kprintf(PSTR("Error sem posting on get_continue_sending_ngblist()\r\n"));
				break;
			
			
			
			case 25: 
				nrk_kprintf(PSTR("Error sem posting on pkt_type()\r\n"));
				break;
				
			case 26: 
				nrk_kprintf(PSTR("Error sem posting on tl_type()\r\n"));
				break;
				
			case 27: 
				nrk_kprintf(PSTR("Error sem posting on nw_ctrl_type()\r\n"));
				break;
			
			case 28: 
				nrk_kprintf(PSTR("Error sem posting on process_app_pkt()\r\n"));
				break;
			
			case 29: 
				nrk_kprintf(PSTR("Error sem posting on process_nw_ctrl_pkt()\r\n"));
				break;
				
			case 30:
				nrk_kprintf(PSTR("Error sem posting on process_other_pkt()\r\n"));
				break;
				
			case 31:
				nrk_kprintf(PSTR("Error sem posting on build_Msg_Hello()\r\n"));
				break;
				
			case 32:
				nrk_kprintf(PSTR("Error sem posting on build_Msg_NgbList()\r\n"));
				break;
				
			case 33:
				nrk_kprintf(PSTR("Error sem posting on nl_rx_task()\r\n"));
				break;
				
			case 34:
				nrk_kprintf(PSTR("Error sem posting on nl_tx_task()\r\n"));
				break;
				
			case 35: 
				nrk_kprintf(PSTR("Error sem posting on create_network_layer_tasks()\r\n"));
				break;
			
			case 36: 
				nrk_kprintf(PSTR("Error sem posting on initialise_network_layer()\r\n"));
				break;
				
			/****************************** BufferManager.c *********************************/
			case 37: 
				nrk_kprintf(PSTR("Error sem posting on initialise_buffer_manager()\r\n"));
				break;
			
			case 38: 
				nrk_kprintf(PSTR("Error sem posting on is_excess_policy_valid()\r\n"));
				break;
			
			case 39: 
				nrk_kprintf(PSTR("Error sem posting on set_excess_policy()\r\n"));
				break;
			
			case 40: 
				nrk_kprintf(PSTR("Error sem posting on get_excess_policy()\r\n"));
				break;
			
			case 41: 
				nrk_kprintf(PSTR("Error sem posting on get_index_unallocated_rx_buf()\r\n"));
				break;
			
			case 42: 
				nrk_kprintf(PSTR("Error sem posting on insert_rx_pq()\r\n"));
				break;
			
			case 43: 
				nrk_kprintf(PSTR("Error sem posting on remove_rx_pq()\r\n"));
				break;
			
			case 44: 
				nrk_kprintf(PSTR("Error sem posting on insert_rx_fq()\r\n"));
				break;
			
			case 45: 
				nrk_kprintf(PSTR("Error sem posting on remove_rx_fq()\r\n"));
				break;
				
			case 46: 
				nrk_kprintf(PSTR("Error sem posting on insert_tx_aq()\r\n"));
				break;
			
			case 47: 
				nrk_kprintf(PSTR("Error sem posting on remove_tx_aq()\r\n"));
				break;
			
			case 48: 
				nrk_kprintf(PSTR("Error sem posting on insert_tx_fq()\r\n"));
				break;
			
			case 49: 
				nrk_kprintf(PSTR("Error sem posting on remove_tx_fq()\r\n"));
				break;
			
			case 50: 
				nrk_kprintf(PSTR("Error sem posting on get_in_process_buf_count()\r\n"));
				break;
			
			/******************************************************************************/
			
			default:
				nrk_kprintf(PSTR("leave_cr(): Unknown function number\r\n"));
				break;
		} // end switch 
	} // end if 
			
	return;
}
/*******************************************************************************************************/

