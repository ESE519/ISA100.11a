#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <isa.h>
#include <nrk_error.h>
//#include <sys/time.h>
#include <spi_matrix.h>

#define MY_CHANNEL 19 
	 2//change

//#define MY_TX_SLOT_SYNC  2
//#define s  17
//#define MY_RX_SLOT  2
//#define MY_TX_SLOT  7
//#define MY_TX_SLOT1  8


#define MY_CLK_SRC_ID  1

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

//*********************Making a callback function***************************************

void transmitCallback1(ISA_QUEUE *entry , bool status){
uint8_t length;
MESSAGE *message;
DLMO_DROUT *dRout;
	 	message = &tx_buf[PKT_DATA_START];
	 	message->type = DUMMY_PAYLOAD;
	  sprintf( &message->data,"node" );
	  length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+2;
	  sendPacket(entry->tx_buf[DEST_INDEX],dRout->GraphId, length, tx_buf, transmitCallback1);
	  	isaFreePacket(entry);
}
//*******************************************************************************

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
  isa_set_channel_pattern(1);
  isa_init (ISA_REPEATER, MY_ID, MY_CLK_SRC_ID);//change
  
  dlmoInit(); 	//Initialize the Data Link Management Object

  addNeighbor(1,0,0,0,true,0,0,0);
  addNeighbor(5,0,0,0,false,0,0,0);
  addNeighbor(6,0,0,0,false,0,0,0);
  addNeighbor(7,0,0,0,false,0,0,0);
  addNeighbor(8,0,0,0,false,0,0,0);
  addNeighbor(9,0,0,0,false,0,0,0);

  addLink(26,0,0,4,0);//ad
  addLink(4,1,0,1,0);
  addLink(7,5,0,1,0);
  addLink(8,6,0,1,0);
  addLink(9,7,0,1,0);
  addLink(10,8,0,1,0);
  addLink(11,9,0,1,0);
  addLink(12,0,0,8,0);//receive from 5
  addLink(13,0,0,8,0);//6
  addLink(14,0,0,8,0);//7
  addLink(15,0,0,8,0);//8
  addLink(16,0,0,8,0);//9
  addLink(3,0,0,8,0);//1
  addLink(25,0,0,8,0);
  addLink(27,0,0,8,0);//Ad


  /*addLink(17,0,0,4,0);//transmit ad
  addLink(40,0,0,8,0);//receive ad
  addLink(18,0,0,8,0);
  addLink(19,0,0,8,0);
  addLink(20,0,0,8,0);
  addLink(21,0,0,8,0);

  addLink(41,0,0,8,0);//receive from 1
  addLink(42,1,0,1,0);//transmit to 1
  addGraph(1,1,4,0,0);//graph
  addLink(5,4,1,1,2);//transmit to 4
  addLink(6,0,0,8,0);//receive from 4*/
  //addLink(26,0,0,4,0);

 /* addLink(7,5,0,1,0);
  addLink(8,6,0,1,0);
  addLink(9,7,0,1,0);
  addLink(10,8,0,1,0);
  addLink(11,9,0,1,0);

  addLink(12,0,0,8,0);
  addLink(13,0,0,8,0);
  addLink(14,0,0,8,0);
  addLink(15,0,0,8,0);
  addLink(16,0,0,8,0);
  addLink(3,0,0,8,0);
  */

  //addGraph(1,3,5,3,4);
 // addLink(2,1,1,1,0);//transmitting on slot 2
 // addLink(10,1,1,8,0); //receiving on slot 10
 // addLink(1,1,1,8,0);//receiving on slot 1

/*
  configureSlot(2, 1, TX_NO_ADV, true,0,0,0,0,0,NEIGHBOR);
  configureSlot(7,5,TX_NO_ADV,false,1,1,5,0,0,GRAPH_NEIGHBOR);
 // configureSlot(5, 1, TX_NO_ADV, true,0,0,0,0,0, NEIGHBOR);
  //configureSlot(2,3, RX, false,0,0,0,0,0, NEIGHBOR);
  //configureSlot(7,10,ADV,false,0,0,0,0,0, NEIGHBOR);
  //configureSlot(6,3, RX, false,0,0,0,0,0, NEIGHBOR);
  configureSlot(11,0, RX, false,0,0,0,0,0, NEIGHBOR);
  configureSlot(8,0, RX, false,0,0,0,0,0, NEIGHBOR);

  configureSlot(19,0,ADV,false,0,0,0,0,0,NEIGHBOR);
  configureSlot(20,0, RX, false,0,0,0,0,0, NEIGHBOR);
  configureSlot(21,0, RX, false,0,0,0,0,0, NEIGHBOR);
  configureSlot(22,0, RX, false,0,0,0,0,0, NEIGHBOR);
  configureSlot(23,0, RX, false,0,0,0,0,0, NEIGHBOR);
  configureSlot(24,0, RX, false,0,0,0,0,0, NEIGHBOR);
*/

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
  



 // isa_set_schedule(ISA_REPEATER, MY_CLK_SRC_ID);

 // isa_set_channel(MY_CHANNEL);

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


//Spit out log info

	  	  if (txCount % 1000 == 0){
	  	printf ("Tx: %d\r\nRX: %d\r\nPL:%d", txCount,rxCount, packetsLost);
	  	  }

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
	    isa_tx_pkt(rx_buf,length,configDHDR(8),MY_TX_SLOT1);//change forward the message from recipient
*/
	    //printf(" Forward message is sent.\n\r");
 	    //printf("pkt length:%d",length);
	    //printf("%d\r\n",cnt++);
	  //  printf( "%c",local_rx_buf[PKT_DATA_START]);
	    isa_rx_pkt_release();
	   // printf("\r\n");

	}

    //   if(isa_tx_pkt_check(MY_TX_SLOT)!=0){
       	  // printf("Pending TX\r\n");
    //   	}
   //    else{
	/*sprintf( &tx_buf[PKT_DATA_START],local_rx_buf+PKT_DATA_START);
	length=strlen(&rx_buf[PKT_DATA_START])+PKT_DATA_START+1; //change
	//isa_tx_pkt(rx_buf,length,configDHDR(),my_tx_slot[0]);//change forward the message from recipient
	isa_tx_pkt(rx_buf,length,configDHDR(),MY_TX_SLOT);
	isa_wait_until_rx_or_tx ();*/
    	   if (cnt ==0 ){
    		   MESSAGE *message;
    		   	message = &tx_buf[PKT_DATA_START];
    		   	message->type = DUMMY_PAYLOAD;
	sprintf( &message->data,"2");
	length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+2;
	sendPacket(1,0, length, tx_buf, transmitCallback1);
	//sendPacket(5,0, length, tx_buf, transmitCallback1);
	//sendPacket(6, length, tx_buf, transmitCallback1);
    	cnt++;
    	   }

	/*sprintf( &tx_buf2[PKT_DATA_START],"Hello from slot 2!");
	length=strlen(&tx_buf2[PKT_DATA_START])+PKT_DATA_START+1;
	isa_tx_pkt(tx_buf2,length,configDHDR(),2);
	isa_wait_until_rx_or_tx ();*/
    	   setMatrix();
    	     	  nrk_wait_until_next_period();
    //   }
    //	   nrk_terminate_task();
     //  isa_wait_until_rx_or_tx ();
      // 	putchar('\n');
      // 	putchar('\r');
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
  TaskOne.period.nano_secs = 10*NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs = 20*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 50*NANOS_PER_MS;
  nrk_activate_task (&TaskOne);


  nrk_kprintf( PSTR("Create Done\r\n") );
}


