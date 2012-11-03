#include <nrk.h>
#include <bmac.h>
#include <nrk_eeprom.h>
#include <nrk_error.h>
#include "phoenix.h"
#include "globals.h"
#include "bootloader.h"

#ifdef PHOENIX

#define PH_TXT_DEBUG

#define ABORT            1
#define SUCCESS          0

#define TRUE             1
#define FALSE            0

#define INIT_MSG        'I'
#define DATA_MSG        'D'
#define NACK_MSG        'N'

#define ABORT_MSG       'A'
#define SUCCESS_MSG     'S'

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


static int8_t v, val;
static uint8_t rssi,len,i;
static uint8_t *local_rx_buf;
static nrk_time_t cr;

static uint8_t UpdatePages = 255;// Invalid Large Value
static uint8_t UpdateLessBytes;
static uint8_t UpdateMode;
static uint8_t UpdateVersion;
static uint8_t UpdateChecksum;

static uint8_t CurrentImageChecksum;

// Get from EEPROM
static uint8_t LoadPages;

static uint8_t needReply;
// Current rx status
static uint8_t pgNumber = 0;
static uint8_t pgOffset = 0;

extern void NanoPatch(uint16_t ee_update_section_byte_size, uint16_t ee_load_section_byte_size, uint8_t newChecksum);

uint8_t msgHandler(void);

void phoenix_wireless_update ()
{

  nrk_kprintf (PSTR ("Phoenix Wireless Update Active...\r\n"));
  nrk_led_set(RED_LED);
  // Once you get the packet, you still need to release it before
  // you start your work...
  bmac_rx_pkt_release();

  // For some reason switching channels causes node crash, so lets ignore that for now...
  // As long as the pkt type which is the first byte in the packet is unkown, things should be
  // okay.  So we can make sure your header is an invalid flash flood header
  // Switch channels
  // nrk_kprintf( PSTR("Channel\r\n" ));
  // v = bmac_set_channel (16);
  
  // Init reply to false
  needReply = FALSE;
  
  cr.secs = 0;
  cr.nano_secs = FAST_CHECK_RATE * NANOS_PER_MS;
  v = bmac_set_rx_check_rate (cr);
  // rx_buf is a global buffer defined in globals.h
  // This buffer has already been set as the rx_buffer
  val=bmac_addr_decode_set_my_mac(((uint16_t)my_subnet_mac[0]<<8)|my_mac);
  val=bmac_addr_decode_dest_mac((uint16_t)0xffff);  // broadcast by default
  bmac_addr_decode_enable();
      
  nrk_kprintf( PSTR("Buffer\r\n" ));
  bmac_rx_pkt_set_buffer (rx_buf, RF_MAX_PAYLOAD_SIZE);

  nrk_kprintf( PSTR("Loop\r\n" ));

  // Enable Hardware Acknowledgements
  bmac_auto_ack_enable();
  
  while (1) {

    // Wait until an RX packet is received
    v = bmac_wait_until_rx_pkt ();

    nrk_kprintf( PSTR("Pkt received\r\n" ));
    // Get the RX packet 
    nrk_led_set (ORANGE_LED);
    local_rx_buf = bmac_rx_pkt_get (&len, &rssi);

    nrk_led_clr (ORANGE_LED);

    // Handle message
    if(msgHandler() == ABORT){
      #ifdef PH_TXT_DEBUG
         nrk_kprintf(PSTR("\r\nChecksum Failed: Update Aborted\r\n"));
      #endif
      // Return if update is aborted
      return;
    }

    // Release the RX buffer so future packets can arrive 
    bmac_rx_pkt_release ();

    // Send a reply packet if needed
    if(needReply == TRUE){
        nrk_kprintf( PSTR( "Reply Sent\r\n" ));
        v=bmac_tx_pkt(tx_buf, len);
        nrk_led_toggle(BLUE_LED);
    }
        
  }

// reboot, don't ever return except on update abort
}


// Handle message
uint8_t msgHandler()
{
  uint32_t addr;
  nrk_kprintf( PSTR( "MSG_HANDLER\r\n" ));

  // Acknowledge the wireless update pkt again
  if(local_rx_buf[PKT_TYPE] == WIRELESS_UPDATE_PKT){
    nrk_kprintf( PSTR( "WIRELESS_UPDATE_PKT\r\n" ));
    tx_buf[PKT_TYPE] = UNKNOWN_PKT;
    tx_buf[MSG_TYPE] = SUCCESS_MSG;
    len = PKT_TYPE + 2;
    needReply = TRUE;
  }
  else if (local_rx_buf[PKT_TYPE] == UNKNOWN_PKT  ){
    switch (local_rx_buf[MSG_TYPE])
    {
      case INIT_MSG: 
        needReply = TRUE;
        nrk_kprintf( PSTR( "INIT_MSG\r\n" ));
        // Read load section size from EEPROM
        val = read_eeprom_current_image_checksum(&CurrentImageChecksum);
        #ifdef PH_TXT_DEBUG
          if(val == NRK_OK)
            nrk_kprintf(PSTR("Read EE Checksum Success\r\n"));
        #endif
        UpdateChecksum  = rx_buf[UP_CSUM];
        //Verify checksum, abort/return if fail
        if(UpdateChecksum != CurrentImageChecksum)
        {
          // Reply with abort message
          tx_buf[PKT_TYPE] = UNKNOWN_PKT;
          tx_buf[MSG_TYPE] = ABORT_MSG;
          len = PKT_TYPE + 2;
          return ABORT;
        }
        else
        {
          // Reply with success message
          tx_buf[PKT_TYPE] = UNKNOWN_PKT;
          tx_buf[MSG_TYPE] = SUCCESS_MSG;
          len = PKT_TYPE + 2;
        } 
        // Store update specifications
        UpdatePages     = rx_buf[UP_PAGES];
        UpdateLessBytes = rx_buf[UP_LESSB];
        UpdateMode      = rx_buf[UP_MODE];
        UpdateVersion   = rx_buf[UP_VER];
        break;

      case DATA_MSG:
        nrk_kprintf( PSTR( "DATA_MSG\r\n" ));
        #ifdef PH_TXT_DEBUG
          printf(" %d = %d, %d = %d\r\n", local_rx_buf[PG_NUM], pgNumber,local_rx_buf[PG_OFF], pgOffset);
        #endif
        if( (local_rx_buf[PG_NUM] == pgNumber) && (local_rx_buf[PG_OFF] == pgOffset) )
        {
          // Store binary in buffer/flash
          add_packet_to_page(&local_rx_buf[DATA_HEAD], pgOffset * DATA_PAYLOAD, ph_buf);
          if(pgOffset == 3)
          {
            pgOffset = 0;
            addr = ((uint32_t)pgNumber*(uint32_t)PAGESIZE)+(uint32_t)UPDATE_SECTION;
            #ifdef PH_TXT_DEBUG
              printf("Commit page at: %lu\r\n", addr);
            #endif
            commit_page(addr, ph_buf);
            pgNumber++;
          }
          else{
            pgOffset++;
          }
        }
        else
          nrk_kprintf(PSTR("Error: Page Number/Page Offset Mismatch\r\n"));
        needReply = FALSE;
        break;

      case NACK_MSG:
        nrk_kprintf( PSTR( "NACK_MSG\r\n" ));
        if(pgNumber >= UpdatePages)
        {
          nrk_kprintf(PSTR("PROGRAMMING COMPLETE\r\n"));
          /* UpdateMode can be used to do a patch/full image update */
          nrk_led_set (ORANGE_LED);
          nrk_led_set (GREEN_LED);
          nrk_led_set (RED_LED);
          nrk_led_set (BLUE_LED);

          // Read load section size from EEPROM
          val = read_eeprom_load_img_pages(&LoadPages);
          #ifdef PH_TXT_DEBUG
            if(val == NRK_OK)
              nrk_kprintf(PSTR("Read EE Load Pages Success\r\n"));
          #endif
            
          // Apply acquired patch
          printf("LoadPages: %X\r\n", LoadPages);
          printf("UpdatePages: %X\r\n", UpdatePages);
          printf("ImgSize: %X\r\n", (uint16_t)LoadPages * (uint16_t)PAGESIZE);
          printf("UpdSize: %X\r\n", ((uint16_t)UpdatePages * (uint16_t)PAGESIZE) - UpdateLessBytes);
            
          NanoPatch( ((uint16_t)UpdatePages * (uint16_t)PAGESIZE) - UpdateLessBytes,
                      (uint16_t)LoadPages * (uint16_t)PAGESIZE,
                       UpdateChecksum
                  );
        }
        else
          nrk_kprintf(PSTR("Wireless Update Error\r\n"));
        
        needReply = FALSE;
        break;
      default:
        nrk_kprintf(PSTR("Error: Invalid Phoenix Msg Type"));
        needReply = FALSE;
        break;
    }
  }
  else
  {
    #ifdef TXT_DEBUG
      nrk_kprintf (PSTR ("Error: Packet Not Valid\r\n"));
    #endif
  }
  return SUCCESS;
}

#else

void phoenix_wireless_update ()
{
  nrk_kprintf (PSTR ("Phoenix Wireless Update NOT Supported...\r\n"));
}
#endif
