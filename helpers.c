#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

void *get_in_addr(struct sockaddr *sa) {
    // IPv4 address so cast to sockaddr_in to get in_addr
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }    

    // IPv6 address so cast to sockaddr_in6 to get in6_addr
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

ssize_t sendall(int sockfd, char *buf, size_t n, int flags) {
    ssize_t sent = 0;
    size_t total_sent = 0;
    while (total_sent < n) {
        sent = send(sockfd, buf+total_sent, n-total_sent, flags);
        if (sent == -1) {
            return sent;
        }
        total_sent += sent;
    }

    return total_sent;
}

ssize_t recvall(int sockfd, char **buf, int flags) {
    // Get message length with first receive
    char *msg = malloc(MSG_LEN_SIZE);
    ssize_t recvd = recv(sockfd, msg, MSG_LEN_SIZE, flags);
    if (recvd <= 0) {
        return recvd;
    }
    MSG_LEN msg_len = ntohl(*((MSG_LEN *) msg));

    // Get rest of message with remaining receives
    msg = realloc(msg, msg_len);
    size_t total_recvd = recvd;
    while (total_recvd < msg_len) {
        recvd = recv(sockfd, msg+total_recvd, msg_len-total_recvd, flags);
        if (recvd <= 0) {
            return recvd;
        }
        total_recvd += recvd;
    }

    *buf = msg;

    return total_recvd;
}

int serialize(struct message *msg, char **buf, size_t *len) {
    // Determine message length
    size_t text_size = strlen(msg->text) + 1;   // +1 for null character
    size_t msg_len = MSG_LEN_SIZE + TEXT_LEN_SIZE + text_size;
    
    *len = msg_len;
    char *p = malloc(msg_len);
    *buf = p;

    // Write message length
    msg_len = htonl(msg_len);
    memcpy(p, &msg_len, MSG_LEN_SIZE);
    p += MSG_LEN_SIZE;

    // Write text length
    uint16_t text_len = htons(text_size);
    memcpy(p, &text_len, TEXT_LEN_SIZE);
    p += TEXT_LEN_SIZE;

    // Write text
    memcpy(p, msg->text, text_size);

    return 0;
}

int deserialize(char *buf, struct message **msg) {
    struct message *m = malloc(sizeof(struct message));

    // Skip over message length
    buf += MSG_LEN_SIZE;

    // Get text length
    TEXT_LEN text_len = ntohs(*((TEXT_LEN *) buf));
    buf += TEXT_LEN_SIZE;

    // Get text 
    m->text = malloc(text_len);
    memcpy(m->text, buf, text_len);

    *msg = m;

    return 0;
}

void clear_previous_line() {
    printf("\033[A");   // Move cursor up one line
    printf("\033[2K");  // Clear the entire line
}