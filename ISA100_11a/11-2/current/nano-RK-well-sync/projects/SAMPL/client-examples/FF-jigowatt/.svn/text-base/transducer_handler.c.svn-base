#include <globals.h>
#include <nrk.h>
#include <sampl.h>
#include <transducer_handler.h>
#include "../../transducer_pkts/transducer_registry.h"
#include "power_vars.h"


#define socket_0_enable()       nrk_gpio_set(NRK_DEBUG_0)
#define socket_0_disable()      nrk_gpio_clr(NRK_DEBUG_0)
#define socket_1_enable()       nrk_gpio_set(NRK_DEBUG_1)
#define socket_1_disable()      nrk_gpio_clr(NRK_DEBUG_1)

int8_t transducer_handler(uint8_t key, uint8_t value, TRANSDUCER_REPLY_PKT_T *pkt)
{
uint8_t v;
v=0;
printf( "key=%d value=%d\r\n", key,value );
if( key==TRAN_POWER_CTRL_SOCK_0 )
	{
	nrk_kprintf( PSTR( "SOCKET 0: " ));
	if(value == SOCKET_ON)
	{
		socket_0_enable();	
		socket_0_active=1;
		nrk_kprintf( PSTR( "enabled\r\n" ));
	}
	else
	{
		socket_0_disable();
		socket_0_active=0;
		nrk_kprintf( PSTR( "disabled\r\n" ));
	}
	pkt->type=TRAN_ACK;
	pkt->len=0;
	v=1;
	}

else if( key == TRAN_POWER_CTRL_SOCK_1 )
	{
	nrk_kprintf( PSTR( "SOCKET 1: " ));
	if(value == SOCKET_ON)
	{
		socket_1_enable();	
		nrk_kprintf( PSTR( "enabled\r\n" ));
	}
	else
	{
		socket_1_disable();
		nrk_kprintf( PSTR( "disabled\r\n" ));
	}
	pkt->type=TRAN_ACK;
	pkt->len=0;
	v=1;	
	}
else if( key == TRAN_POWER_SENSE )
	{
	pkt->type=TRAN_POWER_SENSE;
	pkt->len=24;
	pkt->payload[0]=(rms_current>>8)&0xff;
	pkt->payload[1]=rms_current&0xff;
	pkt->payload[2]=(rms_voltage>>8)&0xff;
	pkt->payload[3]=rms_voltage&0xff;
	pkt->payload[4]=freq&0xff;
	pkt->payload[5]=(true_power>>16)&0xff;
	pkt->payload[6]=(true_power>>8)&0xff;
	pkt->payload[7]=(true_power)&0xff;
	pkt->payload[8]=(tmp_energy>>24)&0xff;
	pkt->payload[9]=(tmp_energy>>16)&0xff;
	pkt->payload[10]=(tmp_energy>>8)&0xff;
	pkt->payload[11]=(tmp_energy)&0xff;
	pkt->payload[12]=(l_v_p2p_high>>8)&0xff;
	pkt->payload[13]=(l_v_p2p_high)&0xff;
	pkt->payload[14]=(l_v_p2p_low>>8)&0xff;
	pkt->payload[15]=(l_v_p2p_low)&0xff;
	pkt->payload[16]=(l_c_p2p_high>>8)&0xff;
	pkt->payload[17]=(l_c_p2p_high)&0xff;
	pkt->payload[18]=(l_c_p2p_low>>8)&0xff;
	pkt->payload[19]=(l_c_p2p_low)&0xff;
	pkt->payload[20]=(total_secs>>24)&0xff;
	pkt->payload[21]=(total_secs>>16)&0xff;
	pkt->payload[22]=(total_secs>>8)&0xff;
	pkt->payload[23]=(total_secs)&0xff;
	v=1;
	}
else {
	pkt->type=TRAN_NCK;
	pkt->len=0;
	v=1;
     }

return v;
}
