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
*******************************************************************************/

#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_ext_int.h>
#include <nrk_stack_check.h>
#include <nrk_driver_list.h>
#include <nrk_driver.h>
#include <ff_basic_sensor.h> 
#include "mmc.h"


uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t sectorbuffer[512];

#define BPM_LIST_SIZE 15

NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void Task2 (void);

NRK_STK Stack3[NRK_APP_STACKSIZE];
nrk_task_type TaskThree;
void Task3 (void);


NRK_STK Stack4[NRK_APP_STACKSIZE];
nrk_task_type TaskFour;
void Task4 (void);
void nrk_create_taskset();

nrk_time_t l,c,t;
uint8_t bpm_list[BPM_LIST_SIZE];
uint8_t bpm_sort[BPM_LIST_SIZE];
uint8_t bpm_index;

void oled_draw_square( uint8_t x, uint8_t y, uint8_t x2, uint8_t y2, uint16_t color  );
void oled_write_str( uint8_t x, uint8_t y, uint8_t font, uint16_t color, char *str );
void oled_on();
void oled_off();

uint8_t hrm_get_value();
void heart_rate_int();

void oled_on()
{
nrk_time_t t;
nrk_gpio_set(NRK_DEBUG_1);
t.secs=1;
t.nano_secs=500*NANOS_PER_MS;
nrk_kprintf( PSTR( "start wait\r\n" ));
nrk_wait(t);
nrk_kprintf( PSTR( "stop wait\r\n" ));
putc0(0x55);
}

void oled_off()
{
nrk_gpio_clr(NRK_DEBUG_1);
}

void oled_setup()
{
INIT_UART0( UART_BAUDRATE_9K6, (UART_OPT_NO_PARITY|UART_OPT_8_BITS_PER_CHAR|UART_OPT_ONE_STOP_BIT));
ENABLE_UART0();
}

int
main ()
{
  uint8_t t;
  nrk_setup_ports();
  nrk_setup_uart(UART_BAUDRATE_115K2);
  oled_setup();
  nrk_init();

  nrk_gpio_direction(NRK_DEBUG_0, NRK_PIN_INPUT);
  nrk_gpio_direction(NRK_DEBUG_1, NRK_PIN_OUTPUT);
  oled_off();
  nrk_gpio_direction(NRK_DEBUG_2, NRK_PIN_INPUT);
  nrk_gpio_direction(NRK_DEBUG_3, NRK_PIN_INPUT);
  nrk_led_clr(ORANGE_LED);
  nrk_led_clr(BLUE_LED);
  nrk_led_clr(GREEN_LED);
  nrk_led_clr(RED_LED);
 
  nrk_time_set(0,0);
  bmac_task_config ();
  nrk_create_taskset ();
  nrk_start();
  
  return 0;
}


void button_press_int()
{

nrk_kprintf( PSTR("button press!\r\n"));


}

void heart_rate_int()
{
uint8_t bpm;
  nrk_led_set(BLUE_LED);
  nrk_time_get(&c);
  nrk_time_sub(&t,c,l);
  bpm=(uint16_t)60000 / (uint16_t)(t.nano_secs/1000000);
if(bpm>20 && bpm<220 )
  {
    bpm_list[bpm_index]=bpm;
    bpm_index++;
    if(bpm_index==BPM_LIST_SIZE)bpm_index=0;
  }

  l.secs=c.secs;
  l.nano_secs=c.nano_secs;
  nrk_led_clr(BLUE_LED);
}


uint8_t hrm_get_value()
{
uint16_t acc;
uint8_t i,j,tmp;

  nrk_time_get(&c);
  nrk_time_sub(&t,c,l);

  if(t.secs>5) {
	for(i=0; i<BPM_LIST_SIZE; i++ ) bpm_list[i]=0;
	return 0;
	}

for(i=0; i<BPM_LIST_SIZE; i++ ) 
	{
	bpm_sort[i]=bpm_list[i];
	if(bpm_list[i]==0) return 0;
	}

for(i=0; i<BPM_LIST_SIZE; i++ )
  {
	for(j=0; j<BPM_LIST_SIZE-1; j++ )
		{
		if(bpm_sort[j]>bpm_sort[j+1] )
			{
			tmp=bpm_sort[j];
			bpm_sort[j]=bpm_sort[j+1];
			bpm_sort[j+1]=tmp;
			}
		}
  }
/*
for(i=0; i<BPM_LIST_SIZE; i++ ) 
	{
	if(bpm_index==i) printf( ">" );
	printf( "%u %d %d\r\n",i,bpm_sort[i],bpm_list[i]);
	}
*/
acc=0;
for(i=(BPM_LIST_SIZE/2)-2; i<(BPM_LIST_SIZE/2)+2; i++ )
   acc+=bpm_sort[BPM_LIST_SIZE/2];
//return bpm_sort[BPM_LIST_SIZE/2];
return (acc/4);
}


void Task1()
{
uint16_t cnt;
int8_t fd;
nrk_kprintf( PSTR("Nano-RK Version ") );
printf( "%d\r\n",NRK_VERSION );

//while(1) nrk_wait_until_next_period();


nrk_gpio_direction(NRK_MMC_9,NRK_PIN_INPUT);
nrk_gpio_direction(NRK_MMC_10,NRK_PIN_INPUT);
nrk_gpio_direction(NRK_MMC_11,NRK_PIN_INPUT);

bpm_index=0;

nrk_gpio_pullups(1);

nrk_ext_int_configure(NRK_PC_INT_5, NULL, &button_press_int);
nrk_ext_int_configure(NRK_PC_INT_6, NULL, &button_press_int);
nrk_ext_int_configure(NRK_PC_INT_7, NULL, &button_press_int);
nrk_ext_int_enable(NRK_PC_INT_5);
nrk_ext_int_enable(NRK_PC_INT_6);
nrk_ext_int_enable(NRK_PC_INT_7);

nrk_ext_int_configure(NRK_EXT_INT_0, NRK_RISING_EDGE, &heart_rate_int);
nrk_ext_int_enable(NRK_EXT_INT_0);

cnt=0;
printf( "My node's address is %u\r\n",NODE_ADDR );
  
fd=nrk_open(FIREFLY_SENSOR_BASIC,READ);
        if(fd==NRK_ERROR) nrk_kprintf(PSTR("Failed to open sensor driver\r\n"));

printf( "Task1 PID=%u\r\n",nrk_get_pid());

  while(1) {
	//printf( "Task1 cnt=%u\r\n",cnt );
	nrk_wait_until_next_period();
	printf( "9=%d ",nrk_gpio_get(NRK_MMC_9));
	printf( "10=%d ",nrk_gpio_get(NRK_MMC_10));
	printf( "11=%d ",nrk_gpio_get(NRK_MMC_11));
	printf( "%d %d\r\n",cnt,hrm_get_value() );
	nrk_led_toggle(GREEN_LED);
        // Uncomment this line to cause a stack overflow
	cnt++;
	}
}

void Task2()
{
  int16_t cnt;
  int8_t val;
uint32_t sector = 0;
nrk_sem_t *radio_sem;


while(1) nrk_wait_until_next_period();
radio_sem= rf_get_sem();
if(radio_sem==NULL) nrk_kprintf( PSTR("radio sem failed!\r\n" ));
  printf( "Task2 PID=%u\r\n",nrk_get_pid());
  cnt=0;


  val=mmc_init();
  if(val!=0 ) {
	printf( "val=%d\r\n",val );
        nrk_kprintf( PSTR("card init failed\r\n") );
        while(1);
        }

  while(1) {
	nrk_sem_pend (radio_sem);
        printf("\nsector %lu\n\r",sector);                // show sector number
        val=mmc_readsector(sector,sectorbuffer);    // read a data sector
        printf( "readsector returned %d\n",val );
        for(cnt=0; cnt<32; cnt++ )
                printf( "%d ",sectorbuffer[cnt] );
        printf( "\n\r" );

        val=sectorbuffer[0];
        val++;
        for(cnt=0; cnt<512; cnt++ )
        {
        sectorbuffer[cnt]=val;
        }

        nrk_kprintf( PSTR("Writting\r\n") );
        val=mmc_writesector(sector,sectorbuffer);    // read a data sector
        printf( "writesector returned %d\n",val );
        printf( "After write:\r\n" );
        val=mmc_readsector(sector,sectorbuffer);    // read a data sector
	nrk_sem_post(radio_sem);
        printf( "readsector returned %d\n",val );
        if(val==0)
        {
         for(cnt=0; cnt<32; cnt++ )
                printf( "%d ",sectorbuffer[cnt] );
        nrk_kprintf( PSTR("\n\r") );
        }
sector++;
//	nrk_led_toggle(RED_LED);
//	printf( "Task2 signed cnt=%d\r\n",cnt );
	nrk_wait_until_next_period();
	cnt--;
	}
}

void Task3()
{
  uint8_t i, len,cnt;
  int8_t rssi, val;
  uint8_t *local_rx_buf;
  nrk_sig_mask_t ret;
  nrk_time_t check_period;  

  printf( "Task3 PID=%u\r\n",nrk_get_pid());
  // init bmac on channel 25 
  bmac_init (25);
  bmac_rx_pkt_set_buffer (rx_buf, RF_MAX_PAYLOAD_SIZE);

  while (!bmac_started ())
    nrk_wait_until_next_period ();

/* while (1) {
    // Wait until an RX packet is received
  nrk_kprintf( PSTR( "Waiting for packet\r\n" ));
    val = bmac_wait_until_rx_pkt ();
    // Get the RX packet 
    nrk_led_set (ORANGE_LED);
    local_rx_buf = bmac_rx_pkt_get (&len, &rssi);
    printf ("Got RX packet len=%d RSSI=%d [", len, rssi);
    for (i = 0; i < len; i++)
      printf ("%c", rx_buf[i]);
    printf ("]\r\n");
    nrk_led_clr (ORANGE_LED);
    // Release the RX buffer so future packets can arrive 
    bmac_rx_pkt_release ();
  }
*/

  while (1) {

    // Build a TX packet
    sprintf (tx_buf, "This is a test %d", cnt);
    cnt++;
    nrk_led_set (RED_LED);

    // For blocking transmits, use the following function call.
    // For this there is no need to register  
     val=bmac_tx_pkt(tx_buf, strlen(tx_buf)+1);

    // Task gets control again after TX complete
    nrk_kprintf (PSTR ("Tx task sent data!\r\n"));
    nrk_led_clr (RED_LED);
    nrk_wait_until_next_period ();
   }

}

void oled_clear_screen()
{
   putc0(0x45);
}

void oled_write_str( uint8_t x, uint8_t y, uint8_t font, uint16_t color, char *str )
{
uint8_t i;

putc0(0x73);
putc0(x);	
putc0(y);	
putc0(font);	
putc0(((color>>8)&0xFF));	
putc0((color&0xFF));
printf( "led write: " );
for(i=0; i<strlen(str); i++ )
{
	  putc0(str[i]);
	  printf( "%c",str[i] );
}
printf( "\r\n" );
putc0(0x00);	
}


void oled_draw_square( uint8_t x, uint8_t y, uint8_t x2, uint8_t y2, uint16_t color  )
{
uint8_t i;

putc0(0x72);
putc0(x);	
putc0(y);	
putc0(x2);	
putc0(y2);	
putc0(((color>>8)&0xFF));	
putc0((color&0xFF));
printf( "led write: " );
//putc0(0x00);	
}


char txt_str[32];

void Task4()
{
uint16_t cnt;
uint8_t v;
uint8_t row;

  printf( "Task4 PID=%u\r\n",nrk_get_pid());
  oled_on();

  cnt=0;
  row=0;
  while(1) {
//	v=hrm_get_value();
//	sprintf(txt_str,"%d%d%d cnt=%d hrm=%d",nrk_gpio_get(NRK_MMC_9), nrk_gpio_get(NRK_MMC_10), nrk_gpio_get(NRK_MMC_11), cnt,v);
	oled_clear_screen();
	nrk_wait_until_ticks(50);
	sprintf(txt_str,"Neighbor List:");
	oled_write_str( 2, 3, 0, 0xFFFF, txt_str );
	nrk_wait_until_ticks(50);
	sprintf(txt_str,"   0x100002a  -27 4");
	oled_write_str( 2, 4, 0, 0xFFFF, txt_str );
	nrk_wait_until_ticks(50);
	sprintf(txt_str,"   0x1000012: -30 2");
	oled_write_str( 2, 5, 0, 0xFFFF, txt_str );
	nrk_wait_until_ticks(50);
	sprintf(txt_str,"   0x100000a: -5 3");
	oled_write_str( 2, 6, 0, 0xFFFF, txt_str );
	nrk_wait_until_ticks(50);
	sprintf(txt_str,"Round Trip Pkt:  93%%");
	oled_write_str( 2, 9, 0, 0xFFFF, txt_str );
	nrk_wait_until_ticks(50);
	//putc0(0x70);	
	//putc0(0x01);	
	//nrk_wait_until_ticks(50);
	oled_draw_square( 8, 83, 150, 92, 0x001F  );
	nrk_wait_until_ticks(50);
	//putc0(0x70);	
	//putc0(0x00);	
	//nrk_wait_until_ticks(50);
	oled_draw_square( 10, 85, 100, 90, 0x07e0  );
	nrk_wait_until_ticks(50);
        cnt++;
	row++;
	if(row==15) {
	//	oled_clear_screen();
		row=0;
		}
	nrk_wait_until_next_period();
	}
}





void
nrk_create_taskset()
{
int8_t val;

  nrk_task_set_entry_function( &TaskOne, Task1);
  nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
  TaskOne.prio = 1;
  TaskOne.FirstActivation = TRUE;
  TaskOne.Type = BASIC_TASK;
  TaskOne.SchType = PREEMPTIVE;
  TaskOne.period.secs = 1;
  TaskOne.period.nano_secs = 0*NANOS_PER_MS;
  TaskOne.cpu_reserve.secs = 0;
  TaskOne.cpu_reserve.nano_secs =  50*NANOS_PER_MS;
  TaskOne.offset.secs = 0;
  TaskOne.offset.nano_secs= 0;
  nrk_activate_task (&TaskOne);

  nrk_task_set_entry_function( &TaskTwo, Task2);
  nrk_task_set_stk( &TaskTwo, Stack2, NRK_APP_STACKSIZE);
  TaskTwo.prio = 2;
  TaskTwo.FirstActivation = TRUE;
  TaskTwo.Type = BASIC_TASK;
  TaskTwo.SchType = PREEMPTIVE;
  TaskTwo.period.secs = 10;
  TaskTwo.period.nano_secs = 500*NANOS_PER_MS;
  TaskTwo.cpu_reserve.secs = 10;
  TaskTwo.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskTwo.offset.secs = 0;
  TaskTwo.offset.nano_secs= 0;
  nrk_activate_task (&TaskTwo);


  nrk_task_set_entry_function( &TaskThree, Task3);
  nrk_task_set_stk( &TaskThree, Stack3, NRK_APP_STACKSIZE);
  TaskThree.prio = 2;
  TaskThree.FirstActivation = TRUE;
  TaskThree.Type = BASIC_TASK;
  TaskThree.SchType = PREEMPTIVE;
  TaskThree.period.secs = 10;
  TaskThree.period.nano_secs = 0;
  TaskThree.cpu_reserve.secs = 0;
  TaskThree.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskThree.offset.secs = 0;
  TaskThree.offset.nano_secs= 0;
  nrk_activate_task (&TaskThree);


  nrk_task_set_entry_function( &TaskFour, Task4);
  nrk_task_set_stk( &TaskFour, Stack4, NRK_APP_STACKSIZE);
  TaskFour.prio = 2;
  TaskFour.FirstActivation = TRUE;
  TaskFour.Type = BASIC_TASK;
  TaskFour.SchType = PREEMPTIVE;
  TaskFour.period.secs = 2;
  TaskFour.period.nano_secs = 0;
  TaskFour.cpu_reserve.secs = 1;
  TaskFour.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
  TaskFour.offset.secs = 0;
  TaskFour.offset.nano_secs= 0;
  nrk_activate_task (&TaskFour);

  val=nrk_register_driver( &dev_manager_ff_sensors,FIREFLY_SENSOR_BASIC);
  if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to load my ADC driver\r\n") );

}


