#ifndef _XMPP_PKT_H
#define _XMPP_PKT_H

#include <sampl.h>

#define XMPP_PASSWD_START_OFFSET	5
#define XMPP_TIMEOUT_OFFSET		4

typedef struct xmpp_pkt 
{
	uint8_t passwd_size;    // byte 0 
	uint8_t jid_size;	// byte 1 
	uint8_t msg_size;	// byte 2
	uint8_t timeout;	// byte 3
	char *passwd;		// byte 4
	char *jid;		// byte 4+passwd_size
	char *msg;  		// byte 4+passwd_size+jid_size
} XMPP_PKT_T;



int8_t xmpp_generate( SAMPL_UPSTREAM_PKT_T *pkt,SAMPL_DOWNSTREAM_PKT_T *ds_pkt);
uint8_t xmpp_pkt_pack( XMPP_PKT_T *p, uint8_t *buf, uint8_t index );
void  xmpp_pkt_unpack( XMPP_PKT_T *p, uint8_t *buf, uint8_t index );

#endif
