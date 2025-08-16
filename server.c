#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "data_structures/pollfd_array.h"
#include "data_structures/user_table.h"
#include "types/messages/chat_message.h"
#include "types/messages/name_message.h"
#include "types/messages/message.h"
#include "types/messages/reply_message.h"
#include "utils/net_utils.h"
#include "utils/sockaddr_utils.h"

#define BACKLOG_LIMIT 10

/**
 * Gets the address info of the server for the given port and stores it in res. The IP address will be the wildcard
 * address so connections can be accpeted on any of the host's network addresses.
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
    hints.ai_flags = AI_PASSIVE;     // Setting this and the node parameter to NULL makes the wildcard address be returned

    return getaddrinfo(NULL, port, &hints, res);
}

/**
 * Creates a socket for listening to incoming connections to the address provided in res
 *
 * @param res   Pointer to a linked list of addrinfos which contain the address used to create the socket
 *
 * @return  The socket file descriptor for the listening socket.
 *          -1 on error.
 */
int create_listener_socket(struct addrinfo *res)
{
    struct addrinfo *p;
    int listener;
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("failed to create listener socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        {
            perror("error occurred while attempting to allow the listener socket to reuse an address");
            return -1;
        }

        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("failed to bind listener socket");
            close(listener);
            continue;
        }

        printf("created listener socket %d\n", listener);

        return listener;
    }

    return -1;
}

/**
 * Accepts an incoming connection on the given socket.
 *
 * @param listener  The socket to accept the connection on
 *
 * @return  0 on success.
 *          -1 on error (errno is set appropriately).
 */
int accept_connection(int listener)
{
    struct sockaddr client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int sockfd;
    if ((sockfd = accept(listener, &client_addr, &client_addr_size)) == -1)
        return -1;

    char ip[INET6_ADDRSTRLEN];
    get_ip_address(&client_addr, ip, sizeof(ip));
    printf("new connection from: %s, port %d\n", ip, get_port(&client_addr));

    return sockfd;
}

/**
 * Handles a new client connection.
 *
 * - Accepts the connection on the given socket
 * - Adds the new socket file descriptor to an array containing all open sockets
 * - Creates a new user for the connection and adds it to a hash table containing all users
 *
 * @param listener      The socket to accept the new connection on
 * @param pollfds       Pointer to an array containing all open socket fds
 * @param user_table    Double pointer to a hash table containing all users
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_new_client(int listener, struct pollfd_array *pollfds, struct user **user_table)
{
    int sockfd;
    if ((sockfd = accept_connection(listener)) == -1)
    {
        perror("failed to accept the connection");
        return -1;
    }

    if (pollfd_array_append(pollfds, sockfd, POLLIN) != 0)
    {
        printf("failed to add fd %d to pollfd array\n", sockfd);
        close(sockfd);
        return -1;
    }

    if (user_table_add(user_table, sockfd))
    {
        printf("failed to add user with id %d\n", sockfd);
        return -1;
    }

    printf("created new connection\n");

    return 0;
}

/**
 * Handles a chat message from a client.
 *
 * A client sends this kind of message when it wants to send a message to the chat room they are in. As a result, this
 * function will take their message and send it to all other clients in the room.
 *
 * @param listener      The listener socket
 * @param buf           Pointer to a char buffer containing the message
 * @param user          Pointer to the user data for the client
 * @param pollfds       Pointer to an array containing all open socket fds
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_chat_message(int listener, char *buf, struct user *user, struct pollfd_array *pollfds)
{
    struct chat_message msg;
    if (chat_message_deserialize(buf, &msg) != 0)
    {
        perror("failed to deserialize the message");
        return -1;
    }

    printf("deserialized message\n");

    time(&msg.timestamp);
    strcpy(msg.name, user->name);

    printf("added name and timestamp to message\n");

    char *send_buf;
    size_t len;
    if (chat_message_serialize(&msg, &send_buf, &len) != 0)
    {
        perror("failed to serialize the message");
        return -1;
    }

    printf("serialized message\n");

    for (uint32_t i = 0; i < pollfds->len; i++)
    {
        int receiver = pollfds->fds[i].fd;

        if (receiver == listener)
            continue;

        if (sendall(receiver, send_buf, len) == -1)
        {
            printf("failed to send data to socket %d: %s\n", receiver, strerror(errno));
            free(send_buf);
            return -1;
        }
    }
    free(send_buf);

    printf("sent message to all other clients\n");

    return 0;
}

/**
 * Sends a reply from the server to the client.
 *
 * @param client    The client socket
 * @param reply     The reply which may contain format specifiers
 * @param ...       Value(s) for format specifier(s) (if any)
 */
int send_reply_message(int client, char *reply, ...)
{
    // Fill in format specifiers with values
    va_list args;
    va_start(args, reply); // Initialize args with the variable arguments starting after "reply"

    char total_reply[REPLY_SIZE_LIMIT];
    if (vsnprintf(total_reply, sizeof(total_reply), reply, args) >= (int)sizeof(total_reply))
        printf("Reply message truncated: %s\n", total_reply);

    va_end(args);

    struct reply_message msg;
    strcpy(msg.reply, total_reply);

    char *send_buf;
    size_t len;
    if (reply_message_serialize(&msg, &send_buf, &len) != 0)
    {
        perror("failed to serialize the message");
        return -1;
    }

    printf("serialized message\n");

    if (sendall(client, send_buf, len) == -1)
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
 * Handles a name message from a client.
 *
 * A client sends this kind of message when it wants to update their name. As a result, this function will update their
 * name then send a message back to the client to inform them that the update was successful.
 *
 * @param buf   Pointer to a char buffer containing the message
 * @param user  Pointer to the user data for the client
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_name_message(char *buf, struct user *user)
{
    struct name_message msg;
    if (name_message_deserialize(buf, &msg) != 0)
    {
        perror("failed to deserialize the message");
        send_reply_message(user->id, "failed to set name");
        return -1;
    }

    printf("deserialized message\n");

    strcpy(user->name, msg.name);
    printf("set name of user %d to %s\n", user->id, user->name);

    if (send_reply_message(user->id, "set name to %s", msg.name) != 0)
        printf("failed to send reply to client %d\n", user->id); // Don't need to return -1 here because its ok if client doesn't get this message

    return 0;
}

/**
 * Handles a message from a client.
 *
 * - Receives message from the server
 * - Determines the type of message and handles it accordingly
 *
 * @param listener      The listener socket
 * @param client        The client socket to receive the message from
 * @param user_table    Double pointer to a hash table containing all users
 * @param pollfds       Pointer to an array containing all open socket fds
 *
 * @return  1 on success.
 *          0 when the client closes the connection.
 *          -1 on error.
 */
int handle_client_message(int listener, int client, struct user **user_table, struct pollfd_array *pollfds)
{
    char *recv_buf;
    ssize_t recvd = recvall(client, &recv_buf);
    if (recvd == -1)
    {
        printf("failed to receive message on socket %d, %s\n", client, strerror(errno));
        return -1;
    }
    else if (recvd == 0)
    {
        printf("connection to socket %d closed\n", client);
        return 0;
    }

    printf("received message\n");

    struct user *user = user_table_find(user_table, client);
    if (user == NULL)
    {
        printf("failed to find user %d\n", client);
        return -1;
    }

    switch (get_message_type(recv_buf))
    {
    case CHAT_MESSAGE:
        if (handle_chat_message(listener, recv_buf, user, pollfds) != 0)
        {
            printf("failed to handle chat message\n");
            free(recv_buf);
            return -1;
        }
        break;
    case NAME_MESSAGE:
        if (handle_name_message(recv_buf, user) != 0)
        {
            printf("failed to handle name message\n");
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

    return 1;
}

/**
 * Handles terminiation of a client.
 *
 * - Removes the socket fd from the array of socket fds
 * - Removes the user associated with the client from the hash table of users
 * - Closes the connection to the given client
 *
 * @param client        The client socket to close
 * @param i             The index of the socket fd in pollfds
 * @param pollfds       Pointer to an array containing all open socket fds
 * @param user_table    Double pointer to a hash table containing all users
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_client_termination(int client, uint32_t i, struct pollfd_array *pollfds, struct user **user_table)
{
    if (pollfd_array_delete(pollfds, i) != 0)
    {
        printf("failed to delete fd %d from pollfd array\n", client);
        return -1;
    }

    if (user_table_delete(user_table, client) != 0)
    {
        printf("failed to delete user %d\n", client);
        return -1;
    }

    close(client);
    printf("closed connection to socket %d\n", client);

    return 0;
}

int main()
{
    int listener;
    struct pollfd_array *pollfds;
    struct user *user_table = NULL;

    int status;
    struct addrinfo *res;
    if ((status = get_server_addr_info(PORT, &res)) != 0)
    {
        printf("failed to get server's address info: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    if ((listener = create_listener_socket(res)) == -1)
    {
        printf("failed to create listener socket\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
    res = NULL;

    if (listen(listener, BACKLOG_LIMIT) == -1)
    {
        perror("error occurred while attempting to allow connections on the listener socket");
        exit(EXIT_FAILURE);
    }

    if ((pollfds = pollfd_array_init()) == NULL)
    {
        printf("failed to initialize pollfd array\n");
        exit(EXIT_FAILURE);
    }

    if (pollfd_array_append(pollfds, listener, POLLIN) != 0)
    {
        printf("failed to append listener socket to pollfd array");
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
            int sockfd = pfd.fd;
            short revents = pfd.revents;

            if (revents & POLLHUP)
            {
                if (handle_client_termination(sockfd, i, pollfds, &user_table) != 0)
                {
                    printf("failed to close connection to socket %d\n", sockfd);
                    exit(EXIT_FAILURE);
                }
                i--; // repeat same index because last element in pollfd array has taken its place
                continue;
            }

            if (revents & POLLIN)
            {
                if (sockfd == listener)
                {
                    if (handle_new_client(listener, pollfds, &user_table) != 0)
                    {
                        printf("failed to create new connection\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    int status = handle_client_message(listener, sockfd, &user_table, pollfds);
                    if (status == 0)
                    {
                        if (handle_client_termination(sockfd, i, pollfds, &user_table) != 0)
                        {
                            printf("failed to close connection to socket %d\n", sockfd);
                            exit(EXIT_FAILURE);
                        }
                        i--; // repeat same index because last element in pollfd array has taken its place
                    }
                    else if (status == -1)
                    {
                        printf("failed to handle client data on socket %d\n", sockfd);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}
