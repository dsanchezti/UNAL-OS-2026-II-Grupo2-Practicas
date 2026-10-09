CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LDLIBS = -pthread -lrt

all: fifo/servidor_fifo fifo/cliente_fifo shm/servidor_shm shm/cliente_shm

fifo: fifo/servidor_fifo fifo/cliente_fifo

shm: shm/servidor_shm shm/cliente_shm

fifo/servidor_fifo: fifo/servidor_fifo.c common/protocolo.h common/utilidades.h common/utilidades.c common/csv.h common/csv.c
	$(CC) $(CFLAGS) fifo/servidor_fifo.c common/utilidades.c common/csv.c -o fifo/servidor_fifo $(LDLIBS)

fifo/cliente_fifo: fifo/cliente_fifo.c common/protocolo.h common/utilidades.h common/utilidades.c common/interfaz.h common/interfaz.c
	$(CC) $(CFLAGS) fifo/cliente_fifo.c common/interfaz.c common/utilidades.c -o fifo/cliente_fifo $(LDLIBS)

shm/servidor_shm: shm/servidor_shm.c shm/memoria_compartida.h common/protocolo.h common/csv.h common/csv.c
	$(CC) $(CFLAGS) shm/servidor_shm.c common/csv.c -o shm/servidor_shm $(LDLIBS)

shm/cliente_shm: shm/cliente_shm.c shm/memoria_compartida.h common/protocolo.h common/interfaz.h common/interfaz.c common/utilidades.h common/utilidades.c
	$(CC) $(CFLAGS) shm/cliente_shm.c common/interfaz.c common/utilidades.c -o shm/cliente_shm $(LDLIBS)

clean:
	rm -f fifo/servidor_fifo fifo/cliente_fifo shm/servidor_shm shm/cliente_shm

.PHONY: all fifo shm clean
