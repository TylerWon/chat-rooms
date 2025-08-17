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
#include "data_structures/room_array.h"
#include "data_structures/user_table.h"
#include "lib/log.h"
#include "types/messages/chat_message.h"
#include "types/messages/join_message.h"
#include "types/messages/name_message.h"
#include "types/messages/message.h"
#include "types/messages/reply_message.h"
#include "utils/net_utils.h"
#include "utils/sockaddr_utils.h"

#define BACKLOG_LIMIT 10
#define NUM_ROOMS 5

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
            LOG_WARN("failed to create listener socket, trying again...");
            continue;
        }

        int yes = 1;
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        {
            LOG_ERROR("failed to allow socket to re-use an address: %s", strerror(errno));
            return -1;
        }

        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1)
        {
            LOG_WARN("failed to bind listener socket to address, trying again...");
            close(listener);
            continue;
        }

        LOG_INFO("created listener socket %d", listener);

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
    LOG_INFO("new connection from: %s, port %d", ip, get_port(&client_addr));

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
        LOG_ERROR("failed to accept the connection");
        return -1;
    }

    if (pollfd_array_append(pollfds, sockfd, POLLIN) != 0)
    {
        LOG_ERROR("failed to add socket fd %d to pollfd array", sockfd);
        close(sockfd);
        return -1;
    }

    if (user_table_add(user_table, sockfd))
    {
        LOG_ERROR("failed to add user %d to user table", sockfd);
        return -1;
    }

    LOG_INFO("created new connection to client %d", sockfd);

    return 0;
}

/**
 * Sends a reply from the server to the client.
 *
 * @param client    The client socket
 * @param reply     The reply which may contain format specifiers
 * @param ...       Value(s) for format specifier(s) (if any)
 */
void send_reply_message(int client, char *reply, ...)
{
    // Fill in format specifiers with values
    va_list args;
    va_start(args, reply); // Initialize args with the variable arguments starting after "reply"

    char total_reply[REPLY_SIZE_LIMIT];
    if (vsnprintf(total_reply, sizeof(total_reply), reply, args) >= (int)sizeof(total_reply))
        LOG_WARN("reply message truncated: %s", total_reply);

    va_end(args);

    struct reply_message msg;
    strcpy(msg.reply, total_reply);

    char *send_buf;
    size_t len;
    if (reply_message_serialize(&msg, &send_buf, &len) != 0)
    {
        LOG_ERROR("failed to serialize the reply message");
        return;
    }

    if (sendall(client, send_buf, len) == -1)
    {
        LOG_ERROR("failed to send the reply message");
        free(send_buf);
        return;
    }
    free(send_buf);

    LOG_INFO("sent reply message to client %d", client);
}

/**
 * Handles a chat message from a client.
 *
 * A client sends this kind of message when it wants to send a message to the chat room they are in. As a result, this
 * function will take their message and send it to all other clients in the room.
 *
 * @param buf           Pointer to a char buffer containing the message
 * @param user          Pointer to the user data for the client
 * @param rooms         Pointer to an array containing all open chat rooms
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_chat_message(char *buf, struct user *user, struct room_array *rooms)
{
    if (user->room == INVALID_ROOM)
    {
        LOG_INFO("did not send chat message: client %d is not in a room", user->room);
        send_reply_message(user->id, "you are not in a chat room: type '/join [room number]' to join a room");
        return 0;
    }

    struct chat_message msg;
    chat_message_deserialize(buf, &msg);
    time(&msg.timestamp);
    strcpy(msg.name, user->name);

    char *send_buf;
    size_t len;
    if (chat_message_serialize(&msg, &send_buf, &len) != 0)
    {
        LOG_ERROR("failed to serialize the chat message");
        return -1;
    }

    struct room *room = room_array_get_room(rooms, user->room);
    for (uint8_t i = 0; i < room->num_users; i++)
    {
        int receiver = room->users[i];

        if (sendall(receiver, send_buf, len) == -1)
        {
            LOG_ERROR("failed to send chat message to client %d", receiver);
            free(send_buf);
            return -1;
        }
    }
    free(send_buf);

    LOG_INFO("sent chat message from client %d to all clients in room %d", user->id, room->id);

    return 0;
}

/**
 * Handles a name message from a client.
 *
 * A client sends this kind of message when it wants to update their name. As a result, this function will update their
 * name then send a message back to inform them that the update was successful.
 *
 * @param buf   Pointer to a char buffer containing the message
 * @param user  Pointer to the user data for the client
 */
void handle_name_message(char *buf, struct user *user)
{
    struct name_message msg;
    name_message_deserialize(buf, &msg);
    strcpy(user->name, msg.name);
    LOG_INFO("set name of user %d to %s", user->id, user->name);

    send_reply_message(user->id, "set name to %s", msg.name);
}

/**
 * Handles a join message from a client.
 *
 * A client sends this kind of message when it wants to join a room. As a result, this function will add them to the
 * room then send a message back to inform them that the join was successful.
 *
 * @param buf   Pointer to a char buffer containing the message
 * @param rooms Pointer to an array containing all open chat rooms
 * @param user  Pointer to the user data for the client
 */
void handle_join_message(char *buf, struct room_array *rooms, struct user *user)
{
    struct join_message msg;
    join_message_deserialize(buf, &msg);

    struct room *new_room = room_array_get_room(rooms, msg.room_id);
    if (new_room == NULL)
    {
        LOG_INFO("did not add user %d to room %d: room does not exist", user->id, msg.room_id);
        send_reply_message(user->id, "room %d does not exist", msg.room_id);
        return;
    }

    if (user->room == new_room->id)
    {
        LOG_INFO("did not add user %d to room %d: user already in room", user->id, new_room->id);
        send_reply_message(user->id, "you are already in room %d", new_room->id);
        return;
    }
    else if (user->room != INVALID_ROOM)
    {
        struct room *current_room = room_array_get_room(rooms, user->room);
        room_remove_user(current_room, user);
    }

    if (room_add_user(new_room, user) != 0)
    {
        LOG_INFO("did not add user %d to room %d: room is full", user->id, new_room->id);
        send_reply_message(user->id, "room %d is full", new_room->id);
        return;
    }

    send_reply_message(user->id, "you have joined room %d", new_room->id);
}

/**
 * Handles a message from a client.
 *
 * - Receives message from the server
 * - Determines the type of message and handles it accordingly
 *
 * @param client        The client socket to receive the message from
 * @param user_table    Double pointer to a hash table containing all users
 * @param rooms         Pointer to an array containing all open chat rooms
 *
 * @return  1 on success.
 *          0 when the client closes the connection.
 *          -1 on error.
 */
int handle_client_message(int client, struct user **user_table, struct room_array *rooms)
{
    char *recv_buf;
    ssize_t recvd = recvall(client, &recv_buf);
    if (recvd == -1)
    {
        LOG_ERROR("failed to receive message from client %d: %s", client, strerror(errno));
        return -1;
    }
    else if (recvd == 0)
    {
        LOG_INFO("connection to client %d closed", client);
        return 0;
    }

    struct user *user = user_table_find(user_table, client);
    if (user == NULL)
    {
        LOG_ERROR("failed to find user %d", client);
        return -1;
    }

    switch (get_message_type(recv_buf))
    {
    case CHAT_MESSAGE:
        LOG_INFO("received chat message from client %d", user->id);
        if (handle_chat_message(recv_buf, user, rooms) != 0)
        {
            LOG_ERROR("failed to handle chat message");
            free(recv_buf);
            return -1;
        }
        break;
    case JOIN_MESSAGE:
        LOG_INFO("received join message from client %d", user->id);
        handle_join_message(recv_buf, rooms, user);
        break;
    case NAME_MESSAGE:
        LOG_INFO("received name message from client %d", user->id);
        handle_name_message(recv_buf, user);
        break;
    default:
        LOG_ERROR("invalid message type");
        free(recv_buf);
        return -1;
    }
    free(recv_buf);

    return 1;
}

/**
 * Handles terminiation of a client.
 *
 * - Removes the client's socket fd from the array of socket fds
 * - Removes the client from the room they were in (if they were in one)
 * - Removes the user data associated with the client from the hash table of users
 * - Closes the connection to the given client
 *
 * @param client        The client socket to close
 * @param i             The index of the socket fd in pollfds
 * @param pollfds       Pointer to an array containing all open socket fds
 * @param rooms         Pointer to an array containing all open chat rooms
 * @param user_table    Double pointer to a hash table containing all users
 *
 * @return  0 on success.
 *          -1 on error.
 */
int handle_client_termination(int client, uint32_t i, struct pollfd_array *pollfds, struct room_array *rooms, struct user **user_table)
{
    if (pollfd_array_delete(pollfds, i) != 0)
    {
        LOG_ERROR("failed to delete fd %d from pollfd array", client);
        return -1;
    }

    struct user *user = user_table_find(user_table, client);
    if (user == NULL)
    {
        LOG_ERROR("user %d not found", client);
        return -1;
    }

    struct room *room = room_array_get_room(rooms, user->room);
    if (room != NULL)
        if (room_remove_user(room, user) != 0)
        {
            LOG_ERROR("failed to remove user %d from room %d", user->id, room->id);
            return -1;
        }

    if (user_table_delete(user_table, client) != 0)
    {
        LOG_ERROR("failed to delete user %d", client);
        return -1;
    }

    close(client);
    LOG_INFO("closed connection to client %d", client);

    return 0;
}

int main()
{
    int listener;
    struct pollfd_array *pollfds = pollfd_array_init();
    struct room_array *rooms = room_array_init(NUM_ROOMS);
    struct user *user_table = NULL;

    int status;
    struct addrinfo *res;
    if ((status = get_server_addr_info(PORT, &res)) != 0)
    {
        LOG_ERROR("failed to get server's address info: %s", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    if ((listener = create_listener_socket(res)) == -1)
    {
        LOG_ERROR("failed to create listener socket");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
    res = NULL;

    if (listen(listener, BACKLOG_LIMIT) == -1)
    {
        LOG_ERROR("failed to set-up listener socket for listening: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pollfd_array_append(pollfds, listener, POLLIN) != 0)
    {
        LOG_ERROR("failed to append listener socket to pollfd array");
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
            int sockfd = pfd.fd;
            short revents = pfd.revents;

            if (revents & POLLHUP)
            {
                if (handle_client_termination(sockfd, i, pollfds, rooms, &user_table) != 0)
                {
                    LOG_ERROR("failed to close connection to client %d", sockfd);
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
                        LOG_ERROR("failed to create new connection");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    int status = handle_client_message(sockfd, &user_table, rooms);
                    if (status == 0)
                    {
                        if (handle_client_termination(sockfd, i, pollfds, rooms, &user_table) != 0)
                        {
                            LOG_ERROR("failed to close connection to client %d", sockfd);
                            exit(EXIT_FAILURE);
                        }
                        i--; // repeat same index because last element in pollfd array has taken its place
                    }
                    else if (status == -1)
                    {
                        LOG_ERROR("failed to handle message from client %d", sockfd);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
}
