#include <curses.h>
#include "inventory.h"
#include "hero.h"

int player_use_item(Hero *player, inventory_node *node) {
    if (!player || !node) return 0;//снова проверки на выделение памяти больше комментить наверн не буду
    
    if (node->type == ITEM_ARTIFACT) {
        return player_equip_artifact(player, node->item.Art);
    } else {
        return player_use_consumable(player, node->item.Cons);
    }
}

int player_equip_artifact(Hero *player, artifact *artifact) {
    if (!player || !artifact || !player->inventory) return 0;
    

    for (int i = 0; i < MAX_EQUIPPED; i++) {
        if (!player->inventory->equipped[i]) {
            inventory_equip_artifact(player->inventory, artifact, i);
            player_apply_artifact_bonuses(player);
            return 1;
        }
    }
    
    return 0;  // нет свободных слотов
}


int player_use_consumable(Hero *player, consumable *consumable) {
    if (!player || !consumable || consumable->quantity <= 0) {
        return 0;
    }
    
    int result = 0;
    
    switch (consumable->type) {
        case FOOD:
        player->hp += consumable->power;
            if (player->hp > player->max_hp) {
                player->hp = player->max_hp;
            }
            result = 1;
            break;
        case HELTH_POT:
            player->hp += consumable->power;
            if (player->hp > player->max_hp) {
                player->hp = player->max_hp;
            }
            result = 1;
            break;
            
        case MANA_POT:
            player->mp += consumable->power;
            if (player->mp > player->max_mp) {
                player->mp = player->max_mp;
            }
            result = 1;
            break;
            
         case STRENGHT_POT:
        case DEXTERITY_POT:
        case MAGIC_POT:
            // Добавляем эффект в список активных
            if (player->effect_count < MAX_CONSUMABLE_EFFECTS) {
                consumable_effect new_effect;
                new_effect.type = consumable->type;
                new_effect.power = consumable->power;
                new_effect.remaining_duration = consumable->duration;
                
                player->active_effects[player->effect_count] = new_effect;
                player->effect_count++;
                
                // Пересчитываем характеристики
                player_apply_artifact_bonuses(player);
                result = 1;
            }
            break;
    }
    
    if (result) {
        consumable->quantity--;
        
        // кдаляем потраченное
        if (consumable->quantity <= 0) {
            inventory_node *current = player->inventory->head;
            inventory_node *prev = NULL;
            
            while (current) {
                if (current->type == ITEM_CONSUMABLE && 
                    current->item.Cons == consumable) {
                    
                    if (prev) {
                        prev->next = current->next;
                    } else {
                        player->inventory->head = current->next;
                    }
                    
                    if (current == player->inventory->tail) {
                        player->inventory->tail = prev;
                    }
                    
                    free(consumable);
                    free(current);
                    player->inventory->count--;
                    break;
                }
                
                prev = current;
                current = current->next;
            }
        }
    }
    
    return result;
}

// 4. Обновление эффектов зелий (вызывать каждый ход)
void player_update_potion_effects(Hero *player) {
    if (!player) return;
    
    int i = 0;
    while (i < player->effect_count) {
        player->active_effects[i].remaining_duration--;
        
        if (player->active_effects[i].remaining_duration <= 0) {
            for (int j = i; j < player->effect_count - 1; j++) {
                player->active_effects[j] = player->active_effects[j + 1];
            }
            player->effect_count--;
        } else {
            i++;
        }
    }
    

    if (player->effect_count > 0) {
        player_apply_artifact_bonuses(player);
    }
}

// 5. Применение бонусов от артефактов
void player_apply_artifact_bonuses(Hero *player) {
    if (!player || !player->inventory) return;
    
    // Сбрасываем характеристики к базовым
    player->strength = player->base_strength;
    player->dexterity = player->base_dexterity;
    player->magic = player->base_magic;
    
    // Добавляем бонусы от экипированных артефактов
    for (int i = 0; i < MAX_EQUIPPED; i++) {
        if (player->inventory->equipped[i]) {
            artifact *art = player->inventory->equipped[i];
            player->strength += art->strenght_bonus;
            player->dexterity += art->dexterity_bonus;
            player->magic += art->magic_bonus;
        }
    }
    
    // Добавляем бонусы от зелий
    for (int i = 0; i < player->effect_count; i++) {
        consumable_effect effect = player->active_effects[i];
        
        switch (effect.type) {
            case STRENGHT_POT:
                player->strength += effect.power;
                break;
            case DEXTERITY_POT:
                player->dexterity += effect.power;
                break;
            case MAGIC_POT:
                player->magic += effect.power;
                break;
            default:
                break;  // FOOD, HELTH_POT, MANA_POT - мгновенные эффекты
        }
    }
}

void player_get_active_effects_info(Hero *player, char *buffer, size_t buffer_size) {
    if (!player || !buffer) return;
    
    buffer[0] = '\0';
    
    if (player->effect_count == 0) {
        strncpy(buffer, "Нет активных эффектов", buffer_size);
        return;
    }
    
    char temp[100];
    for (int i = 0; i < player->effect_count; i++) {
        consumable_effect effect = player->active_effects[i];
        
        const char *effect_name = "";
        switch (effect.type) {
            case STRENGHT_POT: effect_name = "Сила"; break;
            case DEXTERITY_POT: effect_name = "Ловкость"; break;
            case MAGIC_POT: effect_name = "Магия"; break;
            default: continue;
        }
        
        snprintf(temp, sizeof(temp), "%s +%d (%d хв) ", 
                effect_name, effect.power, effect.remaining_duration);
        
        // Проверяем, поместится ли еще один эффект в буфер
        if (strlen(buffer) + strlen(temp) < buffer_size - 10) {
            strcat(buffer, temp);
        } else {
            strcat(buffer, "...");
            break;
        }
    }
}