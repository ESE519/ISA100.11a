#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <dlmo.h>
#include <isa.h>
#include <nrk_error.h>
#include <slip.h>

//#include <sys/time.h>


#define MY_CHANNEL 19 
#define MY_ID 0 //change

#define MY_TX_SLOT  2
#define NUM_OF_TEST_SET 16
#define MAX_SLIP_BUF 17
#define NUM_OF_NODES 3
//#define JOIN_TX_SLOT_START 22
//#define MY_RX_SLOT  15
#define MY_RX_SLOT  3//  change for test

#define CLOCK_CORRECTION_REQUIRED;


NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void Task2 (void);

void nrk_create_taskset();
void packet_measurement_better(uint8_t * local_rx_buf);

/*Buffers*/
uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t slip_tx_buf[MAX_SLIP_BUF];
uint8_t slip_rx_buf[MAX_SLIP_BUF];

/*packet evaluation related*/
uint8_t pkt_measure[NUM_OF_NODES][NUM_OF_TEST_SET];
uint8_t sendFlag;
uint8_t frame_cnt[NUM_OF_NODES];  //add 1 every 8 packets
uint8_t pkt_cnt[NUM_OF_NODES];
uint8_t current_pkt_index[NUM_OF_NODES]; 
uint8_t received_pkt_index[NUM_OF_NODES];
uint8_t current_node;
uint8_t send_node;

/* signal related declaration */
int8_t pkt_record_done_signal;



int8_t pkt_record_check()
{
  return sendFlag;
}

int8_t wait_until_record_full()
{
    nrk_signal_register(pkt_record_done_signal);
    if (pkt_record_check() != 0)
        return NRK_OK;
    nrk_event_wait (SIG(pkt_record_done_signal));
    return NRK_OK;
}

int main ()
{
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);

  nrk_kprintf( PSTR("Starting up...\r\n") );
	
  nrk_init();

  nrk_led_clr(0);
  nrk_led_clr(1);
  nrk_led_clr(2);
  nrk_led_clr(3);
  
  nrk_time_set(0,0);
  
  isa_task_config();
  
  nrk_create_taskset ();

  nrk_start();
  
  return 0;
}

//*********************Making a callback function***************************************

void transmitCallback1(ISA_QUEUE *entry){
uint8_t length;
	 isaFreePacket(entry);
	  sprintf( &tx_buf[PKT_DATA_START],"node" );
	  length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;

	  	sendPacket(1, length, tx_buf, transmitCallback1);
}

void transmitCallback2(ISA_QUEUE *entry){
uint8_t length;
	 isaFreePacket(entry);
	  sprintf( &tx_buf[PKT_DATA_START],"node" );
	  length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;

	  	sendPacket(2, length, tx_buf, transmitCallback2);
}
//*****************************************************************************************

void Task1()
{

  uint8_t j, i;
  uint8_t length,slot,len;
  uint8_t *local_rx_buf;
  //uint32_t Score = 0;
  int8_t rssi;
  uint8_t cnt=0;
  //char c = -1;
  nrk_sig_t uart_rx_signal;
  uint8_t finished = 0;

  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  printf("Gateway");

  nrk_led_set(RED_LED);
  nrk_led_set(BLUE_LED);
  
  isa_set_channel_pattern(1); // must before isa_init
  isa_init (ISA_GATEWAY, MY_ID, MY_ID);//change
  //isa_set_schedule(ISA_GATEWAY, MY_ID);
  isa_set_channel(MY_CHANNEL);
  dlmoInit(); 	//Initialize the Data Link Management Object

  configureSlot(1, 1, TX_NO_ADV, false);
  configureSlot(2, 2, TX_NO_ADV, true);
  //configAdvDAUX(1, 0, 25, 1, NULL, NULL, NULL, 2, NULL, NULL, NULL);

  isa_start();

  isa_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE);

  //slip_init (stdin, stdout, 0, 0);

  //while (slip_started () != 1) nrk_wait_until_next_period ();

  
  while(!isa_ready())  nrk_wait_until_next_period(); 
  printf("isa start!\n\r");

   
    pkt_record_done_signal=nrk_signal_create();
    if(pkt_record_done_signal==NRK_ERROR){
	nrk_kprintf(PSTR("ERROR: creating packet record signal failed\r\n"));
	nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
	return NRK_ERROR;
    }  


  while(1){

	  //Spit out log info
	/*  	  if (txCount % 1000 == 0){
	  	printf ("TxCount: %d\r\nRXCount: %d\r\nPacketLoss:%d", txCount,rxCount, packetsLost);
	  	  }
      */
       nrk_gpio_set(NRK_DEBUG_3);
       
       if( isa_rx_pkt_check()!=0 ) {
	   // printf("message is received.\n\r");
	    local_rx_buf=isa_rx_pkt_get(&length, &rssi);
	    //printf("RXLEN:%d\r\n",length);
	    //for(i=PKT_DATA_START; i<length-1; i++ )
	    	//printf( "node %c,%d\r\n",local_rx_buf[PKT_DATA_START+5],local_rx_buf[PKT_DATA_START+7]);
	    //packet_measurement(local_rx_buf,length);
 	    packet_measurement_better(local_rx_buf);

	    //printf( "%c",local_rx_buf[PKT_DATA_START]);

	    isa_rx_pkt_release();
	  //  printf("\r\n");
	}
	/*
	if(isa_tx_pkt_check(MY_TX_SLOT)!=0){
	  // printf("Pending TX\r\n");
	}
	*/
	//else{
if (cnt ==0 )
{
	sprintf( &tx_buf[PKT_DATA_START],"node %d,%c",MY_ID,cnt++);
  	length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
  	sendPacket(1, length, tx_buf, transmitCallback1);
  	sendPacket(2, length, tx_buf, transmitCallback2);
}

  	//isa_tx_pkt(tx_buf,length,configDHDR(0),MY_TX_SLOT);
	//printf("Len:%d\r\n",length);
  	//printf("Hello world is sent.\n\r");
  	//}


	isa_wait_until_rx_or_tx ();


  }
  

}


void Task2 ()
{

  uint8_t len,i;
  uint8_t zero_killer=0xaa;

  slip_init (stdin, stdout, 0, 0);

  wait_until_record_full(); //wait for first batch of packets

  while (1) {
    //nrk_led_set (ORANGE_LED);
    //sprintf (slip_tx_buf, pkt_measure);
	//if(sendFlag){
		//printf("")
		nrk_gpio_set(NRK_DEBUG_1);
		//printf("CN:%d\r\n",send_node);
		slip_tx_buf[0]=send_node+1;  // get rid of '\0'
		for(uint8_t i=0;i<NUM_OF_TEST_SET;i++){
		   slip_tx_buf[i+1]=pkt_measure[send_node][i] ^ zero_killer;  //get rid of '\0'
		}
		//slip_tx_buf[i+1]=0; // add '\0' at the end
    	len = strlen (slip_tx_buf);
	//printf("%d\r\n",len);
    	slip_tx (slip_tx_buf, len);
		sendFlag=0;
		for(i=0;i<NUM_OF_TEST_SET;i++){
		    pkt_measure[send_node][i]=0;	    
		}
	printf("KO,%d,%d\r\n",send_node,resync_times);
    	//nrk_wait_until_next_period ();
		nrk_gpio_clr(NRK_DEBUG_1);
	//}	
	wait_until_record_full();
  }
}



void
nrk_create_taskset()
{
  TaskOne.task = Task1;
  TaskOne.Ptos = (void *) &Stack1[NRK_APP_STACKSIZE-1];
  TaskOne.Pbos = (void *) &Stack1[0];
  TaskOne.prio = 2;
  TaskOne.FirstActivation = TRUE;
  TaskOne.Type = BASIC_TASK;
  TaskOne.SchType = PREEMPTIVE;
  TaskOne.period.secs = 10;
  TaskOne.period.nano_secs = 0*NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs = 500*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 60*NANOS_PER_MS;
  nrk_activate_task (&TaskOne);
	
  TaskTwo.task = Task2;
  nrk_task_set_stk( &TaskTwo, Stack2, NRK_APP_STACKSIZE);
  TaskTwo.prio = 3;
  TaskTwo.FirstActivation = TRUE;
  TaskTwo.Type = BASIC_TASK;
  TaskTwo.SchType = PREEMPTIVE;
  TaskTwo.period.secs = 20;
  TaskTwo.period.nano_secs = 0;
  TaskTwo.cpu_reserve.secs = 0;
  TaskTwo.cpu_reserve.nano_secs = 0;
  TaskTwo.offset.secs = 0;
  TaskTwo.offset.nano_secs = 100*NANOS_PER_MS;
  nrk_activate_task (&TaskTwo);

  nrk_kprintf( PSTR("Create Done\r\n") );
}

void packet_measurement_better(uint8_t * local_rx_buf)
{
	uint8_t i,length;
	uint8_t next_pkt_offset;
	uint8_t temp;

	if(local_rx_buf[PKT_DATA_START]=='n'){
		current_node = local_rx_buf[PKT_DATA_START+5]-'0';  // node number
		received_pkt_index[current_node] = local_rx_buf[PKT_DATA_START+7]; 
		

		next_pkt_offset = received_pkt_index[current_node]-current_pkt_index[current_node];  // packet index difference
		//printf("%d,%d\r\n",next_pkt_offset,current_node);

		//if(next_pkt_offset!=1){
//printf("%d,%d,%d,%d,%d\r\n", local_rx_buf[PKT_DATA_START+7],current_pkt_index[current_node],next_pkt_offset,current_node,isa_get_channel());
			if(next_pkt_offset>=20){
				printf("HUGE LOSS\r\n");
				printf("%d,%d,%d,%d,%d\r\n", local_rx_buf[PKT_DATA_START+7],current_pkt_index[current_node],next_pkt_offset,current_node,isa_get_channel());
			}
		//}
		current_pkt_index[current_node] = local_rx_buf[PKT_DATA_START+7];  // update current pakcet index
		
		pkt_cnt[current_node] += next_pkt_offset; // add the number of packet been measured
		temp = current_pkt_index[current_node] % 8; // use 1 byte to record 8 packets
		//printf("%d,%d,%d\r\n",temp,frame_cnt[0],pkt_cnt[0]);

		if(pkt_cnt[current_node]>=8){
		   frame_cnt[current_node]+=pkt_cnt[current_node]/8;
		   pkt_cnt[current_node]=temp;
		   //printf("current frame cnt: %d\r\n", frame_cnt[current_node]);		   
		}
		
		
		if(frame_cnt[current_node]<NUM_OF_TEST_SET){
		  //printf("%d,%d,%d\r\n",temp,frame_cnt[0],pkt_cnt[0]);
		  pkt_measure[current_node][frame_cnt[current_node]] |= ((uint8_t) 1) << temp;
		}	

		if(frame_cnt[current_node]>=NUM_OF_TEST_SET){
		  /*for(i=0;i<NUM_OF_TEST_SET;i++){
		    printf("pkt: %x\r\n",pkt_measure[current_node][i]);
		  }*/
		  //printf("KO %d\r\n",current_node);
		  // reboot buffer for further test
		  frame_cnt[current_node]=0;
		  sendFlag=1;
		  send_node=current_node;
		  nrk_event_signal (pkt_record_done_signal);
		  //nrk_spin_wait_us(3000);
		  /*for(i=0;i<NUM_OF_TEST_SET;i++){
		    pkt_measure[current_node][i]=0;
		  }*/
 		}

		
	}

}

