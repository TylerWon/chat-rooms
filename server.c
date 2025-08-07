#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "net_utils.h"

#define BACKLOG_LIMIT 10

/**
 * Gets the address info of the server for the given port. The IP address will be the wildcard address so connections
 * can be accpeted on any of the host's network addresses.
 * 
 * On success, returns 1 and stores the address info in *res which is a linked list of struct addrinfos. Otherwise, 
 * returns a non-zero error code (same codes as getaddrinfo()).
 */
int get_server_addr_info(char *port, struct addrinfo **res) {
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // Stream socket
    hints.ai_flags = AI_PASSIVE;        // Setting this and the node parameter to NULL makes the wildcard address be returned

    return getaddrinfo(NULL, port, &hints, res); 
}

/**
 * Creates a socket for listening to incoming connections on the address provided in res (a linked list of struct 
 * addrinfos).
 * 
 * On success, returns the socket file descriptor. Otherwise, returns -1. 
 */
int create_listener_socket(struct addrinfo *res) {
    struct addrinfo *p;
    int sockfd;
    for (p = res; p != NULL; p = p->ai_next) {
        // Create socket from address info
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("socket error");
            continue;
        }

        // Allow socket to reuse an address if it's already in use
        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("setsockopt error");
            return -1;
        }
        
        // Bind socket to address (my IP, port 4000)
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind error");
            close(sockfd);
            continue;
        }

        return sockfd;
    }

    return -1;
}

int main() {
    int status;
    struct addrinfo *res;
    if ((status = get_server_addr_info(PORT, &res)) != 0) {
        printf("failed to get server's address info: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    int listener;
    if ((listener = create_listener_socket(res)) == -1) {
        printf("failed to create listener socket\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    res = NULL;

    // Set-up socket to listen for incoming connections
    if (listen(listener, BACKLOG_LIMIT) == -1) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    // Accept connection
    struct sockaddr client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int sockfd;
    if ((sockfd = accept(listener, &client_addr, &client_addr_size)) == -1) {
        perror("accept error");
        exit(EXIT_FAILURE);
    }

    char ip[INET6_ADDRSTRLEN];
    inet_ntop(client_addr.sa_family, get_in_addr(&client_addr), ip, sizeof(ip));
    printf("connected to: %s, port %s\n", ip, PORT);

    while (1) {
        // Receive messsage from client
        char *buf;
        ssize_t recvd = recvall(sockfd, &buf);
        if (recvd == -1) {
            perror("receive error");
            continue;
        } else if (recvd == 0) {
            printf("connection closed\n");
            exit(EXIT_SUCCESS);
        }

        printf("received message\n");

        // Repeat message back to client
        if (sendall(sockfd, buf, recvd) == -1) {
            perror("send error");
            continue;
        }

        printf("sent message back\n");

        free(buf);
        buf = NULL;
    }

    exit(EXIT_SUCCESS);
}