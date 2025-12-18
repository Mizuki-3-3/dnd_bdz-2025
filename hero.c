#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hero.h"

Hero* create_hero(const char* name, int class) {
    if (!name || strlen(name) == 0) strncpy(name, "Браконьер", sizeof("Браконьер"));

    Hero* h = malloc(sizeof(Hero));
    if (!h) return NULL;
    strncpy(h->name, name, MAX_NAME_LENGTH - 1);
    h->name[MAX_NAME_LENGTH - 1] = '\0';

    switch (class){
        case 1: // маг
            h->level = 1;
            h->hp = 25;
            h->mp = h->max_mp = 40;
            h->max_hp = 25;
            h->strength = 6;    // слабый физически
            h->dexterity = 8;  // средняя ловкость
            h->magic = 18;      // сильная магия
            h->exp = 0;
            h->exp_to_next = 50;
            h->type = MAGICIAN;
            break;
        case 2: // рыцарь
            h->level = 1;
            h->hp = h->max_hp = 40;
            h->mp = h->max_mp = 15;
            h->strength = 18;   // сильный физически
            h->dexterity = 7;   // низкая ловкость
            h->magic = 5;       // слабая магия
            h->exp = 0;
            h->exp_to_next = 50;
            h->type = KNIGHT;
            break;
        
        case 3: // плут
            h->level = 1;
            h->hp = h->max_hp = 30;
            h->mp = h->max_mp = 25;
            h->strength = 12;   // средняя сила
            h->dexterity = 16;  // высокая ловкость
            h->magic = 9;      // средняя магия
            h->exp = 0;
            h->exp_to_next = 50;
            h->type = RASCAL;
            break;
        
        default:
            free(h);
            return NULL;
    }
    h->base_strength = h->strength;
    h->base_dexterity = h->dexterity;
    h->base_magic = h->magic;
    h->effect_count = 0;
    return h;
}
void destroy_hero(Hero* hero) {
    free(hero);
}

void level_up(Hero *hero, inventory *inv) {
    if (!hero) return;
    
    if (hero->exp >= hero->exp_to_next) {
        hero->level++;
        hero->exp -= hero->exp_to_next;
        hero->exp_to_next = (int)(hero->exp_to_next * 1.5);
        
        hero->max_hp += 10;
        hero->hp = hero->max_hp;
        hero->max_mp += 5;
        hero->mp = hero->max_mp;
        hero->base_strength += 2;
        hero->base_dexterity += 1;
        hero->base_magic += 1;
        
        hero->strength = hero->base_strength;
        hero->dexterity = hero->base_dexterity;
        hero->magic = hero->base_magic;

        if (inv) {
            inventory_update_capacity(inv, hero->level);
        }
    }
}

void heal_hero(Hero *hero, int amount) {
    if (!hero) return;
    
    hero->hp += amount;
    if (hero->hp > hero->max_hp) {
        hero->hp = hero->max_hp;
    }
}

void apply_effect(Hero *hero, consumable_type type, int power, int duration) {
    if (!hero || hero->effect_count >= MAX_CONSUMABLE_EFFECTS) return;
    
    hero->active_effects[hero->effect_count].type = type;
    hero->active_effects[hero->effect_count].power = power;
    hero->active_effects[hero->effect_count].remaining_duration = duration;
    hero->effect_count++;
    
    switch (type) {
        case STRENGTH_POT:
            hero->strength += power;
            break;
        case DEXTERITY_POT:
            hero->dexterity += power;
            break;
        case MAGIC_POT:
            hero->magic += power;
            break;
        default:
            break;
    }
}

void update_effects(Hero *hero) {
    if (!hero) return;
    
    for (int i = 0; i < hero->effect_count; i++) {
        hero->active_effects[i].remaining_duration--;
        
        // Если эффект закончился, убираем его
        if (hero->active_effects[i].remaining_duration <= 0) {
            // Убираем бонус
            switch (hero->active_effects[i].type) {
                case STRENGTH_POT:
                    hero->strength -= hero->active_effects[i].power;
                    break;
                case DEXTERITY_POT:
                    hero->dexterity -= hero->active_effects[i].power;
                    break;
                case MAGIC_POT:
                    hero->magic -= hero->active_effects[i].power;
                    break;
                default:
                    break;
            }
            
            // Сдвигаем остальные эффекты
            for (int j = i; j < hero->effect_count - 1; j++) {
                hero->active_effects[j] = hero->active_effects[j + 1];
            }
            hero->effect_count--;
            i--; // Проверяем текущую позицию снова
        }
    }
}