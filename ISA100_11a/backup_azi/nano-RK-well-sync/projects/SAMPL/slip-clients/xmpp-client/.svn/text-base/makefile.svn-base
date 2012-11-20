XMPP_LIBRARY_PATH=/home/agr/xmpp/xmpp_lib_c/
include $(XMPP_LIBRARY_PATH)/vars.mk

CC=gcc
LIBS=-llm-library -lgthread-2.0 -lm
INCLUDE=-I. -I$(GLIB_INCLUDE) -I$(GLIB_LIB_INCLUDE) -I$(LOUDMOUTH_INCLUDE) -I$(XMPP_LIBRARY_PATH) -I../../include/ -I../../app_pkt_handlers/

CFLAGS=-Wall $(INCLUDE)
LDFLAGS=-L. -L$(XMPP_LIBRARY_PATH) -L$(GLIB_LIB) $(LIBS)
SOURCES=main.c slipstream.c ../../include/pkt_packer.c node_list.c sensor_cal.c ../../app_pkt_handlers/ping_pkt.c ../../app_pkt_handlers/xmpp_pkt.c ../../app_pkt_handlers/ack_pkt.c xmpp_pkt_writer.c  ../../app_pkt_handlers/ff_basic_sensor_pkt.c
OBJS=$(SOURCES:.c=.o)

all: gateway_client 

default: gateway_client 

gateway_client: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) -g -c $< -o $@

clean:
	rm -rf *~ $(OBJS) gateway_client
