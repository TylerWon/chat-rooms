#include <poll.h>
#include <stdint.h>

#include "message.h"

// Represents a user
struct user {
    int id;
    char name[NAME_SIZE_LIMIT];
};

// A dynamic array of struct user
struct user_array
{
    struct user *users;
    uint32_t len;
    uint32_t capacity; 
};

/**
 * Initializes a new user_array structure to manage a dynamic array of struct user entries.
 *
 * The returned struct should be freed by the caller when no longer needed.
 *
 * @return Pointer to an initialized user_array on success.
 *         NULL if initialization fails.
 */
struct user_array *user_array_init();

/**
 * Appends a new struct user with the specified id to the given user_array. 
 *
 * @param id        The id of the user.
 * @param users     Pointer to the user_array to append to.
 *
 * @return 0 on success.
 *         -1 on failure.
 */
int user_array_append(int id, struct user_array *users);

/**
 * Removes the user with the specified id from the given user_array. The entry is removed by replacing it with the last
 * element in the array. 
 *
 * If no entry with the specified fd is found, the function does nothing.
 *
 * @param id      The id of the entry to remove.
 * @param users   Pointer to the user_array to modify.
 *
 * @return 0 on success.
 *         -1 on failure.
 */
int user_array_delete(int id, struct user_array *users);
