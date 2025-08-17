#include <arpa/inet.h>
#include <errno.h>
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
#include "lib/log.h"
#include "types/messages/chat_message.h"
#include "types/messages/join_message.h"
#include "types/messages/name_message.h"
#include "types/messages/message.h"
#include "types/messages/reply_message.h"
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
 * Creates a socket for communicating with the server
 *
 * @param res   Pointer to a linked list of addrinfos containing the address used to create the socket
 *
 * @return  The socket file descriptor for the new socket.
 *          -1 on error.
 */
int create_server_socket(struct addrinfo *addr)
{
    struct addrinfo *p;
    int server;
    for (p = addr; p != NULL; p = p->ai_next)
    {
        // Create socket from address info
        if ((server = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            LOG_WARN("failed to create socket, trying again...");
            continue;
        }

        // Connect to server
        if (connect(server, p->ai_addr, p->ai_addrlen) != 0)
        {
            LOG_WARN("failed to connect to server, trying again...");
            close(server);
            continue;
        }

        char ip[INET6_ADDRSTRLEN];
        get_ip_address(p->ai_addr, ip, sizeof(ip));
        LOG_INFO("connected to: %s, port %d", ip, get_port(p->ai_addr));

        return server;
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
 *
 * @return  0 on success.
 *          -1 on error.
 */
int send_name_message(int server, char *name)
{
    struct name_message msg;
    strcpy(msg.name, name);

    char *send_buf;
    size_t len;
    if (name_message_serialize(&msg, &send_buf, &len) != 0)
    {
        LOG_ERROR("failed to serialize the name message");
        return -1;
    }

    if (sendall(server, send_buf, len) == -1)
    {
        LOG_ERROR("failed to send the name message");
        free(send_buf);
        return -1;
    }
    free(send_buf);

    LOG_INFO("sent name message to server");

    return 0;
}

/**
 * Sends a room id entered by the user to the server. The server will attempt to add the user to the chat room.
 *
 * @param server    The server socket.
 * @param room_id   The id of the room the user wants to join.
 *
 * @return  0 on success.
 *          -1 on error.
 */
int send_join_message(int server, ROOM_ID room_id)
{
    struct join_message msg;
    msg.room_id = room_id;

    char *send_buf;
    size_t len;
    if (join_message_serialize(&msg, &send_buf, &len) != 0)
    {
        LOG_ERROR("failed to serialize the join message");
        return -1;
    }

    if (sendall(server, send_buf, len) == -1)
    {
        LOG_ERROR("failed to send the join message");
        free(send_buf);
        return -1;
    }
    free(send_buf);

    LOG_INFO("sent join message to server");

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
        LOG_ERROR("not a valid command");
        return;
    }

    if (strcmp(command, "name") == 0)
    {
        char new_name[NAME_SIZE_LIMIT];
        if (sscanf(str, "/%5s %100s", command, new_name) != 2)
        {
            LOG_ERROR("name not provided");
            return;
        }
        if (send_name_message(server, new_name) != 0)
        {
            LOG_ERROR("failed to set name");
            return;
        }
    }
    else if (strcmp(command, "join") == 0)
    {
        ROOM_ID room_id;
        if (sscanf(str, "/%5s %hhd", command, &room_id) != 2)
        {
            LOG_ERROR("room id not provided");
            return;
        }
        if (send_join_message(server, room_id) != 0)
        {
            LOG_ERROR("failed to join room");
            return;
        }
    }
    else if (strcmp(command, "exit") == 0)
        exit(EXIT_SUCCESS);
    else
        LOG_ERROR("not a valid command");
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
        LOG_ERROR("failed to serialize the chat message");
        return -1;
    }

    if (sendall(server, send_buf, len) == -1)
    {
        LOG_ERROR("failed to send the chat message");
        free(send_buf);
        return -1;
    }
    free(send_buf);

    LOG_INFO("sent chat message to server");

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
        LOG_ERROR("failed to read user input");
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
        LOG_ERROR("failed to send chat message");
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
 */
void handle_chat_message(char *buf)
{
    struct chat_message msg;
    chat_message_deserialize(buf, &msg);
    chat_message_print(&msg);
}

/**
 * Handles a reply message from the server.
 *
 * @param buf   Pointer to a char buffer containing the message
 */
void handle_reply_message(char *buf)
{
    struct reply_message msg;
    reply_message_deserialize(buf, &msg);
    printf("** %s **\n", msg.reply);
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
        LOG_ERROR("failed to receive server message");
        return -1;
    }
    else if (recvd == 0)
    {
        LOG_ERROR("connection to server closed");
        return -1;
    }

    switch (get_message_type(recv_buf))
    {
    case CHAT_MESSAGE:
        LOG_INFO("received chat message from server");
        handle_chat_message(recv_buf);
        break;
    case REPLY_MESSAGE:
        LOG_INFO("received reply message from server");
        handle_reply_message(recv_buf);
        break;
    default:
        LOG_ERROR("invalid message type");
        free(recv_buf);
        return -1;
    }
    free(recv_buf);

    return 0;
}

int main()
{
    int server;
    struct pollfd_array *pollfds = pollfd_array_init();

    int status;
    struct addrinfo *res;
    if ((status = get_server_addr_info(PORT, &res)) != 0)
    {
        LOG_ERROR("failed to get server's address info: %s", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    if ((server = create_server_socket(res)) == -1)
    {
        LOG_ERROR("failed to create server socket");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    res = NULL;

    if (pollfd_array_append(pollfds, server, POLLIN) != 0)
    {
        LOG_ERROR("failed to append server socket to pollfd array");
        exit(EXIT_FAILURE);
    }

    if (pollfd_array_append(pollfds, STDIN_FILENO, POLLIN) != 0)
    {
        LOG_ERROR("failed to append stdin fd to pollfd array");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        if (poll(pollfds->fds, pollfds->len, -1) == -1)
        {
            LOG_ERROR("failed to poll open sockets: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        for (uint32_t i = 0; i < pollfds->len; i++)
        {
            struct pollfd pfd = pollfds->fds[i];
            int fd = pfd.fd;
            short revents = pfd.revents;

            if (revents & POLLHUP)
            {
                LOG_ERROR("connection to server closed");
                exit(EXIT_FAILURE);
            }

            if (revents & POLLIN)
            {
                if (fd == STDIN_FILENO)
                {
                    if (handle_input(server) != 0)
                    {
                        LOG_ERROR("failed to handle user input");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    if (handle_server_message(server))
                    {
                        LOG_ERROR("failed to handle message from server");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}