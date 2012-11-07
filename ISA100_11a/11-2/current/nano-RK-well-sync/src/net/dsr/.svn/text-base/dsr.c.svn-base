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
*******************************************************************************/

#include<dsr.h>
DSR_PKT_INFO rxtx_pkt;
DSR_PKT_INFO util_pkt;
nrk_time_t dsr_timeout_timer; 
uint8_t dsr_src_route_table[MAX_MAC_ADDR];
uint8_t dsr_src_route_path[MAX_ROUTE][MAX_PATH_LEN];
uint8_t dsr_src_long_hop[MAX_ROUTE];//if there exist a path longer than path len concat across multiple paths
uint8_t dsr_route_freq[MAX_ROUTE];
uint8_t dsr_util_dest;
uint8_t dsr_util_route[MAX_ROUTE];

uint8_t _NW_LAYER_REL_TIME;// counts times in nw layer tasks sent data
uint8_t _DSR_OP_MODE; //what mode route request, recieve etc...
uint8_t _nw_route_cnt;// total number of routes that exist
uint8_t _curr_hop_cnt;//how many hops for rout request 

uint8_t dsr_add_path(uint8_t *path)
{
	uint8_t i;
	uint8_t used;
	uint8_t least;
	uint8_t oldmac;
	least=0;
	if(_nw_route_cnt!=MAX_ROUTE)//if you dont' have to start evictions
	{
		for(i=0;i<_nw_route_cnt;i++)
		{
			dsr_src_route_path[_nw_route_cnt][i]=path[i];
			dsr_src_route_table[path[i]]=_nw_route_cnt; //insert new enteries to dest table
		}
		_nw_route_cnt++;
		return (_nw_route_cnt-1);
	}
	else   //LRU policy
	{
		for(i=0;i<MAX_ROUTE;i++)
			if(dsr_route_freq[i] <=least)
				used=i;
		for(i=0;i<MAX_ROUTE;i++)
		{
			oldmac=dsr_src_route_path[used][i];
			dsr_src_route_table[oldmac]=0;      //remove old entries
			dsr_src_route_table[path[i]]=used; //insert new enteries to dest table
			dsr_src_route_path[used][i]=path[i];//insert new enteries to path table
		}
		return used;
	}

}

uint8_t dsr_bridge_table(uint8_t before,uint8_t after)
{


	dsr_src_long_hop[after]=before;//set the hop that comes before 


	return 0;
}
uint8_t * dsr_get_path(uint8_t mac)
{
	uint8_t path_addr;
	path_addr=dsr_src_route_table[mac];
	if(path_addr!=0)
	{
		return dsr_src_route_path[path_addr];
	}
	return NULL;
}
uint8_t dsr_path_exist(uint8_t mac)
{
	uint8_t path_addr;
	path_addr=dsr_src_route_table[mac];
	return  path_addr;
}
uint8_t dsr_request_path()
{
	util_pkt.type=ROUTE_REQUEST;
	util_pkt.len=_curr_hop_cnt;//how many hops to propogate the request upon time out goes up etc
	util_pkt.src_cnt=0;
	util_pkt.src_route=&dsr_util_dest;
	util_pkt.payload=dsr_util_route;
	//TODO discuss + 1 req of bmac_tx_packet
	bmac_tx_packet((uint8_t *)&util_pkt,sizeof(DSR_PKT_INFO)+MAX_ROUTE+1);//total mount of stuff TODO
	return 0;//TODO MAKE ERROR CODE RIGHT
}
//used for data pkt transmission
void _dsr_tx_packet(uint8_t dest,uint8_t* payload,uint8_t len)
{
	//add check for complex more than 16hop single path  case
	uint8_t path_addr; 
	path_addr=dsr_src_route_table[dest];
	rxtx_pkt.src_cnt=0;
	rxtx_pkt.len=len;
	rxtx_pkt.src_route=dsr_src_route_path[path_addr];
	rxtx_pkt.payload=payload;
	rxtx_pkt.type=DATA_PACKET;
	for(int i=0;i<MAX_ROUTE;i++)
		if(dsr_src_route_path[path_addr][i]==dest) //overly cautious here maybe add check TODO
			rxtx_pkt.dest_cnt=i;//so final path  
	bmac_tx_packet((uint8_t *)&rxtx_pkt,len+sizeof(DSR_PKT_INFO));
}
void dsr_rate_adjust(uint8_t increase)//increase rate decrease timeout rate
{
	if(!increase)
	{
		dsr_timeout_timer.nano_secs=NANOS_PER_MS * 500;
		dsr_timeout_timer.secs=0;  
		_curr_hop_cnt=1;
		nrk_set_task_timeout(dsr_timeout_timer);          
	}
	else
	{
		if(_curr_hop_cnt%2==0)
			dsr_timeout_timer.nano_secs+=(2*dsr_timeout_timer.nano_secs)*( _curr_hop_cnt-1 );
		else
			dsr_timeout_timer.nano_secs+=(2*dsr_timeout_timer.nano_secs);
		nrk_set_task_timeout(dsr_timeout_timer);          
	}

}

void dsr_tx_packet(uint8_t dest,uint8_t*payload,uint8_t len)
{
	RF_RX_INFO *pkt;
	DSR_PKT_INFO *dsr_pkt;
	uint8_t src_route[MAX_PATH_LEN];
	int8_t i;
	uint8_t j;
	uint8_t rssi;
	uint32_t mask;
	if(dsr_path_exist(dest))
	{
		_dsr_tx_packet(dest,payload,len);
		nrk_event_signal(SIG(EVENT_DSR_PKT_TX));
		return;
	}
	else
	{
		dsr_util_dest=dest;
			printf("path doesn't exist\r\n");
		while(!dsr_path_exist(dest)||_curr_hop_cnt!=DSR_MAX_HOP_COUNT)
		{
			dsr_request_path(dest);        //TODO CHECK AND SAVE OLD TIMEOUT STATE THEN AFTER REQUEST ETC DONE RESTORE OLD STATE
			mask=nrk_event_wait(SIG(BMAC_RX_PKT_EVENT)|SIG(TASK_TIMEOUT_EVENT));
			if(mask==TASK_TIMEOUT_EVENT)
			{
				if(_curr_hop_cnt==DSR_MAX_HOP_COUNT)
				{
					dsr_rate_adjust(0); //reset timeout rate on retransmission
					nrk_event_signal(SIG(DSR_PACKET_TIMEOUT));
					return;
				}
				dsr_rate_adjust(1); //increase rate exponentially 
			}                                                                                         
			else   //get bmac packet
			{                                                                                         
				pkt=(RF_RX_INFO*)bmac_rx_pkt_get(&len,&rssi);
				printf("the wrong length and rssi(%i,%i)\r\n",len,rssi);
				if(pkt!=NULL)
				{
					dsr_pkt=(DSR_PKT_INFO*)pkt->pPayload;
					if(dsr_pkt->type==ROUTE_RETURN)
					{
						for(i=dsr_pkt->src_cnt-1;i>=0;i--)
						{
							src_route[j]=dsr_pkt->src_route[i];
							j++;
						}
						dsr_add_path(src_route);
						bmac_rx_pkt_release();//done with the packet
						_dsr_tx_packet(dest,payload,len);
						nrk_event_signal(SIG(EVENT_DSR_PKT_TX));
						dsr_rate_adjust(0); //increase rate exponentially 
						return;
					}
				}
			}
		}
		return;
	}
}

DSR_PKT_INFO *  dsr_rx_pkt()
{
        uint8_t len;
	uint8_t rssi;
	RF_RX_INFO *pkt;
	nrk_event_wait(SIG(EVENT_DSR_PKT_RX));
	pkt=(RF_RX_INFO*)bmac_rx_pkt_get(&len,&rssi);
	bmac_rx_pkt_release();
	return (DSR_PKT_INFO*)pkt->pPayload;

}
void dsr_update_clist()      //periodically clean cache frequency
{

	uint8_t i;
	for(i=0;i<MAX_ROUTE;i++)
		dsr_route_freq[i]=0;


}              
void dsr_network_task()
{
	uint32_t mask;
	uint8_t len;
	uint8_t rssi;  
	int8_t i; 
	uint8_t j; 
	RF_RX_INFO *pkt;
	DSR_PKT_INFO *dsr_pkt;
	uint8_t path[MAX_ROUTE];
	printf("network task started\r\n");
	_curr_hop_cnt++;
	mask=nrk_event_wait(SIG(BMAC_RX_PKT_EVENT));
	printf("in nw task\r\n");
	pkt=(RF_RX_INFO*)bmac_rx_pkt_get(&len,&rssi); 
	
	dsr_pkt=(DSR_PKT_INFO*)pkt->pPayload;
	if(dsr_pkt->type==DATA_PACKET)
		if(dsr_pkt->src_route[dsr_pkt->src_cnt]==NODE_ADDR) //Packet is for me
		{
			if(dsr_pkt->dest_cnt==dsr_pkt->src_cnt)   //@ final destination
				nrk_event_signal(SIG(EVENT_DSR_PKT_RX));
			else //forward packet along update its source pos
			{
				dsr_pkt->src_cnt++;
				bmac_tx_packet(pkt->pPayload,pkt->length);// TODO  check node leaving case no longer exists case
			}
		}
	if(dsr_pkt->type==ROUTE_REQUEST)
	{
		if(dsr_pkt->dest_cnt>0)//TODO note that hop_count is dest_cnt
		{
			if(dsr_path_exist(dsr_pkt->src_route[0]))  //destination exists in your paths?
			{
				j=0;
				for(i=dsr_pkt->src_cnt;i>-1;i--) //BOUND THIS BY MAX_ROUTE
				{

					path[j]=dsr_pkt->payload[i];   //make new path
				}  
				for(i=j+1;i<MAX_ROUTE;i++)
				{
					path[i]= dsr_src_route_path[dsr_src_route_table[dsr_pkt->src_route[0]]][i];
					if(path[i]==dsr_pkt->src_route[0])
						break;
				}
				dsr_pkt->src_route=path;
				dsr_pkt->type=ROUTE_RETURN;
				dsr_pkt->src_cnt=0;
				bmac_tx_packet(pkt->pPayload,pkt->length);// TODO  check node leaving case no longer exists case
			}
			else
			{
				dsr_pkt->payload[dsr_pkt->src_cnt]=NODE_ADDR;
				dsr_pkt->src_cnt++;
				dsr_pkt->dest_cnt--;
				dsr_pkt->type=ROUTE_RETURN;
				bmac_tx_packet(pkt->pPayload,pkt->length+MAX_ROUTE+16);// TODO  check node leaving case no longer exists case
			}
		}
		if(dsr_pkt->type==ROUTE_RETURN)
		{
			if(dsr_pkt->src_route[dsr_pkt->src_cnt]!=NODE_ADDR) //Packet is for me
			{
				dsr_pkt->src_cnt++;
				bmac_tx_packet(pkt->pPayload,pkt->length+MAX_ROUTE+16);// TODO  check node leaving case no longer exists case
			}
		}
	}

}              
void dsr_task_config()
{
	dsr_task.task = dsr_network_task;
	dsr_task.Ptos = (void *) &dsr_task_stack[DSR_STACK_SIZE-1];
	dsr_task.Pbos = (void *) &dsr_task_stack[0];
	dsr_task.prio = 19;
	dsr_task.FirstActivation = TRUE;
	dsr_task.Type = BASIC_TASK;
	dsr_task.SchType = PREEMPTIVE;
	dsr_task.period.secs = 0;
	dsr_task.period.nano_secs = 175 * NANOS_PER_MS;
	dsr_task.cpu_reserve.secs = 15;      // Way larger than period
	dsr_task.cpu_reserve.nano_secs = 0;
	dsr_task.offset.secs = 0;
	dsr_task.offset.nano_secs = 0;
	printf( "dsr activate task\r\n" );
	nrk_activate_task (&dsr_task); 
}
