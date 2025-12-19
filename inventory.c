#include "inventory.h"
#include"database.h"
#include "hero.h"
#include <stdlib.h>
#include <string.h>
#define MAX_ITEM_DESC 70
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
    inv->max_slots = BASE_CAPACITY + ((player_level-1) * CAPACITY_PER_LEVEL);
}

int inventory_add_item_by_id(inventory *inv, item_database *db, int item_id, int quantity) {
    
    if (inv->count >= inv->max_slots) {
        return -1;  // Инвентарь переполнен
    }

    item_template* template = itemdb_find_by_id(db, item_id);
    if (!template) {
        return 0; 
    }

    inventory_node *new_node = malloc(sizeof(inventory_node));
    if (!new_node) {
        return 0;  
    }
    
    new_node->item_id = item_id;
    new_node->next = NULL;

    if (item_id >= 200 && item_id < 300) {
        // артефакты: 200-299
        new_node->type = ITEM_ARTIFACT;
        new_node->state.artifact_state.is_equipped = 0;  // Не надет
    } else if (item_id >= 100 && item_id < 200) {
        // расходники: 100-199
        new_node->type = ITEM_CONSUMABLE;
        new_node->state.consumable_state.quantity = quantity;
    } else {
        free(new_node);
        return 0;
    }
    //добавим в список
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
    if (!inv->head) return 0;
    
    // если удаляем первый элемент
    if (inv->head == node_to_remove) {
        inventory_node *temp = inv->head;
        
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
        
        // если список стал пустым
        if (!inv->head) {
            inv->tail = NULL;
        }
        
        return 1;
    }
    
    inventory_node *current = inv->head;
    while (current->next && current->next != node_to_remove) {
        current = current->next;
    }
    
    if (!current->next) return 0;  
    
    inventory_node *temp = current->next;
    current->next = temp->next;
    
    if (temp == inv->tail) {
        inv->tail = current;
    }
    
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
    if (index < 0 || index >= inv->count) {
        return NULL;
    }
    
    inventory_node *current = inv->head;
    for (int i = 0; i < index && current; i++) {
        current = current->next;
    }
    
    return current;
}

equipment_slot get_slot_for_artifact_type(artifact_type type) {
    switch(type) {
        case ART_WEAPON:    return SLOT_WEAPON;
        case ART_ARMOR:     return SLOT_ARMOR;
        case ART_PANTS:      return SLOT_PANTS;
        case ART_BOOTS:     return SLOT_BOOTS;
        default:            return -1;
    }
}

int inventory_equip_artifact(inventory *inv, inventory_node *node, item_database *db) {
    if (node->type != ITEM_ARTIFACT) {
        return 0;
    }
    
    item_template *item = itemdb_find_by_id(db, node->item_id);
    if (!item || item->id < 200 || item->id >= 300) {
        return 0;
    }
    artifact *art = &item->template.artifact_template;
    equipment_slot slot = get_slot_for_artifact_type(art->type);
    
    if (slot == -1) {
        return 0;
    }
    
    if (inv->equipped[slot]) {
        inventory_unequip_artifact(inv, slot);
    }
    
    inv->equipped[slot] = node;
    node->state.artifact_state.is_equipped = 1;
    node->state.artifact_state.slot = slot;  // Сохраняем слот
    
    return 1;
}

const char* get_slot_name(equipment_slot slot) {
    static const char* slot_names[] = {
        "Оружие", "Броня", "Штаны", "Обувь",
    };
    
    if (slot >= 0 && slot < MAX_EQUIPPED) {
        return slot_names[slot];
    }
    return "Неизвестно";
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
    if (!inv->win) return;
    
    werase(inv->win);
    int line = 1,max_y = getmaxy(inv->win), max_x = getmaxx(inv->win);
    
    if (max_x < 80 || max_y < 20) {
        mvwprintw(inv->win, 1, 1, "Окно слишком мало!");
        wrefresh(inv->win);
        return;
    }
    
    int left_width = max_x / 2 - 4, right_column = max_x / 2 + 2;
    
    // ЛЕВАЯ ЧАСТЬ
    if (hero) {
        mvwprintw(inv->win, line++, 2, "=== %s ===", hero->name);
        mvwprintw(inv->win, line++, 2, "Уровень: %d  Опыт: %d/%d", hero->level, hero->exp, hero->exp_to_next);
        mvwprintw(inv->win, line++, 2, "HP: %d/%d", hero->hp, hero->max_hp);
        mvwprintw(inv->win, line++, 2, "MP: %d/%d", hero->mp, hero->max_mp);
        mvwprintw(inv->win, line++, 2, "Сила: %d ", hero->strength);
        mvwprintw(inv->win, line++, 2, "Ловкость: %d", hero->dexterity);
        mvwprintw(inv->win, line++, 2, "Магия: %d", hero->magic);
        mvwhline(inv->win, line++, 0, ACS_HLINE, max_x/2);
        
        int total_str_bonus = 0, total_dex_bonus = 0, total_mag_bonus = 0;
        mvwprintw(inv->win, line++, 2, "=== ЭКИПИРОВКА ===");
        
        for (int i = 0; i < MAX_EQUIPPED; i++) {
            if (inv->equipped[i]) {
                item_template* item = itemdb_find_by_id(db, inv->equipped[i]->item_id);
                if (item){
                    artifact *art = &item->template.artifact_template;
                    total_str_bonus += art->strength_bonus;
                    total_dex_bonus += art->dexterity_bonus;
                    total_mag_bonus += art->magic_bonus;
                    mvwprintw(inv->win, line, 2, "%s:", get_slot_name(i));
                    mvwprintw(inv->win, line, 12, "%s", item->name);
                    line++;
                }
            } else {
                mvwprintw(inv->win, line, 2, "%s:", get_slot_name(i));
                mvwprintw(inv->win, line, 12, "[пусто]");
                line++;
            }
        }
        mvwhline(inv->win,line++,0,ACS_HLINE,max_x/2);
        mvwprintw(inv->win, line++, 2, "=== БОНУСЫ ЭКИПИРОВКИ ===");
        mvwprintw(inv->win, line++, 2, "Сила: +%d", total_str_bonus);
        mvwprintw(inv->win, line++, 2, "Ловкость: +%d", total_dex_bonus);
        mvwprintw(inv->win, line++, 2, "Магия: +%d", total_mag_bonus);
        
        line++; // пустая строка
        
    }
    if (max_y > 2) {
        mvwvline(inv->win, 1, max_x/2, ACS_VLINE, max_y - 2);
    }
    
    // ПРАВАЯ ЧАСТЬ
    int right_line = 1;
    mvwprintw(inv->win, right_line++, right_column, "=== ИНВЕНТАРЬ ===");
    mvwprintw(inv->win, right_line++, right_column, "Слоты: [%d/%d]", inv->count, inv->max_slots);
    inventory_node *current = inv->head;
    int index = 0;
    
    while (current && right_line < max_y - 5) { // oставляем место для описания и подсказок
        item_template* template = itemdb_find_by_id(db, current->item_id);
        if (!template) {
            current = current->next;
            index++;
            continue;
        }
        if (index == selected_index) {
            wattron(inv->win, A_REVERSE);
        }
        
        char display_name[MAX_NAME_LENGTH];
        if (current->type == ITEM_ARTIFACT) {
            char equipped = current->state.artifact_state.is_equipped ? 'E' : ' ';
            snprintf(display_name, sizeof(display_name), "%c %s", equipped, template->name);
        } else {
            snprintf(display_name, sizeof(display_name), "  %s x%d", 
                    template->name, 
                    current->state.consumable_state.quantity);
        }
        display_name[sizeof(display_name) - 1] = '\0';
        mvwprintw(inv->win, right_line, right_column, "%s", display_name);
        
        if (index == selected_index) {
            wattroff(inv->win, A_REVERSE);
        }
        
        current = current->next;
        right_line++;
        index++;
    }
    
    mvwhline(inv->win, max_y - 6, 2, ACS_HLINE, max_x - 4);
    //описание выбранного предмета
    inventory_node *selected_node = inventory_get_node_at_index(inv, selected_index);
    if (selected_node) {
        item_template* template = itemdb_find_by_id(db, selected_node->item_id);
        if (template) {
            int desc_line = max_y - 5;
            
            if (selected_node->type == ITEM_ARTIFACT) {
                artifact *art = &template->template.artifact_template;
                mvwprintw(inv->win, desc_line++, 2, "%s", template->description);
                char bonuses[MAX_ITEM_DESC];
                snprintf(bonuses, sizeof(bonuses), 
                        "Бонусы: Сила +%d, Ловкость +%d, Магия +%d",
                        art->strength_bonus, art->dexterity_bonus, art->magic_bonus);
                bonuses[sizeof(bonuses) - 1] = '\0';
                
                if (strlen(bonuses) > max_x - 4) {
                    bonuses[max_x - 7] = '\0';
                    strcat(bonuses, "...");
                }
                
                mvwprintw(inv->win, desc_line, 2, "%s", bonuses);
                
            } else {
                consumable *cons = &template->template.consumable_template;
                
                const char* cons_type = "";
                switch (cons->type) {
                    case HEALTH_POT: cons_type = "Здоровье"; break;
                    case MANA_POT: cons_type = "Мана"; break;
                    case STRENGTH_POT: cons_type = "Сила"; break;
                    case DEXTERITY_POT: cons_type = "Ловкость"; break;
                    case MAGIC_POT: cons_type = "Магия"; break;
                    default: cons_type = "Неизвестно";
                }
                mvwprintw(inv->win, desc_line++, 2, "%s", template->description);
                
                char effect[MAX_ITEM_DESC];
                if (cons->type == HEALTH_POT || cons->type == MANA_POT) {
                    snprintf(effect, sizeof(effect), 
                            "Восстанавливает %d единиц %s мгновенно",
                            cons->power, cons_type);
                } else {
                    snprintf(effect, sizeof(effect), 
                            "Увеличивает %s на %d на %d ходов",
                            cons_type, cons->power, cons->duration);
                }
                effect[sizeof(effect) - 1] = '\0';
                //слишком длинное
                if (strlen(effect) > max_x - 4) {
                    effect[max_x - 7] = '\0';
                    strcat(effect, "...");
                }
                
                mvwprintw(inv->win, desc_line, 2, "%s", effect);
            }
        }
    }
    // подсказки
    mvwhline(inv->win, max_y - 3, 2, ACS_HLINE, max_x - 4);
    mvwprintw(inv->win, max_y - 2, 2, "E - Использовать/Экипировать  D - Выбросить  I - Выход");
    wrefresh(inv->win);
}

int inventory_use_consumable(Hero *hero, inventory *inv, inventory_node *node, item_database *db) {
    if (node->type != ITEM_CONSUMABLE) return 0;
    
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
            // добавляем временный эффект
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
    // сбрасываем статы к базовым
    hero->strength = hero->base_strength;
    hero->dexterity = hero->base_dexterity;
    hero->magic = hero->base_magic;
    
    // добавляем бонусы от экипированных предметов
    for (int i = 0; i < MAX_EQUIPPED; i++) {
        if (inv->equipped[i]) {
            item_template *item = itemdb_find_by_id(db, inv->equipped[i]->item_id);
            if (item && item->id >= 200 && item->id < 300) {
                artifact *art = &item->template.artifact_template;
                hero->strength += art->strength_bonus;
                hero->dexterity += art->dexterity_bonus;
                hero->magic += art->magic_bonus;
            }
        }
    }
    
    // добавляем бонусы от активных эффектов
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

