#ifndef GAME_H
#define GAME_H

#include "interface.h"
#include "hero.h"
#include "inventory.h"
#include "database.h"
#include <time.h> 
#include <ctype.h>

// Глобальные переменные игры (extern для доступа из других файлов)
extern curw *narrative_win;
extern curw *inventory_win;
extern curw *equipment_win;
extern item_database global_db;
extern Hero *player;
extern inventory *player_inv;
extern game_state state;
extern int current_location;
extern int prev_location;

// Основные игровые функции
void game_loop(void);
void init_new_game(const char *name, int clas);
int load_game(const char *name);
void save_game(void);
int choose_class(WINDOW *win);

// Функции навигации
void move_to_location(int new_location);
void handle_location_action(int location_id);
void show_location_description(int location_id);
void show_current_location(item_template *loc);

// Функции боя
void start_combat(int monster_id);
void player_attack(Monster* monster);
void monster_attack_player(Monster* monster);
void end_combat(int victory);
void combat_with_monster(int monster_id);
void take_treasure(int location_id);

// Функции инвентаря
void open_inventory(void);
void use_item_from_inventory(int index);
void drop_item_from_inventory(int index);

int roll_dice(int sides);
int calculate_damage(int base, int dice_roll);
void check_level_up(void);
void game_over(void);
int get_location_connection(int from, int to);
const char* get_location_name_by_id(int id);

#endif