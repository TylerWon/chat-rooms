#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "net_utils.h"
#include "server.h"
#include "sockaddr_utils.h"

int get_server_addr_info(char *port, struct addrinfo **res) {
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // Stream socket
    hints.ai_flags = AI_PASSIVE;        // Setting this and the node parameter to NULL makes the wildcard address be returned

    return getaddrinfo(NULL, port, &hints, res); 
}

int create_listener_socket(struct addrinfo *res) {
    struct addrinfo *p;
    int sockfd;
    for (p = res; p != NULL; p = p->ai_next) {
        // Create socket from address info
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("failed to create listener socket");
            continue;
        }

        // Allow socket to reuse an address if it's already in use
        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("error occurred while attempting to allow listener socket to resuse addresses");
            return -1;
        }
        
        // Bind socket to address (my IP, port 4000)
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("failed to bind listener socket");
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
        perror("error while preparing listener socket to accept connections");
        exit(EXIT_FAILURE);
    }

    // Accept connection
    struct sockaddr client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int sockfd;
    if ((sockfd = accept(listener, &client_addr, &client_addr_size)) == -1) {
        perror("failed to accept connection");
        exit(EXIT_FAILURE);
    }

    char ip[INET6_ADDRSTRLEN];
    inet_ntop(client_addr.sa_family, get_ip_address(&client_addr), ip, sizeof(ip));
    printf("connection from: %s, port %d\n", ip, get_port(&client_addr));

    while (1) {
        // Receive messsage from client
        char *buf;
        ssize_t recvd = recvall(sockfd, &buf);
        if (recvd == -1) {
            printf("failed to receive message\n");
            continue;
        } else if (recvd == 0) {
            printf("connection closed\n");
            exit(EXIT_SUCCESS);
        }

        printf("received message\n");

        // Repeat message back to client
        if (sendall(sockfd, buf, recvd) == -1) {
            printf("failed to send message\n");
            continue;
        }

        printf("sent message back\n");

        free(buf);
        buf = NULL;
    }

    exit(EXIT_SUCCESS);
}