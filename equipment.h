#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <string.h>

#define MAX_NAME_LENGTH  64

typedef enum{
    FOOD,
    HEALTH_POT,
    MANA_POT,   //ВОССТАНОВЛЕНИЕ МАНЫ
    STRENGTH_POT,
    DEXTERITY_POT,
    MAGIC_POT    //БАФФ МАГИЧЕСКОЙ СИЛЫ
}consumable_type;

typedef enum {
    ART_WEAPON,
    ART_ARMOR,
    ART_PANTS,
    ART_BOOTS,
} artifact_type;

typedef struct{
    int weight_bonus;
    int magic_bonus;
    int strength_bonus;
    int dexterity_bonus;
    artifact_type type;
}artifact;

typedef struct {
    consumable_type type;   
    int power;
    int duration;         //сила эффекта
}consumable;


#endif