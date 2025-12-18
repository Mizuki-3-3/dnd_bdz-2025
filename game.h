#ifndef GAME_H
#define GAME_H

#include "interface.h"
#include "hero.h"
#include "inventory.h"
#include "database.h"
#include <time.h> 
#include <ctype.h>

// Глобальные переменные игры
extern curw *narrative_win;
extern curw *inventory_win;
extern item_database global_db;
extern Hero *player;
extern inventory *player_inv;
extern game_state state;
extern int current_location;
extern int prev_location;
extern char player_name[MAX_NAME_LENGTH];
extern int chosen_class;

// Основные игровые функции
void game_loop(void);
void init_new_game(const char *name, int clas);
int load_saved_game(const char *save_name);  
int save_current_game(void);                
int choose_class(WINDOW *win);

// Функции навигации
void move_to_location(int new_location);
void show_current_location(item_template *loc);

// Функции боя
void start_combat(int monster_id);
void player_attack(Monster* monster, int use_magic);
void monster_attack_player(Monster* monster);
void end_combat(int victory);
void take_treasure(int location_id);

// Функции инвентаря
void open_inventory(void);
void use_item_from_inventory(int index);
void drop_item_from_inventory(int index);

int roll_dice();
int calculate_damage(int base, int dice_roll);
void check_level_up(void);
void game_over(void);
int get_location_connection(int from, int to);
void print_hint(WINDOW *win, int line, int max_y, item_template *loc);

#endif