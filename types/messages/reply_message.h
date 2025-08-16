#ifndef REPLY_MESSAGE_H
#define REPLY_MESSAGE_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t REPLY_LEN;

#define REPLY_SIZE_LIMIT 100

struct reply_message
{
    char reply[REPLY_SIZE_LIMIT];
};

/**
 * Serializes a reply message so it can be sent to the client. The buffer should be freed when it is no longer needed.
 *
 * Message structure:
 * - message length (4 bytes)
 * - message type (1 byte)
 * - reply length (1 byte)
 * - reply (max 100 bytes)
 *
 * @param msg   The message to serialize
 * @param buf   Double pointer to a char buffer which will store the serialized message
 * @param len   Pointer to a size_t which will store the size of the buffer
 *
 * @return  0 on success.
 *          -1 on error.
 */
int reply_message_serialize(struct reply_message *msg, char **buf, size_t *len);

/**
 * Deserializes a reply message received from the client/server.
 *
 * Message structure:
 * - message length (4 bytes)
 * - message type (1 byte)
 * - reply length (1 byte)
 * - reply (max 100 bytes)
 *
 * @param buf   Pointer to a char buffer which contains the message
 * @param msg   Pointer to a message which will store the deserialized message
 */
void reply_message_deserialize(char *buf, struct reply_message *msg);

#endif
