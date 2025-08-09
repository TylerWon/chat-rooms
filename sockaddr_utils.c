#include <arpa/inet.h>

#include "sockaddr_utils.h"

void *get_ip_address(struct sockaddr *sa) {
    // IPv4 address so cast to sockaddr_in
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr.s_addr);
    }    

    // IPv6 address so cast to sockaddr_in6
    return &(((struct sockaddr_in6 *) sa)->sin6_addr.s6_addr);
}


uint16_t get_port(struct sockaddr *sa) {
    // IPv4 address so cast to sockaddr_in
    if (sa->sa_family == AF_INET) {
        return ntohs(((struct sockaddr_in *) sa)->sin_port);
    }    

    // IPv6 address so cast to sockaddr_in6
    return ntohs(((struct sockaddr_in6 *) sa)->sin6_port);
}
