#include <poll.h>
#include <stdint.h>

#include "../messages/chat_message.h"
#include "../lib/uthash.h"

// Represents a user
struct user
{
    int id;
    char name[NAME_SIZE_LIMIT];
    UT_hash_handle hh; // Makes the structure hashable with uthash
};

/**
 * Adds a user with the specified id to the given user hash table.
 *
 * @param user_table    Pointer to the hash table to modify.
 * @param id            The id of the user.
 *
 * @return 0 on success.
 *         -1 on failure.
 */
int user_table_add(struct user **user_table, int id);

/**
 * Removes the user with the specified id from the given user hash table.
 *
 * @param user_table    Pointer to the hash table to modify.
 * @param id            The id of the user to remove.
 *
 * @return 0 on success.
 *         -1 on failure.
 */
int user_table_delete(struct user **user_table, int id);

/**
 * Finds the user with the specified id in the given user hash table.
 *
 * @param user_table    Pointer to the hash table to search.
 * @param id            The id of the user to remove.
 *
 * @return  Pointer to the user.
 *          NULL if the user is not found.
 */
struct user *user_table_find(struct user **user_table, int fd);
