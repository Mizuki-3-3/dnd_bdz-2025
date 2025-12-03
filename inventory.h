#ifndef INVENTORY_H
#define INVENTORY_H

#include "equipment.h"
#include <curses.h>

typedef struct Hero Hero;

#define MAX_EQUIPPED 5
#define BASE_CAPACITY 10
#define CAPACITY_PER_LEVEL 2

typedef enum {
    ITEM_ARTIFACT,
    ITEM_CONSUMABLE
}item_type;
typedef enum {
    SLOT_WEAPON,
    SLOT_ARMOR,
    SLOT_HELMET,
    SLOT_BOOTS,
    SLOT_AMULET
} equipment_slot;

typedef struct inventory_node{
    item_type type;
    union{
        artifact *Art;
        consumable *Cons;
    }item;
    struct inventory_node *next;
}inventory_node;

typedef struct {
    inventory_node *head;
    inventory_node *tail;
    int count; 
    int max_slots;

    artifact *equipped[MAX_EQUIPPED];
    WINDOW *win;
} inventory;

inventory* create_inventory();
void inventory_update_capacity(inventory *inv, int player_level);
int inventory_add_item(inventory *inv, void *item, item_type type);
int inventory_remove_item(inventory *inv, inventory_node *node_to_remove);
inventory_node* inventory_get_node_at_index(inventory *inv, int index);
int inventory_equip_artifact(inventory *inv, artifact *artifact, equipment_slot slot);
int inventory_unequip_artifact(inventory *inv, equipment_slot slot);
void free_inventory(inventory *inv);
void display_inventory(inventory *inv, int selected_index);
int player_use_item(Hero *hero, inventory_node *node);


#endif

