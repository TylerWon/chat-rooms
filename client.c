#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>

#include "helpers.h"

#define INPUT_LIMIT 65536 // 2^16 - 1 

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

    while (1) {
        // Read user input from STDIN
        char input[INPUT_LIMIT]; 
        if (fgets(input, INPUT_LIMIT, stdin) == NULL) {
            perror("failed to read user input");
            continue;
        }

        clear_previous_line();

        printf("read input\n");

        // Create message
        struct message msg;
        msg.text = input;
        
        // Serialize message
        char *buf;
        size_t len;
        if (serialize(&msg, &buf, &len) != 0) {
            perror("serialization error");
            continue;
        }

        printf("serialized message\n");

        // Send message to server
        if (sendall(sockfd, buf, len, 0) == -1) {
            perror("send error");
            continue;
        }
        printf("sent: %s", msg.text);

        // Receive reply from server
        free(buf);
        ssize_t recvd = recvall(sockfd, &buf, 0);
        if (recvd == -1) {
            perror("receive error");
            continue;
        } else if (recvd == 0) {
            printf("connection closed\n");
            return 1;
        }

        printf("received message\n");

        // Deserialize message
        struct message *reply;
        if (deserialize(buf, &reply) != 0) {
            perror("deserialization error");
            continue;
        }

        printf("deserialized message: %s", reply->text);

        free(buf);
        free(reply->text);
        free(reply);
    }

    return 0;
}