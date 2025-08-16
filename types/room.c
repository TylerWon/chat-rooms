#include <stdio.h>

#include "room.h"

int room_add_user(struct room *room, struct user *user)
{
    if (room->num_users == MAX_USERS_PER_ROOM)
        return -1;

    room->users[room->num_users] = user->id;
    room->num_users++;
    user->room = room->id;

    printf("added user %d to room %d\n", user->id, room->id);

    return 0;
}

int room_remove_user(struct room *room, struct user *user)
{
    if (room->id != user->room)
        return -1;

    int i;
    for (i = 0; i < room->num_users; i++)
    {
        if (room->users[i] == user->id)
            break;
    }

    room->users[i] = room->users[room->num_users - 1];
    room->num_users--;
    user->room = INVALID_ROOM;

    printf("removed user %d from room %d\n", user->id, room->id);

    return 0;
}
