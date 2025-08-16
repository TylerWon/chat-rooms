#include "../data_structures/user_table.h"

#define INVALID_ROOM 0
#define MAX_USERS_PER_ROOM 25

struct room
{
    uint8_t id;
    int users[MAX_USERS_PER_ROOM]; // Stores user ids
    uint8_t num_users;
};

/**
 * Adds a user to the room. Returns an error if the room is full or user is already in a room.
 *
 * @param room      Pointer to the room
 * @param user_id   Id of the user
 *
 * @return  0 on success.
 *          -1 if the room is full.
 *          -2 if the user is already in a room.
 */
int room_add_user(struct room *room, struct user *user);

/**
 * Removes a user from the room. Returns an error if the user is not in the room.
 *
 * @param room      Pointer to the room
 * @param user_id   Id of the user
 *
 * @return  0 on success.
 *          -1 on error.
 */
int room_remove_user(struct room *room, struct user *user);
