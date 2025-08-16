#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <stdlib.h>

#define PORT "4000"

#define SEND_FLAGS 0
#define RECV_FLAGS 0

/**
 * Sends a message stored in buf on the socket sockfd, handling partial sends so the entire messsage is delivered.
 *
 * @param sockfd    The socket to send the mesage on
 * @param buf       Pointer to a buffer containing the message
 * @param len       Length of the buffer in bytes
 *
 * @return  Number of bytes sent on success.
 *          -1 on error (errno is set appropriately).
 */
ssize_t sendall(int sockfd, char *buf, size_t len);

/**
 * Receives a message on sockfd, handling partial receives so the entire message is obtained. Since messages have
 * variable lengths, *buf will be dynamically allocated based on the incoming message's size and should be freed when
 * no longer needed.
 *
 * @param sockfd    The socket to receive the message on
 * @param buf       Double pointer to a char buffer which will store the message
 *
 * @return  Number of bytes received on success.
 *          0 when the peer closes the connection.
 *          -1 on error (errno is set appropriately).
 */
ssize_t recvall(int sockfd, char **buf);

#endif
