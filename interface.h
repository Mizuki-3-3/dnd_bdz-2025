#ifndef INTERFACE_H
#define INTERFACE_H

#include <curses.h>
#include <panel.h>
#include <stdlib.h>
#include <string.h>
// #include "inventory.h"
// #include "equipment.h"

typedef enum {
    MODE_NARRATIVE,  
    MODE_INVENTORY,
    MODE_MENU    
} game_mode;

typedef struct {
    game_mode current_mode;
    game_mode previous_mode;
    int inventory_selected_index;
    int quit_flag;
} game_state;


typedef struct cursed_window{
    WINDOW *background;
    WINDOW *decoration;
    WINDOW *overlay;
    PANEL *panel;
}curw;

int start_to_work(void);
curw *make_new_win(int y, int x, int height, int width, char *label);
void tui_win_label(WINDOW *win, char *label, int pos);
void about_location(WINDOW *win, char *description);
// void display_inventory(inventory *inv, item_database *db, int selected_index);
// curw *create_inventory_popup(int height, int width);
// void draw_equipment_panel(curw *eq_win, inventory *inv, item_database *db);
// void draw_inventory_popup(curw *inv_popup, inventory *inv, item_database *db, int selected_index, game_state *state);


#endif