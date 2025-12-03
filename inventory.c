#include "inventory.h"
#include <stdlib.h>


inventory* create_inventory(void) {
    inventory *inv = malloc(sizeof(inventory));
    if (!inv) return NULL;
    
    inv->head = NULL;
    inv->tail = NULL;
    inv->count = 0;
    inv->max_slots = BASE_CAPACITY;
    inv->win = NULL;
    
    for (int i = 0; i < MAX_EQUIPPED; i++) {
        inv->equipped[i] = NULL;     //сначала инвентарь пустой
    }
    
    return inv;
}

void inventory_update_capacity(inventory *inv, int player_level) {
    if (!inv) return;
    inv->max_slots = BASE_CAPACITY + ((player_level-1) * CAPACITY_PER_LEVEL);
}


int inventory_add_item(inventory *inv, void *item, item_type type) {
    if (!inv || !item) {
        return 0;  // проверка выделения памяти под инвентарь и под предмет
    }
    
    if (inv->count >= inv->max_slots) {
        return -1;  // Инвентарь переполнен
    }
    inventory_node *new_node = malloc(sizeof(inventory_node));
    if (!new_node) {
        return 0;  // память не выделилась подэлемент в инвентаре
    }
    
    new_node->type = type;
    new_node->next = NULL;

    if (type == ITEM_ARTIFACT) {
        new_node->item.Art = (artifact*)item;
    } else {
        new_node->item.Cons = (consumable*)item;
    }
    if (!inv->head) {
        inv->head = new_node;
        inv->tail = new_node;
    } else {
        inv->tail->next = new_node;
        inv->tail = new_node;
    }
    inv->count++;
    return 1;
}

int inventory_remove_item(inventory *inv, inventory_node *node_to_remove) {
    if (!inv || !node_to_remove) return 0; //проверка на выд памяти
    
    if (!inv->head) return 0;     // оно пустое
    
    if (inv->head == node_to_remove) {
        inventory_node *temp = inv->head;
        
        
        if (temp->type == ITEM_ARTIFACT) {    //надо снять экипированое 
            for (int i = 0; i < MAX_EQUIPPED; i++) {
                if (inv->equipped[i] == temp->item.Art) {
                    inv->equipped[i] = NULL;
                }
            }
        }
        
        inv->head = inv->head->next;
        
        if (temp->type == ITEM_ARTIFACT) {
            free(temp->item.Art);
        } else {
            free(temp->item.Cons);
        }
        
        free(temp);
        inv->count--;
        
        // Если список стал пустым
        if (!inv->head) {
            inv->tail = NULL;
        }
        
        return 1;
    }
    
    inventory_node *current = inv->head;
    while (current->next && current->next != node_to_remove) {
        current = current->next;
    }
    
    if (!current->next) return 0;  // проверка что арт\вкусняшка нашелся
    
    inventory_node *temp = current->next;
    current->next = temp->next; //слепили предыдущий со следущим
    
    if (temp == inv->tail) {
        inv->tail = current;
    }
    
    if (temp->type == ITEM_ARTIFACT) {
        for (int i = 0; i < MAX_EQUIPPED; i++) {
            if (inv->equipped[i] == temp->item.Art) {
                inv->equipped[i] = NULL;
            }
        }
    }
    
    inv->count--;
    
    if (temp->type == ITEM_ARTIFACT) {  //чистим
        free(temp->item.Art);
    } else {
        free(temp->item.Cons);
    }
    
    free(temp);
    return 1;
}

inventory_node* inventory_get_node_at_index(inventory *inv, int index) {
    if (!inv || index < 0 || index >= inv->count) {
        return NULL;
    }
    
    inventory_node *current = inv->head;
    for (int i = 0; (i < index) && current; i++) {
        current = current->next;
    }
    
    return current;
}

int inventory_equip_artifact(inventory *inv, artifact *artifact, equipment_slot slot) {
    if (!inv || !artifact || slot < 0 || slot >= MAX_EQUIPPED) {
        return 0;
    }
    
    inventory_node *current = inv->head;
    int found = 0;
    while (current) {
        if (current->type == ITEM_ARTIFACT && current->item.Art == artifact) {
            found = 1;
            break;
        }
        current = current->next;
    }
    
    if (!found) return 0;
    
    // Снимаем предыдущий предмет из этого слота если есть
    if (inv->equipped[slot]) {
        inv->equipped[slot]->is_equipped = 0;
    }
    
    // Экипируем новый
    inv->equipped[slot] = artifact;
    artifact->is_equipped = 1;
    
    return 1;
}

int inventory_unequip_artifact(inventory *inv, equipment_slot slot) {
    if (!inv || slot < 0 || slot >= MAX_EQUIPPED || !inv->equipped[slot]) {
        return 0;
    }
    
    inv->equipped[slot]->is_equipped = 0;
    inv->equipped[slot] = NULL;
    
    return 1;
}

void free_inventory(inventory *inv) {
    if (!inv) return;
    
    inventory_node *current = inv->head;
    while (current) {
        inventory_node *next = current->next;

        if (current->type == ITEM_ARTIFACT) {    //чистим чистим
            free(current->item.Art);
        } else {
            free(current->item.Cons);
        }
        
        free(current);
        current = next;
    }
    free(inv);
}