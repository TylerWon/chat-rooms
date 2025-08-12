#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "net_utils.h"
#include "pollfd_array.h"
#include "sockaddr_utils.h"

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
 * Accepts a new connection on the given socket and appends the resulting socket file descriptor to the pollfd_array
 * so it can be monitored.
 *
 * @param listener  The socket to accept the new connection on
 * @param pollfds   Pointer to a pollfd_array which the new socket should be added to
 *
 * @return  0 on success.
 *          -1 on error.
 */
int create_connection(int listener, struct pollfd_array *pollfds)
{
    int sockfd;
    if ((sockfd = accept_connection(listener)) == -1)
    {
        perror("failed to accept the connection");
        return -1;
    }

    if (pollfd_array_append(sockfd, POLLIN, pollfds) != 0)
    {
        printf("failed to add socket %d to pollfds\n", sockfd);
        close(sockfd);
        return -1;
    }

    printf("created new connection\n");

    return 0;
}

/**
 * Receives data from the socket client and sends it to all sockets (except for the listener socket) being monitored in
 * the pollfd_array.
 *
 * @param listener  The listener socket
 * @param client    The socket to receive data from
 * @param pollfds   Pointer to a pollfd_array which contains the sockets the data should be sent to
 *
 * @return  1 on success.
 *          0 when the client closes the connection.
 *          -1 on error.
 */
int handle_client_data(int listener, int client, struct pollfd_array *pollfds)
{
    char *buf;
    ssize_t recvd = recvall(client, &buf);
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

    for (uint32_t i = 0; i < pollfds->len; i++)
    {
        int receiver = pollfds->fds[i].fd;

        if (receiver == listener)
            continue;

        if (sendall(receiver, buf, recvd) == -1)
        {
            printf("failed to send data to socket %d: %s\n", receiver, strerror(errno));
            return -1;
        }
    }
    free(buf);

    printf("sent message to all open connections\n");

    return 1;
}

/**
 * Closes the connection to the given socket and removes it from being monitored in the pollfd_array.
 *
 * @param sockfd    The socket to close
 * @param i         The index of the socket in pollfd_array
 * @param pollfds   Pointer to the pollfd_array
 *
 * @return  0 on success.
 *          -1 on error.
 */
int close_connection(int sockfd, uint32_t i, struct pollfd_array *pollfds)
{
    if (pollfd_array_delete(i, pollfds) != 0)
    {
        printf("failed to delete socket %d from pollfds\n", sockfd);
        return -1;
    }

    close(sockfd);
    printf("closed connection to socket %d\n", sockfd);

    return 0;
}

int main()
{
    int listener;
    struct pollfd_array *pollfds;

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
        printf("failed to initialize pollfds\n");
        exit(EXIT_FAILURE);
    }

    if (pollfd_array_append(listener, POLLIN, pollfds) != 0)
    {
        printf("failed to append listener socket to pollfds");
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
                if (close_connection(sockfd, i, pollfds) != 0)
                {
                    printf("failed to close connection to socket %d\n", sockfd);
                    exit(EXIT_FAILURE);
                }
                i--; // repeat same index because last element in pollfds has taken its place
                continue;
            }

            if (revents & POLLIN)
            {
                if (sockfd == listener)
                {
                    if (create_connection(listener, pollfds) != 0)
                    {
                        printf("failed to create new connection\n");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    int status = handle_client_data(listener, sockfd, pollfds);
                    if (status == 0)
                    {
                        if (close_connection(sockfd, i, pollfds) != 0)
                        {
                            printf("failed to close connection to socket %d\n", sockfd);
                            exit(EXIT_FAILURE);
                        }
                        i--; // repeat same index because last element in pollfds has taken its place
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
