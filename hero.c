#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hero.h"

Hero* create_hero(const char* name, int class) {
    if (!name || strlen(name) == 0) return NULL;

    Hero* h = malloc(sizeof(Hero));
    if (!h) return NULL;
    switch (class){
        case 1:
            h->level = 1;
            h->hp = 20;
            h->mp = 35;
            h->max_hp = 20;
            h->max_mp = 35;
            h->strength = 10;
            h->dexterity = 10;
            h->magic = 20;
            h->exp = 0;
            h->exp_to_next = 50;
            h->base_strength = h->strength;
            h->base_dexterity = h->dexterity;
            h->base_magic=h->magic;
            h->effect_count=0;
            h->type = MAGICIAN;
            for (int i = 0; i < MAX_CONSUMABLE_EFFECTS; i++) {
                h->active_effects[i].type = FOOD;  
                h->active_effects[i].power = 0;
                h->active_effects[i].remaining_duration = 0;
            }
            break;
        case 2:
            h->level = 1;
            h->hp = 20;
            h->mp = 35;
            h->max_hp = 20;
            h->max_mp = 35;
            h->strength = 10;
            h->dexterity = 10;
            h->magic = 20;
            h->exp = 0;
            h->exp_to_next = 50;
            h->base_strength = h->strength;
            h->base_dexterity = h->dexterity;
            h->base_magic=h->magic;
            h->effect_count=0;
            h->type = MAGICIAN;
            for (int i = 0; i < MAX_CONSUMABLE_EFFECTS; i++) {
                h->active_effects[i].type = FOOD;  
                h->active_effects[i].power = 0;
                h->active_effects[i].remaining_duration = 0;
            }
            break;
        case 3:
            h->level = 1;
            h->hp = 20;
            h->mp = 35;
            h->max_hp = 20;
            h->max_mp = 35;
            h->strength = 10;
            h->dexterity = 10;
            h->magic = 20;
            h->exp = 0;
            h->exp_to_next = 50;
            h->base_strength = h->strength;
            h->base_dexterity = h->dexterity;
            h->base_magic=h->magic;
            h->effect_count=0;
            h->type = MAGICIAN;
            for (int i = 0; i < MAX_CONSUMABLE_EFFECTS; i++) {
                h->active_effects[i].type = FOOD;  
                h->active_effects[i].power = 0;
                h->active_effects[i].remaining_duration = 0;
            }
            break;
            
        default:
            break;
        }
    return h;
}
void destroy_hero(Hero* hero) {
    free(hero);
}
int save_hero(const Hero* hero, const char* name) {
    if (!hero || !name) return 0;
    char filename[256];
    snprintf(filename, sizeof(filename), "%s.save", name);
    FILE* f = fopen(filename, "wb");
    if (!f) return 0;
    fwrite(hero, sizeof(Hero), 1, f);
    fclose(f);
    return 1;
}
int load_hero(Hero* hero, const char* name) {
    if (!hero || !name) return 0;
    char filename[256];
    snprintf(filename, sizeof(filename), "%s.save", name);
    FILE* f = fopen(filename, "rb");
    if (!f) return 0;
    size_t read = fread(hero, sizeof(Hero), 1, f);
    fclose(f);
    return (read == 1);
}

void level_up(Hero *hero, inventory *inv){
    int choice;
    if (!hero) return;
    if (hero->exp_to_next<=hero->exp){
        hero->level++;
        hero->exp -= hero->exp_to_next;
        hero->exp_to_next = (int)(hero->exp_to_next * 1.5);
        printf("Вы достигли %d уровня!\n", hero->level);
        printf("Распределите 5 очков улучшения:\n");
        printf("1. +5 к здоровью\n2. +2 к силе\n3. +2 к грузоподъемности\nВаш выбор: ");
    }

    scanf("%d", &choice);
    switch (choice) {
        case 1:
            hero->max_hp += 5;
            break;
        case 2:
            hero->base_strength += 2;
            break;
        
        case 3:
            inventory_update_capacity(inv, hero->level);
            break;
        default: printf("Выбор некорректен, попробуйте ещё раз\n"); level_up(hero, inv);
    }
    printf("Характеристики обновлены!");
}