#ifndef INVENTORY_H
#define INVENTORY_H

#include "equipment.h"
#include <curses.h>

typedef struct Hero Hero;
typedef struct item_database item_database;

#define MAX_EQUIPPED 4
#define BASE_CAPACITY 10
#define CAPACITY_PER_LEVEL 2

typedef enum {
    ITEM_ARTIFACT,
    ITEM_CONSUMABLE
}item_type;

typedef enum {
    SLOT_WEAPON = 0,
    SLOT_ARMOR,
    SLOT_PANTS,
    SLOT_BOOTS,
} equipment_slot;

typedef struct inventory_node {
    item_type type;
    int item_id;
    union {
        struct {
            int is_equipped;
            equipment_slot slot; 
        } artifact_state;
        struct {
            int quantity;
        } consumable_state;
    } state;
    struct inventory_node *next;
} inventory_node;



typedef struct {
    inventory_node *head;
    inventory_node *tail;
    int count; 
    int max_slots;
    inventory_node *equipped[MAX_EQUIPPED];
    WINDOW *win;
} inventory;

inventory* create_inventory();
void inventory_update_capacity(inventory *inv, int player_level);
int inventory_add_item_by_id(inventory *inv, item_database *db, int item_id, int quantity);

inventory_node* inventory_get_node_at_index(inventory *inv, int index);
int inventory_equip_artifact(inventory *inv, inventory_node *node, item_database *db);
int inventory_unequip_artifact(inventory *inv, equipment_slot slot);
void free_inventory(inventory *inv);
void display_inventory(inventory *inv, item_database *db, Hero *hero, int selected_index);

// функции использования предметов
int inventory_use_consumable(Hero *hero, inventory *inv, inventory_node *node, item_database *db);
void calculate_stats_from_equipment(Hero *hero, inventory *inv, item_database *db);
int inventory_remove_item(inventory *inv, inventory_node *node_to_remove);
#endif

