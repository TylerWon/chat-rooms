#include <stdint.h>

typedef uint32_t MSG_LEN;
typedef int64_t TIMESTAMP;
typedef uint8_t NAME_LEN;
typedef uint16_t TEXT_LEN;

#define MSG_LEN_SIZE sizeof(MSG_LEN)
#define TIMESTAMP_SIZE sizeof(TIMESTAMP)
#define NAME_LEN_SIZE sizeof(NAME_LEN)
#define TEXT_LEN_SIZE sizeof(TEXT_LEN)

#define NAME_SIZE_LIMIT 50
#define TEXT_SIZE_LIMIT 1000

struct message {
    TIMESTAMP timestamp;
    char name[NAME_SIZE_LIMIT];
    char text[TEXT_SIZE_LIMIT];
};

/**
 * Serializes a message to be sent the client/server.
 * 
 * Message structure: 
 * - message length (4 bytes)
 * - timestamp (4 bytes)
 * - name length (1 byte)
 * - name (max 50 bytes)
 * - text length (2 bytes)
 * - text (max 1000 bytes) 
 * 
 * On success, returns 0 and *buf will contain the serialized message while len will be its size in bytes. Otherwise,
 * -1 is returned and errno is set to indicate the error.
 */
int serialize(struct message *msg, char **buf, size_t *len);

/**
 * Deserializes a message received from the client/server.
 * 
 * Message structure: 
 * - message length (4 bytes)
 * - timestamp (4 bytes)
 * - name length (1 byte)
 * - name (max 50 bytes)
 * - text length (2 bytes)
 * - text (max 1000 bytes) 
 * 
 * On success, returns 0 and msg will contain the deserialized message. Otherwise, -1 is returned and errno is set to 
 * indicate the error.
 */
int deserialize(char *buf, struct message *msg);
