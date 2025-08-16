#ifndef JOIN_MESSAGE_H
#define JOIN_MESSAGE_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t ROOM_ID;

struct join_message
{
    ROOM_ID room_id;
};

/**
 * Serializes a join message so it can be sent to the client/server. The buffer should be freed when it is no longer
 * needed.
 *
 * Message structure:
 * - message length (4 bytes)
 * - message type (1 byte)
 * - room ID (1 byte)
 *
 * @param msg   The message to serialize
 * @param buf   Double pointer to a char buffer which will store the serialized message
 * @param len   Pointer to a size_t which will store the size of the buffer
 *
 * @return  0 on success.
 *          -1 on error.
 */
int join_message_serialize(struct join_message *msg, char **buf, size_t *len);

/**
 * Deserializes a join message received from the client/server.
 *
 * Message structure:
 * - message length (4 bytes)
 * - message type (1 byte)
 * - room ID (1 byte)
 *
 * @param buf   Pointer to a char buffer which contains the message
 * @param msg   Pointer to a message which will store the deserialized message
 */
void join_message_deserialize(char *buf, struct join_message *msg);

#endif
