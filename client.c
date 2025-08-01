#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "helpers.h"

int main() {
    // Get address info for my IP, port 4000
    int status;
    struct addrinfo hints, *res;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // Stream socket
    hints.ai_flags = AI_PASSIVE;        // Get my IP

    if ((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
        printf("getaddrinfo error: %s\n", gai_strerror(status));
        return 1;
    }

    struct addrinfo *p;
    int sockfd;
    for (p = res; p != NULL; p = p->ai_next) {
        // Create socket from address info
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("socket error");
            continue;
        }

        // Connect to server
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) != 0) {
            perror("connect error");
            close(sockfd);
            return 1;
        }

        char ip[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, get_in_addr(p->ai_addr), ip, sizeof(ip));
        printf("connected to: %s, port %s\n", ip, PORT);

        break;
    }

    if (p == NULL) {
        printf("socket could not be created\n");
        return 1;
    }

    freeaddrinfo(res);

    return 0;
}