#ifndef INTERFACE_H
#define INTERFACE_H

#include <curses.h>
#include <panel.h>
#include "inventory.h"

struct curw
{
    WINDOW *background;
    WINDOW *decoration;
    WINDOW *overlay;
    PANEL *panel;
};

void display_inventory(inventory *inv, int selected_index);
void display_inventory_status(WINDOW *win, inventory *inv);

#endif