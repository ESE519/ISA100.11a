#include "node_list.h"
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "lm-library.h"
#include "globals.h"

char nodeIDs[MAX_NODE_ELEMENTS][MAX_NODE_LEN];
uint8_t node_id_cnt;

void node_list_init()
{
int i;
node_id_cnt=0;
  for(i=0; i<MAX_NODE_ELEMENTS; i++ ) nodeIDs[i][0]='\0'; 
}


int node_list_exists(char *name)
{
int i;

for(i=0; i<node_id_cnt; i++ )
   if(strcmp(name,nodeIDs[i])==0) return 1;

return 0;
}

int node_list_add(char *name)
{

  if(node_id_cnt<MAX_NODE_ELEMENTS)
  {
    strcpy( nodeIDs[node_id_cnt], name );
    node_id_cnt++;
    return 1;
  }

return 0;
}


void check_and_create_node(char *node_name)
{
int ret;
char buf[1024];
char timeStr[64];
time_t timestamp;
	// If I have already created the node this run
	if( node_list_exists(node_name)==0)
	{
	// Add it to my list to stop creating new nodes
	node_list_add(node_name);
	// generate parent node for gateway
    	ret = create_event_node(connection, node_name,NULL,FALSE);
	if(ret != XMPP_NO_ERROR) {
      		if(ret == XMPP_ERROR_NODE_EXISTS)
		{
			if(debug_txt_flag) printf("Node '%s' already exists\n",node_name);
      		} else {
		g_printerr("Could not create event node '%s'. Error='%s'\n",node_name,ERROR_MESSAGE(ret));
		return;
      		}
	}
	else 
	{
		
		if(debug_txt_flag) printf("Created event node '%s'\n",node_name);
	
		// First time add a description of node
		// publish XML data for node
		time(&timestamp);
    		strftime(timeStr,100,"%Y-%m-%d %X",localtime(&timestamp));
    		sprintf(buf,"<Node id=\"%s\" type=\"FIREFLY\" description=\"A Firefly Node\" timestamp=\"%s\"></Node>",node_name,timeStr);
		if(xmpp_flag==1) ret = publish_to_node(connection,node_name,buf);
 
}
	}

}
