#include <stdio.h>
#include <stdlib.h>

#include "room_array.h"
#include "../lib/log.h"

struct room_array *room_array_init(int n)
{
    struct room_array *room_array = malloc(sizeof(struct room_array));
    if (room_array == NULL)
    {
        LOG_ERROR("failed to allocate space for room array");
        return NULL;
    }

    struct room *rooms = calloc(n, sizeof(struct room));
    if (rooms == NULL)
    {
        LOG_ERROR("failed to allocate space for %d rooms", n);
        free(room_array);
        return NULL;
    }
    room_array->rooms = rooms;
    room_array->len = n;

    for (int i = 0; i < room_array->len; i++)
    {
        room_array->rooms[i].id = i + 1; // room 1 at index 0
        room_array->rooms[i].num_users = 0;
    }

    return room_array;
}

struct room *room_array_get_room(struct room_array *room_array, int id)
{
    if (id < 1 || id > room_array->len) // Room ids start at 1 and go to len
    {
        LOG_ERROR("room %d not in room array", id);
        return NULL;
    }

    return &room_array->rooms[id - 1];
}
