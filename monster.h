#ifndef MONSTER_H
#define MONSTER_H

#include "inventory.h"

#define MAX_NAME_LENGTH  32
#define MAX_LOOT_COUNT 3

typedef struct {

    int level;
    int health;
    int strength;
    int loot_id[MAX_LOOT_COUNT];
    int is_killed;
} Monster;


#endif