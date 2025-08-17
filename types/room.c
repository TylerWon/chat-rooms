#include <stdio.h>

#include "room.h"
#include "../lib/log.h"

int room_add_user(struct room *room, struct user *user)
{
    if (room->num_users == MAX_USERS_PER_ROOM)
    {
        LOG_ERROR("room %d is full", room->id);
        return -1;
    }

    room->users[room->num_users] = user->id;
    room->num_users++;
    user->room = room->id;

    LOG_INFO("added user %d to room %d", user->id, room->id);

    return 0;
}

int room_remove_user(struct room *room, struct user *user)
{
    if (room->id != user->room)
    {
        LOG_ERROR("user %d is not in room %d", user->id, room->id);
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

    LOG_INFO("removed user %d from room %d", user->id, room->id);

    return 0;
}
