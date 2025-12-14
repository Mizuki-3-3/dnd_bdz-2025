#ifndef HERO_H
#define HERO_H


#include "inventory.h"

#define MAX_NAME_LENGTH  32
#define MAX_CONSUMABLE_EFFECTS 10
typedef enum{
    MAGICIAN,
    KNIGHT,
    RASCAL
}hero_class;

typedef struct {
    consumable_type type;    // Тип зелья (STRENGHT_POT, DEXTERITY_POT и т.д.)
    int power;               // Сила эффекта
    int remaining_duration;  // Оставшаяся длительность в ходах
} consumable_effect;


typedef struct Hero{
    char name[MAX_NAME_LENGTH];
    hero_class type;
    int level;
    int hp;
    int mp;
    int max_hp;
    int max_mp;
    int strength;        //статы для рассчета
    int dexterity;
    int magic;
    int base_strength;  //статы без усилений (чтоб вернуться)
    int base_dexterity;
    int base_magic;
    int exp;
    int exp_to_next;
    consumable_effect active_effects[MAX_CONSUMABLE_EFFECTS];  //какие зельки заюзаны
    int effect_count;
    int current_location;  // Текущая локация героя
    int prev_location; 
} Hero;

Hero* create_hero(const char* name, int class);
void level_up(Hero *hero, inventory *inv);
int save_hero(const Hero* hero, const char* name);
int load_hero(Hero* hero, const char* name);
void destroy_hero(Hero* hero);
void heal_hero(Hero *hero, int amount);
void apply_effect(Hero *hero, consumable_type type, int power, int duration);
void update_effects(Hero *hero);

#endif