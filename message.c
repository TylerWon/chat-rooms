#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"

int serialize(struct message *msg, char **buf, size_t *len) {
    if (msg == NULL || buf == NULL || len == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Determine message length
    NAME_LEN name_len = strlen(msg->name) + 1; // +1 for null character
    TEXT_LEN text_len = strlen(msg->text) + 1; // +1 for null character
    MSG_LEN msg_len = MSG_LEN_SIZE + TIMESTAMP_SIZE + NAME_LEN_SIZE + name_len + TEXT_LEN_SIZE + text_len;
    
    char *b = malloc(msg_len); // Use b so we don't have to dereference *buf every time to get a single pointer
    if (b == NULL) {
        return -1;
    }

    *len = msg_len;
    *buf = b;

    // Write message length
    MSG_LEN msg_len_nbe = htonl(msg_len);
    memcpy(b, &msg_len_nbe, MSG_LEN_SIZE);
    b += MSG_LEN_SIZE;

    // Write timestamp
    TIMESTAMP timestamp_nbe = htonl(msg->timestamp);
    memcpy(b, &timestamp_nbe, TIMESTAMP_SIZE);
    b += TIMESTAMP_SIZE;

    // Write name length
    memcpy(b, &name_len, NAME_LEN_SIZE); // Don't need to convert name_len to Network Byte Order because it is one byte long
    b += NAME_LEN_SIZE;

    // Write name
    memcpy(b, &msg->name, name_len);
    b += name_len;

    // Write text length
    TEXT_LEN text_len_nbe = htons(text_len);
    memcpy(b, &text_len_nbe, TEXT_LEN_SIZE);
    b += TEXT_LEN_SIZE;

    // Write text
    memcpy(b, msg->text, text_len);

    return 0;
}

int deserialize(char *buf, struct message *msg) {
    if (buf == NULL || msg == NULL) {
        errno = EINVAL;
        return -1;
    }

    // Skip over message length
    buf += MSG_LEN_SIZE;

    // Get timestamp
    TIMESTAMP timestamp = ntohl(*(TIMESTAMP *) buf);
    memcpy(&msg->timestamp, &timestamp, TIMESTAMP_SIZE);
    buf += TIMESTAMP_SIZE;

    // Get name length
    NAME_LEN name_len = (*(NAME_LEN *) buf); // Don't need to convert name_len to Host Byte Order because it is one byte long
    buf += NAME_LEN_SIZE;
    
    // Get name
    memcpy(msg->name, buf, name_len);
    buf += name_len;

    // Get text length
    TEXT_LEN text_len = ntohs(*((TEXT_LEN *) buf));
    buf += TEXT_LEN_SIZE;

    // Get text 
    memcpy(msg->text, buf, text_len);

    return 0;
}
