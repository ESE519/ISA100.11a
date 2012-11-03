#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <isa.h>
#include <nrk_error.h>
#include <slip.h>

//#include <sys/time.h>


#define MY_CHANNEL 19 
#define MY_ID 0 //change

#define MY_TX_SLOT  0
#define NUM_OF_TEST_SET 16
#define MAX_SLIP_BUF 16
#define JOIN_TX_SLOT_START 22
//#define MY_RX_SLOT  15
//#define MY_RX_SLOT  2//  change for test




NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void Task2 (void);

void nrk_create_taskset();
void packet_measurement(uint8_t * local_rx_buf, uint8_t len);
void packet_measurement_better(uint8_t * local_rx_buf,uint8_t len);

uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t slip_tx_buf[MAX_SLIP_BUF];
uint8_t slip_rx_buf[MAX_SLIP_BUF];

nrk_time_t timestart;
nrk_time_t timeend;
nrk_time_t newtime;
nrk_time_t timeout;

uint8_t pkt_measure[NUM_OF_TEST_SET];
uint8_t sendFlag=0;
uint8_t frame_cnt=0;  //add 1 every 8 packets
uint8_t pkt_cnt;
char current_pkt_index='0';

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


void Task1()
{

  uint8_t j, i;
  uint8_t length,slot,len;
  uint8_t *local_rx_buf;
  //uint32_t Score = 0;
  int8_t rssi;
  //uint8_t cnt=0;
  //uint8_t pkt_cnt=0;
  
  




  //char c = -1;
  nrk_sig_t uart_rx_signal;
  uint8_t finished = 0;

  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  
  nrk_led_set(RED_LED);

  nrk_led_set(BLUE_LED);
  
  isa_set_channel_pattern(1);
  
  isa_set_channel_pattern(3);	

  isa_init (ISA_GATEWAY, MY_ID);//change
  
  isa_set_schedule(ISA_GATEWAY, MY_ID);

  isa_set_channel(MY_CHANNEL);

  //configAdvDAUX(1, 0, 25, 1, NULL, NULL, NULL, 2, NULL, NULL, NULL);

  isa_start();
  
  isa_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE);

  //slip_init (stdin, stdout, 0, 0);

  //while (slip_started () != 1) nrk_wait_until_next_period ();
  
  while(!isa_ready())  nrk_wait_until_next_period(); 
  printf("isa start!\n\r");

  //i=0;
  while(1){
//nrk_gpio_toggle(NRK_DEBUG_0);
       
       if( isa_rx_pkt_check()!=0 ) {
	    //printf("message is received.\n\r");
	    local_rx_buf=isa_rx_pkt_get(&length, &rssi);
	    //printf("RXLEN:%d\r\n",length);
	    //for(i=PKT_DATA_START; i<length-1; i++ )
	    	printf( "%d",local_rx_buf[PKT_DATA_START+10]);
	    //packet_measurement(local_rx_buf,length);
 	    //packet_measurement_better(local_rx_buf,length);
	    
	    //pkt_cnt++;
	    isa_rx_pkt_release();
	    printf("\r\n");
	}
	
	
	sprintf( &tx_buf[PKT_DATA_START],"Hello");
  	length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
  	isa_tx_pkt(tx_buf,length,configDHDR(),MY_TX_SLOT);
	//printf("Len:%d\r\n",length);
  	//printf("Hello world is sent.\n\r");
  	//printf("Recieved %d packets!\r\n",pkt_cnt);
	
	isa_wait_until_rx_or_tx ();
  }
  

}


void Task2 ()
{
  uint16_t cnt;
  uint8_t len,i;

  printf ("My node's address is %d\r\n", NODE_ADDR);

  printf ("Task1 PID=%d\r\n", nrk_get_pid ());
  cnt = 0;
  slip_init (stdin, stdout, 0, 0);

  while (1) {
    //nrk_led_set (ORANGE_LED);
    //sprintf (slip_tx_buf, pkt_measure);
	if(sendFlag){
		for(uint8_t i=0;i<NUM_OF_TEST_SET;i++){
			slip_tx_buf[i]=pkt_measure[i];
			//printf("%x",slip_tx_buf[i]);
		}
		//sprintf (slip_tx_buf, pkt_measure);
		//printf("\r\n");
    	len = strlen (slip_tx_buf);
    	slip_tx (slip_tx_buf, len);
		sendFlag=0;
		for(i=0;i<NUM_OF_TEST_SET;i++){
		    pkt_measure[i]=0;	    
		}
		
    	//nrk_wait_until_next_period ();
    	//cnt++;
	}	
	nrk_wait_until_next_period ();
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
  TaskOne.period.secs = 0;
  TaskOne.period.nano_secs = 500*NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs = 500*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);
	
  TaskTwo.task = Task2;
  nrk_task_set_stk( &TaskTwo, Stack2, NRK_APP_STACKSIZE);
  TaskTwo.prio = 2;
  TaskTwo.FirstActivation = TRUE;
  TaskTwo.Type = BASIC_TASK;
  TaskTwo.SchType = PREEMPTIVE;
  TaskTwo.period.secs = 1;
  TaskTwo.period.nano_secs = 500 * NANOS_PER_MS;
  TaskTwo.cpu_reserve.secs = 0;
  TaskTwo.cpu_reserve.nano_secs = 0;
  TaskTwo.offset.secs = 0;
  TaskTwo.offset.nano_secs = 0;
  //nrk_activate_task (&TaskTwo);

  nrk_kprintf( PSTR("Create Done\r\n") );
}

/*void packet_measurement(uint8_t * local_rx_buf,uint8_t len)
{
	uint8_t i,length;
	length=len;

	

	if(local_rx_buf[PKT_DATA_START]=='r'){
		//printf("first : %c\r\n", local_rx_buf[length-2]);
		//printf("second : %c\r\n", local_rx_buf[length-3]);
		//printf("third : %c\r\n", local_rx_buf[length-4]);
		uint8_t temp_buf[3];
		uint8_t temp;
		uint8_t firstCheck;

		temp_buf[0]=local_rx_buf[length-2];

		if (local_rx_buf[length-3]>='0' && local_rx_buf[length-3]<='9'){
	 	   temp_buf[0]=local_rx_buf[length-3];
		   temp_buf[1]=local_rx_buf[length-2];
		   if (local_rx_buf[length-4]>='0' && local_rx_buf[length-4]<='9'){
	 	      temp_buf[0]=local_rx_buf[length-4];
		      temp_buf[1]=local_rx_buf[length-3];
		      temp_buf[2]=local_rx_buf[length-2];
		   }
		}
		else{
		   temp_buf[1]=0;
		   temp_buf[2]=0;
		}
			
		temp = atoi(temp_buf);
		firstCheck = temp;		
		temp = temp%8;
		//printf("final temp: %d\r\n",temp);
		pkt_measure[frame_cnt] |= ((uint32_t) 1) << temp;
		if(temp==0 && firstCheck>8){
		   frame_cnt++;
		   //printf("current frame cnt: %d\r\n", frame_cnt);		   
		}
		
		if(frame_cnt>=NUM_OF_TEST_SET){
		  for(i=0;i<NUM_OF_TEST_SET;i++){
		    printf("pkt measurement: %x\r\n",pkt_measure[i]);
		  }
		  
		  // reboot buffer for further test
		  sendFlag=1;
		  for(i=0;i<NUM_OF_TEST_SET;i++){
		    pkt_measure[i]=0;	    
		  }
		  frame_cnt=0;
 		}
     }
}*/

void packet_measurement_better(uint8_t * local_rx_buf,uint8_t len)
{
	uint8_t i,length;
	uint8_t next_pkt_offset;
	uint8_t temp;

	length=len;	
	if(local_rx_buf[PKT_DATA_START]=='r'){
		next_pkt_offset = local_rx_buf[length-2]-current_pkt_index;
		//printf("Next PKT OFFSET: %d", next_pkt_offset);
		current_pkt_index = local_rx_buf[length-2];
		pkt_cnt += next_pkt_offset;
		temp = pkt_cnt%8;
		pkt_measure[frame_cnt] |= ((uint8_t) 1) << temp;
		if(temp==0 && pkt_cnt>8){
		   frame_cnt++;
		   //printf("current frame cnt: %d\r\n", frame_cnt);		   
		}
		
		if(frame_cnt>=NUM_OF_TEST_SET){
		  //for(i=0;i<NUM_OF_TEST_SET;i++){
		    //printf("pkt measurement: %x\r\n",pkt_measure[i]);
		  //}
		  printf("Sending info..\r\n");
		  // reboot buffer for further test
		  sendFlag=1;
		  frame_cnt=0;
 		}
	}

}

