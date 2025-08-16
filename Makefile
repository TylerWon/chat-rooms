CC = gcc
CFLAGS = -Wall -Wextra -g -MMD -MP

# Find all source files
SRC_COMMON := $(wildcard data_structures/*.c types/*.c types/messages/*.c utils/*.c)
SRC_CLIENT := client.c $(SRC_COMMON)
SRC_SERVER := server.c $(SRC_COMMON)

# Convert .c -> .o
OBJS_CLIENT := $(patsubst %.c, %.o, $(SRC_CLIENT))
OBJS_SERVER := $(patsubst %.c, %.o, $(SRC_SERVER))

# Default target
all: client server

# Pattern rule
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

client: $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

server: $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o */*.o */*/*.o client server *.d */*.d */*/*.d

# Auto dependencies
-include $(OBJS_CLIENT:.o=.d) $(OBJS_SERVER:.o=.d)
