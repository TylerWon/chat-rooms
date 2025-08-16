#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"
#include "reply_message.h"
#include "../../lib/log.h"

int reply_message_serialize(struct reply_message *msg, char **buf, size_t *len)
{
    // Determine total message length
    REPLY_LEN reply_len = strlen(msg->reply) + 1; // +1 for null character
    TOTAL_MSG_LEN total_len = sizeof(TOTAL_MSG_LEN) + sizeof(MSG_TYPE) + sizeof(REPLY_LEN) + reply_len;
    *len = total_len;

    // Allocate space for the buffer
    *buf = malloc(total_len);
    if (*buf == NULL)
    {
        LOG_DEBUG("failed to allocate space for buffer\n");
        return -1;
    }

    char *b = *buf; // Use b instead of *buf since we're going to be adding to it

    // Write total message length
    TOTAL_MSG_LEN total_len_nbe = htonl(total_len);
    memcpy(b, &total_len_nbe, sizeof(total_len_nbe));
    b += sizeof(total_len_nbe);

    // Write message type
    MSG_TYPE msg_type = REPLY_MESSAGE;
    memcpy(b, &msg_type, sizeof(msg_type));
    b += sizeof(msg_type);

    // Write reply length
    memcpy(b, &reply_len, sizeof(reply_len)); // Don't need to convert reply_len to Network Byte Order because it is one byte long
    b += sizeof(reply_len);

    // Write reply
    memcpy(b, &msg->reply, reply_len);

    return 0;
}

void reply_message_deserialize(char *buf, struct reply_message *msg)
{
    // Skip over total message length and message type
    buf += sizeof(TOTAL_MSG_LEN) + sizeof(MSG_TYPE);

    // Get reply length
    REPLY_LEN reply_len = (*(REPLY_LEN *)buf); // Don't need to convert reply_len to Host Byte Order because it is one byte long
    buf += sizeof(reply_len);

    // Get reply
    memcpy(msg->reply, buf, reply_len);
}
