#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "../../include/sampl.h"
#include <slipstream.h>
//#include "tree_route.h"
//#include "slipstream.h"

#define gw_mac		0

uint8_t debug_txt_flag;
uint8_t xmpp_flag;
uint8_t no_slip_flag;
uint8_t print_input_flag;

#define NONBLOCKING  0
#define BLOCKING     1

#define HEX_STR_SIZE	5

void handle_incomming_pkt(uint8_t *rx_buf,uint8_t len);
void error(char *msg);
void print_ds_packet(SAMPL_DOWNSTREAM_PKT_T *ds_pkt );

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
  int32_t v,cnt,i,len;
  uint8_t nav_time_secs;
  int32_t tmp;
  time_t t;
  uint8_t cmd,error;
  char buf[1024];

debug_txt_flag=0;
xmpp_flag=0;
no_slip_flag=0;
print_input_flag=0;

  if (argc < 3 || argc > 4) {
    printf ("Usage: server port [-dx]\n");
    printf ("  d Debug Input Text\n");
    printf ("  x Use XMPP server\n");
    printf ("  n Don't send SLIP packets (but receive them)\n");
    exit (1);
  }

  if(argc==4)
	{
	// Grab dash command line options
	if(strstr(argv[3],"d")!=NULL )
		{	
		debug_txt_flag=1;
		}
	if(strstr(argv[3],"x")!=NULL )
		{	
		xmpp_flag=1;
		}
	if(strstr(argv[3],"n")!=NULL )
		{	
		no_slip_flag=1;
		}
	}

  fp=fopen( "ff_config.txt","r" );
  if(fp==NULL) {
	printf( "Could not open ff_config.txt!\n" );
	printf( "This is required for sending control commands\n" );
	exit(0);
  }

  	v=slipstream_open(argv[1],atoi(argv[2]),NONBLOCKING);
  nav_time_secs=25;  
  cnt = 0;
  while (1) {
     error=0;
     cmd=0;


     // Check if TX queue has 

     // Read Data packet from script file
     while(cmd==0)
     {
     v=fscanf( fp, "%[^\n]\n", buf);
     if(v==-1) rewind(fp);
     if(buf[0]!='#' && v!=-1)
	{
		uint8_t offset;
		offset=0;
		i=0;
		tmp=1;
		while(tmp==1) {
		tmp=sscanf( &buf[offset*HEX_STR_SIZE],"0x%x ",&tx_buf[i] );
		// printf( "i=%d tmp=%d val=0x%x\n",i,tmp,tx_buf[i] );
		if(tmp==1) { offset++; i++; }
		}

	// Setup the packet to send out to the network which was read from the file
	len=offset;
	ds_pkt.buf_len=offset;
	ds_pkt.buf=tx_buf;
	unpack_downstream_packet( &ds_pkt, 0 );
	// write to the structure and raw buffer
	// We end up transmitting the raw buffer after adding thecorrect sequence number 
	tx_buf[SEQ_NUM]=cnt;
	ds_pkt.seq_num=cnt;
	
if(debug_txt_flag==1)
	print_ds_packet(&ds_pkt );

        if(i<20 ) 
	  {
		error=1;	
		printf( "Error parsing input file!\n" );
	  }
	
    	cnt++;
	nav_time_secs=tx_buf[DS_NAV];
	cmd=1;
	}
    }


    // Send the packet
    if(len>128) len=128;
    if(!no_slip_flag && error==0) 
    	v=slipstream_send(tx_buf,len);
    if(debug_txt_flag==1)
    {
    if (v == 0) printf( "Error sending\n" );
    else printf( "Sent request %d\n",tx_buf[SEQ_NUM]);
    }
 
   if(debug_txt_flag==1)
	printf( "Waiting %d seconds\n",nav_time_secs );
   t=time(NULL);
   t+=nav_time_secs;

   // Collect Reply packets for NAV seconds
   while(t>time(NULL))
   {
    v=slipstream_receive( rx_buf);
    if (v > 0) {
      handle_incomming_pkt(rx_buf,v);
    }
    usleep(1000);
   }

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
	printf( "  subnet mac\t\t0x%x\n",ds_pkt->subnet_mac);
	printf( "  hop_cnt\t\t0x%x\n",ds_pkt->hop_cnt);
	printf( "  hop_max\t\t0x%x\n",ds_pkt->hop_max);
	printf( "  delay_per_level\t0x%x\n",ds_pkt->delay_per_level);
	printf( "  nav\t\t\t0x%x\n",ds_pkt->nav);
	printf( "  mac_check_rate\t0x%x\n",ds_pkt->mac_check_rate);
	printf( "  rssi_threshold\t0x%x\n",ds_pkt->rssi_threshold);
	printf( "  last_hop_mac\t\t0x%x\n",ds_pkt->last_hop_mac);
	printf( "  mac_filter_num\t0x%x\n",ds_pkt->mac_filter_num);
	printf( "  aes_ctr\t\t0x%x 0x%x 0x%x 0x%x\n",ds_pkt->aes_ctr[3], ds_pkt->aes_ctr[3], 
			ds_pkt->aes_ctr[2], ds_pkt->aes_ctr[1], ds_pkt->aes_ctr[0]);
	printf( "Extra Data: " );
        for(i=DS_PAYLOAD_START; i<ds_pkt->buf_len; i++ )
		printf( "0x%x ",ds_pkt->buf[i] );
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
	printf( "  subnet mac\t\t0x%x\n",gw_pkt->subnet_mac);
	printf( "  rssi\t\t0x%x\n",gw_pkt->rssi);
	printf( "  last_hop_mac\t\t0x%x\n",gw_pkt->last_hop_mac);
	printf( "Extra Data: " );
        for(i=DS_PAYLOAD_START; i<gw_pkt->buf_len; i++ )
		printf( "0x%x ",gw_pkt->buf[i] );
	printf( "\n\n" );


}
void handle_incomming_pkt(uint8_t *rx_buf,uint8_t len)
{
int i;
SAMPL_GATEWAY_PKT_T gw_pkt;

printf( "Raw Pkt [%d] = ",len );
for(i=0; i<len; i++ ) printf( "%d ",rx_buf[i] );
printf( "\n" );

	gw_pkt.buf=rx_buf;
	gw_pkt.buf_len=len;
	unpack_gateway_packet(&gw_pkt );
	print_gw_packet(&gw_pkt);
}



void error(char *msg)
{
  perror(msg);
  exit(0);
}


