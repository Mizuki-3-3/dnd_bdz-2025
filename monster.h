#ifndef MONSTER_H
#define MONSTER_H

typedef struct {
    int level;
    int health;
    int strength;
    int is_killed;
} Monster;

Monster* create_monster(int hero_level);

#endif