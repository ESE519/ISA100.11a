#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <rt_link.h>
#include <nrk_error.h>
//#include <sys/time.h>

// Change this to your group channel
#define MY_CHANNEL 13 

#define MAX_MOLES  3 //for five nodes it should be 5

#define MY_TX_SLOT  0
#define MOLE_1_RX   2 
#define MOLE_2_RX   4 
#define MOLE_3_RX   6 
#define MOLE_4_RX   8 
#define MOLE_5_RX   10 
#define ROUNDS      50

#define MOLE_INIT_STATUS 0x07 //for five nodes it should be 0x1f

NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);


void nrk_create_taskset();

uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];

nrk_time_t timestart;
nrk_time_t timeend;
nrk_time_t newtime;
nrk_time_t timeout;



int
main ()
{
  uint16_t div;
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);

  nrk_kprintf( PSTR("Starting up...\r\n") );
	
  nrk_init();

  nrk_led_clr(0);
  nrk_led_clr(1);
  nrk_led_clr(2);
  nrk_led_clr(3);
  
  nrk_time_set(0,0);

  
  rtl_task_config();
  
  nrk_create_taskset ();

  nrk_start();
  
  return 0;
}


void Task1()
{

  uint8_t j, i;
  uint8_t length;
  uint8_t rssi,slot,oldMole,newMole=0,Rounds =0;
  uint8_t pre_slot_detect = MOLE_INIT_STATUS;//used for recording previous slot status
  uint8_t cur_slot_detect = MOLE_INIT_STATUS;//used for recording current slot status
  uint8_t tmp = 0x01;
  uint8_t mole_remove = 0;
  uint8_t number_timeouts =0;
  uint8_t number_moles = 0;
  uint8_t user_limit_timeout = 0; //used for recording user's limited responding timeout
  uint8_t *local_rx_buf;
  uint16_t counter;
  uint32_t Score = 0;
  char c = -1;
  nrk_sig_t uart_rx_signal;
  uint8_t finished = 0;

  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  counter=0;
  
  nrk_led_set(RED_LED);
  
  rtl_init (RTL_COORDINATOR);
  
  rtl_set_schedule( RTL_TX, MY_TX_SLOT, 1 );
//  rtl_set_schedule( RTL_RX, MOLE_1_RX, 1 );
//  rtl_set_schedule( RTL_RX, MOLE_2_RX, 1 );
//  rtl_set_schedule( RTL_RX, MOLE_3_RX, 1 );
  rtl_set_schedule( RTL_RX, MOLE_4_RX, 1 );
//  rtl_set_schedule( RTL_RX, MOLE_5_RX, 1 );

  rtl_set_channel(MY_CHANNEL);
  rtl_start();
  rtl_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE);
  
  while(!rtl_ready())  nrk_wait_until_next_period(); 

  // Get the signal for UART RX
  uart_rx_signal=nrk_uart_rx_signal_get();
  // Register task to wait on signal
  nrk_signal_register(uart_rx_signal); 


  // This shows you how to wait until a key is pressed to start
  nrk_kprintf( PSTR("Press any key to start\r\n" ));


  do{
  	if(nrk_uart_data_ready(NRK_DEFAULT_UART))
  		c=getchar();
  	else nrk_event_wait(SIG(uart_rx_signal));
  		nrk_time_get(&timestart);
  } while(c==-1);
  c = -1;
  //generate the first mole
  newMole = rand()%MAX_MOLES;

  //at the very beginning, master has to wait for nodes finishing scheduling their slots
  j=0;
  while(rtl_rx_pkt_check()==0){
  	printf("Waiting for connection, time %d \r\n",j++);
  	sprintf( &tx_buf[PKT_DATA_START],"Master count is S and new mole is S and Round = S");
	length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
	rtl_tx_pkt( tx_buf, length, MY_TX_SLOT);
	rtl_rx_pkt_release();
	rtl_wait_until_rx_or_tx();	
  }

  //record the timeout for timing out
  nrk_time_get(&timeout);
  //initial timeout interval
  user_limit_timeout = 5;
  printf("\r\nGame starts!");
  while(finished==0){
  //begin the game round
  while(Rounds<=ROUNDS){
	if( rtl_tx_pkt_check(MY_TX_SLOT)!=0 ){
		//printf( "Pending on slot %d\r\n",MY_TX_SLOT );
	}
	else {
		//printf("\r\nslot detect value:%d.\r\n",cur_slot_detect);
	        nrk_time_get(&timeend);
		//game round continues
		if (Rounds<=ROUNDS){
			if(timeend.secs-timeout.secs > user_limit_timeout/*ROUNDS/2-Rounds/2*/){
			//a round times out (extra credit)
				nrk_time_get(&timeout);
				oldMole = newMole;
				if(cur_slot_detect==0x00){
					while(oldMole==newMole)
		     			newMole = rand()%MAX_MOLES;
				}else{
					if(cur_slot_detect==0x01)
						mole_remove = 0;
					else if(cur_slot_detect==0x02)
						mole_remove = 1;
					else if(cur_slot_detect==0x04)
						mole_remove = 2;
					else if(cur_slot_detect==0x08)
						mole_remove = 3;
					else if(cur_slot_detect==0x10)
						mole_remove = 4;
					while(oldMole==newMole||mole_remove==newMole)
						newMole = rand()%MAX_MOLES;					
				}
				
				cur_slot_detect = MOLE_INIT_STATUS;
				Rounds++;
				Score -= 10;
				//printf("\nRounds = %d \nnumber_timeouts = %d \npresent time = %d\n", Rounds, number_timeouts, timeout.secs-timestart.secs);
				printf("\r\nRound %d times out! Get a penalty of 10! Score is %d",Rounds,Score);
			}
		}
//		if(pre_slot_detect!=cur_slot_detect)
//			cur_slot_detect = MOLE_INIT_STATUS;
		pre_slot_detect = cur_slot_detect;
		cur_slot_detect = MOLE_INIT_STATUS;
		 // added the next mole to light up into the buffer
		sprintf( &tx_buf[PKT_DATA_START],"Master count is %d and new mole is %d and Round = %d",counter,newMole,Rounds);
		// PKT_DATA_START + length of string + 1 for null at end of string
		//if(Rounds>=ROUNDS){		  
		//	Rounds++;
		//	sprintf( &tx_buf[PKT_DATA_START],"Master count is %d and new mole is %d and Round = %d",counter,6,Rounds); 
		//}

		length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
		rtl_tx_pkt( tx_buf, length, MY_TX_SLOT);
                //printf( "\nTX on slot %d\r\n",MY_TX_SLOT);
                //for(i=PKT_DATA_START;i<length;i++)
                   //printf("%c",tx_buf[i]);
		nrk_led_toggle(BLUE_LED);
		//printf("\n\n");
	}
 

	  // Check for received packet	
  	if( rtl_rx_pkt_check()!=0 ){
		tmp = 0x01;
		uint8_t mole_index,state;
		local_rx_buf=rtl_rx_pkt_get(&length, &rssi, &slot);
		//printf( "RX slot %d %d: ",slot,length );
  		
		//To detect if the node is turned off
		//printf("\n\rtmp:%d");		
		tmp <<= (slot-2)/2;
		cur_slot_detect &= ~tmp;
 		  
                //buffer position 11 stores the value of the moleid from the slaves
                //buffer position 19 stores the value of light 
                // '1' indicates mole whacked (light closed)
                // '0' indicates mole not whacked yet (light open)
		if(((
			local_rx_buf[11]-48) == newMole) &&
			(local_rx_buf[19]=='1') &&
			(Rounds <=ROUNDS)
		){
			//printf("NEW MOLE:%d",newMole);
			oldMole = newMole;
			if(pre_slot_detect==0x00){
				while(oldMole==newMole)
		     		newMole = rand()%MAX_MOLES;
			}else{
				if(pre_slot_detect==0x01)
					mole_remove = 0;
				else if(pre_slot_detect==0x2)
					mole_remove = 1;
				else if(pre_slot_detect==0x04)
					mole_remove = 2;
				else if(pre_slot_detect==0x08)
					mole_remove = 3;
				else if(pre_slot_detect==0x10)
					mole_remove = 4;
				while(oldMole==newMole||mole_remove==newMole)
					newMole = rand()%MAX_MOLES;					
			}                     
			Rounds++;
			user_limit_timeout -= 1;
			if(user_limit_timeout<1)
				user_limit_timeout = 1;		     
			nrk_time_get(&timeend);
			nrk_time_get(&timeout);
			Score += 100;//timeend.secs-timestart.secs; //+ number_timeouts * 10;
			//number_timeouts = 0;
			printf("\r\n You got it. Round: %d, Score : %d",Rounds, Score);
			//cur_slot_detect = MOLE_INIT_STATUS;
		}
		   //printf( "\r\n" ); 
		   rtl_rx_pkt_release();
	} 
	rtl_wait_until_rx_or_tx();		  
  }//while(Rounds<=ROUNDS)

  printf("\r\nDONE and Score = %d \r\n",Score);

    // This shows you how to continue this game or not
  printf("\r\ncontinue this game or not? (y/n)\r\n");
  
  // Get the signal for UART RX
  uart_rx_signal=nrk_uart_rx_signal_get();
  // Register task to wait on signal
  nrk_signal_register(uart_rx_signal);
  //c = -1;
  do{
  	if(nrk_uart_data_ready(NRK_DEFAULT_UART))
  		c=getchar();
  	else nrk_event_wait(SIG(uart_rx_signal));
  		nrk_time_get(&timestart);
  } while(c==-1);

  if(c=='y'||c=='Y'){
	c = -1;
	newMole = 0;
	pre_slot_detect = MOLE_INIT_STATUS;//used for recording previous slot status
	cur_slot_detect = MOLE_INIT_STATUS;//used for recording current slot status
	tmp = 0x01;
	mole_remove = 0;
	number_timeouts =0;
	number_moles = 0;
	Score = 0;
	Rounds = 0;	  
	//at the very beginning, master has to wait for nodes finishing scheduling their slots
  	j=0;
	rtl_rx_pkt_release();
 	while(rtl_rx_pkt_check()==0){
  		printf("Waiting for nodes scheduling their slots, time %d \r\n",j++);
  		sprintf( &tx_buf[PKT_DATA_START],"Master count is S and new mole is S and Round = S");
		length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
		rtl_tx_pkt( tx_buf, length, MY_TX_SLOT);
		rtl_rx_pkt_release();
		rtl_wait_until_rx_or_tx();	
  	}
  }else if(c=='n'||c=='N'){
	printf("Game ends");
	nrk_terminate_task();
  }
}//while(finished)
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
  TaskOne.period.secs = 1;
  TaskOne.period.nano_secs = 0;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);


  nrk_kprintf( PSTR("Create Done\r\n") );
}


