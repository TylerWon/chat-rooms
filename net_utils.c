#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "message.h"
#include "net_utils.h"

ssize_t sendall(int sockfd, char *buf, size_t len)
{
    ssize_t sent = 0;
    size_t total_sent = 0;

    while (total_sent < len)
    {
        sent = send(sockfd, buf + total_sent, len - total_sent, SEND_FLAGS);
        if (sent == -1)
        {
            perror("error occurred while sending message");
            return sent;
        }
        total_sent += sent;
    }

    return total_sent;
}

ssize_t recvall(int sockfd, char **buf)
{
    // Get message length with first receive
    char *msg = malloc(MSG_LEN_SIZE);
    if (msg == NULL)
    {
        printf("failed to allocate space for message\n");
        return -1;
    }

    ssize_t recvd = recv(sockfd, msg, MSG_LEN_SIZE, RECV_FLAGS);
    if ((recvd == -1 && errno == ECONNRESET) || recvd == 0)
    { // 0 = graceful close, -1 with ECONNRESET = abrupt close
        printf("connection to socket %d closed\n", sockfd);
        free(msg);
        return 0;
    }
    else if (recvd == -1)
    {
        perror("error occurred while receiving message length");
        free(msg);
        return -1;
    }

    MSG_LEN msg_len = ntohl(*((MSG_LEN *)msg));

    // Get rest of message with remaining receives
    msg = realloc(msg, msg_len);
    if (msg == NULL)
    {
        printf("failed to allocate more space for message\n");
        return -1;
    }

    size_t total_recvd = recvd;
    while (total_recvd < msg_len)
    {
        recvd = recv(sockfd, msg + total_recvd, msg_len - total_recvd, RECV_FLAGS);
        if ((recvd == -1 && errno == ECONNRESET) || recvd == 0)
        {
            printf("connection to socket %d closed\n", sockfd);
            free(msg);
            return 0;
        }
        else if (recvd == -1)
        {
            perror("error occurred while receiving message length");
            free(msg);
            return -1;
        }
        total_recvd += recvd;
    }

    *buf = msg;

    return total_recvd;
}
