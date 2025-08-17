#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net_utils.h"
#include "../types/messages/message.h"
#include "../lib/log.h"

ssize_t sendall(int sockfd, char *buf, size_t len)
{
    ssize_t sent = 0;
    size_t total_sent = 0;

    while (total_sent < len)
    {
        sent = send(sockfd, buf + total_sent, len - total_sent, SEND_FLAGS);
        if (sent == -1)
        {
            LOG_ERROR("failed to send data to socket %d: %s", sockfd, strerror(errno));
            return -1;
        }

        total_sent += sent;
    }

    return total_sent;
}

ssize_t recvall(int sockfd, char **buf)
{
    // Get total message length with first receive
    char *msg = malloc(sizeof(TOTAL_MSG_LEN));
    if (msg == NULL)
    {
        LOG_ERROR("failed to allocate space for buffer");
        return -1;
    }

    ssize_t recvd = recv(sockfd, msg, sizeof(TOTAL_MSG_LEN), RECV_FLAGS);
    if (recvd <= 0)
    {
        free(msg);
        if (recvd == 0 || (recvd == -1 && errno == ECONNRESET)) // 0 = graceful close, -1 with ECONNRESET = abrupt close
        {
            LOG_INFO("connection to socket %d terminated", sockfd);
            return 0;
        }
        else
        {
            LOG_ERROR("failed to receive data from socket %d: %s", sockfd, strerror(errno));
            return -1;
        }
    }

    TOTAL_MSG_LEN total_len = ntohl(*((TOTAL_MSG_LEN *)msg));

    // Get rest of message with remaining receives
    msg = realloc(msg, total_len);
    if (msg == NULL)
    {
        LOG_ERROR("failed to reallocate space for buffer");
        return -1;
    }

    size_t total_recvd = recvd;
    while (total_recvd < total_len)
    {
        recvd = recv(sockfd, msg + total_recvd, total_len - total_recvd, RECV_FLAGS);
        if (recvd <= 0)
        {
            free(msg);
            if (recvd == 0 || (recvd == -1 && errno == ECONNRESET)) // 0 = graceful close, -1 with ECONNRESET = abrupt close
            {
                LOG_INFO("connection to socket %d terminated", sockfd);
                return 0;
            }
            else
            {
                LOG_ERROR("failed to receive data from socket %d: %s", sockfd, strerror(errno));
                return -1;
            }
        }
        total_recvd += recvd;
    }

    *buf = msg;

    return total_recvd;
}
