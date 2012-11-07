#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <isa.h>
#include <nrk_error.h>
#include <nrk_events.h>

#define MAX_SLIP_BUF 48
uint8_t slip_rx_buf[MAX_SLIP_BUF];
uint8_t slip_tx_buf[MAX_SLIP_BUF];


//#define MY_CHANNEL 19 
//#define MY_TX_SLOT 2
//#define MY_RX_SLOT 2

#define MY_ID  2
#define MY_CLK_SRC_ID  1

NRK_STK Stack1[NRK_APP_STACKSIZE];
NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
nrk_task_type TaskTwo;
void Task1(void);
void Task2(void);


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
    uint8_t i;
    uint8_t length;
    uint8_t *local_rx_buf;
    int8_t rssi;
    uint8_t cnt; 
    uint8_t my_tx_slot[4];

    printf( "Task1 PID=%d\r\n",nrk_get_pid());

    nrk_led_set(GREEN_LED); 

    isa_init(ISA_RECIPIENT,MY_ID,MY_CLK_SRC_ID);
  
    //isa_set_channel_pattern(1);

    //isa_set_schedule(ISA_RECIPIENT,MY_CLK_SRC_ID);
  
    //isa_set_channel(MY_CHANNEL);

    isa_start();
 
    isa_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE);
 
    while(!isa_ready())  nrk_wait_until_next_period(); 
	
    while(isa_join_ready()!=1) nrk_wait_until_next_period();
    
    for(i=0;i<4;i++){  // set tx slots
	if(tx_slot_from_join[i]==0)
		break;
	else
	    my_tx_slot[i]=tx_slot_from_join[i];
    }	   
    printf("MAIN_TX:%d\r\n",my_tx_slot[0]);

    while(1) {
	//printf("check %d",isa_rx_pkt_check());
	/*if( isa_rx_pkt_check()!=0 ) {
	    local_rx_buf=isa_rx_pkt_get(&length, &rssi);
	    //local_rx_buf[PKT_DATA_START+length-1]='\0';
	    //printf("length is %d, rssi is %d.\n\r",length,rssi);
	    for(i=PKT_DATA_START; i<length-1; i++ )
		printf( "%c",local_rx_buf[i]);
	    cnt++;
	}*/
	//printf("send message %d\r\n",cnt);
	sprintf( &tx_buf[PKT_DATA_START],"recipient %c", cnt++);
	length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
	isa_tx_pkt(tx_buf,length,configDHDR(),my_tx_slot[0]);
	//isa_rx_pkt_release();	
	isa_wait_until_rx_or_tx();
    }
}

void Task2()
{  uint16_t cnt;
  uint8_t len;
  printf ("My node's address is %d\r\n", NODE_ADDR);

  printf ("Task1 PID=%d\r\n", nrk_get_pid ());
  cnt = 0;
  slip_init (stdin, stdout, 0, 0);

  while (1) {
    //nrk_led_set (ORANGE_LED);
    //printf("Start\n\r");
    sprintf (slip_tx_buf, "Hello ALex");
    len = strlen (slip_tx_buf);
    slip_tx (slip_tx_buf, len);
    nrk_wait_until_next_period ();
    //nrk_led_clr (ORANGE_LED);
    nrk_wait_until_next_period ();
    cnt++;
    //printf("End\n\r");
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
  TaskOne.cpu_reserve.nano_secs = 0;
  TaskOne.cpu_reserve.nano_secs = 500*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);
  nrk_kprintf( PSTR("Create Done\r\n") );

  TaskTwo.task = Task2;
  TaskTwo.Ptos = (void *) &Stack2[NRK_APP_STACKSIZE-1];
  TaskTwo.Pbos = (void *) &Stack2[0];
  TaskTwo.prio = 15;
  TaskTwo.FirstActivation = TRUE;
  TaskTwo.Type = BASIC_TASK;
  TaskTwo.SchType = PREEMPTIVE;
  TaskTwo.period.secs = 1;
  TaskTwo.period.nano_secs = 500*NANOS_PER_MS;
  TaskTwo.cpu_reserve.secs = 1;
  TaskTwo.cpu_reserve.nano_secs = 0;
  TaskTwo.cpu_reserve.nano_secs = 500*NANOS_PER_MS;
  TaskTwo.offset.secs = 0;
  TaskTwo.offset.nano_secs= 0;
  //nrk_activate_task (&TaskTwo);
  nrk_kprintf( PSTR("Create Done\r\n") );
}

