CC=gcc
CFLAGS=-I.

all: mutex_no_lock mutex_lock

mutex_lock : mutex_lock.c
	$(CC) -pthread -o mutex_lock mutex_lock.c $(CFLAGS)

mutex_no_lock : mutex_no_lock.c
	$(CC) -pthread -o mutex_no_lock mutex_no_lock.c $(CFLAGS)
