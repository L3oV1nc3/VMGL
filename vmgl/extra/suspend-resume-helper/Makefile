## Copyright (c) H. Andres Lagar-Cavilla, University of Toronto, 2006-2007
## vmgl project
## See the file LICENSE.txt for information on redistributing this software.                                                                                 

CC=gcc
RM=rm
CFLAGS=-Wall

all: sr_daemon sr_client

sr_daemon: sr_daemon.c
	$(CC) $(CFLAGS) $@.c -o $@ 

sr_client: sr_client.c
	$(CC) $(CFLAGS) $@.c -o $@ 

clean:
	$(RM) -f sr_daemon
	$(RM) -f sr_client
