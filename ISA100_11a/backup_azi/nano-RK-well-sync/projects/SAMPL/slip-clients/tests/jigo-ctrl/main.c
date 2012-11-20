#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "../../../include/sampl.h"
#include <slipstream.h>
#include <ack_pkt.h>
#include <ff_basic_sensor_pkt.h>
#include <transducer_pkt.h>
#include <transducer_registry.h>


//#include "tree_route.h"
//#include "slipstream.h"

#define gw_mac		0

uint8_t debug_txt_flag;

#define NONBLOCKING  0
#define BLOCKING     1

#define HEX_STR_SIZE	5

void handle_incomming_pkt(uint8_t *rx_buf,uint8_t len);
void error(char *msg);
void print_ds_packet(SAMPL_DOWNSTREAM_PKT_T *ds_pkt );
void print_gw_packet(SAMPL_GATEWAY_PKT_T *gw_pkt );

  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  int size;
  char buffer[2048];

SAMPL_DOWNSTREAM_PKT_T ds_pkt;

int main (int argc, char *argv[])
{
  FILE *fp;
  uint8_t tx_buf[128];
  uint8_t rx_buf[128];
  TRANSDUCER_CMD_PKT_T	tran_cmd_pkt;
  TRANSDUCER_MSG_T	tran_msg_pkts[10];
  int32_t v,cnt,i,len;
  uint8_t nav_time_secs, reply_time_secs;
  int32_t tmp;
  time_t reply_timeout,nav_timeout,t;
  uint8_t cmd,error;
  char buf[1024];
  uint8_t num_msgs;

debug_txt_flag=0;

  if (argc < 6 ) {
    printf ("Usage: server port num-msgs [mac-addr socket state] [-d]\n");
    printf ("  ex: ./jigo-ctrl localhost 5000 1 0xf0 1 on\n");
    printf ("  mac-addr MAC address of node to actuate\n");
    printf ("  socket 0 or 1\n");
    printf ("  state on or off\n");
    printf ("  d Debug Output\n");
    exit (1);
  }

  for(i=0; i<argc; i++ )
	{
	// Grab dash command line options
	if(strstr(argv[i],"-d")!=NULL )
		{	
		debug_txt_flag=1;
		}
	}

  sscanf( argv[3],"%d", &num_msgs );
  if(num_msgs>9 ) { printf( "Sorry, too many messages...\n" ); return 0; }
  if(debug_txt_flag==1)
  printf( "Composing %d actuation msgs\n", num_msgs );
  for(i=0; i<num_msgs; i++ )
  {
	sscanf( argv[(i*3)+4],"%x",&tmp );
	tran_msg_pkts[i].mac_addr=tmp;	
  	if(debug_txt_flag==1) printf( "MAC_ADDR: 0x%x ",tran_msg_pkts[i].mac_addr );
	sscanf( argv[(i*3)+5],"%d",&tmp );
	if(tmp==0) 
		{
			tran_msg_pkts[i].key=TRAN_POWER_CTRL_SOCK_0;
  			if(debug_txt_flag==1)
				printf( " socket 0 " );
		}
	else {
		tran_msg_pkts[i].key=TRAN_POWER_CTRL_SOCK_1;
  		if(debug_txt_flag==1)
			printf( " socket 1 " );
	}
	if( strstr(argv[(i*3)+6],"off")!=NULL ) tran_msg_pkts[i].value=SOCKET_OFF;	
	else tran_msg_pkts[i].value=SOCKET_ON;
  	if(debug_txt_flag==1)
		printf( " %s\n",argv[(i*3)+6] );
  }

  v=slipstream_open(argv[1],atoi(argv[2]),NONBLOCKING);
  nav_time_secs=25; 

  cnt = 0;
  while (1) {
     error=0;
     cmd=0;


	// Setup the packet to send out to the network

	// These values setup the internal data structure and probably don't
	// need to be changed
	ds_pkt.payload_len=0;
	ds_pkt.buf=tx_buf;
	ds_pkt.buf_len=DS_PAYLOAD_START;
	ds_pkt.payload_start=DS_PAYLOAD_START;
	ds_pkt.payload=&(tx_buf[DS_PAYLOAD_START]);
	
	// These are parameters that can be adjusted for different packets
	//ds_pkt.pkt_type=PING_PKT; 
	ds_pkt.pkt_type=TRANSDUCER_CMD_PKT; 
	ds_pkt.ctrl_flags= DS_MASK | LINK_ACK | DEBUG_FLAG | LED_FLOOD ; 
	ds_pkt.seq_num=0; // Use the gateway's spiffy auto-cnt when set to 0
	ds_pkt.priority=0;
	ds_pkt.ack_retry=10;
	ds_pkt.subnet_mac[0]=0;
	ds_pkt.subnet_mac[1]=0;
	ds_pkt.subnet_mac[2]=0;
	ds_pkt.hop_cnt=0;  // Starting depth, always keep at 0
	ds_pkt.hop_max=5;  // Max tree depth
	ds_pkt.delay_per_level=0;  // Reply delay per level in seconds
	ds_pkt.nav=2;  // Time in seconds until next message to be sent
	ds_pkt.mac_check_rate=100;  // B-mac check rate in ms
	ds_pkt.rssi_threshold=-45;  // Reply RSSI threshold
	ds_pkt.last_hop_mac=0;
	ds_pkt.mac_filter_num=0; // Increase if MAC_FILTER is active
	ds_pkt.aes_ctr[0]=0;   // Encryption AES counter
	ds_pkt.aes_ctr[1]=0;
	ds_pkt.aes_ctr[2]=0;
	ds_pkt.aes_ctr[3]=0;

	//tran_msg_pkts[0].mac_addr=0xf0;
	//tran_msg_pkts[0].key=TRAN_POWER_CTRL_SOCK_0;
	//tran_msg_pkts[0].value=SOCKET_OFF;
	tran_cmd_pkt.num_msgs=num_msgs;	
	tran_cmd_pkt.msg=tran_msg_pkts;	
	ds_pkt.payload_len=transducer_cmd_pkt_add( &tran_cmd_pkt, ds_pkt.payload);
	transducer_cmd_pkt_checksum( &tran_cmd_pkt, ds_pkt.payload);
        // This takes the structure and packs it into the raw
	// array that is sent using SLIP
        pack_downstream_packet( &ds_pkt);	

     // Add MAC filter entries below
     //  downstream_packet_add_mac_filter( &ds_pkt, 0x07 ); 
     //  downstream_packet_add_mac_filter( &ds_pkt, 3 ); 
     //  downstream_packet_add_mac_filter( &ds_pkt, 4 ); 
     //  downstream_packet_add_mac_filter( &ds_pkt, 5 ); 

    // Print your packet on the screen
    if(debug_txt_flag==1)
	print_ds_packet(&ds_pkt );


    cnt++;

    for(i=0; i<ds_pkt.buf_len; i++ )
    {
	printf( "%02x",ds_pkt.buf[i] );
    }
    printf( "\n" );
    if(error==0) 
    	v=slipstream_send(ds_pkt.buf,ds_pkt.buf_len);

    if(debug_txt_flag==1)
    {
    if (v == 0) printf( "Error sending\n" );
    else printf( "Sent request %d\n",ds_pkt.seq_num);
    }
 
    nav_time_secs=ds_pkt.nav;
    reply_time_secs=ds_pkt.delay_per_level * ds_pkt.hop_max;

   t=time(NULL);
   reply_timeout=t+reply_time_secs+1;
   nav_timeout=t+nav_time_secs;

   // Collect Reply packets 
   while(reply_timeout>time(NULL))
   	{
    		v=slipstream_receive( rx_buf);
    		if (v > 0) {
      			handle_incomming_pkt(rx_buf,v);
    			}
    		usleep(1000);
   	}

   // What for NAV and service incoming messages 
   // This is the time window when the network is idle and can
   // be used for asynchronous communications.
   while(nav_timeout>time(NULL))
   	{
    		v=slipstream_receive( rx_buf);
    		if (v > 0) {
      			// Check for mobile/p2p packets
			handle_incomming_pkt(rx_buf,v);
    			}
    		usleep(1000);
   	}
	
	// only run once
	break;
}
}


void handle_incomming_pkt(uint8_t *rx_buf,uint8_t len)
{
int i,j;
SAMPL_GATEWAY_PKT_T gw_pkt;
FF_SENSOR_SHORT_PKT_T sensor_short;
ACK_PKT_T ack;
TRANSDUCER_REPLY_PKT_T tran_pkt;

if(debug_txt_flag)
{
printf( "Raw Pkt [%d] = ",len );
for(i=0; i<len; i++ ) printf( "%d ",rx_buf[i] );
printf( "\n" );
}
	gw_pkt.buf=rx_buf;
	gw_pkt.buf_len=len;
	unpack_gateway_packet(&gw_pkt );
	// You will have a gateway packet here to operate on.
	// The gateway packet has a payload which contains user defined packets.
	
	// Lets print the raw packet:
	if(debug_txt_flag)
		print_gw_packet(&gw_pkt);

	// Now lets parse out some application data:
	switch(gw_pkt.pkt_type)
	{
	// PING and ACK are the same
	case PING_PKT:
	case ACK_PKT:
		for(i=0; i<gw_pkt.num_msgs; i++ )
		{
		  ack_pkt_get( &ack, gw_pkt.payload, i);
		  printf( "Ack pkt from 0x%x\n",ack.mac_addr );
		}
		break;

	// Default FireFly sensor packet
	case TRANSDUCER_REPLY_PKT:
		for(i=0; i<gw_pkt.num_msgs; i++ )
		{
		  transducer_reply_pkt_get( &tran_pkt, gw_pkt.payload, i);
		  switch(tran_pkt.type)
			{
			case TRAN_FF_BASIC_SHORT:
				ff_basic_sensor_short_unpack( &tran_pkt, &sensor_short );	
		  		printf( "Sensor pkt from 0x%x\n",tran_pkt.mac_addr );
		  		printf( "   Light: %d\n",sensor_short.light);
		  		printf( "   Temperature: %d\n",sensor_short.temperature);
		  		printf( "   Acceleration: %d\n",sensor_short.acceleration);
		  		printf( "   Sound Level: %d\n",sensor_short.sound_level);
		  		printf( "   Battery: %d\n",sensor_short.battery+100);
				break;
			case TRAN_ACK:
			   	printf( "ACK 0x%x\n", tran_pkt.mac_addr );
			   	break;
			case TRAN_NCK:
			   	printf( "NCK 0x%x\n", tran_pkt.mac_addr );
			   	break;
			default:
			   printf( "unkown transducer packet\n" );
			}
		}
		
		break;
/*
	case FF_SENSOR_SHORT_PKT:
		for(i=0; i<gw_pkt.num_msgs; i++ )
		{
		  sensor_short_pkt_get( &sensor_short, gw_pkt.payload, i);
		  printf( "Sensor pkt from 0x%x",sensor_short.mac_addr );
		  printf( "   Light: %d",sensor_short.light);
		  printf( "   Temperature: %d",sensor_short.temperature);
		  printf( "   Acceleration: %d",sensor_short.acceleration);
		  printf( "   Sound Level: %d",sensor_short.sound_level);
		  printf( "   Battery: %d\n",sensor_short.battery+100);
		}
		
		break;
*/
	default:
		printf( "Unknown pkt type: %d\r\n", gw_pkt.pkt_type );
	}
}



void print_ds_packet(SAMPL_DOWNSTREAM_PKT_T *ds_pkt )
{
int i;
	printf( "Downstream Packet Header info:\n" ); 
	printf( "  pkt type\t\t0x%x\n",ds_pkt->pkt_type);
	printf( "  ctrl flags\t\t0x%x\n",ds_pkt->ctrl_flags );
	printf( "  seq num\t\t0x%x\n",ds_pkt->seq_num );
	printf( "  priority\t\t0x%x\n",ds_pkt->priority);
	printf( "  ack retry\t\t0x%x\n",ds_pkt->ack_retry);
	printf( "  subnet mac\t\t0x%x ",ds_pkt->subnet_mac[2]);
	printf( " 0x%x",ds_pkt->subnet_mac[1]);
	printf( " 0x%x\n",ds_pkt->subnet_mac[0]);
	printf( "  hop_cnt\t\t0x%x\n",ds_pkt->hop_cnt);
	printf( "  hop_max\t\t0x%x\n",ds_pkt->hop_max);
	printf( "  delay_per_level\t%d seconds\n",ds_pkt->delay_per_level);
	printf( "  nav\t\t\t%d seconds\n",ds_pkt->nav);
	printf( "  mac_check_rate\t%d ms\n",ds_pkt->mac_check_rate);
	printf( "  rssi_threshold\t%d\n",(int8_t)ds_pkt->rssi_threshold);
	printf( "  last_hop_mac\t\t0x%x\n",ds_pkt->last_hop_mac);
	printf( "  mac_filter_num\t0x%x\n",ds_pkt->mac_filter_num);
	printf( "  aes_ctr\t\t0x%x 0x%x 0x%x 0x%x\n",ds_pkt->aes_ctr[3], ds_pkt->aes_ctr[3], 
			ds_pkt->aes_ctr[2], ds_pkt->aes_ctr[1], ds_pkt->aes_ctr[0]);
	printf( "Mac Filter List: " );
        for(i=0; i<ds_pkt->mac_filter_num; i++ )
		printf( "0x%x ",ds_pkt->buf[DS_PAYLOAD_START+i] );
	printf( "\n\n" );
	printf( "Payload Data: " );
        for(i=0; i<ds_pkt->payload_len; i++ )
		printf( "0x%x ",ds_pkt->payload[i] );
	printf( "\n\n" );


}

void print_gw_packet(SAMPL_GATEWAY_PKT_T *gw_pkt )
{
int i;
	printf( "Gateway Packet Header info:\n" ); 
	printf( "  pkt type\t\t0x%x\n",gw_pkt->pkt_type);
	printf( "  ctrl flags\t\t0x%x\n",gw_pkt->ctrl_flags );
	printf( "  seq num\t\t0x%x\n",gw_pkt->seq_num );
	printf( "  priority\t\t0x%x\n",gw_pkt->priority);
	printf( "  ack retry\t\t0x%x\n",gw_pkt->ack_retry);
	printf( "  subnet mac\t\t0x%x\n",gw_pkt->subnet_mac[0]);
	printf( "  src mac\t\t0x%x\n",gw_pkt->src_mac);
	printf( "  dst mac\t\t0x%x\n",gw_pkt->dst_mac);
	printf( "  last hop mac\t\t0x%x\n",gw_pkt->last_hop_mac);
	printf( "  rssi\t\t0x%x\n",gw_pkt->rssi);
	printf( "  error code\t\t%d\n",gw_pkt->error_code);
	printf( "Payload Data: " );
        for(i=0; i<gw_pkt->payload_len; i++ )
		printf( "0x%x ",gw_pkt->payload[i] );
	printf( "\n\n" );


}



void error(char *msg)
{
  perror(msg);
  exit(0);
}


