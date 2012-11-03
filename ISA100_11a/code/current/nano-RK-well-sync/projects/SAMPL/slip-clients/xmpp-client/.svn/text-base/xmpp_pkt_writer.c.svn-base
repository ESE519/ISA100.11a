#include <stdlib.h>
#include <time.h>
#include "xmpp_pkt_writer.h"
#include <sampl.h>
#include "lm-library.h"
#include "node_list.h"
#include "globals.h"
#include <ping_pkt.h>
#include <xmpp_pkt.h>
#include <ack_pkt.h>
#include <neighbor_pkt.h>
#include <ff_basic_sensor_pkt.h>
#include <sensor_cal.h>


#define TEMPERATURE_OFFSET 400

char buf[1024];

void xmpp_pkt_handler(SAMPL_GATEWAY_PKT_T *gw_pkt)
{
XMPP_PKT_T p;
char node_name[MAX_NODE_LEN];

if(gw_pkt->payload_len==0)
  {
  if(debug_txt_flag) printf( "Malformed packet!\n" );
  return;
  }


	xmpp_pkt_unpack(&p, gw_pkt->payload, 0);
	sprintf(node_name,"%02x%02x%02x%02x",gw_pkt->subnet_mac[2],
		gw_pkt->subnet_mac[1],
		gw_pkt->subnet_mac[0],
		gw_pkt->src_mac);
  	printf( "XMPP msg:\n" ); 
	printf( "  node-jid=%s\n",node_name );
	printf( "  node-passwd=%s\n",p.passwd);
	printf( "  dst-jid=%s\n",p.jid);
	printf( "  msg=%s\n",p.msg);
}

void extended_nlist_pkt_handler(SAMPL_GATEWAY_PKT_T *gw_pkt)
{
char node_name[MAX_NODE_LEN];
char publisher_node_name[MAX_NODE_LEN];
uint8_t num_msgs,i;
char timeStr[64];
time_t timestamp;
int8_t rssi,ret;

if(gw_pkt->payload_len==0)
  {
  if(debug_txt_flag) printf( "Malformed packet!\n" );
  return;
  }


	sprintf(publisher_node_name,"%02x%02x%02x%02x",gw_pkt->subnet_mac[2],
		gw_pkt->subnet_mac[1],
		gw_pkt->subnet_mac[0],
		gw_pkt->src_mac);
	if(debug_txt_flag==1) printf( "Data for node: %s\n",publisher_node_name );
	// Check nodes and add them if need be
	if(xmpp_flag==1) check_and_create_node(publisher_node_name);
	// publish XML data for node
	time(&timestamp);
    	strftime(timeStr,100,"%Y-%m-%d %X",localtime(&timestamp));

	sprintf(buf,"<Node id=\"%s\" type=\"FIREFLY\" timestamp=\"%s\">", publisher_node_name,timeStr );

	num_msgs=gw_pkt->payload[0];
  	printf( "Extended Neighbor List %d:\n", num_msgs );
	if(gw_pkt->num_msgs>(MAX_PKT_PAYLOAD/NLIST_PKT_SIZE) ) return;
	for(i=0; i<num_msgs; i++ )
	{
	sprintf( node_name,"%02x%02x%02x%02x",gw_pkt->payload[1+i*5],
	        gw_pkt->payload[1+i*5+1],
		gw_pkt->payload[1+i*5+2],
		gw_pkt->payload[1+i*5+3] );
	rssi=(int8_t)gw_pkt->payload[1+i*5+4];
	sprintf( &buf[strlen(buf)],"<Link linkNode=\"%s\" rssi=\"%d\"/>",node_name,rssi );
	}
	sprintf( &buf[strlen(buf)],"</Node>" );
	
	if(debug_txt_flag==1 ) printf( "Publish: %s\n",buf);
	if(xmpp_flag==1 ) ret = publish_to_node(connection, publisher_node_name,buf);
	if(xmpp_flag && ret!=XMPP_NO_ERROR) printf( "XMPP Error: %s\n",ERROR_MESSAGE(ret));


	if(debug_txt_flag==1) printf( "Publish done\n");
}




void send_xmpp_ping_pkt(SAMPL_GATEWAY_PKT_T *gw_pkt )
{
PING_PKT_T p;
char node_name[MAX_NODE_LEN];
char timeStr[64];
time_t timestamp;
int i,ret;

return;

if(gw_pkt->payload_len==0)
  {
  if(debug_txt_flag) printf( "Malformed packet!\n" );
  return;
  }

if(gw_pkt->num_msgs>(MAX_PKT_PAYLOAD/PING_PKT_SIZE) ) return;
for(i=0; i<gw_pkt->num_msgs; i++ )
  {
	ping_pkt_get(&p, gw_pkt->payload, i);
	sprintf(node_name,"%02x%02x%02x%02x",gw_pkt->subnet_mac[2],
		gw_pkt->subnet_mac[1],
		gw_pkt->subnet_mac[0],
		p.mac_addr); 
	if(debug_txt_flag==1) printf( "Data for node: %s\n",node_name );
	// Check nodes and add them if need be
	if(xmpp_flag==1) check_and_create_node(node_name);
	// publish XML data for node
	time(&timestamp);
    	strftime(timeStr,100,"%Y-%m-%d %X",localtime(&timestamp));
    	sprintf(buf,"<Node id=\"%s\" type=\"FIREFLY\" timestamp=\"%s\"></Node>",node_name,timeStr);
   	
	if(debug_txt_flag==1) printf( "Publish: %s\n",buf);
	if(xmpp_flag==1) ret = publish_to_node(connection,node_name,buf);
	if(xmpp_flag==1 && ret!=XMPP_NO_ERROR) printf( "XMPP Error: %s\n",ERROR_MESSAGE(ret));
 

  }

}

void send_xmpp_sensor_short_pkt(SAMPL_GATEWAY_PKT_T *gw_pkt )
{
FF_SENSOR_SHORT_PKT_T p;
char node_name[MAX_NODE_LEN];
char timeStr[64];
time_t timestamp;
int i,ret,error;
uint32_t c_mac;

if(gw_pkt->payload_len==0)
  {
  if(debug_txt_flag) printf( "Malformed packet!\n" );
  return;
  }

printf( "short sense: num msgs: %d\n",gw_pkt->num_msgs );
if(gw_pkt->num_msgs>(MAX_PKT_PAYLOAD/FF_SENSOR_SHORT_PKT_SIZE) ) return;

for(i=0; i<gw_pkt->num_msgs; i++ )
  {
   float temp_cal;
   uint16_t light_cal;

	sensor_short_pkt_get(&p, gw_pkt->payload, i);

	sprintf(node_name,"%02x%02x%02x%02x",gw_pkt->subnet_mac[2],
		gw_pkt->subnet_mac[1],
		gw_pkt->subnet_mac[0],
		p.mac_addr); 
	
	c_mac=gw_pkt->subnet_mac[2]<<24 | gw_pkt->subnet_mac[1]<<16 | gw_pkt->subnet_mac[0]<<8 | p.mac_addr;
	printf( "temp offset=%f\n",cal_get_temp_offset(c_mac));

	temp_cal=cal_get_temp((p.temperature<<1)+TEMPERATURE_OFFSET)+cal_get_temp_offset(c_mac);
	light_cal=cal_get_light(p.light, ((float)(p.battery+100))/100)+cal_get_light_offset(c_mac);
	light_cal=255-light_cal;
	if(debug_txt_flag==1) printf( "Data for node: %s\n",node_name );
	// Check nodes and add them if need be
	if(xmpp_flag==1) check_and_create_node(node_name);
	// publish XML data for node
	time(&timestamp);
    	strftime(timeStr,100,"%Y-%m-%d %X",localtime(&timestamp));

	error=0;
	switch(p.type)
	{
	 case 1:
	 	sprintf(buf,"<Node id=\"%s\" type=\"FIREFLY\" timestamp=\"%s\"> <Sensor name=\"Light\" value=\"%d\" id=\"0001\"/> <Sensor name=\"Temperature\" value=\"%2.2f\" id=\"0002\"/> <Sensor name=\"Acceleration\" value=\"%d\" id=\"0003\"/> <Sensor name=\"Voltage\" value=\"%1.2f\" id=\"0004\"/> <Sensor name=\"Audio\" value=\"%d\" id=\"0005\"/> </Node>",
		node_name,timeStr, light_cal, temp_cal, p.acceleration, ((float)(p.battery+100))/100,p.sound_level);
   		break;
	 default:
		error=1;
	}

	if(error==0)
	{
	if(debug_txt_flag==1 ) printf( "Publish: %s\n",buf);
	if(xmpp_flag==1 ) ret = publish_to_node(connection,node_name,buf);
	if(xmpp_flag && ret!=XMPP_NO_ERROR) printf( "XMPP Error: %s\n",ERROR_MESSAGE(ret));
	if(debug_txt_flag==1) printf( "Publish done\n");
	}	
  }
}

