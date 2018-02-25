CC = gcc
CFLAGS = -g -Wall -std=c99

default: BISHell

BISHell: Command.o BISHell.o
	$(CC) $(CFLAGS) -o BISHell Command.o BISHell.o
	$(RM) *.o

BISHell.o: Command.h Utils.h BISHell.c
	$(CC) $(CFLAGS) -c BISHell.c

Command.o: Command.c Command.h Utils.h
	$(CC) $(CFLAGS) -c Command.c

clean: 
	$(RM) BISHell *.o