CC = gcc
CFLAGS = -Wall -Wextra -g -MMD -MP  # -MMD generates .d files, -MP avoids errors if headers are deleted

# Object files
OBJS_COMMON = net_utils.o pollfd_array.o sockaddr_utils.o messages/chat_message.o
OBJS_CLIENT = client.o $(OBJS_COMMON)
OBJS_SERVER = server.o user_table.o $(OBJS_COMMON)

# Default target
all: client server

# Pattern rule for building .o files from .c files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

client: $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

server: $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o */*.o client server *.d */*.d

# Include auto-generated dependency files
-include $(OBJS_CLIENT:.o=.d) $(OBJS_SERVER:.o=.d)
