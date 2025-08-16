#include <stdio.h>

#include "room.h"

int room_add_user(struct room *room, struct user *user)
{
    if (room->num_users == MAX_USERS_PER_ROOM)
    {
        printf("room %d is full\n", room->id);
        return -1;
    }
    else if (user->room != INVALID_ROOM)
    {
        printf("user %d already in room %d\n", user->id, user->room);
        return -1;
    }

    room->users[room->num_users] = user->id;
    room->num_users++;
    user->id = room->id;

    return 0;
}

int room_remove_user(struct room *room, struct user *user)
{
    if (room->id != user->room)
    {
        printf("user %d is not in room %d\n", user->id, room->id);
        return -1;
    }

    int i;
    for (i = 0; i < room->num_users; i++)
    {
        if (room->users[i] == user->id)
            break;
    }

    room->users[i] = room->users[room->num_users - 1];
    room->num_users--;
    user->room = INVALID_ROOM;

    return 0;
}
