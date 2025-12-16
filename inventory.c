#include "inventory.h"
#include"database.h"
#include "hero.h"
#include <stdlib.h>
#include <string.h>

inventory* create_inventory(void) {
    inventory *inv = malloc(sizeof(inventory));
    if (!inv) return NULL;
    
    inv->head = NULL;
    inv->tail = NULL;
    inv->count = 0;
    inv->max_slots = BASE_CAPACITY;
    inv->win = NULL;
    
    for (int i = 0; i < MAX_EQUIPPED; i++) {
        inv->equipped[i] = NULL;     // сначала инвентарь пустой
    }
    
    return inv;
}

void inventory_update_capacity(inventory *inv, int player_level) {
    if (!inv) return;
    inv->max_slots = BASE_CAPACITY + ((player_level-1) * CAPACITY_PER_LEVEL);
}

int inventory_add_item_by_id(inventory *inv, item_database *db, int item_id, int quantity) {
    if (!inv || !db) {
        return 0; 
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
        return 0;  // память не выделилась под элемент в инвентаре
    }
    
    new_node->item_id = item_id;
    new_node->next = NULL;

    // Определяем тип предмета по ID
    if (item_id >= 200 && item_id < 300) {
        // Артефакты: 200-299
        new_node->type = ITEM_ARTIFACT;
        new_node->state.artifact_state.is_equipped = 0;  // Не надет
    } else if (item_id >= 100 && item_id < 200) {
        // Расходники: 100-199
        new_node->type = ITEM_CONSUMABLE;
        new_node->state.consumable_state.quantity = quantity;
    } else {
        // Монстры (300-399) и локации (400-499) не должны быть в инвентаре
        free(new_node);
        return 0;
    }
    
    // Добавляем в список
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
    if (!inv || index < 0 || index >= inv->count) {
        return NULL;
    }
    
    inventory_node *current = inv->head;
    for (int i = 0; i < index && current; i++) {
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
void display_inventory(inventory *inv, item_database *db, Hero *hero, int selected_index) {
    if (!inv || !inv->win || !db) return;
    
    werase(inv->win);
    int line = 1;
    int max_y = getmaxy(inv->win);
    int max_x = getmaxx(inv->win);
    
    if (hero) {
        mvwprintw(inv->win, line++, 2, "=== %s ===", hero->name);
        mvwprintw(inv->win, line++, 2, "Уровень: %d", hero->level);
        mvwprintw(inv->win, line++, 2, "HP: %d/%d", hero->hp, hero->max_hp);
        mvwprintw(inv->win, line++, 2, "MP: %d/%d", hero->mp, hero->max_mp);
        mvwprintw(inv->win, line++, 2, "Сила: %d Ловк: %d", hero->strength, hero->dexterity);
        mvwprintw(inv->win, line++, 2, "Магия: %d", hero->magic);
        line++; // Пустая строка
    }
    
    mvwhline(inv->win, line++, 1, ACS_HLINE, max_x - 2);
    
    inventory_node *current = inv->head;
    int index = 0;
    
    while (current && line < max_y - 1) {
        item_template* template = itemdb_find_by_id(db, current->item_id);
        if (!template) {
            current = current->next;
            index++;
            continue;
        }
        
        if (index == selected_index) {
            wattron(inv->win, A_REVERSE);
        }
        
        char display[80];
        if (current->type == ITEM_ARTIFACT) {
            char equipped = current->state.artifact_state.is_equipped ? 'E' : ' ';
            snprintf(display, sizeof(display), "%c %s", equipped, template->name);
        } else {
            snprintf(display, sizeof(display), "  %s x%d", 
                    template->name, current->state.consumable_state.quantity);
        }
        display[sizeof(display) - 1] = '\0';
        
        mvwprintw(inv->win, line, 2, "%s", display);
        
        if (index == selected_index) {
            wattroff(inv->win, A_REVERSE);
        }
        
        current = current->next;
        line++;
        index++;
    }
    
    
    // подсказки
    if (line < max_y - 2) {
        line = max_y - 3;
        mvwprintw(inv->win, line++, 2, "E/U - Использовать  D - Выбросить");
        mvwprintw(inv->win, line++, 2, "ESC/I - Закрыть");
    }
    
    wrefresh(inv->win);
}

int inventory_use_consumable(Hero *hero, inventory *inv, inventory_node *node, item_database *db) {
    if (!hero || !inv || !node || !db || node->type != ITEM_CONSUMABLE) return 0;
    
    item_template *item = itemdb_find_by_id(db, node->item_id);
    if (!item) return 0;
    
    consumable *cons = &item->template.consumable_template;
    
    switch (cons->type) {
        case HEALTH_POT:
            hero->hp += cons->power;
            if (hero->hp > hero->max_hp) hero->hp = hero->max_hp;
            break;
            
        case MANA_POT:
            hero->mp += cons->power;
            if (hero->mp > hero->max_mp) hero->mp = hero->max_mp;
            break;
            
        case STRENGTH_POT:
        case DEXTERITY_POT:
        case MAGIC_POT:
            // Добавляем временный эффект
            if (hero->effect_count < MAX_CONSUMABLE_EFFECTS) {
                hero->active_effects[hero->effect_count].type = cons->type;
                hero->active_effects[hero->effect_count].power = cons->power;
                hero->active_effects[hero->effect_count].remaining_duration = cons->duration;
                hero->effect_count++;
                
                switch (cons->type) {
                    case STRENGTH_POT: hero->strength += cons->power; break;
                    case DEXTERITY_POT: hero->dexterity += cons->power; break;
                    case MAGIC_POT: hero->magic += cons->power; break;
                    default: break;
                }
            }
            break;
            
        default:
            return 0;
    }
    node->state.consumable_state.quantity--;
    if (node->state.consumable_state.quantity <= 0) {
        inventory_remove_item(inv, node);
    }
    
    return 1;
}


void calculate_stats_from_equipment(Hero *hero, inventory *inv, item_database *db) {
    if (!hero || !inv || !db) return;
    
    // Сбрасываем статы к базовым
    hero->strength = hero->base_strength;
    hero->dexterity = hero->base_dexterity;
    hero->magic = hero->base_magic;
    
    // Добавляем бонусы от экипированных предметов
    for (int i = 0; i < MAX_EQUIPPED; i++) {
        if (inv->equipped[i]) {
            item_template *item = itemdb_find_by_id(db, inv->equipped[i]->item_id);
            if (item && item->id >= 200 && item->id < 300) { // Это артефакт
                artifact *art = &item->template.artifact_template;
                hero->strength += art->strength_bonus;
                hero->dexterity += art->dexterity_bonus;
                hero->magic += art->magic_bonus;
            }
        }
    }
    
    // Добавляем бонусы от активных эффектов
    for (int i = 0; i < hero->effect_count; i++) {
        switch (hero->active_effects[i].type) {
            case STRENGTH_POT:
                hero->strength += hero->active_effects[i].power;
                break;
            case DEXTERITY_POT:
                hero->dexterity += hero->active_effects[i].power;
                break;
            case MAGIC_POT:
                hero->magic += hero->active_effects[i].power;
                break;
            default:
                break;
        }
    }
}

