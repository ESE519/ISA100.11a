#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <isa.h>
#include <nrk_error.h>
//#include <sys/time.h>


#define MY_CHANNEL 19 
#define MY_ID 2 //change

//#define MY_TX_SLOT_SYNC  2
//#define MY_RX_SLOT  17
#define MY_RX_SLOT  2
#define MY_TX_SLOT  3

#define MY_CLK_SRC_ID  0

NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);


void nrk_create_taskset();

uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
//uint8_t tx_buf2[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];

nrk_time_t timestart;
nrk_time_t timeend;
nrk_time_t newtime;
nrk_time_t timeout;



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
  uint8_t length,slot;
  uint8_t *local_rx_buf;
  uint32_t Score = 0;
  int8_t rssi;
  uint8_t cnt=0;
  //uint8_t tx[3]={2,15,16};
  //uint8_t rx[3]={3,18,19};
  //uint8_t my_tx_slot[4];


  char c = -1;
  nrk_sig_t uart_rx_signal;
  uint8_t finished = 0;

  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  
  nrk_led_set(RED_LED);
  
  isa_set_channel_pattern(1);

  isa_init (ISA_REPEATER, MY_ID, MY_ID);//change
  
  isa_set_schedule(ISA_REPEATER, MY_CLK_SRC_ID);

  isa_set_channel(MY_CHANNEL);

  isa_start();
  
  isa_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE);
  
  while(!isa_ready())  nrk_wait_until_next_period(); 

  /*while(isa_join_ready()!=1) nrk_wait_until_next_period();
    
    for(i=0;i<4;i++){  // set tx slots
	if(tx_slot_from_join[i]==0)
		break;
	else
	    my_tx_slot[i]=tx_slot_from_join[i];
    }	   
  printf("MAIN_TX:%d\r\n",my_tx_slot[0]);*/

  printf("isa start!\n\r");

  //i=0;
  while(1){
//nrk_gpio_toggle(NRK_DEBUG_0);
       if( isa_rx_pkt_check()!=0 ) {

	    local_rx_buf=isa_rx_pkt_get(&length, &rssi);

	    //printf("length is %d, rssi is %d.\n\r",length,rssi);
	    //local_rx_buf[PKT_DATA_START+length-2]='\0';
	    //printf("RX[%d]",slot);
	    /*for(i=PKT_DATA_START; i<length-1; i++ )
		printf( "%c",local_rx_buf[i]);*/
	    //printf("\r\n");
	    //sprintf( &tx_buf[PKT_DATA_START],"Hello Mingzhe!");
	    //length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
	    //isa_tx_pkt(tx_buf,length,configDHDR(),MY_TX_SLOT);
/*
	    length=strlen(&rx_buf[PKT_DATA_START])+PKT_DATA_START+1; //change
	    isa_tx_pkt(rx_buf,length,configDHDR(),MY_TX_SLOT1);//change forward the message from recipient
*/
	    //printf(" Forward message is sent.\n\r");
 	    //printf("pkt length:%d",length);
	    //printf("%d\r\n",cnt++);
	  //  printf( "%c",local_rx_buf[PKT_DATA_START]);
	    isa_rx_pkt_release();
	   // printf("\r\n");

	}
	
	

	/*sprintf( &tx_buf[PKT_DATA_START],local_rx_buf+PKT_DATA_START);
	length=strlen(&rx_buf[PKT_DATA_START])+PKT_DATA_START+1; //change
	//isa_tx_pkt(rx_buf,length,configDHDR(),my_tx_slot[0]);//change forward the message from recipient
	isa_tx_pkt(rx_buf,length,configDHDR(),MY_TX_SLOT);
	isa_wait_until_rx_or_tx ();*/

	sprintf( &tx_buf[PKT_DATA_START],"2");
	length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
	isa_tx_pkt(tx_buf,length,configDHDR(),MY_TX_SLOT);
	isa_wait_until_rx_or_tx ();
	putchar('\n');
	putchar('\r');
	/*sprintf( &tx_buf2[PKT_DATA_START],"Hello from slot 2!");
	length=strlen(&tx_buf2[PKT_DATA_START])+PKT_DATA_START+1;
	isa_tx_pkt(tx_buf2,length,configDHDR(),2);
	isa_wait_until_rx_or_tx ();*/

	
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
  TaskOne.period.nano_secs = 20*NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs = 20*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 50*NANOS_PER_MS;
  nrk_activate_task (&TaskOne);


  nrk_kprintf( PSTR("Create Done\r\n") );
}


