#include <sampl.h>
#include <xmpp_pkt.h>
#include <ack_pkt.h>

#ifdef NANORK 
#include <nrk.h>
#include <nrk_error.h>
#else
#define my_mac 0
#define RF_MAX_PAYLOAD_SIZE  128
#endif



int8_t xmpp_generate(SAMPL_UPSTREAM_PKT_T *pkt, SAMPL_DOWNSTREAM_PKT_T *ds_pkt)
{
   // For downstream messages, generate an ACK reply to show
   // where the message has traveled 

   return ack_generate(pkt);  
}


void xmpp_pkt_unpack( XMPP_PKT_T *p, uint8_t *buf, uint8_t index )
{
   p->passwd_size=strlen(&buf[XMPP_PASSWD_START_OFFSET])+1;
   p->jid_size=strlen(&buf[XMPP_PASSWD_START_OFFSET+p->passwd_size])+1;
   p->msg_size=strlen(&buf[XMPP_PASSWD_START_OFFSET+p->passwd_size+p->jid_size]);
   p->timeout=buf[XMPP_TIMEOUT_OFFSET];
   p->passwd=&(buf[XMPP_PASSWD_START_OFFSET]);
   p->jid=&(buf[XMPP_PASSWD_START_OFFSET+p->passwd_size]);
   p->msg=&(buf[XMPP_PASSWD_START_OFFSET+p->passwd_size+p->jid_size]);
}


uint8_t xmpp_pkt_pack( XMPP_PKT_T *p, uint8_t *buf, uint8_t index )
{
uint8_t i;

   buf[0]=strlen(p->passwd)+1;  
   buf[1]=strlen(p->jid)+1;  
   buf[2]=strlen(p->msg)+1; 
   buf[3]=p->timeout;
   p->passwd_size=buf[0];
   p->jid_size=buf[1];
   p->msg_size=buf[2];
   for(i=0; i<buf[0]; i++ )
	buf[XMPP_PASSWD_START_OFFSET+i]=p->passwd[i]; 
   for(i=0; i<buf[1]; i++ )
	buf[XMPP_PASSWD_START_OFFSET+p->passwd_size+i]=p->jid[i]; 
   for(i=0; i<buf[2]; i++ )
	buf[XMPP_PASSWD_START_OFFSET+p->passwd_size+p->jid_size+i]=p->msg[i]; 

   return (XMPP_PASSWD_START_OFFSET+p->passwd_size+p->jid_size+p->msg_size);
}

