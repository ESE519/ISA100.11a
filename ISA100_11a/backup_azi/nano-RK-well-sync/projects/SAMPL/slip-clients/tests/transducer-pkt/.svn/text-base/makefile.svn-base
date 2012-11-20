ROOT_DIR=../../../
CC=gcc
INCLUDE=-I. -I$(ROOT_DIR)include/ -I$(ROOT_DIR)pkt_handlers/ -I$(ROOT_DIR)transducer_pkts/ 

CFLAGS=-Wall $(INCLUDE)
SOURCES=main.c slipstream.c $(ROOT_DIR)include/pkt_packer.c $(ROOT_DIR)pkt_handlers/ack_pkt.c $(ROOT_DIR)transducer_pkts/ff_basic_sensor_pkt.c $(ROOT_DIR)pkt_handlers/transducer_pkt.c

OBJS=$(SOURCES:.c=.o)

all: simple-client 

default: simple-client 

simple-client: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *~ $(OBJS) simple-client $(ROOT_DIR)app_pkt_handlers/*.o 
