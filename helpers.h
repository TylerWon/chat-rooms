#define PORT "4000"

typedef uint32_t MSG_LEN;
typedef uint8_t NAME_LEN;
typedef uint16_t TEXT_LEN;

#define MSG_LEN_SIZE sizeof(MSG_LEN)
#define NAME_LEN_SIZE sizeof(NAME_LEN)
#define NAME_SIZE 50
#define TEXT_LEN_SIZE sizeof(TEXT_LEN)
#define TEXT_SIZE 1000

#define SEND_FLAGS 0
#define RECV_FLAGS 0

struct message {
    char name[NAME_SIZE];
    char text[TEXT_SIZE];
};

/* Get struct in_addr/in6_addr from a struct sockaddr. Use the sa_family field to determine if it's IPv4 or IPv6. */
void *get_in_addr(struct sockaddr *sa);

/**
 * Sends a message stored in buf on sockfd, handling partial sends so the entire messsage is delivered. 
 * 
 * On success, returns the number of bytes sent. -1 is returned on error and errno is set to indicate the error.
 */
ssize_t sendall(int sockfd, char *buf, size_t len);

/**
 * Receives a message on sockfd, handling partial receives so the entire message is obtained. Since messages have
 * variable lengths, *buf will be dynamically allocated based on the incoming message's size. 
 * 
 * On success, returns the number of bytes received and *buf will contain the message. -1 is returned on error and errno
 * is set to indicate the error. 0 is returned if the connection is closed.
 */
ssize_t recvall(int sockfd, char **buf);

/**
 * Serializes a message to be sent the client/server.
 * 
 * Message structure: 
 * message length (4 bytes) | name length (1 byte) | name (max 50 bytes) | text length (2 bytes) | text (max 1000 bytes)
 * 
 * On success, returns 0 and *buf will contain the serialized message while len will be its size in bytes. Otherwise,
 * -1 is returned and errno is set to indicate the error.
 */
int serialize(struct message *msg, char **buf, size_t *len);

/**
 * Deserializes a message received from the client/server.
 * 
 * Message structure: 
 * message length (4 bytes) | name length (1 byte) | name (max 50 bytes) | text length (2 bytes) | text (max 1000 bytes)
 * 
 * On success, returns 0 and msg will contain the deserialized message. Otherwise, -1 is returned and errno is set to 
 * indicate the error.
 */
int deserialize(char *buf, struct message *msg);

/**
 * Clears previous line from terminal.
 */
void clear_previous_line();