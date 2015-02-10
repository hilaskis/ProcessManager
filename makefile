CC= gcc
CFLAGS= -Wall -g 

default: all

all: pcm svr proc

pcm: procMan.o
	$(CC) $(CFLAGS) procMan.c procMan.h -o pcm

svr: server.o
	$(CC) $(FLAGS) server.c procMan.h -o svr

proc: process.o
	$(CC) $(CFLAGS) process.c procMan.h -o proc

clean:
	$(RM) pcm svr proc *.o *~
