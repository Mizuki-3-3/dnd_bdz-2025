#include "interface.h"

void display_inventory(inventory *inv, int selected_index) {
    if (!inv || !inv->win) return;
    
    werase(inv->win);
    box(inv->win, 0, 0);
    
    mvwprintw(inv->win, 0, 2, " ИНВЕНТАРЬ [%d/%d] ", inv->count, inv->max_slots);
    
    inventory_node *current = inv->head;
    int line = 1;
    int index = 0;
    
    while (current && line < getmaxy(inv->win) - 1) {
        if (index == selected_index) {
            wattron(inv->win, A_REVERSE);
        }

        if (current->type == ITEM_ARTIFACT) {
            artifact *art = current->item.Art;
            char equipped = art->is_equipped ? 'E' : ' ';
            mvwprintw(inv->win, line, 2, "%c %s", equipped, art->name);
        } else {
            consumable *cons = current->item.Cons;
            const char *type_name = "";
            switch (cons->type) {
                case FOOD: type_name = "Еда"; break;
                case HELTH_POT: type_name = "Зелье здоровья"; break;
                case MANA_POT: type_name = "Зелье маны"; break;
                case STRENGHT_POT: type_name = "Зелье силы"; break;
                case DEXTERITY_POT: type_name = "Зелье ловкости"; break;
                case MAGIC_POT: type_name = "Зелье магии"; break;
            }
            mvwprintw(inv->win, line, 2, "  %s x%d", cons->name, cons->quantity);
            mvwprintw(inv->win, line, getmaxx(inv->win) - 15, "%s", type_name);
        }
        
        if (index == selected_index) {
            wattroff(inv->win, A_REVERSE);
        }
        
        current = current->next;
        line++;
        index++;
    }
    
    for (int i = inv->count; i < inv->max_slots && line < getmaxy(inv->win) - 1; i++) {
        mvwprintw(inv->win, line, 2, "[Свободный слот]");
        line++;
    }
    
    wrefresh(inv->win);
}

// Отображение статуса инвентаря
void display_inventory_status(WINDOW *win, inventory *inv) {
    if (!win || !inv) return;
    
    int free_slots = inv->max_slots - inv->count;
    
    mvwprintw(win, 0, 0, "Свободно слотов: %d", free_slots);
    
    if (free_slots <= 2) {
        wattron(win, COLOR_PAIR(1));  // Красный цвет
        mvwprintw(win, 1, 0, "Мало места!");
        wattroff(win, COLOR_PAIR(1));
    }
}