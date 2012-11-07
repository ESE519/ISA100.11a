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
#include <avr/sleep.h>
#include <hal.h>
#include <bmac.h>
#include <nrk_error.h>
#include "../include/sampl.h"
#include "../include/pkt_packer.h"

#include "readflash.h"
#include "update.h"

#define REPLY_WAIT_SECS	0
#define REPLY_WAIT_NANOSECS 400 * NANOS_PER_MS

#define PING_WAIT_SECS	3

#define my_subnet_mac	0	
#define my_mac  0


#define TXT_DEBUG


//***************************************************

#define TRUE            1
#define FALSE           0

//Phoenix packet structures
#define DATA_PAYLOAD    64
#define CMD_PAYLOAD     PKT_TYPE + 7

#define INIT_MSG        'I'
#define DATA_MSG        'D'
#define NACK_MSG        'N'

#define ABORT_MSG       'A'
#define SUCCESS_MSG     'S'

// Program states
#define PING    0
#define UPDATE  1
#define NONE    2

// PKT_TYPE defined in ../include/SAMPL.h

#define MSG_TYPE         PKT_TYPE + 1
#define UP_PAGES         PKT_TYPE + 2
#define UP_LESSB         PKT_TYPE + 3
#define UP_MODE          PKT_TYPE + 4
#define UP_VER           PKT_TYPE + 5
#define UP_CSUM          PKT_TYPE + 6

#define PG_NUM           PKT_TYPE + 2
#define PG_OFF           PKT_TYPE + 3
#define DATA_HEAD        PKT_TYPE + 4

//***Phoenix Variables***
uint8_t cnt;
int8_t len;
int8_t rssi, val;
uint8_t *local_rx_buf;
uint8_t charCount;
nrk_time_t start, current;

uint8_t programState;

// Setup uart
char c;
nrk_sig_t uart_rx_signal;
nrk_sig_mask_t sm;


// tx task signals
nrk_sig_t tx_done_signal;
nrk_sig_t rx_signal;
nrk_time_t check_period;
nrk_time_t timeout;
nrk_sig_mask_t my_sigs;

// Mac addr vars
char mac_addr[8];
char buffer0[3];
char buffer1[3];
char buffer2[3];
char buffer3[3];

uint8_t dest_mac[4];

// Global state for TX protocol
enum state { START, INIT, DATA, STOP } txState;
// Current tx status
uint8_t pgOffset;
uint8_t pgNumber;
uint8_t needReply;

// Buffer for flash pages
uint8_t ph_buf[PAGESIZE];

//***Function declarations***

void buildTxPkt (void);
void changeState (void);
void initStatus();
void startStatus();
void pingReplyRecieved();
void updateReplyRecieved();
void updateMode();
void pingMode();
void getDestMac();
void phoenix_init();
//*****************************************************


nrk_task_type MOBILE_TASK;
NRK_STK mobile_task_stack[NRK_APP_STACKSIZE];
void tx_task (void);

void nrk_create_taskset ();


uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t aes_key[16]={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee, 0xff};


SAMPL_PEER_2_PEER_PKT_T p2p_pkt;

int main ()
{

  // init phoenix
  phoenix_init();
  
  nrk_setup_ports ();
  nrk_setup_uart (UART_BAUDRATE_115K2);

  nrk_init ();

  nrk_led_clr (0);
  nrk_led_clr (1);
  nrk_led_clr (2);
  nrk_led_clr (3);

  nrk_time_set (0, 0);

  bmac_task_config ();

  nrk_create_taskset ();
  nrk_start ();

  return 0;
}

void tx_task ()
{
  // Get the signal for UART RX
  //uart_rx_signal=nrk_uart_rx_signal_get();
  // Register your task to wakeup on RX Data 
  //if(uart_rx_signal==NRK_ERROR) nrk_kprintf( PSTR("Get Signal ERROR!\r\n") );
  //nrk_signal_register(uart_rx_signal);

  //printf ("tx_task PID=%d\r\n", nrk_get_pid ());

  // Wait until the tx_task starts up bmac
  // This should be called by all tasks using bmac that
  // do not call bmac_init()...
  bmac_init (26);

  bmac_encryption_set_key(aes_key,16);
  bmac_encryption_enable();

  bmac_rx_pkt_set_buffer (rx_buf, RF_MAX_PAYLOAD_SIZE);

  //nrk_kprintf (PSTR ("bmac_started()\r\n"));
  bmac_set_cca_thresh (-45);

  check_period.secs = 0;
  check_period.nano_secs = 100 * NANOS_PER_MS;
  val = bmac_set_rx_check_rate (check_period);

  // Get and register the tx_done_signal if you want to
  // do non-blocking transmits
  tx_done_signal = bmac_get_tx_done_signal ();
  nrk_signal_register (tx_done_signal);

  rx_signal = bmac_get_rx_pkt_signal ();
  nrk_signal_register (rx_signal);

  cnt = 0;

  while (1)
  {
      nrk_kprintf(PSTR("\r\n*************************************************************\r\n"));
      nrk_kprintf(PSTR("               PHOENIX WIRELESS UPDATE SYSTEM                \r\n"));
      nrk_kprintf(PSTR("*************************************************************\r\n"));
      nrk_kprintf(PSTR("Press 'p' : To PING Nodes in Vicinity                        \r\n"));
      nrk_kprintf(PSTR("Press 'u' : To Begin Node Update                             \r\n"));
      nrk_kprintf(PSTR("                                                             \r\n"));
      nrk_kprintf(PSTR("*************************************************************\r\n"));

      printf("Enter Choice: ");
      
      //sm=nrk_event_wait(SIG(uart_rx_signal));
    
      //if(sm != SIG(uart_rx_signal))
      //{
      //  nrk_kprintf( PSTR("UART signal error\r\n") );
      //  while(1);
      //}
      // Wait for UART signal
      while(1)
      {
        if(nrk_uart_data_ready(NRK_DEFAULT_UART)!=0)
        {
          // Read Character
          c=getchar();
          printf( "%c\r\n",c);
          break;
        }
        timeout.secs = 0;
        timeout.nano_secs = 20 * NANOS_PER_MS;
        nrk_wait(timeout);
      }
      // Choose mode
      switch(c){
        case 'p':
          programState = PING;
          break;
        case 'u':
          getDestMac();
          phoenix_init();
          programState = UPDATE;
          break;
        default:
          programState = NONE;
          nrk_kprintf(PSTR("Invalid Command! Please Try Again\r\n"));
      }

      // Reset c
      c = 0;

      nrk_wait_until_next_period();

      // Execute protocol
      switch(programState)
      {
        case PING:
          pingMode();
          break;
        case UPDATE:
          updateMode();
          break;
        case NONE:;// Do nothing
          break;
        default:
          nrk_kprintf(PSTR("Invalid Program State\r\n"));
          break;
      }
      nrk_wait_until_next_period ();
  }
}

void phoenix_init()
{
  //***************************************
  // Init Phoenix variables
  programState = PING;
  txState = START;
  // Initialize tx status
  pgOffset = 0;
  pgNumber = 0;
  needReply = FALSE;

  charCount = 0;
  //***************************************
}

//*************************PING MODE************************************************
void getDestMac()
{

  mac_addr[0] = '\0';
  
  nrk_kprintf(PSTR("\r\n*************************************************************\r\n"));
  nrk_kprintf(PSTR("         Please Enter Address of Node to Program             \r\n"));
  nrk_kprintf(PSTR("     MAC (Subnet + Dest) 4 Bytes Hex E.g. 00000004           \r\n"));
  nrk_kprintf(PSTR("*************************************************************\r\n"));
  printf("Enter MAC:");

  charCount = 0;

  // Enter Mac
  do
  {
    // Wait for UART signal
    while(1)
    {
      if(nrk_uart_data_ready(NRK_DEFAULT_UART)!=0)
      {
        while(nrk_uart_data_ready(NRK_DEFAULT_UART)!=0)
        {
          charCount++;
          // Read Character
          c=getchar();
          printf( "%c",c);
          sprintf(mac_addr,"%s%c",mac_addr,c);
        }
        break;
      }
      timeout.secs = 0;
      timeout.nano_secs = 20 * NANOS_PER_MS;
      nrk_wait(timeout);
    }
  }while(charCount < 8);

  
  buffer0[0]=mac_addr[0]; buffer0[1]=mac_addr[1]; buffer0[3]='\0';
  buffer1[0]=mac_addr[2]; buffer1[1]=mac_addr[3]; buffer1[3]='\0';
  buffer2[0]=mac_addr[4]; buffer2[1]=mac_addr[5]; buffer2[3]='\0';
  buffer3[0]=mac_addr[6]; buffer3[1]=mac_addr[7]; buffer3[3]='\0';
  val=sscanf( buffer0,"%x",&dest_mac[3]);
  val+=sscanf( buffer1,"%x",&dest_mac[2]);
  val+=sscanf( buffer2,"%x",&dest_mac[1]);
  val+=sscanf( buffer3,"%x",&dest_mac[0]);


  nrk_kprintf(PSTR("\r\n*************************************************************\r\n"));
  nrk_kprintf(PSTR("                       Node MAC Accepted                     \r\n"));
            printf("       Initiating Update for Node 0 x %X %X %X %X        \r\n", dest_mac[3],dest_mac[2],dest_mac[1], dest_mac[0]);
  nrk_kprintf(PSTR("*************************************************************\r\n"));

}


void pingMode()
{

  // Broadcast for ping
  val=bmac_addr_decode_set_my_mac(((uint16_t)my_subnet_mac<<8)|my_mac);
  val=bmac_addr_decode_dest_mac((uint16_t)0xffff);  // broadcast by default
  bmac_addr_decode_enable();
  
  uint8_t i;
  
  // Build a TX packet by hand...
  p2p_pkt.pkt_type = PING_PKT;
    // set as p2p packet (no US_MASK or DS_MASK)
  p2p_pkt.ctrl_flags = MOBILE_MASK; // | DEBUG_FLAG ;
  p2p_pkt.ack_retry= 0x00;
  p2p_pkt.ttl = 1;
  p2p_pkt.subnet_mac[0] = 1;
  p2p_pkt.subnet_mac[1] = 2;
  p2p_pkt.subnet_mac[2] = 3;
  p2p_pkt.src_mac = my_mac;
  p2p_pkt.last_hop_mac = my_mac;
  p2p_pkt.dst_mac = BROADCAST;
  p2p_pkt.buf=tx_buf;
  p2p_pkt.buf_len = P2P_PAYLOAD_START;
  p2p_pkt.seq_num = cnt;
  p2p_pkt.priority = 0;

//    p2p_pkt.payload[0]=my_mac;
//    p2p_pkt.payload_len=1;
//    ping_p2p_generate(&p2p_pkt);

  nrk_led_set (BLUE_LED);

  check_period.secs = 0;
  check_period.nano_secs = 100 * NANOS_PER_MS;
  val = bmac_set_rx_check_rate (check_period);

    // Pack data structure values in buffer before transmit
  pack_peer_2_peer_packet(&p2p_pkt);
    // For blocking transmits, use the following function call.
  val = bmac_tx_pkt (p2p_pkt.buf, p2p_pkt.buf_len);

  check_period.secs = 0;
  check_period.nano_secs = FAST_CHECK_RATE * NANOS_PER_MS;
  val = bmac_set_rx_check_rate (check_period);
#ifdef TXT_DEBUG
  nrk_kprintf (PSTR ("\r\nPING Packet Sent. Waiting for Replies...\r\n\n"));
#endif
  nrk_led_clr (BLUE_LED);
  nrk_led_clr(GREEN_LED);

    // Wait for packets or timeout
  nrk_time_get (&start);
    
  while (1) {

    timeout.secs = PING_WAIT_SECS;
    timeout.nano_secs = 0;

      // Wait until an RX packet is received
      //val = bmac_wait_until_rx_pkt ();
    nrk_set_next_wakeup (timeout);
    my_sigs = nrk_event_wait (SIG (rx_signal) | SIG (nrk_wakeup_signal));


    if (my_sigs == 0)
      nrk_kprintf (PSTR ("Error calling nrk_event_wait()\r\n"));
    if (my_sigs & SIG (rx_signal)) {	
        // Get the RX packet 
      local_rx_buf = bmac_rx_pkt_get (&len, &rssi);
	// Check the packet type from raw buffer before unpacking
      if ((local_rx_buf[CTRL_FLAGS] & (DS_MASK | US_MASK)) == 0) {
	// Set the buffer
        p2p_pkt.buf=local_rx_buf;
        p2p_pkt.buf_len=len;
        p2p_pkt.rssi=rssi;
        unpack_peer_2_peer_packet(&p2p_pkt);
        if (p2p_pkt.dst_mac == my_mac || p2p_pkt.dst_mac == BROADCAST) {
          nrk_led_set(GREEN_LED);
          // Packet arrived and is good to go
          printf( "Mac: %x%x%x",p2p_pkt.subnet_mac[0], p2p_pkt.subnet_mac[1], p2p_pkt.subnet_mac[2]);
          printf( "%x ",p2p_pkt.src_mac);
          printf( "| RSSI: %d ",p2p_pkt.rssi);
          printf( "| Type: %d \r\n",p2p_pkt.pkt_type);
        }
      }
      // Release the RX buffer so future packets can arrive
      bmac_rx_pkt_release ();
    }
    nrk_time_get (&current);
    if (start.secs + PING_WAIT_SECS < current.secs)
      break;
  }
  nrk_kprintf (PSTR ("\r\nDone Waiting for Response...\r\n"));
  nrk_led_clr(GREEN_LED);
}

//**************************UPDATE MODE***********************************************************

// Run update Mode Protocol
void updateMode()
{
  // Set destination
  val=bmac_addr_decode_set_my_mac(((uint16_t)my_subnet_mac<<8)|my_mac);
  val=bmac_addr_decode_dest_mac(((uint16_t)dest_mac[1]<<8)|dest_mac[0]);
  bmac_addr_decode_enable();
  
  while(programState == UPDATE)
  {
    // Build tx packet by txState
    buildTxPkt();
    
    nrk_led_toggle (BLUE_LED);

    check_period.secs = 0;
    check_period.nano_secs = 100 * NANOS_PER_MS;
    val = bmac_set_rx_check_rate (check_period);

    // Send with hardware acknowledgement.
    while(1)
    {
      val = bmac_tx_pkt (tx_buf, len);
      // Break if message ack recieved or is a broadcast
      if(val == NRK_OK){
        nrk_kprintf(PSTR("Packet Sent\r\n"));
        break;
      }
      else{
        nrk_kprintf(PSTR("Packet ReSent\r\n"));
      }
      // Resend if ack not recieved
      nrk_wait_until_next_period();
    }

  #ifdef TXT_DEBUG
      nrk_kprintf (PSTR ("\r\nSent Request:\r\n"));
  #endif
    nrk_led_clr (BLUE_LED);
    nrk_led_clr(GREEN_LED);

      // Wait if reply expected / Change TX state if not
      //***********************************************************
    if(needReply == FALSE)
    {
        // Change TX state, update page number
      changeState();
    }
    else
    {
        // Wait for packets or timeout
  #ifdef TXT_DEBUG
        nrk_kprintf (PSTR ("\r\nExpecting Reply\r\n"));
  #endif
      timeout.secs = REPLY_WAIT_SECS;
      timeout.nano_secs = REPLY_WAIT_NANOSECS;

      nrk_set_next_wakeup (timeout);
      my_sigs = nrk_event_wait (SIG (rx_signal) | SIG (nrk_wakeup_signal));

      if (my_sigs == 0)
        nrk_kprintf (PSTR ("Error calling nrk_event_wait()\r\n"));
      if (my_sigs & SIG (rx_signal))
      {
            // Get the RX packet
        local_rx_buf = bmac_rx_pkt_get (&len, &rssi);
        nrk_led_clr (ORANGE_LED);
  #ifdef TXT_DEBUG
        nrk_kprintf (PSTR ("Recieved Reply\r\n"));
  #endif
        // Handle recieved packet
        updateReplyRecieved();
      }
      else
      {
  #ifdef TXT_DEBUG
        nrk_kprintf (PSTR ("Timed Out Waiting for response...\r\n"));
  #endif
      }
    }
    nrk_wait_until_next_period();
  }
}


// Update Protocol reply recieved
void updateReplyRecieved()
{
  // Check if INIT Success
  // Release the RX buffer inside
  if (txState == INIT){
    initStatus();
  }
  else if (txState == START){
    startStatus();
  }
  else
    bmac_rx_pkt_release ();
}


// Check if init succeeded
void initStatus()
{
  if((local_rx_buf[PKT_TYPE] == UNKNOWN_PKT) &&
      (local_rx_buf[MSG_TYPE] == SUCCESS_MSG))
  {
    // Release the RX buffer so future packets can arrive
    bmac_rx_pkt_release ();
    nrk_kprintf (PSTR ("Update Checksum Succeeded\r\n"));
    // Change TX state, update page number
    changeState();
  }
  else
  {
    bmac_rx_pkt_release ();
    nrk_kprintf (PSTR ("Update Checksum Failed\r\n"));
    programState = PING;
  }
}

// Check if start succeeded
void startStatus()
{
  if((local_rx_buf[PKT_TYPE] == UNKNOWN_PKT) &&
      (local_rx_buf[MSG_TYPE] == SUCCESS_MSG))
  {
    // Release the RX buffer so future packets can arrive
    bmac_rx_pkt_release ();
   // Change TX state, update page number
    changeState();
  }
  else
  {
    bmac_rx_pkt_release ();
  }
}

// Build TX pkt by txState
void buildTxPkt()
{

  uint32_t addr;
  uint8_t i,j;
  
  // TX protocol
  switch(txState)
  {
    case START:
        // Send wireless update pkt
        nrk_kprintf (PSTR ("Build Start Packet\r\n"));
        // Build a TX packet by hand...
        p2p_pkt.pkt_type = WIRELESS_UPDATE_PKT;
        p2p_pkt.ctrl_flags = ENCRYPT;  // set as p2p packet (no US_MASK or DS_MASK)
        p2p_pkt.ack_retry= 0x00;
        p2p_pkt.ttl = 1;
        p2p_pkt.check_rate = DEFAULT_CHECK_RATE;
        // FIXME: add proper subnet later
        p2p_pkt.subnet_mac[0] = dest_mac[1];
        p2p_pkt.subnet_mac[1] = dest_mac[2];
        p2p_pkt.subnet_mac[2] = dest_mac[3];
        p2p_pkt.src_mac = my_mac;
        p2p_pkt.last_hop_mac = my_mac;
        p2p_pkt.dst_mac = dest_mac[0];
        p2p_pkt.buf=tx_buf;
        p2p_pkt.buf_len = P2P_PAYLOAD_START;
        p2p_pkt.seq_num = cnt;
        p2p_pkt.priority = 0;
        // p2p_pkt.payload[0]=my_mac;
        // p2p_pkt.payload_len=1;
        // ping_p2p_generate(&p2p_pkt);
        needReply = TRUE;

        // Pack data structure values in buffer before transmit
        pack_peer_2_peer_packet(&p2p_pkt);

        // Copy in tx buffers
        len = p2p_pkt.buf_len;
        for(i=0;i<len;i++)
          tx_buf[i] = p2p_pkt.buf[i];
        break;
      
    case INIT:
        nrk_kprintf (PSTR ("Build Init Packet\r\n"));
        // Send update tx info, checks
        // PKT_TYPE = 0
        tx_buf[PKT_TYPE] = UNKNOWN_PKT;
        tx_buf[MSG_TYPE] = INIT_MSG;
        tx_buf[UP_PAGES] = UpdatePages;
        tx_buf[UP_LESSB] = UpdateLessBytes;
        tx_buf[UP_MODE]  = UpdateMode;
        tx_buf[UP_VER]   = UpdateVersion;
        tx_buf[UP_CSUM]  = UpdateChecksum;
        len = CMD_PAYLOAD;
        // Need a checksum success reply
        needReply = TRUE;
        break;

    case DATA:
      	nrk_kprintf (PSTR ("Build Data Packet\r\n"));
        // Send image binary
        // PKT_TYPE = 0
        tx_buf[PKT_TYPE] = UNKNOWN_PKT;
        tx_buf[MSG_TYPE] = DATA_MSG;
        tx_buf[PG_NUM] = pgNumber;
        tx_buf[PG_OFF] = pgOffset;
        // Read page from flash
        if(pgOffset == 0){
          addr = ((uint32_t)pgNumber*(uint32_t)PAGESIZE) + (uint32_t)UPDATE_SECTION;
          ws_flash_read_page(addr,ph_buf);
        }
        // Add data to pkt
        for(i = 0, j = 0; i < DATA_PAYLOAD; i++, j++){
          tx_buf[j + DATA_HEAD] = ph_buf[i + (pgOffset * DATA_PAYLOAD)];
        }
        len = DATA_PAYLOAD + DATA_HEAD;
        needReply = FALSE;
        break;
        
    case STOP:
        // Send stop and reboot cmd
        nrk_kprintf(PSTR("Reboot\r\n"));
        tx_buf[PKT_TYPE] = UNKNOWN_PKT;
        tx_buf[MSG_TYPE] = NACK_MSG;
        len = CMD_PAYLOAD;
        needReply = FALSE;
        break;
        
    default:
        nrk_kprintf(PSTR("Error: Invalid Build Pkt State"));
        break;
  }

}


// Change TX state, update page number
void changeState ()
{
  
  switch(txState)
  {
    case DATA:
          #ifdef TXT_DEBUG
            nrk_kprintf (PSTR ("\r\nState DATA\r\n"));
          #endif
          // Increment page to send
          #ifdef TXT_DEBUG
            printf("Page Offset:%d, Page Number:%d\r\n", pgOffset, pgNumber);
          #endif
          nrk_led_toggle(GREEN_LED);
          if (pgOffset == 3){
            pgOffset = 0;
            pgNumber ++;
          }
          else{
            pgOffset++;
          }
          // Check for TX completion
          if (pgNumber > UpdatePages){
            txState = STOP;
            nrk_led_clr(ORANGE_LED);
            nrk_led_clr(GREEN_LED);
            nrk_led_clr(RED_LED);
            #ifdef TXT_DEBUG
              nrk_kprintf(PSTR("Transmission Complete\r\n"));
            #endif
          }
          else{
            txState = DATA;
          }
          break;

    case START:
          #ifdef TXT_DEBUG
            nrk_kprintf (PSTR ("\r\nState START\r\n"));
          #endif
          if(my_mac != BROADCAST && my_subnet_mac != BROADCAST){
            bmac_auto_ack_enable();
            nrk_kprintf(PSTR("Auto ack enable\r\n"));
          }
          txState = INIT;
          break;

    case INIT:
          #ifdef TXT_DEBUG
            nrk_kprintf (PSTR ("\r\nState INIT\r\n"));
          #endif
          txState = DATA;
          break;

    case STOP:
          #ifdef TXT_DEBUG
            nrk_kprintf (PSTR ("\r\nState STOP\r\n"));
          #endif
          // Update complete go to ping mode again
          programState = PING;
          break;
    default:
          nrk_kprintf(PSTR("Error: Invalid Tx State\r\n"));
          break;
  }
 
}

//**************************UPDATE MODE ENDS***********************************************************

void nrk_create_taskset ()
{
  MOBILE_TASK.task = tx_task;
  nrk_task_set_stk (&MOBILE_TASK, mobile_task_stack, NRK_APP_STACKSIZE);
  MOBILE_TASK.prio = 2;
  MOBILE_TASK.FirstActivation = TRUE;
  MOBILE_TASK.Type = BASIC_TASK;
  MOBILE_TASK.SchType = PREEMPTIVE;
  MOBILE_TASK.period.secs = 0;
  MOBILE_TASK.period.nano_secs = 200 * NANOS_PER_MS;
  MOBILE_TASK.cpu_reserve.secs = 0;
  MOBILE_TASK.cpu_reserve.nano_secs = 200 * NANOS_PER_MS;
  MOBILE_TASK.offset.secs = 0;
  MOBILE_TASK.offset.nano_secs = 0;
  nrk_activate_task (&MOBILE_TASK);

}
