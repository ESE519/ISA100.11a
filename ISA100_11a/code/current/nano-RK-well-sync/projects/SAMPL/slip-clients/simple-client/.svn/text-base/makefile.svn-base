CC=gcc
INCLUDE=-I. -I../../include/ -I../../pkt_handlers/  -I../../transducer_pkts/

CFLAGS=-Wall $(INCLUDE)
SOURCES=main.c slipstream.c ../../include/pkt_packer.c ../../pkt_handlers/ack_pkt.c ../../transducer_pkts/ff_basic_sensor_pkt.c ../../pkt_handlers/transducer_pkt.c


OBJS=$(SOURCES:.c=.o)

all: simple-client 

default: simple-client 

simple-client: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *~ $(OBJS) simple-client ../../pkt_handlers/*.o ../../transducer_pkts/*.o 
