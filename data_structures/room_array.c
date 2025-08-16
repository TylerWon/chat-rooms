#include <stdio.h>
#include <stdlib.h>

#include "room_array.h"

struct room_array *room_array_init(int n)
{
    struct room_array *room_array = calloc(n, sizeof(struct room));
    if (room_array == NULL)
        return NULL;

    for (int i = 0; i < n; i++)
    {
        room_array->rooms[i].id = i + 1; // room 1 at index 0
        room_array->rooms[i].num_users = 0;
    }
    room_array->len = n;

    return room_array;
}

struct room *room_array_get_room(struct room_array *room_array, int id)
{
    if (id < 1 || id > room_array->len) // Room ids start at 1 and go to len
    {
        printf("room %d not in room array\n", id);
        return NULL;
    }

    return &room_array->rooms[id - 1];
}
