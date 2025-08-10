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
#include "pollfds.h"
#include "server.h"
#include "sockaddr_utils.h"

int listener;

int get_server_addr_info(char *port, struct addrinfo **res)
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Stream socket
    hints.ai_flags = AI_PASSIVE;     // Setting this and the node parameter to NULL makes the wildcard address be returned

    return getaddrinfo(NULL, port, &hints, res);
}

int create_listener_socket(struct addrinfo *res)
{
    struct addrinfo *p;
    int sockfd;
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("failed to create listener socket");
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        {
            perror("error occurred while attempting to allow the listener socket to reuse an address");
            return -1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("failed to bind listener socket");
            close(sockfd);
            continue;
        }

        return sockfd;
    }

    return -1;
}

int accept_connection()
{
    struct sockaddr client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int sockfd;
    if ((sockfd = accept(listener, &client_addr, &client_addr_size)) == -1)
    {
        perror("error occurred while accepting the connection");
        return -1;
    }

    char ip[INET6_ADDRSTRLEN];
    get_ip_address(&client_addr, ip, sizeof(ip));
    printf("connection from: %s, port %d\n", ip, get_port(&client_addr));

    return sockfd;
}

int create_connection()
{
    int sockfd;
    if ((sockfd = accept_connection()) == -1)
    {
        printf("failed to accept the connection");
        return -1;
    }

    if (pollfds_append(sockfd, POLLIN) != 0)
    {
        printf("failed to add socket %d to pollfds\n", sockfd);
        close(sockfd);
        return -1;
    }

    printf("created new connection\n");

    return 0;
}

int handle_data(int sender)
{
    char *buf;
    ssize_t recvd = recvall(sender, &buf);
    if (recvd == -1)
    {
        printf("failed to receive message\n");
        return -1;
    }
    else if (recvd == 0)
    {
        printf("peer on socket %d terminated the connection\n", sender);
        return 0;
    }

    printf("received message\n");

    for (uint32_t i = 0; i < pollfds_n; i++)
    {
        int receiver = pollfds[i].fd;

        if (receiver == listener)
        {
            continue;
        }

        if (sendall(receiver, buf, recvd) == -1)
        {
            printf("failed to send data to socket %d\n", receiver);
            return -1;
        }
    }
    free(buf);

    printf("sent message to all open connections\n");

    return 1;
}

void close_connection(int sockfd, int i)
{
    close(sockfd);
    pollfds_delete(i);

    printf("closed connection to socket %d\n", sockfd);
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

    // Initialize array for tracking open connections
    if (pollfds_init() != 0)
    {
        printf("failed to initialize pollfds\n");
        exit(EXIT_FAILURE);
    }

    pollfds_append(listener, POLLIN);

    while (1)
    {
        if (poll(pollfds, pollfds_n, -1) == -1)
        {
            perror("error occurred while polling sockets for events");
            exit(EXIT_FAILURE);
        }

        for (uint32_t i = 0; i < pollfds_n; i++)
        {
            struct pollfd pfd = pollfds[i];
            int sockfd = pfd.fd;
            short revents = pfd.revents;

            if (revents & POLLHUP)
            {
                close_connection(sockfd, i);
                i--; // repeat same index because last element in pollfds has taken its place
                continue;
            }

            if (revents & POLLIN)
            {
                if (sockfd == listener)
                {
                    if (create_connection() != 0)
                    {
                        printf("failed to create new connection\n");
                    }
                }
                else
                {
                    int status;
                    status = handle_data(sockfd);
                    if (status == -1)
                    {
                        printf("failed to handle data on socket %d\n", sockfd);
                        exit(EXIT_FAILURE);
                    }
                    else if (status == 0)
                    {
                        close_connection(sockfd, i);
                        i--;
                    }
                }
            }
        }
    }
}
