#include <stdint.h>

#include "messages/name_message.h"
#include "../lib/uthash.h"

// Represents a user
struct user
{
    int id;
    int room;
    char name[NAME_SIZE_LIMIT];
    UT_hash_handle hh; // Makes the structure hashable with uthash
};
