#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "helpers.h"

#define BACKLOG 10

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
    int listener;
    for (p = res; p != NULL; p = p->ai_next) {
        // Create socket from address info
        if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("socket error");
            continue;
        }

        // Allow socket to reuse an address if it's already in use
        int yes = 1;
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("setsockopt error");
            return 1;
        }
        
        // Bind socket to address (my IP, port 4000)
        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind error");
            close(listener);
            continue;
        }

        break;
    }

    if (p == NULL) {
        printf("socket could not be created\n");
        return 1;
    }

    freeaddrinfo(res);

    // Set-up socket to listen for incoming connections
    if (listen(listener, BACKLOG) == -1) {
        perror("listen error");
        return 1;
    }

    // Accept connection
    struct sockaddr client_addr;
    socklen_t client_addr_size;
    int sockfd;
    if ((sockfd = accept(listener, &client_addr, &client_addr_size) == -1)) {
        perror("accept error");
        return 1;
    }

    char ip[INET6_ADDRSTRLEN];
    inet_ntop(client_addr.sa_family, get_in_addr(&client_addr), ip, sizeof(ip));
    printf("connected to: %s, port %s\n", ip, PORT);

    return 0;
}