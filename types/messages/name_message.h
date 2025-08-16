#ifndef NAME_MESSAGE_H
#define NAME_MESSAGE_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t NAME_LEN;

#define NAME_SIZE_LIMIT 50

struct name_message
{
    char name[NAME_SIZE_LIMIT];
};

/**
 * Serializes a name message so it can be sent to the client/server. The buffer should be freed when it is no longer
 * needed.
 *
 * Message structure:
 * - message length (4 bytes)
 * - message type (1 byte)
 * - name length (1 byte)
 * - name (max 50 bytes)
 *
 * @param msg   The message to serialize
 * @param buf   Double pointer to a char buffer which will store the serialized message
 * @param len   Pointer to a size_t which will store the size of the buffer
 *
 * @return  0 on success.
 *          -1 on error.
 */
int name_message_serialize(struct name_message *msg, char **buf, size_t *len);

/**
 * Deserializes a name message received from the client/server.
 *
 * Message structure:
 * - message length (4 bytes)
 * - message type (1 byte)
 * - name length (1 byte)
 * - name (max 50 bytes)
 *
 * @param buf   Pointer to a char buffer which contains the message
 * @param msg   Pointer to a message which will store the deserialized message
 */
void name_message_deserialize(char *buf, struct name_message *msg);

#endif
