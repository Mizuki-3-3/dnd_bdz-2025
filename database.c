#include "database.h"
#include <stdlib.h>





void itemdb_init(item_database* db) {
    if (!db) return;
    db->count = 0;

    for (int i = 0; i < MAX_ITEMS; i++) {
        db->items[i].id = 0;
    }
}

item_template* itemdb_create_artifact(item_database* db, const char* name, const char* desc, int weight_bonus, int magic_bonus, int strength_bonus, int dexterity_bonus, int id) {
    if (!db || db->count >= MAX_ITEMS) return NULL;
    
    item_template* item = &db->items[db->count];

    item->id = 200+id; 
    
    strncpy(item->name, name, MAX_NAME_LENGTH - 1);
    strncpy(item->description, desc, MAX_DESC_LENGTH - 1);
    item->template.artifact_template.weight_bonus = weight_bonus;
    item->template.artifact_template.magic_bonus = magic_bonus;
    item->template.artifact_template.strength_bonus = strength_bonus;
    item->template.artifact_template.dexterity_bonus = dexterity_bonus;
    
    db->count++;
    return item;
}

item_template* itemdb_create_consumable(item_database* db, const char* name, const char* desc, consumable_type type, int power, int duration, int id) {
    if (!db || db->count >= MAX_ITEMS) return NULL;
    
    item_template* item = &db->items[db->count];
    
    item->id = 100+id;
    
    strncpy(item->name, name, MAX_NAME_LENGTH - 1);
    strncpy(item->description, desc, MAX_DESC_LENGTH - 1);
    item->template.consumable_template.type = type;
    item->template.consumable_template.power = power;
    item->template.consumable_template.duration = duration;
    
    db->count++;
    return item;
}


item_template* itemdb_create_monster(item_database* db, const char* name, int level, int id, int loot1, int loot2, int loot3){
    if (!db || db->count >= MAX_ITEMS) return NULL;

    item_template* m = &db->items[db->count];
    
    
    m->id = 300+id;
    strncpy(m->name, name, MAX_NAME_LENGTH - 1);
    m->template.monster_template.is_killed = 0;
    m->template.monster_template.health = level*3;
    m->template.monster_template.level = level;
    m->template.monster_template.strength = level+3;
    m->template.monster_template.loot_id[0] = loot1;
    m->template.monster_template.loot_id[0] = loot2;
    m->template.monster_template.loot_id[0] = loot3;
}

item_template* itemdb_create_location(item_database* db, const char* name, const char* desc, int exits_count, loc_type type, int id){
    if (!db || db->count >= MAX_ITEMS) return NULL;
    item_template* loc = &db->items[db->count];
    loc->id = 400+id;
    strncpy(loc->name, name, MAX_NAME_LENGTH - 1);
    strncpy(loc->template.location_template.description, desc, MAX_DESC_LENGTH - 1);
    loc->template.location_template.exit_count = exits_count;
    loc->template.location_template.type = loc->template.location_template.original_type = type;
    
}

item_template* itemdb_find_by_id(item_database* db, int id) {
    if (!db) return NULL;
    
    for (int i = 0; i <= db->count; i++) {
        if (db->items[i].id == id) {
            return &db->items[i];
        }
    }
    return NULL;
}



void init_default_items(item_database* db) {
    if (!db) return;
    
    // Все предметы игры здесь
    itemdb_create_location(db, "Берег бурной реки", " ",2, LOC_EMPTY, 0);
    itemdb_create_location(db, "Лесная часовенка", " ",2, LOC_MONSTER, 1);
    itemdb_create_location(db, "Пещера молчаливых масок", " ",2, LOC_TREASURE, 2);
    itemdb_create_location(db, "Старый указатель", " ",2, LOC_EMPTY, 3);
    itemdb_create_location(db, "Круг поваленных гигантов", " ",2, LOC_MONSTER, 4);
    itemdb_create_location(db, "Холодный родник", " ",2, LOC_TREASURE, 5);
    itemdb_create_location(db, "Лабаз в корнях", " ",2, LOC_MONSTER, 6);

    itemdb_create_artifact(db, "Шило", "Ржавый, но острый резак. Всегда с тобой. Рукоять липкая от смолы.", 0, 0, 5, 0, 1);
    itemdb_create_artifact(db, "Мокрые сапоги", "Базовая броня", 5, 0, 0, 0, 2);
    itemdb_create_artifact(db, "Простая рубаха", "Мокрая, грубая ткань. Холодная и тяжёлая, пахнет тиной.", 0, 0, 0, 0,3);
    itemdb_create_artifact(db, "Простые портки", "Промокшие холщовые штаны. Намокли, стали неудобными и тяжёлыми.", 0,0,0,0,4);
    itemdb_create_consumable(db, "Аптечка", "Лечит раны", HELTH_POT, 30, 0, 3);
}