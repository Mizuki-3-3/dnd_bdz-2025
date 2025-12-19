#include "combat.h"
#include <stdlib.h>
#include <time.h>

int calculate_player_damage(Hero *hero, int is_magic, int dice_roll) {
    int damage = 0;
    if (is_magic) {
        damage = hero->magic + dice_roll;
        if (hero->mp >= 5) {
            hero->mp -= 5;
        } else {
            return 0; // нет маны-нет урона
        }
    } else {
        damage = hero->strength + dice_roll;
    }
    
    if (check_crit(hero)) {
        damage = damage * 2; // критический урон в 2 раза больше
    }
    
    return damage;
}

int calculate_monster_damage(Monster *monster, Hero *player, int dice_roll) {
    int damage = monster->strength + dice_roll;
    // каждые 5 ловкости снижают урон на 1
    int dodge_bonus = player->dexterity / 5;
    damage -= dodge_bonus;
    // урон не может быть меньше 1
    if (damage < 1) {
        damage = 1;
    }
    return damage;
}

int check_dodge(Hero *player) {
    
    int dodge_chance = player->dexterity / 10;
    if (dodge_chance > 50) dodge_chance = 50; // Максимум 50%
    
    int roll = rand() % 100;
    if (roll < dodge_chance) {
        return 1; // уклонился
    }
    return 0; // нет
}

int check_crit(Hero *hero) {
    int crit_chance = hero->dexterity / 20;
    if (crit_chance > 25) crit_chance = 25; // Максимум 25%
    
    int roll = rand() % 100;
    if (roll < crit_chance) {
        return 1; // крит
    }
    return 0; // не крит
}