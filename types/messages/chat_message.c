#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "message.h"
#include "chat_message.h"
#include "../../lib/log.h"

int chat_message_serialize(struct chat_message *msg, char **buf, size_t *len)
{
    // Determine total message length
    NAME_LEN name_len = strlen(msg->name) + 1; // +1 for null character
    TEXT_LEN text_len = strlen(msg->text) + 1; // +1 for null character
    TOTAL_MSG_LEN total_len = sizeof(TOTAL_MSG_LEN) + sizeof(MSG_TYPE) + sizeof(msg->timestamp) + sizeof(NAME_LEN) + name_len + sizeof(TEXT_LEN) + text_len;
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
    MSG_TYPE msg_type = CHAT_MESSAGE;
    memcpy(b, &msg_type, sizeof(msg_type));
    b += sizeof(msg_type);

    // Write timestamp
    TIMESTAMP timestamp_nbe = htonl(msg->timestamp);
    memcpy(b, &timestamp_nbe, sizeof(timestamp_nbe));
    b += sizeof(timestamp_nbe);

    // Write name length
    memcpy(b, &name_len, sizeof(name_len)); // Don't need to convert name_len to Network Byte Order because it is one byte long
    b += sizeof(name_len);

    // Write name
    memcpy(b, &msg->name, name_len);
    b += name_len;

    // Write text length
    TEXT_LEN text_len_nbe = htons(text_len);
    memcpy(b, &text_len_nbe, sizeof(text_len));
    b += sizeof(text_len);

    // Write text
    memcpy(b, msg->text, text_len);

    return 0;
}

void chat_message_deserialize(char *buf, struct chat_message *msg)
{
    // Skip over total message length and message type
    buf += sizeof(TOTAL_MSG_LEN) + sizeof(MSG_TYPE);

    // Get timestamp
    TIMESTAMP timestamp = ntohl(*(TIMESTAMP *)buf);
    memcpy(&msg->timestamp, &timestamp, sizeof(timestamp));
    buf += sizeof(timestamp);

    // Get name length
    NAME_LEN name_len = (*(NAME_LEN *)buf); // Don't need to convert name_len to Host Byte Order because it is one byte long
    buf += sizeof(name_len);

    // Get name
    memcpy(msg->name, buf, name_len);
    buf += name_len;

    // Get text length
    TEXT_LEN text_len = ntohs(*((TEXT_LEN *)buf));
    buf += sizeof(text_len);

    // Get text
    memcpy(msg->text, buf, text_len);
}

void chat_message_print(struct chat_message *msg)
{
    struct tm *sent_timestamp = localtime(&msg->timestamp);
    printf("(%02d:%02d) %s: %s", sent_timestamp->tm_hour, sent_timestamp->tm_min, msg->name, msg->text);
}