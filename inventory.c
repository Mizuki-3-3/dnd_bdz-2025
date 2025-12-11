#include "inventory.h"
#include "database.h"
#include <stdlib.h>

extern item_database global_itemdb;


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


int inventory_add_item_by_id(inventory *inv, item_database *db, int item_id, int quantity) {
    if (!inv || !db) {
        return 0;  // проверка выделения памяти под инвентарь и под предмет
    }
    
    if (inv->count >= inv->max_slots) {
        return -1;  // Инвентарь переполнен
    }

    item_template* template = itemdb_find_by_id(db, item_id);
    if (!template) {
        return 0;  // Предмет не найден в базе
    }

    inventory_node *new_node = malloc(sizeof(inventory_node));
    if (!new_node) {
        return 0;  // память не выделилась подэлемент в инвентаре
    }
    
    new_node->item_id = item_id;
    new_node->next = NULL;

    if (template->id>200) {
        new_node->type = ITEM_ARTIFACT;
        new_node->state.artifact_state.is_equipped = 0;  // Не надет
    } else {
        new_node->type = ITEM_CONSUMABLE;
        new_node->state.consumable_state.quantity = quantity;
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
    if (!inv || !node_to_remove) return 0;
    
    if (!inv->head) return 0;
    
    // Если удаляем первый элемент
    if (inv->head == node_to_remove) {
        inventory_node *temp = inv->head;
        
        // Если предмет был экипирован - снимаем
        if (temp->type == ITEM_ARTIFACT && temp->state.artifact_state.is_equipped) {
            for (int i = 0; i < MAX_EQUIPPED; i++) {
                if (inv->equipped[i] == temp) {
                    inv->equipped[i] = NULL;
                    break;
                }
            }
        }
        
        inv->head = inv->head->next;
        free(temp);
        inv->count--;
        
        // Если список стал пустым
        if (!inv->head) {
            inv->tail = NULL;
        }
        
        return 1;
    }
    
    // Ищем элемент в списке
    inventory_node *current = inv->head;
    while (current->next && current->next != node_to_remove) {
        current = current->next;
    }
    
    if (!current->next) return 0;  // Элемент не найден
    
    inventory_node *temp = current->next;
    current->next = temp->next;
    
    if (temp == inv->tail) {
        inv->tail = current;
    }
    
    // Если предмет был экипирован - снимаем
    if (temp->type == ITEM_ARTIFACT && temp->state.artifact_state.is_equipped) {
        for (int i = 0; i < MAX_EQUIPPED; i++) {
            if (inv->equipped[i] == temp) {
                inv->equipped[i] = NULL;
                break;
            }
        }
    }
    
    free(temp);
    inv->count--;
    return 1;
}

inventory_node* inventory_get_node_at_index(inventory *inv, int index) {
    if (!inv || index < 0 || index > inv->count) {
        return NULL;
    }
    
    inventory_node *current = inv->head;
    for (int i = 1; (i < index) && current; i++) {
        current = current->next;
    }
    
    return current;
}

int inventory_equip_artifact(inventory *inv, inventory_node *node, equipment_slot slot) {
    if (!inv || !node || slot < 0 || slot >= MAX_EQUIPPED) {
        return 0;
    }
    
    // Проверяем, что это артефакт
    if (node->type != ITEM_ARTIFACT) {
        return 0;
    }
    
    // Снимаем предыдущий предмет из этого слота
    if (inv->equipped[slot]) {
        inv->equipped[slot]->state.artifact_state.is_equipped = 0;
    }
    
    // Экипируем новый
    inv->equipped[slot] = node;
    node->state.artifact_state.is_equipped = 1;
    
    return 1;
}

int inventory_unequip_artifact(inventory *inv, equipment_slot slot) {
    if (!inv || slot < 0 || slot >= MAX_EQUIPPED || !inv->equipped[slot]) {
        return 0;
    }
    
    inv->equipped[slot]->state.artifact_state.is_equipped = 0;
    inv->equipped[slot] = NULL;
    
    return 1;
}

void free_inventory(inventory *inv) {
    if (!inv) return;
    
    inventory_node *current = inv->head;
    while (current) {
        inventory_node *next = current->next;
        free(current);
        current = next;
    }
    free(inv);
}

void display_inventory(inventory *inv, item_database *db, int selected_index) {
    if (!inv || !inv->win || !db) return;
    
    werase(inv->win);
    box(inv->win, 0, 0);
    
    // Заголовок
    mvwprintw(inv->win, 0, 2, " ==Inventory== [%d/%d] ", inv->count, inv->max_slots);
    
    inventory_node *current = inv->head;
    int line = 1;
    int index = 0;
    
    while (current && line < getmaxy(inv->win) - 1) {
        // Находим шаблон предмета
        item_template* template = itemdb_find_by_id(db, current->item_id);
        if (!template) {
            // Пропускаем, если шаблон не найден
            current = current->next;
            index++;
            continue;
        }
        
        // Выделение выбранного элемента
        if (index == selected_index) {
            wattron(inv->win, A_REVERSE);
        }
        
        // Отображение предмета
        if (current->type == ITEM_ARTIFACT) {
            char equipped = current->state.artifact_state.is_equipped ? 'E' : ' ';
            mvwprintw(inv->win, line, 2, "%c %s", equipped, template->name);
        } else {
            mvwprintw(inv->win, line, 2, "  %s x%d", 
                     template->name, current->state.consumable_state.quantity);
        }
        
        if (index == selected_index) {
            wattroff(inv->win, A_REVERSE);
        }
        
        current = current->next;
        line++;
        index++;
    }
    
    // Отображаем свободные слоты
    for (int i = inv->count; i < inv->max_slots && line < getmaxy(inv->win) - 1; i++) {
        mvwprintw(inv->win, line, 2, "[Свободный слот]");
        line++;
    }
    
    wrefresh(inv->win);
}