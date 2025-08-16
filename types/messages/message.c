#include "message.h"

enum MessageType get_message_type(char *buf)
{
    buf += sizeof(TOTAL_MSG_LEN);

    switch (*buf)
    {
    case CHAT_MESSAGE:
        return CHAT_MESSAGE;
    case NAME_MESSAGE:
        return NAME_MESSAGE;
    case JOIN_MESSAGE:
        return JOIN_MESSAGE;
    case REPLY_MESSAGE:
        return REPLY_MESSAGE;
    default:
        return INVALID_MESSAGE;
    }
}