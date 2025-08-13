#ifndef MESSAGE_H
#define MESSAGE_H

#include <stddef.h>

typedef uint32_t TOTAL_MSG_LEN;
typedef int64_t TIMESTAMP;
typedef uint8_t NAME_LEN;
typedef uint16_t TEXT_LEN;

#define NAME_SIZE_LIMIT 50
#define TEXT_SIZE_LIMIT 1000

struct message
{
    TIMESTAMP timestamp;
    char name[NAME_SIZE_LIMIT];
    char text[TEXT_SIZE_LIMIT];
};

/**
 * Serializes a message to be sent the client/server into a buffer. The buffer should be freed when it is no longer
 * needed.
 *
 * Message structure:
 * - message length (4 bytes)
 * - timestamp (4 bytes)
 * - name length (1 byte)
 * - name (max 50 bytes)
 * - text length (2 bytes)
 * - text (max 1000 bytes)
 *
 * @param msg   The message to serialize
 * @param buf   Double pointer to a char buffer which will store the serialized message
 * @param len   Pointer to a size_t which will store the size of the buffer
 *
 * @return  0 on success.
 *          -1 on error (errno is set appropriately).
 */
int serialize(struct message *msg, char **buf, size_t *len);

/**
 * Deserializes a message received from the client/server from a buffer.
 *
 * Message structure:
 * - message length (4 bytes)
 * - timestamp (4 bytes)
 * - name length (1 byte)
 * - name (max 50 bytes)
 * - text length (2 bytes)
 * - text (max 1000 bytes)
 *
 * @param buf   Pointer to a char buffer which contains the message
 * @param msg   Pointer to a message which will store the deserialized message
 *
 * @return  0 on success.
 *          -1 on error (errno is set appropriately).
 */
int deserialize(char *buf, struct message *msg);

/**
 * Prints a message in the format: (hh:mm) [name]: [message].
 *
 * Example: (09:00) Tyler: Hello, world!
 *
 * @param msg   The message to print
 */
void print_message(struct message *msg);

#endif
