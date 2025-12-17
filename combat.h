#ifndef COMBAT_H
#define COMBAT_H

#include "hero.h"
#include "monster.h"

int calculate_player_damage(Hero *hero, int is_magic, int dice_roll);
int calculate_monster_damage(Monster *monster, Hero *player, int dice_roll);
int check_dodge(Hero *player);  // возвращает 1 если уклонился
int check_crit(Hero *attacker); // возвращает 1 если крит

#endif