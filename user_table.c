#include "user_table.h"

int user_table_add(int id, struct user **user_table)
{
    struct user *new_user;

    if (user_table_find(id, user_table) != NULL)
    {
        printf("user with id %d already exists in table\n");
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

    return 0;
}

int user_table_delete(int id, struct user **user_table)
{
    struct user *user = user_table_find(id, user_table);
    if (user == NULL)
    {
        printf("user with id %d does not exist in table\n");
        return -1;
    }

    HASH_DEL(*user_table, user);
    free(user);

    return 0;
}

struct user *user_table_find(int id, struct user **user_table)
{
    struct user *user;
    HASH_FIND_INT(*user_table, &id, user);
    return user;
}