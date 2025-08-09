#include <arpa/inet.h>

#include "sockaddr_utils.h"

void get_ip_address(struct sockaddr *sa, char *buf, size_t n) {
    void *ip;

    if (sa->sa_family == AF_INET) {
        // IPv4 address so cast to sockaddr to sockaddr_in
        ip = &(((struct sockaddr_in *) sa)->sin_addr);
    } else {
        // IPv6 address so cast sockaddr to sockaddr_in6
        ip = &(((struct sockaddr_in6 *) sa)->sin6_addr);
    }  

    inet_ntop(sa->sa_family, ip, buf, n);
}


uint16_t get_port(struct sockaddr *sa) {
    // IPv4 address so cast to sockaddr_in
    if (sa->sa_family == AF_INET) {
        return ntohs(((struct sockaddr_in *) sa)->sin_port);
    }    

    // IPv6 address so cast to sockaddr_in6
    return ntohs(((struct sockaddr_in6 *) sa)->sin6_port);
}
