#ifndef GAME_H
#define GAME_H

#include "hero.h"
#include "monster.h"
#include "location.h"
#include "inventory.h"
#include "database.h"
#include "interface.h"

#define MAX_LOCATIONS 7
#define MAX_EXITS 8

// Game structure
typedef struct {
    Hero *hero;
    location **locations;
    int current_location;
    int location_count;
    item_database global_itemdb;
    inventory *player_inventory;
} Game;

// Function declarations
Game* init_game();
void setup_locations(Game *game);
int move_to_location(Game *game, int location_id);
int handle_empty_location(Game *game);
int handle_monster_location(Game *game);
int handle_treasure_location(Game *game);
int roll_dice();
#endif