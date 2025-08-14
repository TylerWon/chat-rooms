#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "message.h"
#include "name_message.h"

int name_message_serialize(struct name_message *msg, char **buf, size_t *len)
{
    if (msg == NULL || buf == NULL || len == NULL)
        return -1;

    // Determine total message length
    NAME_LEN name_len = strlen(msg->name) + 1; // +1 for null character
    TOTAL_MSG_LEN total_len = sizeof(TOTAL_MSG_LEN) + sizeof(MSG_TYPE) + sizeof(NAME_LEN) + name_len;
    *len = total_len;

    // Allocate space for the buffer
    *buf = malloc(total_len);
    if (*buf == NULL)
        return -1;

    char *b = *buf; // Use b instead of *buf since we're going to be adding to it

    // Write total message length
    TOTAL_MSG_LEN total_len_nbe = htonl(total_len);
    memcpy(b, &total_len_nbe, sizeof(total_len_nbe));
    b += sizeof(total_len_nbe);

    // Write message type
    MSG_TYPE msg_type = NAME_MESSAGE;
    memcpy(b, &msg_type, sizeof(msg_type));
    b += sizeof(msg_type);

    // Write name length
    memcpy(b, &name_len, sizeof(name_len)); // Don't need to convert name_len to Network Byte Order because it is one byte long
    b += sizeof(name_len);

    // Write name
    memcpy(b, &msg->name, name_len);
    b += name_len;

    return 0;
}

int name_message_deserialize(char *buf, struct name_message *msg)
{
    if (buf == NULL || msg == NULL)
        return -1;

    // Skip over total message length and message type
    buf += sizeof(TOTAL_MSG_LEN) + sizeof(MSG_TYPE);

    // Get name length
    NAME_LEN name_len = (*(NAME_LEN *)buf); // Don't need to convert name_len to Host Byte Order because it is one byte long
    buf += sizeof(name_len);

    // Get name
    memcpy(msg->name, buf, name_len);
    buf += name_len;

    return 0;
}
