#include <arpa/inet.h>
#include <stdlib.h>

#include "message.h"
#include "net_utils.h"

void *get_ip_address(struct sockaddr *sa) {
    // IPv4 address so cast to sockaddr_in
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr.s_addr);
    }    

    // IPv6 address so cast to sockaddr_in6
    return &(((struct sockaddr_in6 *) sa)->sin6_addr.s6_addr);
}


uint16_t get_port(struct sockaddr *sa) {
    // IPv4 address so cast to sockaddr_in
    if (sa->sa_family == AF_INET) {
        return ntohs(((struct sockaddr_in *) sa)->sin_port);
    }    

    // IPv6 address so cast to sockaddr_in6
    return ntohs(((struct sockaddr_in6 *) sa)->sin6_port);
}

ssize_t sendall(int sockfd, char *buf, size_t len) {
    ssize_t sent = 0;
    size_t total_sent = 0;

    while (total_sent < len) {
        sent = send(sockfd, buf+total_sent, len-total_sent, SEND_FLAGS);
        if (sent == -1) {
            return sent;
        }
        total_sent += sent;
    }

    return total_sent;
}

ssize_t recvall(int sockfd, char **buf) {
    // Get message length with first receive
    char *msg = malloc(MSG_LEN_SIZE);
    if (msg == NULL) {
        return -1;
    }

    ssize_t recvd = recv(sockfd, msg, MSG_LEN_SIZE, RECV_FLAGS);
    if (recvd <= 0) {
        free(msg);
        return recvd;
    }

    MSG_LEN msg_len = ntohl(*((MSG_LEN *) msg));

    // Get rest of message with remaining receives
    msg = realloc(msg, msg_len);
    if (msg == NULL) {
        return -1;
    }

    size_t total_recvd = recvd;
    while (total_recvd < msg_len) {
        recvd = recv(sockfd, msg+total_recvd, msg_len-total_recvd, RECV_FLAGS);
        if (recvd <= 0) {
            free(msg);
            return recvd;
        }
        total_recvd += recvd;
    }

    *buf = msg;

    return total_recvd;
}
