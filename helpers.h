#define PORT "4000"

typedef uint32_t MSG_LEN;
typedef uint16_t TEXT_LEN;

#define MSG_LEN_SIZE sizeof(MSG_LEN)
#define TEXT_LEN_SIZE sizeof(TEXT_LEN)
#define TEXT_SIZE 1000

struct message {
    char text[TEXT_SIZE];
};

/* Get struct in_addr/in5_addr from a struct sockaddr. Use the sa_family field to determine if it's IPv4 or IPv6. */
void *get_in_addr(struct sockaddr *sa);

/**
 * Wrapper around send() to handle partial sends. Returns the same values as send().
 */
ssize_t sendall(int sockfd, char *buf, size_t n, int flags);

/**
 * Wrapper around recv() to handle partial receives. Returns the same values as recv().
 */
ssize_t recvall(int sockfd, char **buf, int flags);

/**
 * Serializes a message to be sent the client/server.
 * 
 * Message structure: message length (4 bytes) | text length (2 bytes) | text (max 1000 bytes)
 * 
 * If successful, 0 will be returned and *buf will contain the serialized message while len will be its size. Otherwise, 
 * -1 is returned and errno is set to indicate the error.
 */
int serialize(struct message *msg, char **buf, size_t *len);

/**
 * Deserializes a message received from the client/server.
 * 
 * Message structure: message length (4 bytes) | text length (2 bytes) | text (max 1000 bytes)
 * 
 * If successful, 0 will be returned and msg will contain the deserialized message. Otherwise, -1 is returned and errno
 * is set to indicate the error.
 */
int deserialize(char *buf, struct message *msg);

/**
 * Clears previous line from terminal.
 */
void clear_previous_line();