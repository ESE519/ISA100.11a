CC=gcc
CFLAGS=-I.

%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

all: main.o
	gcc -o config-eeprom main.o -I.
clean: 
	rm -f *.o *~ core config-eeprom 

