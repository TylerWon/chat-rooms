#ifndef SOCKADDR_UTILS_H
#define SOCKADDR_UTILS_H

#include <stdint.h>
#include <stdlib.h>

/**
 * Gets the IP address from a struct sockaddr and stores it an array buf that has a size of n. Since struct sockaddr is
 * protocol agnostic, its sa_family field is used to determine if the struct represents a IPv4 or IPv6 address so it can
 * be cast to a struct sockaddr_in or sockaddr_in6 for easier access.
 *
 * @param sa    The sockaddr to obtain the IP address from
 * @param buf   Pointer to a char buffer which will store the IP address
 * @param n     Size of the buffer
 */
void get_ip_address(struct sockaddr *sa, char *buf, size_t n);

/**
 * Returns the port number from a struct sockaddr. Since struct sockaddr is protocol agnostic, its sa_family field is
 * used to determine if the struct represents a IPv4 or IPv6 address so it can be cast to a struct sockaddr_in or
 * sockaddr_in6 for easier access.
 *
 * @param sa    The sockaddr to obtain the port number from
 *
 * @return  The port number
 */
uint16_t get_port(struct sockaddr *sa);

#endif
