CC = gcc
CFLAGS = -Wall -Wextra -g

# Header files
DEPS = message.h net_utils.h pollfd_array.h sockaddr_utils.h user_table.h uthash.h

# Object files
OBJS_COMMON = message.o net_utils.o pollfd_array.o sockaddr_utils.o
OBJS_CLIENT = client.o $(OBJS_COMMON)
OBJS_SERVER = server.o user_table.o $(OBJS_COMMON)

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