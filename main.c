// main.c
#include <stdio.h>
#include "hero.h"

int main() {
    Hero player;
    player.effect_count = 0;
    
    // Игрок использует зелье силы на 5 ходов
    consumable strength_pot = {
        .name = "Зелье силы",
        .type = STRENGHT_POT,
        .quantity = 1,
        .power = 5,
        .duration = 5
    };
    
    player_use_consumable(&player, &strength_pot);
    printf("Сила после зелья: %d\n", player.strength);  // Например: 15
    
    // Через 2 хода использует зелье ловкости на 3 хода
    consumable dexterity_pot = {
        .name = "Зелье ловкости",
        .type = DEXTERITY_POT,
        .quantity = 1,
        .power = 3,
        .duration = 3
    };
    
    player_use_consumable(&player, &dexterity_pot);
    printf("Ловкость после зелья: %d\n", player.dexterity);  // Например: 13
    
    // Симуляция ходов
    for (int turn = 1; turn <= 6; turn++) {
        printf("\n--- Ход %d ---\n", turn);
        
        player_update_potion_effects(&player);
        player_apply_artifact_bonuses(&player);
        
        char effects_info[200];
        player_get_active_effects_info(&player, effects_info, sizeof(effects_info));
        
        printf("Активные эффекты: %s\n", effects_info);
        printf("Сила: %d, Ловкость: %d\n", player.strength, player.dexterity);
    }
    
    return 0;
}