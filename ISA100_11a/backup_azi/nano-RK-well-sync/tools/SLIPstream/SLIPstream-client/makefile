CC=gcc
CFLAGS=-I.

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

all: main.o slipstream.o
	gcc -o sample-client main.o slipstream.o -I.
clean: 
	rm -f *.o *~ core sample-client 

