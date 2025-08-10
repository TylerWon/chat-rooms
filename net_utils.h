#define PORT "4000"

#define SEND_FLAGS 0
#define RECV_FLAGS 0

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
