#include <stdio.h>

#include "user_table.h"

int user_table_add(struct user **user_table, int id)
{
    struct user *new_user;

    if (user_table_find(user_table, id) != NULL)
    {
        printf("user %d already exists in table\n", id);
        return -1;
    }

    new_user = malloc(sizeof(*new_user));
    if (new_user == NULL)
    {
        printf("failed to allocate space for new user\n");
        return -1;
    }

    new_user->id = id;
    strcpy(new_user->name, "anonymous");
    HASH_ADD_INT(*user_table, id, new_user);

    printf("added user %d to user table\n", id);

    return 0;
}

int user_table_delete(struct user **user_table, int id)
{
    struct user *user = user_table_find(user_table, id);
    if (user == NULL)
    {
        printf("user %d does not exist in table\n", id);
        return -1;
    }

    HASH_DEL(*user_table, user);
    free(user);

    printf("deleted user %d from user table\n", id);

    return 0;
}

struct user *user_table_find(struct user **user_table, int id)
{
    struct user *user;
    HASH_FIND_INT(*user_table, &id, user);
    return user;
}