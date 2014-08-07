CC = gcc
CFLAGS = -Wall -g
HEADERS = 

all: cfg_parser

depend:
	sudo apt-get install libconfig8-dev

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

cfg_parser: cfg_parser.o
	$(CC) $(CFLAGS) -o $@ $^ -lconfig

clean:
	rm *.o cfg_parser

.PHONY: all depend clean
