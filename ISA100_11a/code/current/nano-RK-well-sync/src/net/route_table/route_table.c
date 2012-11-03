#include "route_table.h"
#include <nrk_error.h>
#include <nrk_time.h>

nrk_time_t last_time;
nrk_time_t current_time;
nrk_time_t elapsed_time;

int8_t route_table_init()
{
uint8_t i,j;

for(i=0; i<ROUTE_TABLE_SIZE; i++ )
	{
	route_table[i].flags=0;	
	route_table[i].dst=0;	
	route_table[i].next_hop=0;	
	route_table[i].ttl=0;	
	for(j=0; j<ROUTE_TABLE_VALUES; j++ )
		route_table[i].value[j]=-1;	
	}
nrk_time_get(&last_time);
}

void _route_table_ttl_update()
{
uint8_t i;
int8_t v;
nrk_time_get(&current_time);
v=nrk_time_sub(&elapsed_time, current_time, last_time);
last_time.secs=current_time.secs;
last_time.nano_secs=current_time.nano_secs;
if(v==NRK_OK)
	{
	for(i=0; i<ROUTE_TABLE_SIZE; i++ )
		{
		  if(route_table[i].ttl!=0)
		  {
		    // stricly larger than so it doesn't equal 0
		    // since 0 is used to disable ttl
		    if(route_table[i].ttl>elapsed_time.secs)
			route_table[i].ttl-=elapsed_time.secs;
		    else
			{
			route_table[i].flags=0;
			route_table[i].ttl=0;
			route_table[i].dst=0;
			route_table[i].next_hop=0;
			}
		  }
		}
	}
}

int8_t route_table_set( uint16_t dst, uint16_t next_hop, uint16_t ttl )
{
uint8_t i; 
int8_t found;
found=-1;
_route_table_ttl_update();
for(i=0; i<ROUTE_TABLE_SIZE; i++ )
	{
	  if(route_table[i].dst==dst )
		{
			route_table[i].next_hop=next_hop;
			route_table[i].ttl=ttl;
			route_table[i].flags|=VALID_MASK;
			return NRK_OK;
		}
	  if(route_table[i].ttl==0 && found==-1) found=i;
	}

if(found>=0)
	{
	   route_table[found].next_hop=next_hop;
	   route_table[found].ttl=ttl;
	   route_table[found].flags|=VALID_MASK;
	   return NRK_OK;
	}
return NRK_ERROR;
}

int8_t route_table_clr( uint16_t dst)
{
uint8_t i;
for(i=0; i<ROUTE_TABLE_SIZE; i++ )
	{
	if(route_table[i].dst==dst)
		{
		  route_table[i].flags=0;
		  route_table[i].dst=0;
		  route_table[i].ttl=0;
		  route_table[i].next_hop=0;
		  return NRK_OK;
		}
	}
NRK_ERROR;
}


// Route Find returns 0xFFFF if no route exists
uint16_t route_table_get( uint16_t dst)
{
uint8_t i;
for(i=0; i<ROUTE_TABLE_SIZE; i++ )
	{
	if((route_table[i].flags & VALID_MASK)!=0 && route_table[i].dst==dst)
		  return (route_table[i].next_hop);
	}

return 0xffff;
}

int8_t route_table_value_set(uint16_t dst, uint8_t value_index, int8_t value)
{
uint8_t i;
if(value_index>=ROUTE_TABLE_VALUES ) return NRK_ERROR;
for(i=0; i<ROUTE_TABLE_SIZE; i++ )
	{
	if((route_table[i].flags & VALID_MASK )!=0 && route_table[i].dst==dst )
		{
		   route_table[i].value[value_index]=value;	
		   return NRK_OK;
		}
	}
return NRK_ERROR;
}

int8_t route_table_value_get(uint16_t dst,uint8_t value_index)
{
uint8_t i;
if(value_index>=ROUTE_TABLE_VALUES ) return NRK_ERROR;
for(i=0; i<ROUTE_TABLE_SIZE; i++ )
	{
	if((route_table[i].flags & VALID_MASK) !=0 && route_table[i].dst==dst!=0)
		{
		   return (route_table[i].value[value_index]);
		}
	}
return NRK_ERROR;
}

// This function returns the next valid index including
// the entered value.
//
// For example if index 0 and 4 are valid
// route_table_get_next_valid_index(0) returns 0
// route_table_get_next_valid_index(1) returns 4
// route_table_get_next_valid_index(4) returns 4
// route_table_get_next_valid_index(5) returns -1 
int8_t route_table_get_next_valid_index(uint8_t start_index)
{
uint8_t i;
if(start_index>=ROUTE_TABLE_SIZE) return NRK_ERROR;
for(i=start_index; i<ROUTE_TABLE_SIZE; i++ )
	if((route_table[i].flags & VALID_MASK) !=0 )
		   return i; 
return NRK_ERROR;
}

uint16_t route_table_get_dst_by_index(int8_t index)
{
if(index>=ROUTE_TABLE_SIZE ) return 0;
return route_table[index].dst;

}
