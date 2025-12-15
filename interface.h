#ifndef INTERFACE_H
#define INTERFACE_H

#include <curses.h>
#include <panel.h>
#include <stdlib.h>
#include <string.h>
#include "inventory.h"
#include "equipment.h"
#include "hero.h"
#include "database.h"

typedef enum {
    MODE_NARRATIVE,
    MODE_MENU,
    MODE_COMBAT
} game_mode;

typedef struct {
    game_mode current_mode;
    game_mode previous_mode;
    int inventory_selected_index;
    int quit_flag;
} game_state;

typedef struct cursed_window {
    WINDOW *background;
    WINDOW *decoration;
    WINDOW *overlay;
    PANEL *panel;
} curw;

int start_to_work(void);
curw *make_new_win(int y, int x, int height, int width, char *label);
void tui_win_label(WINDOW *win, char *label, int pos);

// Функции для игры
void create_game_windows(void);
void display_hero_stats(WINDOW *win, Hero *hero);
char show_item_menu(WINDOW *win, inventory_node *node, item_template *item);
void print_location_description(int location_id);
void update_window_titles(item_template *loc);  // Добавлена новая функция
void print_wrapped_text(WINDOW *win, const char *text);
#endif