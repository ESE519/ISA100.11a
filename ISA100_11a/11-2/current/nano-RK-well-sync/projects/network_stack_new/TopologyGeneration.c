#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "TopologyGeneration.h" 
#include "NetworkGatewayThreaded.h"

/************************************* FUNCTION DEFINITIONS ****************************/
void toString(char *str, uint16_t addr)
{
	sprintf(str, "%u", addr);  
	return;
}
/**************************************************************************************/
void add_link_to_topology_file(FILE *fp, uint16_t f, uint16_t t, int8_t rssi)
{
	char from[ADDR_LENGTH];
	char to[ADDR_LENGTH];
	int8_t label;
	
	toString(from, f);
	toString(to, t);
	
	if(rssi >= RX_POWER_THRESHOLD)	// good link
		label = 1;
	else
		label = RX_POWER_THRESHOLD - rssi;
	 
	fprintf(fp, "\t%s -> %s[label=\" %d (%d) \"];\n", from, to, label, rssi);
	
	if(DEBUG_TG >= 1)
		printf("\t%s -> %s [label = %d; rssi = %d]\n", from, to, label, rssi);
		
	return;
}
/*************************************************************************************/
void begin_topology_file(FILE *fp)
{
	fputs("\ndigraph Topology\n{\nconcentrate=true;\n", fp);
	
	if(DEBUG_TG >= 1)
		printf("digraph Topology\n{\nconcentrate=true;\n");
		
	return;
}
/*************************************************************************************/
void end_topology_file(FILE *fp)
{
	fputs("}\n", fp); // close the topology format
	
	if(DEBUG_TG >= 1)
		printf("}\n");
		
	return;
}
/**************************************************************************************/
void generate_TopGraph()
{
	char inf[PATHNAME_MAX_SIZE];
	char ouf[PATHNAME_MAX_SIZE];
	char cmd[BUFF_SIZE];
	
	sprintf(inf, WEB_SERVER_ROOT);
	strcat(inf, "/SensorTopology.dot");
	
	sprintf(ouf, WEB_SERVER_ROOT);
	strcat(ouf, "/SensorTopology.gif");
	
	sprintf(cmd, "dot -Tgif ");
	strcat(cmd, inf);
	strcat(cmd, " -o ");
	strcat(cmd, ouf);
	
	if(DEBUG_TG == 1)
		printf("TG: generate_TopGraph(): Command = %s\r\n", cmd); 	
	system(cmd); 
			
	return;
}
/***************************************************************************************/
