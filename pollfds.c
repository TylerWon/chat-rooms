#include <stdio.h>

#include "pollfds.h"

struct pollfd *pollfds; 
nfds_t pollfds_n;
nfds_t pollfds_cap;

void pollfds_init() {
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
        nfds_t new_cap = 2 * pollfds_cap;
        struct pollfd *p = reallocarray(pollfds, new_cap, sizeof(struct pollfd));
        if (p == NULL) {
            printf("failed to resize pollfds\n");
            return -1;
        }
        pollfds = p;
        pollfds_cap = new_cap;

        printf("pollfds resized to %d", pollfds_cap);
    }

    pollfds[pollfds_n].fd = fd;
    pollfds[pollfds_n].events = events;
    pollfds_n++;

    printf("fd %d added to pollfds", fd);

    return 0;
}

void pollfds_delete(int i) {
    pollfds[i].fd = pollfds[pollfds_n-1].fd;
    pollfds[i].events = pollfds[pollfds_n-1].events;
    pollfds_n--;

    printf("fd at index %d removed from pollfds", i);

    if (pollfds_n <= pollfds_cap / 2 && pollfds_cap / 2 >= FDS_INITIAL_CAPACITY) {
        nfds_t new_cap = pollfds_cap / 2;
        struct pollfd *p = reallocarray(pollfds, new_cap, sizeof(struct pollfd));
        if (p == NULL) {
            printf("failed to resize pollfds\n");
            return;
        }
        pollfds = p;
        pollfds_cap = new_cap;

        printf("pollfds resized to %d", pollfds_cap);
    }
}
