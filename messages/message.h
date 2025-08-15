#include <stdint.h>

typedef uint32_t TOTAL_MSG_LEN;
typedef uint8_t MSG_TYPE;

enum MessageType
{
    CHAT_MESSAGE,
    NAME_MESSAGE,
    INVALID_MESSAGE,
    REPLY_MESSAGE,
};

/**
 * Gets the message type from a buffer containing a message.
 *
 * In all messages, the message starts with the message length (4 bytes) and is followed by the message type (1 byte).
 *
 * @param buf   Pointer to a char buffer containing a message
 *
 * @return  The message type
 */
enum MessageType get_message_type(char *buf);
