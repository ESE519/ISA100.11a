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
*  Anthony Rowe
*******************************************************************************/


#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <rt_link.h>
#include <nrk_error.h>
#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <ff_basic_sensor.h>

//#define LED_NOTIFY
#define COORDINATOR 
#define NODE_ID	    1

#if NODE_ID == 1
	#define MY_TX_SLOT  2 
	#define MY_RX_SLOT  0 
#endif

#if NODE_ID == 2
	#define MY_TX_SLOT  4 
	#define MY_RX_SLOT  2 
#endif

#if NODE_ID == 3
	#define MY_TX_SLOT  6 
	#define MY_RX_SLOT  4 
#endif


#if NODE_ID == 4
	#define MY_TX_SLOT  8 
	#define MY_RX_SLOT  6 
#endif

#define ADXL_THRESH	3


NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);

void nrk_register_drivers();
uint8_t sigma(uint8_t val1, uint8_t val2);

void nrk_create_taskset();

uint8_t tx_buf[MAX_RTL_PKT_SIZE];
uint8_t rx_buf[MAX_RTL_PKT_SIZE];

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

uint8_t sigma(uint8_t val1, uint8_t val2)
{
if(val1>=val2) return (val1-val2);
return (val2-val1);
}

void Task1()
{
  uint8_t j, i;
  uint8_t *local_rx_buf;
  uint8_t cnt;
  int8_t rssi;
  uint8_t length,slot,vibrate;
  uint16_t counter;
  uint16_t light,adxl_x,adxl_y,adxl_z;
  uint16_t prev_light,prev_adxl_x,prev_adxl_y,prev_adxl_z;
  int8_t fd,val;
  volatile nrk_time_t t;
  printf( "Task1 PID=%d\r\n",nrk_get_pid());
  counter=0;
  cnt=0;

#ifdef COORDINATOR
  nrk_kprintf( PSTR( "Coordinator\r\n")); 
  rtl_init (RTL_COORDINATOR);
  //nrk_led_set(RED_LED);  
#else
  nrk_kprintf( PSTR( "Mobile\r\n")); 
  rtl_init (RTL_MOBILE);
#endif

  printf( "Node %d ",NODE_ID );
  printf( "TX %d ",MY_TX_SLOT);
  printf( "RX %d\r\n",MY_RX_SLOT);

  rtl_set_schedule( RTL_TX, MY_TX_SLOT, 5 ); 
  rtl_set_schedule( RTL_RX, MY_RX_SLOT, 5 ); 
  
  rtl_start();
#if NODE_ID==1
  // Open ADC device as read 
  fd=nrk_open(FIREFLY_SENSOR_BASIC,READ);
  if(fd==NRK_ERROR) nrk_kprintf(PSTR("Failed to open sensor driver\r\n"));
#endif  
  rtl_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE);
  nrk_kprintf( PSTR("start done\r\n") );
  while(!rtl_ready())  nrk_wait_until_next_period(); 

  vibrate=0;
  while(1) {
	

#if NODE_ID==1 
          
		if( rtl_tx_pkt_check(MY_TX_SLOT)!=0 )
	       		{
		  		printf( "Pending on slot %d\r\n",MY_TX_SLOT );
	       		}
	  	else 	{
  				// Open ADC device as read 
  				fd=nrk_open(FIREFLY_SENSOR_BASIC,READ);
  				if(fd==NRK_ERROR) nrk_kprintf(PSTR("Failed to open sensor driver\r\n"));
                		cnt++;
				//val=nrk_set_status(fd,SENSOR_SELECT,LIGHT);
				//val=nrk_read(fd,&light,2);
				//val=nrk_set_status(fd,SENSOR_SELECT,ACC_X);
				//val=nrk_read(fd,&adxl_x,2);
				//val=nrk_set_status(fd,SENSOR_SELECT,ACC_Y);
				//val=nrk_read(fd,&adxl_y,2);
				val=nrk_set_status(fd,SENSOR_SELECT,ACC_Z);
				val=nrk_read(fd,&adxl_z,2);
				nrk_close(fd);
				if(vibrate>0) vibrate--;
				//if(sigma(adxl_x,prev_adxl_x)>ADXL_THRESH) vibrate=3;
				//if(sigma(adxl_y,prev_adxl_y)>ADXL_THRESH) vibrate=3;
				if(sigma(adxl_z,prev_adxl_z)>ADXL_THRESH) vibrate=3;
				//prev_adxl_x=adxl_x;
				//prev_adxl_y=adxl_y;
				prev_adxl_z=adxl_z;
				//prev_light=light;

                		nrk_led_set(2);
				tx_buf[PKT_DATA_START]=NODE_ID;
				tx_buf[PKT_DATA_START+1]=vibrate;
                		length=PKT_DATA_START+2;
                		rtl_tx_pkt( tx_buf, length, MY_TX_SLOT );
                		printf( "Sending Packet on slot %d\r\n",MY_TX_SLOT );
                		nrk_led_clr(2);

			}
	
#else 
	  if( rtl_rx_pkt_check()!=0 )
               {
                   nrk_led_set(GREEN_LED);
		   local_rx_buf=rtl_rx_pkt_get(&length, &rssi, &slot);
                   printf( "Got Packet on slot %d %d: ",slot,length );
                   for(i=PKT_DATA_START; i<length; i++ )
                   {
                        printf( "%d ",local_rx_buf[i] );
                   }
                   nrk_kprintf( PSTR("\r\n") );
               
		if( rtl_tx_pkt_check(MY_TX_SLOT)!=0 )
	       		{
		  	printf( "Pending on slot %d\r\n",MY_TX_SLOT );
	       		}
	  	else 	{
			nrk_led_set(ORANGE_LED);
			#ifdef LED_NOTIFY
			if(local_rx_buf[PKT_DATA_START+1]!=0) nrk_led_clr(BLUE_LED);
			else nrk_led_set(BLUE_LED);
			//if(local_rx_buf[PKT_DATA_START+1]!=0) nrk_led_set(BLUE_LED);
			//else nrk_led_clr(BLUE_LED);
			#endif
			for(i=PKT_DATA_START; i<length; i++ ) tx_buf[i]=rx_buf[i];
			// Add your MAC address to end
			tx_buf[length]=NODE_ID;
                	length++;
			rtl_tx_pkt( tx_buf, length, MY_TX_SLOT );
			printf( "Sending Packet on slot %d\r\n",MY_TX_SLOT );
			nrk_led_clr(ORANGE_LED);
	       		}
                   rtl_rx_pkt_release();
                   nrk_led_clr(GREEN_LED);
		}
#endif 
	  	
	 
	  rtl_wait_until_rx_or_tx();

	  
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
  TaskOne.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);


  nrk_kprintf ( PSTR("Create done\r\n") );
}

void nrk_register_drivers()
{
int8_t val;

// Register the Basic FireFly Sensor device driver
// Make sure to add: 
//     #define NRK_MAX_DRIVER_CNT  
//     in nrk_cfg.h
// Make sure to add: 
//     SRC += $(ROOT_DIR)/src/drivers/platform/$(PLATFORM_TYPE)/source/ff_basic_sensor.c
//     in makefile
val=nrk_register_driver( &dev_manager_ff_sensors,FIREFLY_SENSOR_BASIC);
if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to load my ADC driver\r\n") );

}


