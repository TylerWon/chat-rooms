#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "join_message.h"
#include "../../lib/log.h"

int join_message_serialize(struct join_message *msg, char **buf, size_t *len)
{
    // Determine total message length
    TOTAL_MSG_LEN total_len = sizeof(TOTAL_MSG_LEN) + sizeof(MSG_TYPE) + sizeof(ROOM_ID);
    *len = total_len;

    // Allocate space for the buffer
    *buf = malloc(total_len);
    if (*buf == NULL)
    {
        LOG_ERROR("failed to allocate space for buffer");
        return -1;
    }

    char *b = *buf; // Use b instead of *buf since we're going to be adding to it

    // Write total message length
    TOTAL_MSG_LEN total_len_nbe = htonl(total_len);
    memcpy(b, &total_len_nbe, sizeof(total_len_nbe));
    b += sizeof(total_len_nbe);

    // Write message type
    MSG_TYPE msg_type = JOIN_MESSAGE;
    memcpy(b, &msg_type, sizeof(msg_type));
    b += sizeof(msg_type);

    // Write room ID
    memcpy(b, &msg->room_id, sizeof(msg->room_id)); // Don't need to convert room_id to Network Byte Order because it is one byte long

    return 0;
}

void join_message_deserialize(char *buf, struct join_message *msg)
{
    // Skip over total message length and message type
    buf += sizeof(TOTAL_MSG_LEN) + sizeof(MSG_TYPE);

    // Get room ID
    memcpy(&msg->room_id, buf, sizeof(msg->room_id)); // Don't need to convert room_id to Host Byte Order because it is one byte long
}
