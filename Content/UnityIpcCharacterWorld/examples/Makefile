CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -O2

all: fifo_square udp_square

fifo_square: fifo_square.c
	$(CC) $(CFLAGS) fifo_square.c -o fifo_square

udp_square: udp_square.c
	$(CC) $(CFLAGS) udp_square.c -o udp_square

clean:
	rm -f fifo_square udp_square
