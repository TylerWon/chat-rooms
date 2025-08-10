#include <stdio.h>
#include <stdlib.h>

#include "pollfds.h"

struct pollfd *pollfds; 
nfds_t pollfds_n;
nfds_t pollfds_cap;

/**
 * Resizes pollfds to hold new_cap elements.
 * 
 * On success, returns 1. Otherwise, returns -1.
 */
int _pollfds_resize(nfds_t new_cap) {
    struct pollfd *p = reallocarray(pollfds, new_cap, sizeof(struct pollfd));
    if (p == NULL) {
        printf("failed to resize pollfds\n");
        return -1;
    }
    pollfds = p;
    pollfds_cap = new_cap;

    printf("pollfds resized to %ld\n", pollfds_cap);

    return 0;
}

int pollfds_init() {
    pollfds = calloc(FDS_INITIAL_CAPACITY, sizeof(struct pollfd));
    if (pollfds == NULL) {
        printf("failed to allocate space for pollfds\n");
        return -1;
    }
    pollfds_n = 0;
    pollfds_cap = FDS_INITIAL_CAPACITY;

    return 0;
}

int pollfds_append(int fd, short events) {
    if (pollfds_n + 1 > pollfds_cap) {
        if (_pollfds_resize(2 * pollfds_cap) != 0) {
            printf("error while resizing pollfds\n");
            return -1;
        }
    }

    pollfds[pollfds_n].fd = fd;
    pollfds[pollfds_n].events = events;
    pollfds[pollfds_n].revents = 0;
    pollfds_n++;

    printf("fd %d added to pollfds\n", fd);

    return 0;
}

void pollfds_delete(int i) {
    pollfds[i] = pollfds[pollfds_n-1];
    pollfds_n--;

    printf("fd at index %d removed from pollfds\n", i);

    if (pollfds_n <= pollfds_cap / 2 && pollfds_cap / 2 >= FDS_INITIAL_CAPACITY) {
        _pollfds_resize(pollfds_cap / 2);
    }
}
