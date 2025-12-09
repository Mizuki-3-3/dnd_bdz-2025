#include "equipment.h"
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
    
    // Устанавливаем ID (начинаем с 1)
    item->id = id;
    item->is_artifact = 1;  // Это артефакт
    
    strncpy(item->name, name, MAX_NAME_LENGTH - 1);
    strncpy(item->description, desc, MAX_DESC_LENGTH - 1);
    
    // Заполняем шаблон
    strncpy(item->template.artifact_template.name, name, MAX_NAME_LENGTH - 1);
    strncpy(item->template.artifact_template.description, desc, MAX_DESC_LENGTH - 1);
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
    
    // Устанавливаем ID (начинаем с 100 для расходников)
    item->id = id;
    item->is_artifact = 0;  // Это расходник
    
    // Копируем имя и описание
    strncpy(item->name, name, MAX_NAME_LENGTH - 1);
    strncpy(item->description, desc, MAX_DESC_LENGTH - 1);
    
    // Заполняем шаблон расходника
    strncpy(item->template.consumable_template.name, name, MAX_NAME_LENGTH - 1);
    strncpy(item->template.consumable_template.description, desc, MAX_DESC_LENGTH - 1);
    item->template.consumable_template.type = type;
    item->template.consumable_template.power = power;
    item->template.consumable_template.duration = duration;
    
    db->count++;
    return item;
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
    itemdb_create_artifact(db, "Меч", "Базовое оружие", 0, 0, 5, 0, 1);
    itemdb_create_artifact(db, "Броня", "Базовая броня", 5, 0, 0, 0, 2);
    itemdb_create_consumable(db, "Аптечка", "Лечит раны", HELTH_POT, 30, 0, 3);
}