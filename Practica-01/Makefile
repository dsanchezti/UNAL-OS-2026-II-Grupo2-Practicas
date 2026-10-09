CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -g

all: fifo/servidor_fifo fifo/cliente_fifo

fifo: fifo/servidor_fifo fifo/cliente_fifo

fifo/servidor_fifo: fifo/servidor_fifo.c common/protocolo.h common/utilidades.h common/utilidades.c
	$(CC) $(CFLAGS) fifo/servidor_fifo.c common/utilidades.c -o fifo/servidor_fifo

fifo/cliente_fifo: fifo/cliente_fifo.c common/protocolo.h common/utilidades.h common/utilidades.c
	$(CC) $(CFLAGS) fifo/cliente_fifo.c common/utilidades.c -o fifo/cliente_fifo

clean:
	rm -f fifo/servidor_fifo fifo/cliente_fifo

.PHONY: all clean
