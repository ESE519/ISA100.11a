ROOT_DIR=../../../
CC=gcc
INCLUDE=-I. -I$(ROOT_DIR)include/ -I$(ROOT_DIR)app_pkt_handlers/

CFLAGS=-Wall $(INCLUDE)
SOURCES=main.c slipstream.c $(ROOT_DIR)include/pkt_packer.c $(ROOT_DIR)app_pkt_handlers/ack_pkt.c $(ROOT_DIR)app_pkt_handlers/ff_basic_sensor_pkt.c

OBJS=$(SOURCES:.c=.o)

all: simple-client 

default: simple-client 

simple-client: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *~ $(OBJS) simple-client $(ROOT_DIR)app_pkt_handlers/*.o 
