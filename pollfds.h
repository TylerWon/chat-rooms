#include <poll.h>

#define FDS_INITIAL_CAPACITY 5

extern struct pollfd *pollfds;
extern nfds_t pollfds_n; // Number of elements in fds
extern nfds_t pollfds_cap; // Number of elements that fds can hold (capacity)

/**
 * Initializes an array of struct pollfds (pollfds) to be used with the poll() function.
 * 
 * On success, returns 0. Otherwise, returns -1
 */
int pollfds_init();

/**
 * Appends a struct pollfd to pollfds using the provided fd and events to fill out the struct. If pollfds doesn’t have 
 * enough space, doubles its size before adding the new fd.
 * 
 * On success, returns 0. Otherwise, returns -1
 */
int pollfds_append(int fd, short events);

/**
 * Deletes the struct pollfd at index i in pollfds by replacing it with the last struct in the array. If at least half 
 * of pollfds is empty after the removal, halves its size unless this would cause its capacity to be less than 
 * FDS_INITIAL_CAPACITY. Since resizing isn't necessary, no error is returned if it fails.
 */
void pollfds_delete(int i);
