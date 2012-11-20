#include "neighbor_list.h"
#include <nrk_error.h>
#include <nrk_time.h>

nrk_time_t last_time;
nrk_time_t current_time;
nrk_time_t elapsed_time;

int8_t neighbor_list_init()
{
uint8_t i,j;

for(i=0; i<NEIGHBOR_LIST_SIZE; i++ )
	{
	neighbor_table[i].flags=0;	
	neighbor_table[i].mac=0;	
	neighbor_table[i].rssi=0;	
	neighbor_table[i].ttl=0;	
	}
nrk_time_get(&last_time);
}

int8_t neighbor_list_add( uint16_t mac, int8_t rssi, uint16_t ttl)
{
uint8_t i; 
int8_t found;
found=-1;
neighbor_list_ttl_update();
for(i=0; i<NEIGHBOR_LIST_SIZE; i++ )
	{
	  if(neighbor_table[i].mac==mac)
		{
			neighbor_table[i].mac=mac;
			neighbor_table[i].ttl=ttl;
			neighbor_table[i].rssi=rssi;
			neighbor_table[i].flags|=VALID_MASK;
			return NRK_OK;
		}
	  if(neighbor_table[i].ttl==0 && found==-1) found=i;
	}

if(found>=0)
	{
	   neighbor_table[found].mac=mac;
	   neighbor_table[found].ttl=ttl;
	   neighbor_table[found].rssi=rssi;
	   neighbor_table[found].flags|=VALID_MASK;
	   return NRK_OK;
	}
return NRK_ERROR;

}

void neighbor_list_ttl_update()
{
uint8_t i;
int8_t v;
nrk_time_get(&current_time);
v=nrk_time_sub(&elapsed_time, current_time, last_time);
last_time.secs=current_time.secs;
last_time.nano_secs=current_time.nano_secs;
if(v==NRK_OK)
	{
	for(i=0; i<NEIGHBOR_LIST_SIZE; i++ )
		{
		  if(neighbor_table[i].ttl!=0)
		  {
		    // stricly larger than so it doesn't equal 0
		    // since 0 is used to disable ttl
		    if(neighbor_table[i].ttl>elapsed_time.secs)
			neighbor_table[i].ttl-=elapsed_time.secs;
		    else
			{
			neighbor_table[i].flags=0;
			neighbor_table[i].ttl=0;
			neighbor_table[i].mac=0;
			neighbor_table[i].rssi=0;
			}
		  }
		}
	}
}



// This function returns the next valid index including
// the entered value.
//
// For example if index 0 and 4 are valid
// neighbor_table_get_next_valid_index(0) returns 0
// neighbor_table_get_next_valid_index(1) returns 4
// neighbor_table_get_next_valid_index(4) returns 4
// neighbor_table_get_next_valid_index(5) returns -1 
int8_t neighbor_list_get_next_valid_index(uint8_t start_index)
{
uint8_t i;
if(start_index>=NEIGHBOR_LIST_SIZE ) return NRK_ERROR;
for(i=start_index; i<NEIGHBOR_LIST_SIZE; i++ )
	if((neighbor_table[i].flags & VALID_MASK) !=0 )
		   return i; 
return NRK_ERROR;
}

uint16_t neighbor_list_get_mac(int8_t index)
{
if(index>=NEIGHBOR_LIST_SIZE) return 0;
return neighbor_table[index].mac;

}

int8_t neighbor_list_get_rssi(int8_t index)
{
if(index>=NEIGHBOR_LIST_SIZE) return 0;
return neighbor_table[index].rssi;
}

uint16_t neighbor_list_get_ttl(int8_t index)
{
if(index>=NEIGHBOR_LIST_SIZE) return 0;
return neighbor_table[index].ttl;
}
