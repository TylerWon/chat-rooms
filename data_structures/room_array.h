#ifndef ROOM_ARRAY_H
#define ROOM_ARRAY_H

#include <stdint.h>

#include "../types/room.h"

// An array of rooms
struct room_array
{
    struct room *rooms;
    uint8_t len;
};

/**
 * Initializes an array of n rooms.
 *
 * @param n The number of rooms in the array
 *
 * @return  Pointer to the array of rooms on success.
 *          NULL if initialization fails.
 */
struct room_array *room_array_init(int n);

/**
 * Gets the room with the given id from the array of rooms.
 *
 * @param rooms     Pointer to the array of rooms
 * @param id        The id of the room
 *
 * @return  Pointer to the room on success.
 *          NULL if the room does not exist.
 */
struct room *room_array_get_room(struct room_array *rooms, int id);

#endif
