CC=gcc
CFLAGS=-I.

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

all: main.o
	gcc -o TimeScope main.o -I.
clean: 
	rm -f *.o *~ core TimeScope

