CC = gcc
CFLAGS = -Wall -Wextra -g -MMD -MP  # -MMD generates .d files, -MP avoids errors if headers are deleted

# Object files
OBJS_COMMON =  data_structures/pollfd_array.o messages/chat_message.o messages/message.o messages/name_message.o utils/net_utils.o utils/sockaddr_utils.o
OBJS_CLIENT = client.o $(OBJS_COMMON)
OBJS_SERVER = server.o data_structures/user_table.o $(OBJS_COMMON)

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
