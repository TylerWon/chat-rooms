#include <poll.h>
#include <stdint.h>

struct pollfd_array
{
    struct pollfd *fds;
    nfds_t len;      // Number of elements in fds
    nfds_t capacity; // Number of elements that can be stored in fds
};

/**
 * Allocates and initializes a new pollfd_array structure to manage a dynamic array of struct pollfd entries for use
 * with the poll() system call.
 *
 * The returned struct should be freed by the caller when no longer needed.
 *
 * @return Pointer to an initialized pollfd_array on success.
 *         NULL if allocation fails (errno is set accordingly).
 */
struct pollfd_array *pollfds_init();

/**
 * Appends a new struct pollfd entry with the specified file descriptor and events to the given pollfd_array. If the
 * internal array is full, its capacity is doubled before appending.
 *
 * @param fd        The file descriptor to monitor.
 * @param events    The event flags to watch for (e.g., POLLIN).
 * @param pollfds   Pointer to the pollfd_array to append to.
 *
 * @return 0 on success.
 *         -1 on failure.
 */
int pollfd_array_append(int fd, short events, struct pollfd_array *pollfds);

/**
 * Removes the struct pollfd entry at the specified index from the given pollfd_array. The entry is removed by replacing
 * it with the last element in the array. If, after removal, the array is at least half empty, its capacity is halved to
 * reduce memory usage.
 *
 * If no entry with the specified fd is found, the function does nothing.
 *
 * @param i         The index of the entry to remove.
 * @param pollfds   Pointer to the pollfd_array to modify.
 *
 * @return 0 on success.
 *         -1 on failure.
 */
int pollfd_array_delete(uint32_t i, struct pollfd_array *pollfds);
