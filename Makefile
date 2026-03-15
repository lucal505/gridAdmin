CC = gcc
CFLAGS = -Wall -Iinclude

all: gridServer gridController

gridServer: src/gridServer.c
	$(CC) $(CFLAGS) -o gridServer src/gridServer.c -lpthread -lsqlite3

gridController: src/gridController.c
	$(CC) $(CFLAGS) -o gridController src/gridController.c

clean:
	rm -f gridServer gridController
