#ifndef INTERFACE_H
#define INTERFACE_H

#include <curses.h>
#include <panel.h>
#include <stdlib.h>
#include <string.h>
// #include "inventory.h"

typedef struct cursed_window{
    WINDOW *background;
    WINDOW *decoration;
    WINDOW *overlay;
    PANEL *panel;
}curw;

int start_to_work(void);
curw *make_new_win(int y, int x, int height, int width, char *label);
void tui_win_label(WINDOW *win, char *label, int pos);


// void display_inventory(inventory *inv, int selected_index);
// void display_inventory_status(WINDOW *win, inventory *inv);

#endif