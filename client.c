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

#include "data_structures/pollfd_array.h"
#include "messages/chat_message.h"
#include "messages/message.h"
#include "messages/reply_message.h"
#include "utils/net_utils.h"
#include "utils/sockaddr_utils.h"

#define COMMAND_SIZE_LIMIT 5

/**
 * Gets the address info of the server for the given port and stores it in res. The server runs on the same host as the
 * client so the IP address will be the loopback address (i.e. 127.0.0.1 or ::1).
 *
 * res should be freed when it is no longer in use.
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
 * Creates a socket which is connected to the address provided in addr
 *
 * @param res   Pointer to a linked list of addrinfos containing the address used to create the socket
 *
 * @return  The socket file descriptor for the new socket.
 *          -1 on error.
 */
int create_socket(struct addrinfo *addr)
{
    struct addrinfo *p;
    int sockfd;
    for (p = addr; p != NULL; p = p->ai_next)
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
 * Clears previous line from terminal.
 */
void clear_previous_line()
{
    printf("\033[A");  // Move cursor up one line
    printf("\033[2K"); // Clear the entire line
}

/**
 * Sends a name entered by the user to the server. The server will update the user's display name with this.
 *
 * @param server    The server socket.
 * @param text      Pointer to a char buffer containing the name.
 */
int send_name_message(int server, char *name)
{
    struct name_message msg;
    strcpy(msg.name, name);

    char *send_buf;
    size_t len;
    if (name_message_serialize(&msg, &send_buf, &len) != 0)
    {
        perror("failed to serialize the message");
        return -1;
    }

    printf("serialized message\n");

    if (sendall(server, send_buf, len) == -1)
    {
        perror("failed to send message");
        free(send_buf);
        return -1;
    }
    free(send_buf);

    printf("sent message\n");

    return 0;
}

/**
 * Executes the command in str if it is a valid command.
 *
 * Commands:
 * - /name [name] - Sets the user's name to [name]
 * - /exit - Exits the application
 *
 * @param str       The command
 * @param server    The server socket
 */
void execute_command(char *str, int server)
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
        if (send_name_message(server, new_name) != 0)
        {
            printf("failed to set name\n");
            return;
        }
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
 * Sends text entered by the user to the server. The text will be sent to all other users in the same room as the client.
 *
 * @param server    The server socket.
 * @param text      Pointer to a char buffer containing the user's input.
 *
 * @return  0 on success.
 *          -1 on error.
 */
int send_chat_message(int server, char *text)
{
    // Only the text field of the chat_message gets set by the client so initialize struct with 0s to avoid
    // uninitialized value error for other fields when serializing
    struct chat_message msg;
    memset(&msg, 0, sizeof(msg));
    strcpy(msg.text, text);

    char *send_buf;
    size_t len;
    if (chat_message_serialize(&msg, &send_buf, &len) != 0)
    {
        perror("failed to serialize the message");
        return -1;
    }

    printf("serialized message\n");

    if (sendall(server, send_buf, len) == -1)
    {
        perror("failed to send message");
        free(send_buf);
        return -1;
    }
    free(send_buf);

    printf("sent message\n");

    return 0;
}

/**
 * Handles input from the user.
 *
 * If input is a command, executes the command. Otherwise, input is a message so sends it to the server.
 *
 * @param server  The server socket
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_input(int server)
{
    char buf[TEXT_SIZE_LIMIT];
    if (fgets(buf, sizeof(buf), stdin) == NULL)
    {
        perror("failed to read user input");
        return -1;
    }
    clear_previous_line();

    if (strncmp(buf, "/", 1) == 0)
    {
        execute_command(buf, server);
        return 0;
    }

    if (send_chat_message(server, buf) != 0)
    {
        printf("failed to send chat message\n");
        return -1;
    }

    return 0;
}

/**
 * Handles a chat message from the server.
 *
 * The server sends this kind of message when someone has sent a message to the chat room the client is in. As a result,
 * this function will print message to the terminal.
 *
 * @param buf   Pointer to a char buffer containing the message
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_chat_message(char *buf)
{
    struct chat_message msg;
    if (chat_message_deserialize(buf, &msg) != 0)
    {
        perror("failed to deserialize the message");
        return -1;
    }

    chat_message_print(&msg);

    return 0;
}

/**
 * Handles a reply message from the server.
 *
 * @param buf   Pointer to a char buffer containing the message
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_reply_message(char *buf)
{
    struct reply_message msg;
    if (reply_message_deserialize(buf, &msg) != 0)
    {
        perror("failed to deserialize the message");
        return -1;
    }

    printf("%s\n", msg.reply);

    return 0;
}

/**
 * Handles a message from the server.
 *
 * - Receives message from the server
 * - Determines the type of message and handles it accordingly
 *
 * @param server  The server socket
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_server_message(int server)
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

    switch (get_message_type(recv_buf))
    {
    case CHAT_MESSAGE:
        if (handle_chat_message(recv_buf) != 0)
        {
            printf("failed to handle chat message\n");
            free(recv_buf);
            return -1;
        }
        break;
    case REPLY_MESSAGE:
        if (handle_reply_message(recv_buf) != 0)
        {
            printf("failed to handle reply message\n");
            free(recv_buf);
            return -1;
        }
        break;
    default:
        printf("invalid message type\n");
        free(recv_buf);
        return -1;
    }
    free(recv_buf);

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
        printf("failed to initialize array of pollfds\n");
        exit(EXIT_FAILURE);
    }

    if (pollfd_array_append(pollfds, server, POLLIN) != 0)
    {
        printf("failed to append server fd to pollfd array");
        exit(EXIT_FAILURE);
    }

    if (pollfd_array_append(pollfds, STDIN_FILENO, POLLIN) != 0)
    {
        printf("failed to append stdin fd to pollfd array\n");
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
                    if (handle_server_message(server))
                    {
                        printf("failed to handle server data\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}