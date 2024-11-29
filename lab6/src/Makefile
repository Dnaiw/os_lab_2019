CC=gcc
CFLAGS=-I.
CDBG +=
CFLAGS += $(CDBG)

all: client server 

client : client.o
	$(CC) client.o -pthread -Llib -lmodulo_factorial -o client $(CFLAGS)

server : server.o
	$(CC) server.o -pthread -Llib -lmodulo_factorial -o server $(CFLAGS)

client.o: 
	$(CC) -c client_completed.c -o client.o $(CFLAGS)

server.o: 
	$(CC) -c server_completed.c -o server.o $(CFLAGS)

modulo_factorial_lib : libmodulo_factorial.o 
	ar rcs lib/libmodulo_factorial.a  lib/libmodulo_factorial.o

libmodulo_factorial.o : lib/modulo_factorial.h lib/modulo_factorial.c
	$(CC) -c lib/modulo_factorial.c -o lib/libmodulo_factorial.o $(CFLAGS)