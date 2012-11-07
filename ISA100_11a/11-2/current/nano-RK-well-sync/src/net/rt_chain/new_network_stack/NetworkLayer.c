includes AM;
includes RTChain;

#include "/opt/tinyos-1.x/contrib/chainradio/apps/Utils/MessageTypes.h"

#define MAX_RESEND 3
#define PRI 2

/* 
#define SOURCE
#ifdef SOURCE
#define MOTE_ID 100
#else
#define MOTE_ID 4
#endif
 */ 

#define DEST_ID 1
#define REQ_OPEN_TIMEOUT 500
#define OPEN_WAIT 200
#define IDLE_MUL 100
#define DEST_CONTENTION_CHANNEL 23

#ifdef SOURCE
	#define DROP_THRESHOLD 3
#else
	#define DROP_THRESHOLD 3
#endif


//#define NORMAL_CHANNEL 12
#define NORMAL_CHANNEL 12

#ifdef SOURCE
#define RT_CHANNEL 20
#endif

//#define STATE_DEBUG
//#define IDLE_MAX 2
#define IDLE_TIMER 2000
//#define RTCHAIN_DEBUG

#define QUE_SIZE 25

#define FULL_FUNCTION
/************************************
 * information for chain operation
 * note: each node belongs to 1 chain
 *************************************/
#ifndef SOURCE
#define NEXT_NODE_INDEX MOTE_ID-2 //this is for the 1st chain id from 1...10
//#define NEXT_NODE_INDEX MOTE_ID-12 //this is for the 1st chain id from 11...20

//#define NEXT_NODE_INDEX 0//CUSTOM Motes
//#define CUSTOM

#endif

module RTChainM {
   provides {
      interface StdControl;
#ifdef FULL_FUNCTION
      interface RTChainCtrl;
#endif
      interface ReceiveMsg as Receive;
      interface SendMsg as Send;
   }
   uses {
      interface StdControl as RadioControl;
      interface RadioSendMsg as RadioSend;
      interface ReceiveMsg as RadioReceive;
      interface MacControl;
      interface Leds;
      interface Timer as RTO_Timer;
      interface Timer as DATA_Timer;
      interface Timer as IDLE_Timer;

   }
}
implementation
{
   uint8_t nwk_state;//,cur_next_addr,prev_next_addr;
	uint8_t radio_active = FALSE;

   MsgQueElem MsgQue[QUE_SIZE];
   uint8_t hMsgQue,tMsgQue;

   TOS_Msg GMsg;
   uint8_t num_open=0, resend=0;
   uint16_t packet_drop=0;

   /********************************
    * information for chain operation
    *********************************/
   //a simple hard-coded routing table
#ifndef CUSTOM
   uint8_t routing_table_size = 9;
   //uint8_t routing_table[] = {1,2,3,4,5,6,7,8,9}; //if the node is 2 then forward to 1
   //the second routing table
   uint8_t routing_table[] = {1,12,13,14,15,16,17,18,19}; //if the node is 2 then forward to 1
#endif

#ifdef CUSTOM
   uint8_t routing_table_size = 1;
   uint8_t routing_table[] = {5};
#endif

   
   uint8_t idled_time = 0;
   bool freq_change=FALSE;
   uint8_t myRTChannel;
   uint8_t rt_to_base_forward_channel;
   uint8_t myNextRTNode = MOTE_ID-1;
   bool realtime_comm;
   uint16_t data_period;
   uint32_t idle_timeout;
   uint8_t seq = 0;

   //variable keeping track of seq number of packets sent from base
   uint8_t base_seq = 0;
   //this seq number is for base to send broadcast
   uint8_t sending_seq = 0;

#ifdef SOURCE
   void SendREQOpenChain();
   void SendOpenChain();
   void SendCloseChain();
   void StartSendRTData();
   //void SendRTData(uint8_t data);
#else
   void SendREPOpenChain(uint8_t addr);
   void FwdOpenChain();
   void FwdCloseChain();
   void FwdRTPkt(TOS_MsgPtr Msg, uint16_t address);
#endif

   bool isQueueFull();	
   uint16_t obtain_next_hop(uint16_t address);
   uint8_t get_upstream_neighbor();
   uint8_t get_downstream_neighbor();

#ifdef BEACON_DEBUG
   uint8_t debug_send_success = 0;
   uint8_t debug_send_task_posted= 0;
   uint8_t debug_last_sent_seq = 0;
   uint8_t debug_send_done_success = 0;
#endif

   task void RadioSendTask()
   {
#ifdef BEACON_DEBUG
      debug_send_task_posted++;
#endif

      if(hMsgQue != tMsgQue){
         TOS_Msg *pMsg=&(MsgQue[hMsgQue].msg);
#ifdef BEACON_DEBUG
         pMsg->data[PERIODIC_REFRESH_SEQ_INDEX+2] = hMsgQue; //--> Only for debug purpose
         pMsg->data[PERIODIC_REFRESH_SEQ_INDEX+3] = tMsgQue; //--> Only for debug purpose
         pMsg->data[PERIODIC_REFRESH_SEQ_INDEX+4] = debug_send_success; //--> Only for debug purpose
         pMsg->data[PERIODIC_REFRESH_SEQ_INDEX+5] = debug_send_task_posted; //--> Only for debug purpose
#endif
         if(call RadioSend.send(pMsg,pMsg->priority,MsgQue[hMsgQue].mac_type)){
            //hMsgQue=(hMsgQue+1)%QUE_SIZE;
            resend=0;
         }
#ifdef BEACON_DEBUG
         pMsg->data[PERIODIC_REFRESH_SEQ_INDEX+1] = 0xab; //--> Only for debug purpose
#endif
      }
   }
   bool isQueueFull(){
      return (tMsgQue+1)%QUE_SIZE==hMsgQue;
   }

#ifdef FULL_FUNCTION
   command result_t RTChainCtrl.OpenRTChain(uint16_t period, bool persistent){
#ifdef SOURCE
      if(nwk_state==NWK_NORMAL_OPERATION){
         if(persistent) realtime_comm=TRUE;
         myRTChannel=RT_CHANNEL;
         atomic nwk_state=NWK_REQUESTING_OPEN;
         data_period=period;
         SendREQOpenChain();
         call RTO_Timer.start(TIMER_ONE_SHOT,REQ_OPEN_TIMEOUT);
         return SUCCESS;
      }
      return FAIL;
#else
      return SUCCESS;
#endif

   }
   command result_t RTChainCtrl.CloseRTChain(){
      //chains are automatically closed when there is no data
      call MacControl.setChannel2(NORMAL_CHANNEL);
		nwk_state = NWK_NORMAL_OPERATION;
      realtime_comm=FALSE;
      return SUCCESS;                                                                                                                  
   }
   command result_t RTChainCtrl.sendRTMsg(TOS_MsgPtr pMsg){
      TOS_MsgPtr pQue=&(MsgQue[tMsgQue].msg);
      RTChainMsg *pRTMsg=(RTChainMsg*)(pQue->data);

      if(nwk_state==NWK_RTCHAIN_OPERATION) {
         if(isQueueFull()){ //nwk queue is full
#ifdef RTCHAIN_DEBUG
            call Leds.redOn();
#endif
            return FAIL;
         }
         memcpy(pQue, pMsg, sizeof(TOS_Msg));

         pQue->length = DATA_LENGTH;
         pQue->group = TOS_AM_GROUP;
         pQue->priority=PRI;
         pQue->addr=myNextRTNode;   //by now, Chain is successfully opened
                                    //and a source would know his next node
         if(myNextRTNode == DEST_ID){   
            call MacControl.setChannel2(DEST_CONTENTION_CHANNEL);
         }

         pQue->type=PKT_REALTIME;
         pRTMsg->cmd=RT_DATA;
         pRTMsg->src=MOTE_ID;
         pRTMsg->seq=++seq;
         MsgQue[tMsgQue].mac_type=mtBB;
         //MsgQue[tMsgQue].resend=0;
         tMsgQue=(tMsgQue+1)%QUE_SIZE;
         post RadioSendTask();
#ifdef RTCHAIN_DEBUG
         call Leds.greenToggle();
#endif
         return SUCCESS;
      }
      return FAIL;
   }
#else
#ifdef SOURCE
   task void MainTask(){
      //send REQ_OPEN
      if(nwk_state==NWK_NORMAL_OPERATION){
         realtime_comm=TRUE;
         myRTChannel=RT_CHANNEL;
         atomic nwk_state=NWK_REQUESTING_OPEN;
         SendREQOpenChain();
         call RTO_Timer.start(TIMER_ONE_SHOT,REQ_OPEN_TIMEOUT);
#ifdef RTCHAIN_DEBUG
         call Leds.greenOff();
         call Leds.yellowOn();
         call Leds.redOff();
#endif
      }
   }
#endif
#endif //FULL_FUNCTION                              

   void SendREQOpenChain(){     //broadcast
      TOS_Msg *pMsg=&(MsgQue[tMsgQue].msg);
      RTChainMsg *pRTMsg=(RTChainMsg*)(pMsg->data);
      OpenMsg *pOpen=(OpenMsg*)(pRTMsg->data);
      if(isQueueFull()){ //nwk queue is full
#ifdef RTCHAIN_DEBUG
         call Leds.redOn();
#endif
         return;
      }
      //create chain-opening packet
      //memset(&GMsg,0,sizeof(TOS_Msg));
      pMsg->length = RTMSG_SIZE;
      pMsg->group = TOS_AM_GROUP;
      pMsg->priority=PRI;
      pMsg->addr=TOS_BCAST_ADDR;
      pMsg->type=PKT_REALTIME;
      
      pRTMsg->cmd=REQ_OPEN;
      pRTMsg->src=MOTE_ID;
      pOpen->dest=DEST_ID;

      MsgQue[tMsgQue].mac_type=mtBB;
      //MsgQue[tMsgQue].resend=0;
      tMsgQue=(tMsgQue+1)%QUE_SIZE;
      post RadioSendTask();
      //set request_timer
   }

#ifdef SOURCE

   void SendOpenChain(){
      TOS_Msg *pMsg=&(MsgQue[tMsgQue].msg);
      RTChainMsg *pRTMsg=(RTChainMsg*)pMsg->data;
      OpenMsg *pOpen=(OpenMsg*)pRTMsg->data;

      if(isQueueFull()){
         //nwk queue is full  
          #ifdef RTCHAIN_DEBUG
         call Leds.redOn();
          #endif
         return;
      }

      pMsg->length = RTMSG_SIZE;
      pMsg->group = TOS_AM_GROUP;
      //the priority for open msg is special, see note 7
      pMsg->priority=1;
      pMsg->addr=myNextRTNode;
      pMsg->type=PKT_REALTIME;
      
      pRTMsg->cmd=OPEN_CHAIN;
      pRTMsg->src=MOTE_ID;
      pOpen->dest=DEST_ID;
      pOpen->num=num_open++;
      pOpen->freq_change=0;
      pOpen->channel=RT_CHANNEL;
      pOpen->data_period=data_period;

      MsgQue[tMsgQue].mac_type=mtBB;
      //MsgQue[tMsgQue].resend=0;
      tMsgQue=(tMsgQue+1)%QUE_SIZE;
      post RadioSendTask();
   }

   void SendCloseChain(){
      TOS_Msg *pMsg=&(MsgQue[tMsgQue].msg);
      RTChainMsg *pRTMsg=(RTChainMsg*)(pMsg->data);
      OpenMsg *pOpen=(OpenMsg*)(pRTMsg->data);
      if(isQueueFull()){ //nwk queue is full
#ifdef RTCHAIN_DEBUG
         call Leds.redOn();
#endif
         return;
      }

      pMsg->length = RTMSG_SIZE;
      pMsg->group = TOS_AM_GROUP;
      pMsg->priority=PRI;
      pMsg->addr=myNextRTNode;
      pMsg->type=PKT_REALTIME;
      
      pRTMsg->cmd=CLOSE_CHAIN;                                           
      pRTMsg->src=MOTE_ID;
      pOpen->dest=DEST_ID;

      MsgQue[tMsgQue].mac_type=mtBB;
      //MsgQue[tMsgQue].resend=0;
      tMsgQue=(tMsgQue+1)%QUE_SIZE;
      post RadioSendTask();
   }

#ifndef FULL_FUNCTION
   void StartSendRTData(){
      call DATA_Timer.start(TIMER_REPEAT,data_period);
   }
#endif

/*   void SendRTData(uint8_t data){
      TOS_Msg *pMsg=&(MsgQue[tMsgQue].msg);
      RTChainMsg *pRTMsg=(RTChainMsg*)(pMsg->data);
      OpenMsg *pOpen=(OpenMsg*)(pRTMsg->data);
      if(isQueueFull()){ //nwk queue is full
#ifdef RTCHAIN_DEBUG
         call Leds.redOn();
#endif
         return;
      }

      pMsg->length = DATA_LENGTH;
      pMsg->group = TOS_AM_GROUP;
      pMsg->priority=PRI;
      pMsg->addr=myNextRTNode;
      pMsg->type=PKT_REALTIME;
      
      pRTMsg->cmd=RT_DATA;
      pRTMsg->src=MOTE_ID;

      MsgQue[tMsgQue].mac_type=mtBB;
      //MsgQue[tMsgQue].resend=0;
      tMsgQue=(tMsgQue+1)%QUE_SIZE;
      post RadioSendTask();
   }*/
#else
   void SendREPOpenChain(uint8_t addr){
      TOS_Msg *pMsg=&(MsgQue[tMsgQue].msg);
      RTChainMsg *pRTMsg=(RTChainMsg*)(pMsg->data);
      OpenMsg *pOpen=(OpenMsg*)(pRTMsg->data);
      if(isQueueFull()){ //nwk queue is full
#ifdef RTCHAIN_DEBUG
         call Leds.redOn();
#endif
         return;
      }

      pMsg->length = RTMSG_SIZE;
      pMsg->group = TOS_AM_GROUP;
      pMsg->addr=addr;
      pMsg->type=PKT_REALTIME;
      
      pRTMsg->cmd=REP_OPEN;
      pRTMsg->src=MOTE_ID;

      MsgQue[tMsgQue].mac_type=mtCSMA;
      //MsgQue[tMsgQue].resend=0;
      tMsgQue=(tMsgQue+1)%QUE_SIZE;
      post RadioSendTask();
   }

   void FwdOpenChain(){
      TOS_Msg *pMsg=&(MsgQue[tMsgQue].msg);
      RTChainMsg *pRTMsg=(RTChainMsg*)(pMsg->data);
      OpenMsg *pOpen=(OpenMsg*)(pRTMsg->data);
      if(isQueueFull()){ //nwk queue is full
#ifdef RTCHAIN_DEBUG
         call Leds.redOn();
#endif
         return;
      }

      pMsg->length = RTMSG_SIZE;
      pMsg->group = TOS_AM_GROUP;
      //the priority for open msg is special, see note 7
      pMsg->priority=MOTE_ID%7+2;
      //pMsg->addr=routing_table[NEXT_NODE_INDEX];
      pMsg->addr=obtain_next_hop(DEST_ID);
      pMsg->type=PKT_REALTIME;
      
      pRTMsg->cmd=OPEN_CHAIN;
      pRTMsg->src=MOTE_ID; //this is not the source of the chain
      pOpen->dest=DEST_ID;
      pOpen->freq_change=(freq_change+1)%2;
      //TODO: will take into account reuse freq later
      pOpen->channel=freq_change?myRTChannel+1:myRTChannel;
      pOpen->data_period=data_period;

      MsgQue[tMsgQue].mac_type=mtBB;
      //MsgQue[tMsgQue].resend=0;
      tMsgQue=(tMsgQue+1)%QUE_SIZE;
      post RadioSendTask();
   }
   
	void FwdCloseChain(){
      TOS_Msg *pMsg=&(MsgQue[tMsgQue].msg);
      RTChainMsg *pRTMsg=(RTChainMsg*)(pMsg->data);
      OpenMsg *pOpen=(OpenMsg*)(pRTMsg->data);
      if(isQueueFull()){ //nwk queue is full
#ifdef RTCHAIN_DEBUG
         call Leds.redOn();
#endif
         return;
      }

      pMsg->length = RTMSG_SIZE;
      pMsg->group = TOS_AM_GROUP;
      //the priority for open msg is special, see note 7
      pMsg->priority=MOTE_ID%7+2;
      //pMsg->addr=routing_table[NEXT_NODE_INDEX];
      pMsg->addr=obtain_next_hop(DEST_ID);
      pMsg->type=PKT_REALTIME;
      
      pRTMsg->cmd=CLOSE_CHAIN;
      pRTMsg->src=MOTE_ID; //this is not the source of the chain
      pOpen->dest=DEST_ID;

      MsgQue[tMsgQue].mac_type=mtBB;
      //MsgQue[tMsgQue].resend=0;
      tMsgQue=(tMsgQue+1)%QUE_SIZE;
      post RadioSendTask();
   }
   
   void FwdRTPkt(TOS_MsgPtr Msg, uint16_t address){
      TOS_Msg *pMsg=&(MsgQue[tMsgQue].msg);
      RTChainMsg *pRTMsg=(RTChainMsg*)(pMsg->data);
      OpenMsg *pOpen=(OpenMsg*)(pRTMsg->data);
      if(isQueueFull()){ //nwk queue is full
#ifdef RTCHAIN_DEBUG
         call Leds.redOn();
#endif
         return;
      }

      pMsg->length = Msg->length;
      pMsg->group = TOS_AM_GROUP;
      //TODO: If forwarding to BASE, then contend on an agreed priority
      pMsg->priority=freq_change?Msg->priority-1:Msg->priority+1;
      pMsg->addr=address; 
      pMsg->type=PKT_REALTIME;
      
      memcpy(pMsg->data,Msg->data,Msg->length);
      pRTMsg->cmd=RT_DATA;      
      pRTMsg->src=MOTE_ID; //this is not the source of the chain

      if(address != DEST_ID){
         if(freq_change){ 
            //call MacControl.setChannel2(myRTChannel+1);
            call MacControl.setChannel2(rt_to_base_forward_channel);
         }
      }
      else{
            call MacControl.setChannel2(DEST_CONTENTION_CHANNEL);
      }

      MsgQue[tMsgQue].mac_type=mtBB;
      //MsgQue[tMsgQue].resend=0;
      tMsgQue=(tMsgQue+1)%QUE_SIZE;
      post RadioSendTask();
   }
#endif

/* commands for normal packets
*/
   void FwdNormalPkt(TOS_MsgPtr pMsg){

      TOS_Msg *pQue=&(MsgQue[tMsgQue].msg);
      NormalMsg *pNormal=(NormalMsg*)(pQue->data);
      if(isQueueFull()){ //nwk queue is full
         #ifdef RTCHAIN_DEBUG
         call Leds.redOn();
         #endif
         return;
      }
      //this length already includes NWK_NORMAL_HEADER_SIZE
      pQue->length = pMsg->length;
      pQue->group = TOS_AM_GROUP;
      pQue->priority=0;
      if(pMsg->data[NORMAL_DATA_TYPE_INDEX] == PERIODIC_REFRESH){ //
         if(pMsg->addr == TOS_BCAST_ADDR){
            pMsg->data[PERIODIC_REFRESH_RSSI_INDEX]=pMsg->strength;
            pMsg->data[PERIODIC_REFRESH_LOC_INDEX]=MOTE_ID;
         }
      }
      pQue->addr=obtain_next_hop( ((NormalMsg*)(pMsg->data))->dest  );//TOS_BCAST_ADDR;
      pQue->type=PKT_NORMAL;

      memcpy(pQue->data,pMsg->data,pMsg->length);
      pNormal->src=MOTE_ID; 

      MsgQue[tMsgQue].mac_type=mtCSMA;
      //MsgQue[tMsgQue].resend=0;
      tMsgQue=(tMsgQue+1)%QUE_SIZE;
      post RadioSendTask();
   }

   command result_t Send.send(uint16_t address, uint8_t length, TOS_MsgPtr pMsg){
      TOS_MsgPtr pQue=&(MsgQue[tMsgQue].msg);
      NormalMsg *pNormal=(NormalMsg*)(pQue->data);

      if(nwk_state==NWK_NORMAL_OPERATION) {
         if(isQueueFull()){ //nwk queue is full
            #ifdef RTCHAIN_DEBUG
            call Leds.redOn();
            #endif
            return FAIL;
         } //I assume that when queue is full, data will be overwritten
         memcpy(pQue, pMsg, sizeof(TOS_Msg));

         pQue->length = length + NWK_NORMAL_HEADER_SIZE;
         pQue->group = TOS_AM_GROUP;
         pQue->priority=0;
         //pQue->addr=TOS_BCAST_ADDR;
         
         pQue->addr=obtain_next_hop(address);//TOS_BCAST_ADDR;
         pQue->type=PKT_NORMAL;
         pNormal->src=MOTE_ID;
         pNormal->dest=address;
         pNormal->seq=++sending_seq;
			base_seq = pNormal->seq;
         MsgQue[tMsgQue].mac_type=mtCSMA;

         tMsgQue=(tMsgQue+1)%QUE_SIZE;
         post RadioSendTask();
         return SUCCESS;
      }
      return FAIL;
   }
   
   command result_t StdControl.init() {
      result_t ok1,ok2, ok3;

      ok2 = call RadioControl.init();
      ok3 = call Leds.init();

      dbg(DBG_BOOT, "TOSBase initialized\n");
      return rcombine3(ok1, ok2, ok3);
   }

   command result_t StdControl.start() {
      call RadioControl.start();

      nwk_state = NWK_NORMAL_OPERATION;
      call MacControl.setChannel2(NORMAL_CHANNEL);
//#ifdef STATE_DEBUG
#ifndef SOURCE
      call IDLE_Timer.start(TIMER_REPEAT,IDLE_TIMER);
#endif
//#endif

#ifndef FULL_FUNCTION
#ifdef SOURCE
      post MainTask();
#endif
#endif

#ifdef RTCHAIN_DEBUG
      call Leds.greenOn();
      call Leds.yellowOff();
      call Leds.redOff();
#endif
      seq=0;
      hMsgQue = 0;
      tMsgQue = 0;
      return SUCCESS;
   }

   command result_t StdControl.stop() {
      result_t ok1, ok2;
      call RTO_Timer.stop();
      call DATA_Timer.stop();
      ok2 = call RadioControl.stop();
      return rcombine(ok1, ok2);
   }

   event TOS_MsgPtr RadioReceive.receive(TOS_MsgPtr Msg)
	{
		RTChainMsg *pRTMsg=(RTChainMsg*)Msg->data;
		NormalMsg *pMsg=(NormalMsg*)Msg->data;
		OpenMsg *pOpen=(OpenMsg*)pRTMsg->data;
		idled_time=0;
		radio_active = TRUE;

		if(Msg->crc){

			if(Msg->addr!=MOTE_ID && Msg->addr!=TOS_BCAST_ADDR){
				return Msg;
			}

         if(Msg->type==PKT_NORMAL && nwk_state==NWK_NORMAL_OPERATION){           
            if(MOTE_ID==pMsg->dest){
               //if the packet is for this node
               if(pMsg->seq == base_seq){
                  if(Msg->addr == MOTE_ID){ //non BCAST
                     signal Receive.receive(Msg);
                  }
               }
               else if((Msg->addr == TOS_BCAST_ADDR)&&(pMsg->seq != base_seq)){ //important for BCAST
                  base_seq = pMsg->seq;
                  if((MOTE_ID==DEST_ID)&&(Msg->data[NORMAL_DATA_TYPE_INDEX] == PERIODIC_REFRESH)){
                     Msg->data[PERIODIC_REFRESH_RSSI_INDEX]=Msg->strength;
                     Msg->data[PERIODIC_REFRESH_LOC_INDEX]=MOTE_ID;
                  }
                  signal Receive.receive(Msg);
               }
            }
				else {
					//current version allows only base station to broadcast packet in normal channel
					//so need to keep track seq number of the base only
               if(pMsg->seq!=base_seq){
						//forward only if I am not a source
#ifndef SOURCE
                  call Leds.greenToggle();     
						base_seq=pMsg->seq;
						FwdNormalPkt(Msg);
#endif
					}             
               else{
                  if(Msg->addr != TOS_BCAST_ADDR){
                     FwdNormalPkt(Msg);
                  }
               }
				}
			}

			else if (Msg->type==PKT_REALTIME) {
				switch(pRTMsg->cmd){
#ifdef SOURCE
					//only source receives this
					case REP_OPEN:
						if(nwk_state==NWK_REQUESTING_OPEN){
							atomic nwk_state=NWK_OPENING_RTCHAIN;
							call RTO_Timer.stop();
							myNextRTNode=pRTMsg->src;
							call RTO_Timer.start(TIMER_ONE_SHOT,OPEN_WAIT); 
							//SendOpenChain();                  
						}
#ifdef RTCHAIN_DEBUG
						//call Leds.yellowToggle();
#endif
						break;
						//only non-source receive this
#else
					case REQ_OPEN:
						if(nwk_state==NWK_NORMAL_OPERATION){
							//atomic nwk_state=REQ_OPEN;
							SendREPOpenChain(pRTMsg->src);
						}
#ifdef RTCHAIN_DEBUG
						call Leds.yellowToggle();
#endif
						break;
					case OPEN_CHAIN:
						if(nwk_state==NWK_NORMAL_OPERATION){
                     //If your next node is BASE, then you switch channel
                     //regardless of anything
                     if(obtain_next_hop(DEST_ID)==DEST_ID){
                        freq_change = TRUE;
                        rt_to_base_forward_channel = DEST_CONTENTION_CHANNEL;
                     }
                     else{
                        rt_to_base_forward_channel = pOpen->channel;
   							freq_change=pOpen->freq_change;
                     }
                     //myRTChannel is then used for
                     //both cases (next node to BASE, and otherwise)
                     //to switch back to listenning mode
							myRTChannel=pOpen->channel;
							data_period=pOpen->data_period;
							//if(pOpen->channel) call Leds.redOn();
							idle_timeout=data_period*IDLE_MUL;

							if(MOTE_ID!=DEST_ID) {
								atomic nwk_state=NWK_OPENING_RTCHAIN;
								FwdOpenChain();
								//if(myRTChannel==15)  call Leds.yellowOn();
							}
							else {   
                        //if this is dest, start rt operation
								atomic nwk_state=NWK_RTCHAIN_OPERATION;         
								//call MacControl.setChannel2(myRTChannel);
								call MacControl.setChannel2(DEST_CONTENTION_CHANNEL);
								call RTO_Timer.start(TIMER_ONE_SHOT,idle_timeout);
								seq=0;

#ifdef RTCHAIN_DEBUG
								call Leds.greenOn();
								call Leds.yellowOn();
								call Leds.redOff();
#endif
							}
						}
						break;
					case CLOSE_CHAIN:
						if(nwk_state==NWK_RTCHAIN_OPERATION){
							atomic nwk_state=NWK_CLOSING_RTCHAIN;
							if(MOTE_ID!=DEST_ID){
								FwdCloseChain();
							}
						}
						break;
					case RT_DATA:
						if(nwk_state==NWK_RTCHAIN_OPERATION){
							call RTO_Timer.stop();
							if(seq!=pRTMsg->seq){
								seq=pRTMsg->seq;
								if(MOTE_ID==DEST_ID){
									//if this is destination then sending data up
									signal Receive.receive(Msg);
									call RTO_Timer.start(TIMER_ONE_SHOT,idle_timeout);
								}
								else{
									//FwdRTPkt(Msg, routing_table[NEXT_NODE_INDEX]);
									FwdRTPkt(Msg, obtain_next_hop(DEST_ID));
								}
							}
							else{
								call RTO_Timer.start(TIMER_ONE_SHOT,idle_timeout);
							}
#ifdef RTCHAIN_DEBUG
							call Leds.greenToggle();
#endif
						}
						break;
#endif
				}
			}
			return Msg;
		}
		else
			return Msg;
	}

   event result_t RadioSend.sendDone(TOS_MsgPtr Msg, result_t success) {
      RTChainMsg *pRTMsg=(RTChainMsg*)(Msg->data);
      //try to resend open_chain 3 times,
      //if success = 0 or no ack for 3 times, turn on red light

      if(success==SUCCESS) {
       if(Msg->type==PKT_REALTIME) {
         if (!Msg->ack) {
            if(resend++<MAX_RESEND){
               //check if resend is still necessary
               switch(nwk_state){
                  case NWK_NORMAL_OPERATION:
                     if(pRTMsg->cmd==REP_OPEN || pRTMsg->cmd==NORMAL_DATA ){
                        call RadioSend.send(Msg,Msg->priority,mtCSMA);
                        return SUCCESS;
                     }
                     break;
                  case NWK_OPENING_RTCHAIN:
                     if(pRTMsg->cmd==OPEN_CHAIN){
                        call RadioSend.send(Msg,Msg->priority,mtBB);
                        return SUCCESS;
                     }
                     break;
                  case NWK_RTCHAIN_OPERATION:
                     if(pRTMsg->cmd==RT_DATA){
                        call RadioSend.send(Msg,Msg->priority,mtBB);
                        return SUCCESS;
                     }
                     break;
                  case NWK_CLOSING_RTCHAIN:
                     if(pRTMsg->cmd==CLOSE_CHAIN){
                        call RadioSend.send(Msg,Msg->priority,mtBB);
                        return SUCCESS;
                     }
                     break;
               }
               //if((hMsgQue=(hMsgQue+1)%QUE_SIZE)!=tMsgQue) post RadioSendTask();
            }
            else {
               switch(nwk_state){
                  case NWK_OPENING_RTCHAIN:
                  #ifdef SOURCE
                     //if source node, should do req again, but can be dangerous, the same like note 5
                     //temporarily, do nothing
                     //nwk_state=REQ_OPEN;
                     //SendREQOpenChain();
                     //call RTO_Timer.start(TIMER_ONE_SHOT,REQ_OPEN_TIMEOUT);
                     atomic {
                        nwk_state=NWK_REQUESTING_OPEN;
                         //clear buffer
                        hMsgQue=tMsgQue=0;
                        //resend open request
                        //SendREQOpenChain();
                     }
                     call RTO_Timer.start(TIMER_ONE_SHOT,REQ_OPEN_TIMEOUT);
                     #ifdef RTCHAIN_DEBUG
                     call Leds.yellowOff();
                     call Leds.redOff();
                     call Leds.greenOn();
                     #endif
                     return SUCCESS;
                     #else
                     //for normal node, if it can not open chain then return to normal operation
                     atomic nwk_state=NWK_NORMAL_OPERATION;
                     #ifdef RTCHAIN_DEBUG
                     call Leds.yellowOff();
                     call Leds.redOff();
                     call Leds.greenOn();
                     #endif
                     #endif
                     break;
                  case NWK_RTCHAIN_OPERATION:
                     #ifdef SOURCE
                     #ifdef FULL_FUNCTION
                     signal RTChainCtrl.sendRTDone(&MsgQue[hMsgQue].msg, FALSE);
                     #endif
                     if(packet_drop++>DROP_THRESHOLD){
                        /*atomic nwk_state=NWK_CLOSING_RTCHAIN;
                          SendCloseChain();
                          return SUCCESS;
                        //SendREQOpenChain();
                        //call RTO_Timer.start(TIMER_ONE_SHOT,REQ_OPEN_TIMEOUT);*/
                        #ifdef FULL_FUNCTION
                        signal RTChainCtrl.RTChainStatus(CHAIN_CLOSED);
                        #endif
                        if (realtime_comm){
                           //reopen RT channel
                           atomic {
                              nwk_state=NWK_REQUESTING_OPEN;         
                              call MacControl.setChannel2(NORMAL_CHANNEL);
                              //clear buffer
                              hMsgQue=tMsgQue=0;
                              //resend open request
                              //SendREQOpenChain();
                           }
                           call RTO_Timer.start(TIMER_ONE_SHOT,REQ_OPEN_TIMEOUT);
                           #ifdef RTCHAIN_DEBUG
                           call Leds.yellowOff();
                           call Leds.redOff();
                           call Leds.greenOn();
                           #endif
                           return SUCCESS;
                        }
                        else {
                           atomic nwk_state=NWK_NORMAL_OPERATION;
                           call MacControl.setChannel2(NORMAL_CHANNEL);
                           //clear buffer
                           hMsgQue=tMsgQue=0;
#ifdef RTCHAIN_DEBUG
                           call Leds.yellowOff();
                           call Leds.redOff();
                           call Leds.greenOn();
#endif
                        }
                     }
                     #else //for non source node, after failing to forward a RT msg, return to original channel
                     //if(freq_change) call MacControl.setChannel2(myRTChannel);
                     //call RTO_Timer.start(TIMER_ONE_SHOT,idle_timeout);
                     if(packet_drop++>DROP_THRESHOLD){
                       atomic nwk_state=NWK_NORMAL_OPERATION;         
                       call MacControl.setChannel2(NORMAL_CHANNEL);
                       #ifdef RTCHAIN_DEBUG
                       call Leds.yellowOff();
                       call Leds.redOff();
                       call Leds.greenOn();
                       #endif         
                     }
                     else {
                        if(freq_change){
                           call MacControl.setChannel2(myRTChannel);
                        }
                       call RTO_Timer.start(TIMER_ONE_SHOT,idle_timeout);
                     }
                     #endif
                     break;
                  case NWK_CLOSING_RTCHAIN:
#ifdef SOURCE
                     if (0/*realtime_comm*/){
                        //reopen RT channel
                        atomic nwk_state=NWK_REQUESTING_OPEN;         
                        SendREQOpenChain();
                        call RTO_Timer.start(TIMER_ONE_SHOT,REQ_OPEN_TIMEOUT);
#ifdef RTCHAIN_DEBUG
                        call Leds.yellowOff();
                        call Leds.redOff();
                        call Leds.greenOn();
#endif
                        return SUCCESS;
                     }
                     else {
                        atomic nwk_state=NWK_NORMAL_OPERATION;
                        call MacControl.setChannel2(NORMAL_CHANNEL);
#ifdef RTCHAIN_DEBUG
                        call Leds.yellowOff();
                        call Leds.redOff();
                        call Leds.greenOn();
#endif
                     }
#else
                     atomic nwk_state=NWK_NORMAL_OPERATION;
                     call MacControl.setChannel2(NORMAL_CHANNEL);
#ifdef RTCHAIN_DEBUG
                     call Leds.yellowOff();
                     call Leds.redOff();
                     call Leds.greenOn();
#endif
#endif
                     break;
               }
            }
         }
         else {
            switch(nwk_state) {
               case NWK_OPENING_RTCHAIN:
                  if(pRTMsg->cmd==OPEN_CHAIN){ //if msg successfully sent was OPEN_CHAIN
                     atomic nwk_state=NWK_RTCHAIN_OPERATION;
                     seq=0;
                     //set channel
                     //if(freq_change) call MacControl.setChannel2(myRTChannel-1);
                     //else
                     //call MacControl.setChannel2(myRTChannel);
                     call MacControl.setChannel2(myRTChannel);
                     #ifdef RTCHAIN_DEBUG
                     call Leds.greenOn();
                     call Leds.yellowOn();
                     call Leds.redOff();
                     #endif

                     packet_drop=0;
                     #ifdef SOURCE
                     #ifdef FULL_FUNCTION
                     signal RTChainCtrl.openDone(TRUE,&myNextRTNode);
                     #else
                     StartSendRTData();
                     #endif
                     return SUCCESS;
                     #else
                     //for non source node, after forward a RT open msg set idle_timeout
                     call RTO_Timer.start(TIMER_ONE_SHOT,idle_timeout);
                     #endif
                  }
                  break;
               case NWK_RTCHAIN_OPERATION:
                    packet_drop=0;
                    #ifdef SOURCE
                    #ifdef FULL_FUNCTION
                    signal RTChainCtrl.sendRTDone(&MsgQue[tMsgQue].msg, TRUE);
                    #endif
                    #else
                    //for non source node, after forward a RT msg set idle_timeout
                    //switch back to listenning channel
                    if(freq_change){
                       call MacControl.setChannel2(myRTChannel);
                    }

                    call RTO_Timer.start(TIMER_ONE_SHOT,idle_timeout);
                    #endif
                    break;
               case NWK_CLOSING_RTCHAIN:
#ifdef SOURCE
                  if (0/*realtime_comm*/){
                     //reopen RT channel
                     atomic nwk_state=NWK_REQUESTING_OPEN;
                     SendREQOpenChain();
                     call RTO_Timer.start(TIMER_ONE_SHOT,REQ_OPEN_TIMEOUT);
#ifdef RTCHAIN_DEBUG
                     call Leds.yellowOff();
                     call Leds.redOff();
                     call Leds.greenOn();
#endif
                     return SUCCESS;
                  }
                  else {
                     atomic nwk_state=NWK_NORMAL_OPERATION;
                     call MacControl.setChannel2(NORMAL_CHANNEL);
#ifdef RTCHAIN_DEBUG
                     call Leds.yellowOff();
                     call Leds.redOff();
                     call Leds.greenOn();
#endif
                  }
#else
                  atomic nwk_state=NWK_NORMAL_OPERATION;         
                  call MacControl.setChannel2(NORMAL_CHANNEL);
#ifdef RTCHAIN_DEBUG
                  call Leds.yellowOff();
                  call Leds.redOff();
                  call Leds.greenOn();
#endif
#endif
                  break;
            }
          }
        } //end if pkt_type=RT_PKT
        else {
          if(Msg->ack || Msg->addr==TOS_BCAST_ADDR) {
             signal Send.sendDone(Msg, SUCCESS);            
          }
          else {
            //resend
          }
        }
      }
      else{         
         #ifdef RTCHAIN_DEBUG
         call Leds.redOn();
         #endif
      }

#ifdef BEACON_DEBUG
      debug_send_success++;
      //trying to determine if i get Signal twice at some point
      if(debug_last_sent_seq == Msg->data[PERIODIC_REFRESH_SEQ_INDEX+5]){
          call Leds.redOn();
          if(debug_send_done_success != success){
               call Leds.yellowOn();
          }
      }
      debug_send_done_success = success;
      debug_last_sent_seq = Msg->data[PERIODIC_REFRESH_SEQ_INDEX+5]; 

#endif

      if(hMsgQue == tMsgQue){
         return SUCCESS;
      }
      else if((hMsgQue=(hMsgQue+1)%QUE_SIZE)!=tMsgQue){
         post RadioSendTask();
      }
      return SUCCESS;

   }

   event result_t RTO_Timer.fired(){
		call Leds.redToggle();
      switch(nwk_state){
#ifdef SOURCE
         case NWK_REQUESTING_OPEN:
            //should send a new req now, see note 5
            //temporarily, send nothing

            SendREQOpenChain();
            call RTO_Timer.start(TIMER_ONE_SHOT,REQ_OPEN_TIMEOUT);

#ifdef RTCHAIN_DEBUG
            call Leds.redToggle();
#endif
            break;
         case NWK_OPENING_RTCHAIN:
            SendOpenChain();
            break;
#else
         case NWK_RTCHAIN_OPERATION:
            //idle timeout
            //silently close the channel
            //switch back to normal operation
            atomic nwk_state=NWK_NORMAL_OPERATION;
				//call RadioControl.start();
            call MacControl.setChannel2(NORMAL_CHANNEL);
#ifdef RTCHAIN_DEBUG
            call Leds.yellowOff();
            call Leds.redOff();
            call Leds.greenOn();
#endif
#endif
      }

   }

   event result_t DATA_Timer.fired(){
/*#ifndef FULL_FUNCTION
#ifdef SOURCE
      if(nwk_state==NWK_RTCHAIN_OPERATION) {
         SendRTData(0);
         call Leds.yellowToggle();
      }
      else call DATA_Timer.stop();
#else
#endif
#endif*/
   }

   event result_t IDLE_Timer.fired(){

		if(radio_active == FALSE){
			atomic nwk_state=NWK_NORMAL_OPERATION;
			call MacControl.flushRx();
			call MacControl.setChannel2(NORMAL_CHANNEL);
		}
		else{
			radio_active = FALSE;
		}
#ifdef STATE_DEBUG
      if(idled_time++ > IDLE_MAX){
         uint16_t t= 0;
         for(t=0; t< 40000 ; t++){
            call Leds.greenToggle();
         }
         idled_time=0;
      }
#endif
   }


   /*
      If next_hop (mote_id -1) not found, we broadcast
      If address is upstream(address is smaller), we send it to smaller id
      else we send it to larger id
      If address not found, we broadcast
   */
   uint16_t obtain_next_hop(uint16_t address){
      int8_t index = 0;
      
#ifdef SOURCE
      return TOS_BCAST_ADDR;
#endif

      if(address < MOTE_ID){
         for(; (index<routing_table_size && routing_table[index]<=address); index++){
            if(routing_table[index]==address){
               //return MOTE_ID-1;
               return get_upstream_neighbor();
            }
         }

      }
      else{ //if downstream
         for(index = routing_table_size-1; (index>=0 && routing_table[index]>=address);index++){
            if(routing_table[index]==address){
               //return MOTE_ID+1;
               return get_downstream_neighbor();
            }
         }
      }

      return TOS_BCAST_ADDR;
   }

   uint8_t get_upstream_neighbor(){
      int8_t index = 0;
      for(; index<routing_table_size; index++){
         if(routing_table[index]==MOTE_ID){
            return routing_table[index-1];
         }
      }
   }
   
   uint8_t get_downstream_neighbor(){
      int8_t index = 0;
      for(; index<routing_table_size; index++){
         if(routing_table[index]==MOTE_ID){
            return routing_table[index+1]; //There should always exist such a node
         }
      }
   }

}

