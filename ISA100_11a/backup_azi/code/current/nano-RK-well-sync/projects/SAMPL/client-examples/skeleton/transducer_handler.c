#include <globals.h>
#include <nrk.h>
#include <sampl.h>
#include <transducer_handler.h>
#include "../../transducer_pkts/transducer_registry.h"

int8_t transducer_handler(uint8_t key, uint8_t value, TRANSDUCER_REPLY_PKT_T *pkt)
{
printf( "Transducer %u got key: %u value %u\r\n",pkt->mac_addr, key,value );
pkt->type=TRAN_ACK;
pkt->len=0;
// pkt->payload[0]=1;

return 0;
}
