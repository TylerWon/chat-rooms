#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H

#include <stddef.h>

typedef int64_t TIMESTAMP;
typedef uint8_t NAME_LEN;
typedef uint16_t TEXT_LEN;

#define NAME_SIZE_LIMIT 50
#define TEXT_SIZE_LIMIT 1000

// Represents a message sent in a chat room
struct chat_message
{
    TIMESTAMP timestamp;
    char name[NAME_SIZE_LIMIT];
    char text[TEXT_SIZE_LIMIT];
};

/**
 * Serializes a chat message so it can be sent to the client/server. The buffer should be freed when it is no longer
 * needed.
 *
 * Message structure:
 * - message length (4 bytes)
 * - message type (1 byte)
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
int chat_message_serialize(struct chat_message *msg, char **buf, size_t *len);

/**
 * Deserializes a chat message received from the client/server.
 *
 * Message structure:
 * - message length (4 bytes)
 * - message type (1 byte)
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
int chat_message_deserialize(char *buf, struct chat_message *msg);

/**
 * Prints a message in the format: (hh:mm) [name]: [message].
 *
 * Example: (09:00) Tyler: Hello, world!
 *
 * @param msg   The message to print
 */
void chat_message_print(struct chat_message *msg);

#endif
