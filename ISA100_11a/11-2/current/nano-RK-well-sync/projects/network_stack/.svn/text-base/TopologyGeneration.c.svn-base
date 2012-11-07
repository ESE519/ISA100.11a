#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "TopologyGeneration.h" 

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
	
	toString(from, f);
	toString(to, t);
	 
	fprintf(fp, "\t%s -> %s;\n", from, to);
	
	if(DEBUG_TG >= 1)
		printf("\t%s -> %s [rssi = %d]\n", from, to, rssi);
		
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
void generate_graph()
{
	system("dot -Tgif SensorTopology.dot -o SensorTopology.gif");	// create the .gif file 
			
	return;
}
/***************************************************************************************/
