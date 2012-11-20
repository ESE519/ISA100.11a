#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <rt_link.h>
#include <nrk_error.h>
#include <nrk_events.h>
#include <nrk_driver.h>
#include <nrk_driver_list.h>
#include <ff_basic_sensor.h>
#include  <math.h>


#define MY_CHANNEL 13 
#define MOLE_ID 4
#define LIGHT_DIFF 20 // Determined experimentally



#define MASTER_TX_SLOT  0

#define MY_TX_SLOT ((MOLE_ID*2)+2)


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

  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);
  

  nrk_kprintf( PSTR("Starting up...\r\n") );

	
  nrk_init();

  nrk_led_clr(0);
  nrk_led_clr(1);
  nrk_led_clr(2);
  nrk_led_clr(3);
  
  nrk_time_set(0,0);

  
  nrk_register_drivers();
  rtl_task_config();
  nrk_create_taskset ();

  nrk_start();
  
  return 0;
}

void Task1()
{
  uint8_t j, i;
  uint8_t rssi, slot,length, mole_id;
  uint8_t *local_rx_buf;
  uint8_t fd,val;
  uint16_t light = 0;
  uint16_t light_avg = 0;
  uint8_t whacked = 0;

  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  printf( "Node ID=%d\r\n",MOLE_ID);

  nrk_led_set(GREEN_LED); 

  rtl_init (RTL_MOBILE);
  rtl_set_channel(MY_CHANNEL);
  
  rtl_set_schedule( RTL_RX, MASTER_TX_SLOT, 1 );
  rtl_set_schedule( RTL_TX, MY_TX_SLOT, 1 );
  
  rtl_start();
 
  rtl_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE);
 
  while(!rtl_ready())  nrk_wait_until_next_period(); 
  fd=nrk_open(FIREFLY_SENSOR_BASIC,READ);
  if(fd==NRK_ERROR) nrk_kprintf(PSTR("Failed to open sensor driver\r\n"));

  nrk_time_get(&timeout);
  nrk_time_get(&timeend);

  val=nrk_set_status(fd,SENSOR_SELECT,LIGHT);
  
  // Get current lighting
  for(i=0;i<5;i++){
	nrk_read(fd,&light,2);
	light_avg += light;
  }
  
  /*nrk_read(fd,&light,2);
  light_avg += light;
  nrk_read(fd,&light,2);
  light_avg += light;
  nrk_read(fd,&light,2);
  light_avg += light;
  nrk_read(fd,&light,2);
  light_avg += light;
  nrk_read(fd,&light,2);
  light_avg += light;*/
  
  light_avg /= 5;
  
  //printf( "Light value = %d\r\n", light);
	while(1) {
		if( rtl_tx_pkt_check(MY_TX_SLOT)!=0 ) {
			nrk_led_clr(RED_LED);
			printf( "Pending on slot %d\r\n",MY_TX_SLOT );
		} else {
			//read sensor output to see whether mole is whacked, and print reading
			nrk_read(fd,&light,2);
			printf( "Light value = %d\r\n", light);
			
			// if light closed or mole whacked, then transmit '1' - indicates the master to change the mole
			// If the light darkens by a pre-set difference, count as a whack
			if (light_avg+LIGHT_DIFF < light || whacked == 1) {
				sprintf(&tx_buf[PKT_DATA_START],"MOLE_ID=%d LIGHT=1", MOLE_ID);
			} else {
				sprintf(&tx_buf[PKT_DATA_START],"MOLE_ID=%d LIGHT=0", MOLE_ID);
				// Update average
				/*light_avg = 0;
				for(i=0;i<5;i++){
					nrk_read(fd,&light,2);
					light_avg += light;
  				}
				light_avg /= 5;*/
				light_avg *= 5;
				light_avg += light;
				light_avg /= 6;				
				//light_avg >>= 1;
				//light_avg &= 0xffff;
			}
			
			// Transmit
			length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+1;
			rtl_tx_pkt( tx_buf, length, MY_TX_SLOT);
			printf( "\nTX on slot %d\r\n",MY_TX_SLOT);
			for(i=PKT_DATA_START;i<length;i++)
				printf("%c",tx_buf[i]);
			nrk_led_toggle(BLUE_LED);
		}
	 
		if( rtl_rx_pkt_check()!=0 ) {
			local_rx_buf=rtl_rx_pkt_get(&length, &rssi, &slot);
			printf( "RX on slot %d %d: ",slot,length );
			  
			for(i=PKT_DATA_START; i<length; i++ )
				printf( "%c",local_rx_buf[i] );

			// buffer position 37 stores the next mole id from the master
			// if that is equal to MYMOLEID then turn on the led
			if((local_rx_buf[37]-48) == MOLE_ID) {
				if (light_avg + LIGHT_DIFF < light || whacked == 1) {
					whacked = 1;
					nrk_led_clr(RED_LED);
				} else {
					nrk_led_set(RED_LED);
				}
			} else {
				whacked = 0;
				nrk_led_clr(RED_LED);
			}
			
			//printf(" rounds value  %d ", local_rx_buf[51]-48);
			//printf("\n NEW: %c",local_rx_buf[37]);
			//nrk_kprintf( PSTR("\r\n") );
			rtl_rx_pkt_release();
		}

		//	if((local_rx_buf[51]-48)>=9)
		//	  nrk_led_clr(RED_LED);
		  
		rtl_wait_until_rx_pkt();
		
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
  TaskOne.period.secs = 1;
  TaskOne.period.nano_secs = 0;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs = 0;
  TaskOne.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);


  nrk_kprintf( PSTR("Create Done\r\n") );
}


void nrk_register_drivers()
{
int8_t val;

// Register the Basic FireFly Sensor device driver
// Make sure to add: 
//	 #define NRK_MAX_DRIVER_CNT  
//	 in nrk_cfg.h
// Make sure to add: 
//	 SRC += $(ROOT_DIR)/src/drivers/platform/$(PLATFORM_TYPE)/source/ff_basic_sensor.c
//	 in makefile
val=nrk_register_driver( &dev_manager_ff_sensors,FIREFLY_SENSOR_BASIC);
if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to load my ADC driver\r\n") );

}

