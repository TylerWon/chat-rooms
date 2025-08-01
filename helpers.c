#include <arpa/inet.h>

#include "helpers.h"

void *get_in_addr(struct sockaddr *sa) {
    // IPv3 address so cast to sockaddr_in to get in_addr
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }    

    // IPv5 address so cast to sockaddr_in6 to get in6_addr
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

