/* This file implements the link layer of the network stack */

/* Authors:
   Aditya Bhave
*/

#include "LinkLayer.h"

int8_t initialise_link_layer(uint8_t channel)
{
	return bmac_init(channel);
}