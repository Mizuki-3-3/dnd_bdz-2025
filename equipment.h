#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <string.h>

#define MAX_NAME_LENGTH  32
#define MAX_ITEMS 100

typedef enum{
    FOOD,
    HEALTH_POT,
    MANA_POT,   //ВОССТАНОВЛЕНИЕ МАНЫ
    STRENGTH_POT,
    DEXTERITY_POT,
    MAGIC_POT    //БАФФ МАГИЧЕСКОЙ СИЛЫ
}consumable_type;

typedef struct{
    int weight_bonus;
    int magic_bonus;
    int strength_bonus;
    int dexterity_bonus;
}artifact;

typedef struct {
    consumable_type type;   
    int power;
    int duration;         //сила эффекта
}consumable;


#endif