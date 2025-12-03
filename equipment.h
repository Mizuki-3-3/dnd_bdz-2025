#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <string.h>

#define MAX_NAME_LENGTH  32
#define MAX_DESC_LENGTH 70

typedef struct{
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESC_LENGTH];
    int weight_bonus;
    int magic_bonus;
    int strenght_bonus;
    int dexterity_bonus;
    int is_equipped;
}artifact;

typedef enum{
    FOOD,
    HELTH_POT,
    MANA_POT,   //ВОССТАНОВЛЕНИЕ МАНЫ
    STRENGHT_POT,
    DEXTERITY_POT,
    MAGIC_POT    //БАФФ МАГИЧЕСКОЙ СИЛЫ
}consumable_type;

typedef struct {
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESC_LENGTH];
    consumable_type type;
    int quantity;   
    int power;         //сила эффекта
    int duration;
}consumable;


#endif