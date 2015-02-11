CC= gcc
CFLAGS= -Wall -g

default: all

all: pcm svr

pcm: procMan.o
	$(CC) $(CFLAGS) procMan.c procMan.h -o pcm

svr: server.o
	$(CC) $(FLAGS) server.c procMan.h -o svr

clean:
	$(RM) pcm svr *.o *~
