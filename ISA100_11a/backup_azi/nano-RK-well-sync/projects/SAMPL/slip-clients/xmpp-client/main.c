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
#include <lm-library.h>
#include "node_list.h"
#include "globals.h"
#include "xmpp_pkt_writer.h"
#include "sensor_cal.h"
#include <loudmouth/loudmouth.h>


#define SEQ_CACHE_SIZE	24

#define IGNORE_PACKET	0
#define US_PACKET	1
#define P2P_PACKET	2

uint8_t gw_subnet_2;
uint8_t gw_subnet_1;
uint8_t gw_subnet_0;
uint8_t gw_mac;



#define NONBLOCKING  0
#define BLOCKING     1

#define HEX_STR_SIZE	5


static void handle_xmpp_msgs ( LmMessage *m);

void handle_incomming_pkt(uint8_t *rx_buf,uint8_t len);
void error(char *msg);
void print_ds_packet(SAMPL_DOWNSTREAM_PKT_T *ds_pkt );
void check_and_create_node(char *node_name);

  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  int size,error_debug_txt_flag=1;
  char buffer[2048];


void seq_num_cache_init();
int seq_num_cache_check( uint8_t *mac_addr, uint8_t seq_num, uint8_t pkt_type);

typedef struct seq_num_cache {
  uint8_t addr[4];
  uint8_t seq_num;
  uint8_t pkt_type;
  int valid;
} seq_num_cache_t;

seq_num_cache_t seq_cache[SEQ_CACHE_SIZE];

SAMPL_DOWNSTREAM_PKT_T ds_pkt;

char slip_server[MAX_BUF];
uint32_t slip_port;
FILE *fp;
char buf[1024];

static void handle_xmpp_msgs ( LmMessage *m )
{

  printf( "XMPP message handler\n" );

}



void *main_publish_loop(gpointer data)
{

  uint8_t tx_buf[MAX_BUF];
  uint8_t rx_buf[MAX_BUF];
  int32_t v,i,len;
  uint8_t seq_num;
  uint8_t nav_time_secs;
  uint8_t reply_time_secs;
  int32_t tmp;
  time_t reply_timeout, nav_timeout,t;
  uint8_t cmd_ready,error,ret;
  char token[64];
  char name[64];



printf( "Adding gateway node\n" );
if(xmpp_flag==1)
{
  //sscanf(username,"%[^@]",name);
  sprintf( name, "%02x%02x%02x%02x",gw_subnet_2, gw_subnet_1, gw_subnet_0, gw_mac );
  node_list_add(name);
}

  if(xmpp_flag==1)
  {
	// generate parent node for gateway
    	ret = create_event_node(connection, name,NULL,FALSE);
	if(ret != XMPP_NO_ERROR) {
      		if(ret == XMPP_ERROR_NODE_EXISTS)
			if(debug_txt_flag) printf("Node '%s' already exists\n",name);
      		else {
		g_printerr("Could not create event node '%s'. Error='%s'\n",name,ERROR_MESSAGE(ret));
		return -1;
      		}
    } else if(debug_txt_flag) printf("Created event node '%s'\n",name);

  }



  v=slipstream_open(slip_server,slip_port,NONBLOCKING);
  //v=slipstream_open(argv[1],atoi(argv[2]),NONBLOCKING);
  seq_num = 0;



  while (1) {
     error=0;
     cmd_ready=0;



     // Read Data packet from script file
     while(cmd_ready==0)
     {
     v=fscanf( fp, "%[^\n]\n", buf);
     if(v==-1) rewind(fp);
     if(buf[0]!='#' && v!=-1)
	{
		uint8_t offset;
		offset=0;
		i=0;
		tmp=1;

	if(error_debug_txt_flag==1) printf( "108: Parsing line\n" );

		while(tmp==1) {
		tmp=sscanf( &buf[offset*HEX_STR_SIZE],"0x%x ",&tx_buf[i] );
		// printf( "i=%d tmp=%d val=0x%x\n",i,tmp,tx_buf[i] );
		if(tmp==1) { offset++; i++; }
		}

	if(error_debug_txt_flag==1) printf( "116: Parsing line done\n" );

	len=offset;
	ds_pkt.buf_len=offset;
	ds_pkt.buf=tx_buf;
	// write to the structure and raw buffer
	// We end up transmitting the raw buffer after adding the correct sequence number 
	// also load the correct subnet mac from the command line input 

	tx_buf[SUBNET_MAC_2]=gw_subnet_2;
	tx_buf[SUBNET_MAC_1]=gw_subnet_1;
	tx_buf[SUBNET_MAC_0]=gw_subnet_0;
	tx_buf[DS_LAST_HOP_MAC]=gw_mac;
        tx_buf[SEQ_NUM]=seq_num;
	tx_buf[DS_AES_CTR_0]=seq_num;
	if(tx_buf[DS_AES_CTR_0]==255) tx_buf[DS_AES_CTR_1]++;
	if(tx_buf[DS_AES_CTR_1]==255) tx_buf[DS_AES_CTR_2]++;
	if(tx_buf[DS_AES_CTR_2]==255) tx_buf[DS_AES_CTR_3]++;
	if(error_debug_txt_flag==1) printf( "128: About to unpack gw pkt\n" );
	unpack_downstream_packet( &ds_pkt, 0 );
	if(error_debug_txt_flag==1) printf( "130: gw pkt unpacked\n" );
	
	if(debug_txt_flag==1)
		print_ds_packet(&ds_pkt );

        if(i<21 ) 
	  {
		error=1;	
		printf( "Error parsing input file!\n" );
	  }
	
    	seq_num++;
	nav_time_secs=tx_buf[DS_NAV];
	reply_time_secs=tx_buf[DS_DELAY_PER_LEVEL] * tx_buf[DS_HOP_MAX];
	cmd_ready=1;
	}

    }

	if(error_debug_txt_flag==1) printf( "149: Done parsing file\n" );
    	// Send the packet
    	if(len>128) len=128;
    	if(!no_slip_flag && error==0) 
    		v=slipstream_send(tx_buf,len);
	if(error_debug_txt_flag==1) printf( "154: Slipstream sent\n" );
    	if(debug_txt_flag==1)
    	{
    		if (v == 0) printf( "Error sending\n" );
    		else printf( "Sent request %d\n",tx_buf[SEQ_NUM]);
    	}

   t=time(NULL);
   reply_timeout=t+reply_time_secs+1;
   nav_timeout=t+nav_time_secs;

   if(error_debug_txt_flag==1) printf( "165: Waiting for reply\n" );
   // Collect Reply packets 
   while(reply_timeout>time(NULL))
   	{
    		v=slipstream_receive( rx_buf);
    		if (v > 0) {
   			if(error_debug_txt_flag==1) printf( "171: before in pkt: v= %d\n",v );
      			handle_incomming_pkt(rx_buf,v);
   			if(error_debug_txt_flag==1) printf( "173: after in pkt\n" );
    			}
    		usleep(1000);
   	}

   if(debug_txt_flag==1) printf( "reply wait timeout...\n" );
   // What for NAV and service incomming messages 
   // This is the time window when the network is idle and can
   // be used for asynchronous communications.
   while(nav_timeout>time(NULL))
   	{
    		v=slipstream_receive( rx_buf);
    		if (v > 0) {
   			if(error_debug_txt_flag==1) printf( "186: before in pkt: v= %d\n",v );
      			handle_incomming_pkt(rx_buf,v);
   			if(error_debug_txt_flag==1) printf( "188: after in pkt\n" );
     			// Check if TX queue has data and send the request
    			}
    		usleep(1000);
   	}
	
   if(error_debug_txt_flag==1) printf( "194: Done with rx/tx cycle\n" );

}





}


int main (int argc, char *argv[])
{
  GThread *main_thread = NULL;
  GError *error = NULL;
  GMainLoop *main_loop = NULL;

  uint32_t gw_mac_addr_full;
  char name[64];
  char xmpp_file_name[128];
  char sampl_file_name[128];
  char password[64];
  char xmpp_server[64];
  char xmpp_ssl_fingerprint[64];
  char pubsub_server[64];
  char username[64];
  uint32_t xmpp_server_port;
  uint8_t param; 
  int32_t v,ret;
 
debug_txt_flag=0;
xmpp_flag=0;
no_slip_flag=0;
print_input_flag=0;
connection=NULL;

  if (argc < 4 || argc > 7) {
    printf ("Usage: server port gateway_mac [-vxnf] [xmmp_config_name] [sampl_config_name]\n");
    printf ("  gateway_mac e.g. 0x00000000\n");
    printf ("  v Print Verbose Debug\n");
    printf ("  x Send data to XMPP server\n");
    printf ("  n Don't send SLIP packets (but receive them)\n");
    printf ("  f Use the following config files instead of the defaults\n");
    exit (1);
  }

  sscanf( argv[3],"%x",&gw_mac_addr_full );
  printf( "GW mac: 0x%08x\n",gw_mac_addr_full );
  gw_subnet_2=gw_mac_addr_full>>24;
  gw_subnet_1=gw_mac_addr_full>>16;
  gw_subnet_0=gw_mac_addr_full>>8;
  gw_mac=gw_mac_addr_full&0xff;

  strcpy( xmpp_file_name, "xmpp_config.txt" );
  strcpy( sampl_file_name, "ff_config.txt" );
  if(argc>3)
	{
	// Grab dash command line options
	if(strstr(argv[4],"v")!=NULL )
		{
		printf( "Verbose Mode ON\n" );
		debug_txt_flag=1;
		}
	if(strstr(argv[4],"x")!=NULL )
		{	
		printf( "XMPP ON\n" );
		xmpp_flag=1;
		}
	if(strstr(argv[4],"n")!=NULL )
		{	
		printf( "SLIP TX OFF\n" );
		no_slip_flag=1;
		}
	if(strstr(argv[4],"f")!=NULL )
		{
			printf( "Loading XMPP config: " );
			strcpy(xmpp_file_name, argv[5]);
			printf( "%s\n",xmpp_file_name );
			printf( "Loading SAMPL config: " );
			strcpy(sampl_file_name, argv[6]);
			printf( "%s\n",sampl_file_name );
		}
	}

  if(xmpp_flag)
  {
  fp=fopen( xmpp_file_name,"r" );
  if(fp==NULL)
  	{
	printf( "XMPP config: No %s file!\n",xmpp_file_name );
	exit(0);
	}
	param=0;
	do {
     	  v=fscanf( fp, "%[^\n]\n", buf);
     	  if(buf[0]!='#' && v!=-1)
	  {	
		switch(param)
		{
		case 0: strcpy(username,buf);  break;
		case 1: strcpy(password,buf);  break;
		case 2: strcpy(xmpp_server,buf);  break;
		case 3: xmpp_server_port=atoi(buf);  break;
		case 4: strcpy(pubsub_server,buf); break;
		case 5: strcpy(xmpp_ssl_fingerprint,buf); break;
		}
		param++;
	  }

	}while(v!=-1 && param<6 );

	if(debug_txt_flag)
	{
	printf( "XMPP Client Configuration:\n" );
	printf( "  username: %s\n",username );
	printf( "  password: %s\n",password);
	printf( "  xmpp server: %s\n",xmpp_server);
	printf( "  xmpp server port: %d\n",xmpp_server_port);
	printf( "  xmpp pubsub server: %s\n",pubsub_server);
	printf( "  xmpp ssl fingerprint: %s\n\n",xmpp_ssl_fingerprint);
	}
	if(param<4)
	{
	printf( "Not enough xmpp configuration parameters in xmpp_config.txt\n" );
	exit(0);
	}

	connection = start_xmpp_client(username, 
				password, 
				xmpp_server, 
				xmpp_server_port, 
				xmpp_ssl_fingerprint, 
				pubsub_server, 
				handle_xmpp_msgs );

	if(connection == NULL) {
		g_printerr("Could not start client.\n");
		return -1;
	}
     fclose(fp);
     if(debug_txt_flag) printf("Initialized XMPP client\n");
  }


  fp=fopen( sampl_file_name,"r" );
  if(fp==NULL) {
	printf( "SAMPL config: No %s file!\n",sampl_file_name );
	printf( "This is required for sending control commands\n" );
	exit(0);
  }

  // clear list of nodes
  node_list_init();
  seq_num_cache_init();

  cal_load_params("sensor_cal.txt");
  strcpy(slip_server,argv[1]);
  slip_port=atoi(argv[2]);

  g_thread_init (NULL);
  

   main_loop = g_main_loop_new(NULL,FALSE);

   main_thread =
   g_thread_create ((GThreadFunc) main_publish_loop, connection, TRUE, &error);
	if (error != NULL) {
		g_printerr ("Thread creation error: <%s>\n", error->message);
		return -1;
	}



   g_print("created thread\n");
   g_main_loop_run (main_loop);



}

void handle_incomming_pkt(uint8_t *rx_buf,uint8_t len)
{
int i,ret;
int pkt_type;
uint8_t mac[4];
char node_name[MAX_NODE_LEN];
SAMPL_GATEWAY_PKT_T gw_pkt;
SAMPL_PEER_2_PEER_PKT_T p2p_pkt;

if(debug_txt_flag==1)
{
	printf( "Incomming Pkt [%d] = ",len );
	for(i=0; i<len; i++ ) printf( "%d ",rx_buf[i] );
	printf( "\n" );
}

pkt_type=IGNORE_PACKET;

if((rx_buf[CTRL_FLAGS] & US_MASK) != 0 && ((rx_buf[CTRL_FLAGS] & DS_MASK) !=0 )) 
	pkt_type=P2P_PACKET;
else if ((rx_buf[CTRL_FLAGS] & US_MASK) != 0 && (rx_buf[CTRL_FLAGS] & DS_MASK) == 0) 
	pkt_type=US_PACKET;
else pkt_type=IGNORE_PACKET;

mac[3]=rx_buf[SUBNET_MAC_2];
mac[2]=rx_buf[SUBNET_MAC_1];
mac[1]=rx_buf[SUBNET_MAC_0];
mac[0]=rx_buf[GW_SRC_MAC];

		printf("checking: %02x%02x%02x%02x\n",mac[3],
				mac[2],
				mac[1],
				mac[0] ); 
// Check if it is a repeat packet
if( seq_num_cache_check( mac , rx_buf[SEQ_NUM],rx_buf[PKT_TYPE])==1) 
{
	if(debug_txt_flag==1) printf( "DUPLICATE PACKET!\n" );
		sprintf(node_name,"%02x%02x%02x%02x",rx_buf[SUBNET_MAC_2],
				rx_buf[SUBNET_MAC_1],
				rx_buf[SUBNET_MAC_0],
				rx_buf[GW_SRC_MAC] ); 
		printf( "mac=%s seq_num=%d type=%d\n",node_name, rx_buf[SEQ_NUM],rx_buf[PKT_TYPE]);
} else
{
  // Create an event node if it doesn't already exist and if it is an infrastructure node

  if(xmpp_flag==1)
  {
  if(pkt_type!=IGNORE_PACKET && (rx_buf[CTRL_FLAGS] & MOBILE_MASK) !=0)
  {
		sprintf(node_name,"%02x%02x%02x%02x",rx_buf[SUBNET_MAC_2],
				rx_buf[SUBNET_MAC_1],
				rx_buf[SUBNET_MAC_0],
				rx_buf[GW_LAST_HOP_MAC] ); 

	check_and_create_node(node_name);
   }

  }




// The only p2p packet that we understand from a mobile node is the XMPP_MSG
if(pkt_type==P2P_PACKET && 
   (rx_buf[CTRL_FLAGS] & MOBILE_MASK) !=0 && 
   rx_buf[PKT_TYPE]!=XMPP_PKT ) pkt_type=IGNORE_PACKET; 

	gw_pkt.buf=rx_buf;
	gw_pkt.buf_len=len;
	unpack_gateway_packet(&gw_pkt );
if(debug_txt_flag==1) printf( "Calling pkt handler for pkt_type %d\n",gw_pkt.pkt_type );


switch(gw_pkt.pkt_type)
{
	case PING_PKT:
	case ACK_PKT:
	send_xmpp_ping_pkt( &gw_pkt );
	if(debug_txt_flag==1) printf( "PING or ACK packet\n" );
	break;

	case XMPP_PKT:
	xmpp_pkt_handler( &gw_pkt );
	if(debug_txt_flag==1) printf( "XMPP packet\n" );
	break;


	case EXTENDED_NEIGHBOR_LIST_PKT:
	extended_nlist_pkt_handler( &gw_pkt );
	if(debug_txt_flag==1) printf( "Extended Neighbor List packet\n" );
	break;


	case FF_SENSOR_SHORT_PKT:
	send_xmpp_sensor_short_pkt( &gw_pkt );
	if(debug_txt_flag==1) printf( "SENSOR_SHORT packet\n" );
	break;

	case TRACEROUTE_PKT:
	if(debug_txt_flag==1) printf( "TRACEROUTE packet\n" );
	break;


	default:
	if(debug_txt_flag==1) printf( "Unknown Packet\n" );
}
}
printf( "done with handle pkt\n" );

}




void error(char *msg)
{
  perror(msg);
  exit(0);
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
	printf( "  subnet mac\t\t0x%x\n",ds_pkt->subnet_mac[0]);
	printf( "  hop_cnt\t\t0x%x\n",ds_pkt->hop_cnt);
	printf( "  hop_max\t\t0x%x\n",ds_pkt->hop_max);
	printf( "  delay_per_level\t%d seconds\n",ds_pkt->delay_per_level);
	printf( "  nav\t\t\t%d seconds\n",ds_pkt->nav);
	printf( "  mac_check_rate\t%d ms\n",ds_pkt->mac_check_rate);
	printf( "  rssi_threshold\t%d\n",(int8_t)(ds_pkt->rssi_threshold));
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

void seq_num_cache_init()
{
int i;
for(i=0; i<SEQ_CACHE_SIZE; i++ ) seq_cache[i].valid=0;
}


// This function returns 1 if the packet is a repeat and should be surpressed.
// It returns 0 if the packet is unique.
int seq_num_cache_check( uint8_t *mac_addr, uint8_t seq_num, uint8_t pkt_type)
{
int i,j;
int match;

/*
for(i=0; i<SEQ_CACHE_SIZE; i++ )
	{
	if(seq_cache[i].valid!=0) printf( "cache %d: mac %02x%02x%02x%02x seq=%d type=%d ttl=%d\n",
	i, seq_cache[i].addr[3],seq_cache[i].addr[2],seq_cache[i].addr[1],
	seq_cache[i].addr[0],seq_cache[i].seq_num, seq_cache[i].pkt_type, seq_cache[i].valid );
	}
*/

// This is to stop caching SAMPL reply packets.
// Reply packets all come from the gateway with the same
// seq number and packet type
  if( mac_addr[0]==gw_mac &&
    mac_addr[1]==gw_subnet_0 &&
    mac_addr[2]==gw_subnet_1 &&
    mac_addr[3]==gw_subnet_2 ) return 0;

for(i=0; i<SEQ_CACHE_SIZE; i++ )
{
  if(seq_cache[i].valid>0)
  {
  seq_cache[i].valid--;
  if( mac_addr[0]==seq_cache[i].addr[0] &&
    mac_addr[1]==seq_cache[i].addr[1] &&
    mac_addr[2]==seq_cache[i].addr[2] &&
    mac_addr[3]==seq_cache[i].addr[3] )
	{	
	seq_cache[i].valid=100;
	// This is a repeat packet
	if(seq_num==seq_cache[i].seq_num && pkt_type==seq_cache[i].pkt_type) 
	{
		return 1;

	}
	else
		{
			seq_cache[i].seq_num=seq_num;
			seq_cache[i].pkt_type=pkt_type;
			return 0;
		}
	}
  }

}


for(i=0; i<SEQ_CACHE_SIZE; i++ )
{
  if(seq_cache[i].valid==0)
  {
  seq_cache[i].addr[0]=mac_addr[0];
  seq_cache[i].addr[1]=mac_addr[1];
  seq_cache[i].addr[2]=mac_addr[2];
  seq_cache[i].addr[3]=mac_addr[3];
  seq_cache[i].seq_num=seq_num;
  seq_cache[i].pkt_type=pkt_type;
  seq_cache[i].valid=100;
  return 0;
  }
}

return 0;
}
