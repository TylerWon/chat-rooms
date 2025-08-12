#include <stdio.h>
#include <stdlib.h>

#include "user_array.h"

#define INITIAL_CAPACITY 5

/**
 * Resizes the provided user_array to hold new_cap elements.
 *
 * @param new_cap   The new capacity of the user_array
 * @param user_array   Pointer to the user_array to resize
 *
 * @return 0 on success.
 *         -1 on failure.
 */
int resize(uint32_t new_cap, struct user_array *user_array)
{
    struct user *users = reallocarray(user_array->users, new_cap, sizeof(struct user));
    if (users == NULL)
        return -1;
    user_array->users = users;
    user_array->capacity = new_cap;

    printf("user_array resized to %d\n", user_array->capacity);

    return 0;
}

struct user_array *user_array_init()
{
    struct user_array *user_array = malloc(sizeof(struct user_array));
    if (user_array == NULL)
        return NULL;

    struct user *users = calloc(INITIAL_CAPACITY, sizeof(struct user));
    if (users == NULL)
    {
        free(user_array);
        return NULL;
    }

    user_array->users = users;
    user_array->len = 0;
    user_array->capacity = INITIAL_CAPACITY;

    return user_array;
}

int user_array_append(int id, struct user_array *user_array)
{
    struct user *users = user_array->users;
    uint32_t len = user_array->len;
    uint32_t capacity = user_array->capacity;

    // Double size if too small
    if (len + 1 > capacity)
        if (resize(2 * capacity, user_array) != 0)
        {
            printf("failed to resize user_array\n");
            return -1;
        }

    users[len].id = id;
    strcpy(users[len].name, "anonymous");
    user_array->len++;

    printf("user %d added to user_array\n", id);

    return 0;
}

int user_array_delete(int id, struct user_array *user_array)
{
    struct user *users = user_array->users;
    uint32_t len = user_array->len;
    uint32_t capacity = user_array->capacity;

    uint32_t i = 0;
    for (i = 0; i < len; i++)
    {
        if (users[i].id == id)
            break;
    }

    if (i == len)
    {
        printf("user %d does not exist in user_array\n", id);
        return -1;
    }

    users[i] = users[len - 1];
    user_array->len--;

    printf("user %d removed from user_array\n", id);

    // Halve size if at least half empty, unless resulting array would be smaller than the initial capacity
    if (user_array->len <= capacity / 2 && capacity / 2 >= INITIAL_CAPACITY)
    {
        resize(capacity / 2, user_array); // No need to return error if resize fails because user_array is still valid
    }

    return 0;
}
