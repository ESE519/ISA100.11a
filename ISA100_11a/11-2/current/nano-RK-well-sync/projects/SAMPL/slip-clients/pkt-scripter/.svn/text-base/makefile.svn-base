CC=gcc
CFLAGS=-I.

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

all: main.o slipstream.o ../../include/pkt_packer.o
	gcc -o slip-client main.o ../../include/pkt_packer.o slipstream.o -I.
clean: 
	rm -f *.o *~ core slip-client 

