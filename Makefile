CC = gcc
CFLAGS = -Wall -Wextra -g

# Header files
DEPS = message.h net_utils.h

# Object files
OBJS_COMMON = net_utils.o
OBJS_CLIENT = client.o message.o $(OBJS_COMMON)
OBJS_SERVER = server.o $(OBJS_COMMON)

# Default target
all: client server

# Pattern rule for building .o files from .c files
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

client: $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

server: $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o client server