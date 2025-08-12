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
#include "pollfd_array.h"
#include "sockaddr_utils.h"

#define DEFAULT_NAME "Anonymous"
#define COMMAND_SIZE_LIMIT 5

char name[NAME_SIZE_LIMIT] = DEFAULT_NAME;

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

/**
 * Reads user input from STDIN and either executes it if its a command, or sends it to the server if it isn't.
 *
 * @param server  The socket for the connection to the server
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_input(int server)
{
    struct message msg;

    if (fgets(msg.text, sizeof(msg.text), stdin) == NULL)
    {
        perror("failed to read user input");
        return -1;
    }
    clear_previous_line();

    printf("read input\n");

    if (strncmp(msg.text, "/", 1) == 0)
    {
        execute_command(msg.text, name);
        return 0;
    }

    time(&msg.timestamp);
    strcpy(msg.name, name);

    char *send_buf;
    size_t len;
    if (serialize(&msg, &send_buf, &len) != 0)
    {
        perror("failed to serialize the message");
        return -1;
    }

    printf("serialized message\n");

    if (sendall(server, send_buf, len) == -1)
    {
        perror("failed to send message");
        return -1;
    }
    free(send_buf);

    printf("sent message\n");

    return 0;
}

/**
 * Receives data from the server and prints it to the terminal.
 *
 * @param server  The socket for the connection to the server
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_server_data(int server)
{
    char *recv_buf;
    ssize_t recvd = recvall(server, &recv_buf);
    if (recvd == -1)
    {
        perror("failed to receive message");
        return -1;
    }
    else if (recvd == 0)
    {
        printf("connection closed\n");
        return -1;
    }

    printf("received message\n");

    struct message msg;
    if (deserialize(recv_buf, &msg) != 0)
    {
        perror("failed to deserialize the message");
        return -1;
    }
    free(recv_buf);

    print_message(&msg);

    return 0;
}

int main()
{
    struct pollfd_array *pollfds;
    int server;

    int status;
    struct addrinfo *res;
    if ((status = get_server_addr_info(PORT, &res)) != 0)
    {
        printf("failed to get server's address info: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    if ((server = create_socket(res)) == -1)
    {
        printf("failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    res = NULL;

    if ((pollfds = pollfd_array_init()) == NULL)
    {
        printf("failed to initialize pollfds\n");
        exit(EXIT_FAILURE);
    }

    if (pollfd_array_append(server, POLLIN, pollfds) != 0)
    {
        printf("failed to append socket to pollfds");
        exit(EXIT_FAILURE);
    }

    if (pollfd_array_append(STDIN_FILENO, POLLIN, pollfds) != 0)
    {
        printf("failed to append stdin to pollfds\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        if (poll(pollfds->fds, pollfds->len, -1) == -1)
        {
            perror("error occurred while polling sockets for events");
            exit(EXIT_FAILURE);
        }

        for (uint32_t i = 0; i < pollfds->len; i++)
        {
            struct pollfd pfd = pollfds->fds[i];
            int fd = pfd.fd;
            short revents = pfd.revents;

            if (revents & POLLHUP)
            {
                printf("connection to server closed\n");
                exit(EXIT_FAILURE);
            }

            if (revents & POLLIN)
            {
                if (fd == STDIN_FILENO)
                {
                    if (handle_input(server) != 0)
                    {
                        printf("failed to handle user input\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    if (handle_server_data(server))
                    {
                        printf("failed to handle server data\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}