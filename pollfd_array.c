#include <stdio.h>
#include <stdlib.h>

#include "pollfd_array.h"

#define FDS_INITIAL_CAPACITY 5

/**
 * Resizes the provided pollfd_array to hold new_cap elements.
 *
 * @param new_cap   The new capacity of the pollfd_array
 * @param pollfds   Pointer to the pollfd_array to resize
 *
 * @return 0 on success.
 *         -1 on failure.
 */
int pollfds_resize(nfds_t new_cap, struct pollfd_array *pollfds)
{
    struct pollfd *fds = reallocarray(pollfds->fds, new_cap, sizeof(struct pollfd));
    if (fds == NULL)
        return -1;
    pollfds->fds = fds;
    pollfds->capacity = new_cap;

    printf("pollfd_array resized to %ld\n", pollfds->capacity);

    return 0;
}

struct pollfd_array *pollfds_init()
{
    struct pollfd_array *pollfds = malloc(sizeof(struct pollfd_array));
    if (pollfds == NULL)
        return NULL;

    struct pollfd *fds = calloc(FDS_INITIAL_CAPACITY, sizeof(struct pollfd));
    if (fds == NULL)
    {
        free(pollfds);
        return NULL;
    }

    pollfds->fds = fds;
    pollfds->len = 0;
    pollfds->capacity = FDS_INITIAL_CAPACITY;

    return pollfds;
}

int pollfd_array_append(int fd, short events, struct pollfd_array *pollfds)
{
    struct pollfd *fds = pollfds->fds;
    nfds_t len = pollfds->len;
    nfds_t capacity = pollfds->capacity;

    if (len + 1 > capacity)
        if (pollfds_resize(2 * capacity, pollfds) != 0)
        {
            printf("failed to resize pollfd_array\n");
            return -1;
        }

    fds[len].fd = fd;
    fds[len].events = events;
    fds[len].revents = 0;
    pollfds->len++;

    printf("fd %d added to pollfd_array\n", fd);

    return 0;
}

int pollfd_array_delete(uint32_t i, struct pollfd_array *pollfds)
{
    struct pollfd *fds = pollfds->fds;
    nfds_t len = pollfds->len;
    nfds_t capacity = pollfds->capacity;

    if (i >= len)
    {
        printf("index %i not in pollfd_array\n", i);
        return -1;
    }

    fds[i] = fds[len - 1];
    pollfds->len--;

    printf("fd at index %d removed from pollfd_array\n", i);

    if (pollfds->len <= capacity / 2 && capacity / 2 >= FDS_INITIAL_CAPACITY)
    {
        pollfds_resize(capacity / 2, pollfds); // No need to return error if resize fails because pollfd_array is still valid
    }

    return 0;
}
