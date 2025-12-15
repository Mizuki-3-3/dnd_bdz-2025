#ifndef DATABASE_H
#define DATABASE_H

#include "monster.h"
#include "equipment.h"
#include "location.h"


#define MAX_NAME_LENGTH  32
#define MAX_DESC_LENGTH 300
#define MAX_ITEMS 100

typedef struct{
    int id;
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESC_LENGTH];
    union{
        artifact artifact_template;
        consumable consumable_template;
        location location_template;
        Monster monster_template; 
    }template;
} item_template;

typedef struct item_database{
    item_template items[MAX_ITEMS];
    int count;
} item_database;

void itemdb_init(item_database* db);
item_template* itemdb_create_artifact(item_database* db, const char* name, const char* desc, int weight_bonus, int magic_bonus, int strength_bonus, int dexterity_bonus, int id);
item_template* itemdb_create_consumable(item_database* db, const char* name, const char* desc, consumable_type type, int power, int duration, int id);
item_template* itemdb_find_by_id(item_database* db, int id);
item_template* itemdb_create_location(item_database* db, const char* name, const char* desc, int exits_count, loc_type type, int id);
void init_default_items(item_database* db);

#endif