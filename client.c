#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "message.h"
#include "net_utils.h"

#define DEFAULT_NAME "anonymous"
#define COMMAND_SIZE 5

/**
 * Clears previous line from terminal.
 */
void clear_previous_line() {
    printf("\033[A");   // Move cursor up one line
    printf("\033[2K");  // Clear the entire line
}

/**
 * Executes the command in str if it is a valid command.
 * 
 * Commands:
 * - /name [name] - Sets the user's name to [name]
 * - /exit - Exits the application
 */
void execute_command(char *str, char *name) {
    char command[COMMAND_SIZE];
    if (sscanf(str, "/%5s", command) != 1) {
        printf("command not provided\n");
        return;
    }
    
    if (strcmp(command, "name") == 0) {
        char new_name[NAME_SIZE_LIMIT];
        if (sscanf(str, "/%5s %100s", command, new_name) != 2) {
            printf("name not provided\n");
            return;
        }
        memcpy(name, new_name, strlen(new_name) + 1);
        printf("set name to %s\n", name);
    } else if (strcmp(command, "exit") == 0) {
        exit(EXIT_SUCCESS);
    } else {
        printf("not a valid command\n");
    }
}

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
        exit(EXIT_FAILURE);
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
            continue;
        }

        char ip[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, get_in_addr(p->ai_addr), ip, sizeof(ip));
        printf("connected to: %s, port %s\n", ip, PORT);

        break;
    }

    if (p == NULL) {
        printf("socket could not be created\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    res = NULL;

    struct message msg;
    strcpy(msg.name, DEFAULT_NAME);
    while (1) {
        // Read user input from STDIN
        if (fgets(msg.text, sizeof(msg.text), stdin) == NULL) {
            perror("failed to read user input");
            continue;
        }

        clear_previous_line();

        printf("read input\n");

        // Execute command if input is a command
        if (strncmp(msg.text, "/", 1) == 0) {
            execute_command(msg.text, msg.name);
            continue;
        }

        // Set timestamp
        time(&msg.timestamp);
        
        // Serialize message
        char *send_buf;
        size_t len;
        if (serialize(&msg, &send_buf, &len) != 0) {
            perror("serialization error");
            continue;
        }

        printf("serialized message\n");

        // Send message to server
        if (sendall(sockfd, send_buf, len) == -1) {
            perror("send error");
            continue;
        }

        struct tm *sent_timestamp = localtime(&msg.timestamp);
        if (sent_timestamp == NULL) {
            perror("failed to convert to timestamp to local time");
            continue;
        }
        printf("sent: (%02d:%02d) %s: %s", sent_timestamp->tm_hour, sent_timestamp->tm_min, msg.name, msg.text);

        free(send_buf);
        send_buf = NULL;

        // Receive reply from server
        char *recv_buf;
        ssize_t recvd = recvall(sockfd, &recv_buf);
        if (recvd == -1) {
            perror("receive error");
            continue;
        } else if (recvd == 0) {
            printf("connection closed\n");
            exit(EXIT_FAILURE);
        }

        printf("received message\n");

        // Deserialize reply
        struct message reply;
        if (deserialize(recv_buf, &reply) != 0) {
            perror("deserialization error");
            continue;
        }

        struct tm *recv_timestamp = localtime(&msg.timestamp);
        if (recv_timestamp == NULL) {
            perror("failed to convert to timestamp to local time");
            continue;
        }
        printf("sent: (%02d:%02d) %s: %s", recv_timestamp->tm_hour, recv_timestamp->tm_min, msg.name, msg.text);

        free(recv_buf);
        recv_buf = NULL;
    }

    exit(EXIT_SUCCESS);
}