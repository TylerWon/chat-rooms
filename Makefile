CC = gcc
CFLAGS = -Wall -Wextra -g
DEPS = helpers.h
OBJ = helpers.o

all: server client

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

server: server.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

client: client.o $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o server client
