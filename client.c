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
#include "sockaddr_utils.h"

#define DEFAULT_NAME "Anonymous"
#define COMMAND_SIZE_LIMIT 5

/**
 * Clears previous line from terminal.
 */
void clear_previous_line()
{
    printf("\033[A");  // Move cursor up one line
    printf("\033[2K"); // Clear the entire line
}

/**
 * Executes the command in str if it is a valid command.
 *
 * Commands:
 * - /name [name] - Sets the user's name to [name]
 * - /exit - Exits the application
 *
 * @param str   The command
 * @param name  Pointer to a char buffer which will store the name the user entered (for the /name command)
 */
void execute_command(char *str, char *name)
{
    char command[COMMAND_SIZE_LIMIT];
    if (sscanf(str, "/%5s", command) != 1)
    {
        printf("command not provided\n");
        return;
    }

    if (strcmp(command, "name") == 0)
    {
        char new_name[NAME_SIZE_LIMIT];
        if (sscanf(str, "/%5s %100s", command, new_name) != 2)
        {
            printf("name not provided\n");
            return;
        }
        memcpy(name, new_name, strlen(new_name) + 1);
        printf("set name to %s\n", name);
    }
    else if (strcmp(command, "exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("not a valid command\n");
    }
}

/**
 * Gets the address info of the server for the given port and stores it in res. The server runs on the same host as the
 * client so the IP address will be the loopback address (i.e. 127.0.0.1 or ::1).
 *
 * Res should be freed when it is no longer in use.
 *
 * @param port  The port to get address info for
 * @param res   Double pointer to an addrinfo which will store the result of the address look-up
 *
 * @return  0 on success.
 *          Non-zero error code (same codes as getaddrinfo()) on error.
 */
int get_server_addr_info(char *port, struct addrinfo **res)
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Stream socket

    return getaddrinfo(NULL, port, &hints, res); // Set node parameter to NULL to have loopback address returned
}

/**
 * Creates a socket which is connected to the address provided in res
 *
 * @param res   Pointer to a linked list of addrinfos which contain the address used to create the socket
 *
 * @return  The socket file descriptor for the new socket.
 *          -1 on error.
 */
int create_socket(struct addrinfo *res)
{
    struct addrinfo *p;
    int sockfd;
    for (p = res; p != NULL; p = p->ai_next)
    {
        // Create socket from address info
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("socket error");
            continue;
        }

        // Connect to server
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) != 0)
        {
            perror("connect error");
            close(sockfd);
            continue;
        }

        char ip[INET6_ADDRSTRLEN];
        get_ip_address(p->ai_addr, ip, sizeof(ip));
        printf("connected to: %s, port %d\n", ip, get_port(p->ai_addr));

        return sockfd;
    }

    return -1;
}

int main()
{
    int status;
    struct addrinfo *res;
    if ((status = get_server_addr_info(PORT, &res)) != 0)
    {
        printf("failed to get server's address info: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    int sockfd;
    if ((sockfd = create_socket(res)) == -1)
    {
        printf("failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    res = NULL;

    struct message msg;
    strcpy(msg.name, DEFAULT_NAME);
    while (1)
    {
        // Read user input from STDIN
        if (fgets(msg.text, sizeof(msg.text), stdin) == NULL)
        {
            perror("failed to read user input");
            continue;
        }
        clear_previous_line();

        printf("read input\n");

        // Execute command if input is a command
        if (strncmp(msg.text, "/", 1) == 0)
        {
            execute_command(msg.text, msg.name);
            continue;
        }

        // Set timestamp
        time(&msg.timestamp);

        // Serialize message
        char *send_buf;
        size_t len;
        if (serialize(&msg, &send_buf, &len) != 0)
        {
            perror("failed to serialize the message");
            continue;
        }

        printf("serialized message\n");

        // Send message to server
        if (sendall(sockfd, send_buf, len) == -1)
        {
            perror("failed to send message");
            continue;
        }

        printf("sent message\n");

        free(send_buf);
        send_buf = NULL;

        // Receive reply from server
        char *recv_buf;
        ssize_t recvd = recvall(sockfd, &recv_buf);
        if (recvd == -1)
        {
            perror("failed to receive message");
            continue;
        }
        else if (recvd == 0)
        {
            printf("connection closed\n");
            exit(EXIT_FAILURE);
        }

        printf("received message\n");

        // Deserialize reply
        struct message reply;
        if (deserialize(recv_buf, &reply) != 0)
        {
            perror("failed to deserialize the message");
            continue;
        }

        print_message(&reply);

        free(recv_buf);
        recv_buf = NULL;
    }

    exit(EXIT_SUCCESS);
}