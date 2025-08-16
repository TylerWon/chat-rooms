#ifndef POLLFD_ARRAY_H
#define POLLFD_ARRAY_H

#include <poll.h>
#include <stdint.h>

// A dynamic array of struct pollfd
struct pollfd_array
{
    struct pollfd *fds;
    uint32_t len;      // Number of elements in fds
    uint32_t capacity; // Number of elements that can be stored in fds
};

/**
 * Initializes a dynamic array of struct pollfds to be used with the poll() system call.
 *
 * The returned struct should be freed by the caller when no longer needed.
 *
 * @return Pointer to an array of pollfds on success.
 *         NULL if initialization fails.
 */
struct pollfd_array *pollfd_array_init();

/**
 * Appends a new pollfd with the specified file descriptor and events to the array of pollfds.
 *
 * @param pollfds   Pointer to the array of pollfds to modify.
 * @param fd        The file descriptor for the new struct pollfd.
 * @param events    The events for the new struct pollfd.
 *
 * @return 0 on success.
 *         -1 on failure.
 */
int pollfd_array_append(struct pollfd_array *pollfds, int fd, short events);

/**
 * Removes the pollfd at the specified index from the array of pollfds. The entry is removed by replacing it with the
 * last element in the array. If no entry with the specified fd is found, the function does nothing.
 *
 * @param pollfds   Pointer to the array of pollfds to modify.
 * @param i         The index of the pollfd to remove.
 *
 * @return 0 on success.
 *         -1 on failure.
 */
int pollfd_array_delete(struct pollfd_array *pollfds, uint32_t i);

#endif
