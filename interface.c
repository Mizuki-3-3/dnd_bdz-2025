#include "interface.h"
#include "inventory.h"
#include <locale.h>

int start_to_work(){
  initscr(); //инициализируем библиотеку
  cbreak();  //Не использовать буфер для функции getch()
  raw();
  nonl();
  noecho(); //Не печатать на экране то, что набирает пользователь на клавиатуре
  curs_set(0); //Убрать курсор
  keypad(stdscr, TRUE); //Активировать специальные клавиши клавиатуры (например, если хотим использовать горячие клавиши)
  if (has_colors() == FALSE){
      endwin();
      puts("\nYour terminal does not support color");
      return (1);
  }
  start_color(); //Активируем поддержку цвета
  init_color(COLOR_WHITE, 800, 800, 100);
  init_pair(1, COLOR_BLACK, COLOR_BLUE); //Все цветовые пары (background-foreground) должны быть заданы прежде, чем их используют
  init_pair(2, COLOR_WHITE, COLOR_RED);
  init_pair(3, COLOR_WHITE, COLOR_BLACK);
    
  return 0;
}

curw *make_new_win(int y, int x, int height, int width, char *label){
    curw *new = malloc(sizeof(curw));
    //оформили задник
    new->background = newwin(height, width, y, x);
    wbkgd(new->background, COLOR_PAIR(3));
    for (int i= width*0.1; i<width;i++)
		mvwaddch(new->background, height-1, i, ACS_BLOCK);
    for (int i= height*0.2; i<height;i++)
		mvwaddch(new->background, i, width-1, ACS_BLOCK);
    wattroff(new->background, COLOR_PAIR(3));
    //оформили передник
    new->decoration = derwin(new->background,  height-2, width-2, 1, 1);
    wbkgd(new->decoration, COLOR_PAIR(1));
    box(new->decoration, 0, 0);
    int xfd, yfd;
    getmaxyx(new->decoration, yfd, xfd);
    new->overlay = derwin(new->decoration,  yfd-4, xfd-2, 3, 1);//рабочее дочернее окно
    wbkgd(new->overlay, COLOR_PAIR(1));
    new->panel = new_panel(new->background);    
    tui_win_label(new->decoration, label, 0);
    update_panels();
    doupdate();   
    return new;
}

void tui_win_label(WINDOW *win, char *label, int pos) {
    if (!win || !label) return;
    int maxx = getmaxx(win);
    int len = strlen(label);
    int x;
    x = (maxx - len) / 2;
    if (x < 1) x = 1;
    mvwprintw(win, 1, x, "%s", label);
    for (int i = 1; i < maxx - 1; i++) {
        mvwaddch(win, 2, i, ACS_HLINE);
    }
    mvwaddch(win, 2, 0, ACS_LTEE);       
    mvwaddch(win, 2, maxx - 1, ACS_RTEE);
}



// Отрисовка постоянной панели экипировки
void draw_equipment_panel(curw *eq_win, inventory *inv, item_database *db) {
    if (!eq_win || !inv || !db) return;
    
    WINDOW *win = eq_win->overlay;
    werase(win);
    
    // Заголовок
    mvwprintw(win, 0, 1, "==ЭКИПИРОВКА==");
    
    // Отображаем каждый слот
    const char* slot_names[MAX_EQUIPPED] = {
        "Оружие", "Броня", "Шлем", "Сапоги", "Амулет"
    };
    
    for (int i = 0; i < MAX_EQUIPPED; i++) {
        mvwprintw(win, i*2 + 2, 1, "%s:", slot_names[i]);
        
        if (inv->equipped[i]) {
            item_template *item = itemdb_find_by_id(db, inv->equipped[i]->item_id);
            if (item) {
                // Выделяем цветом по редкости
                wattron(win, COLOR_PAIR(2)); // Например, красный для артефактов
                mvwprintw(win, i*2 + 2, 12, "%-20s", item->name);
                wattroff(win, COLOR_PAIR(2));
            }
        } else {
            mvwprintw(win, i*2 + 2, 12, "[пусто]");
        }
    }
    
    // Статус инвентаря внизу
    mvwprintw(win, 12, 1, "Инвентарь: %d/%d", inv->count, inv->max_slots);
    
    wrefresh(win);
}

// Всплывающее окно инвентаря
curw* create_inventory_popup(int height, int width) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Центрируем окно
    int start_y = (max_y - height) / 2;
    int start_x = (max_x - width) / 2;
    
    curw *inv_popup = make_new_win(start_y, start_x, height, width, "ИНВЕНТАРЬ");
    
    // Добавляем подсказки
    WINDOW *win = inv_popup->overlay;
    mvwprintw(win, height-4, 2, "↑↓: Выбор  E: Использовать/Надеть");
    mvwprintw(win, height-3, 2, "D: Выбросить  TAB: Вернуться");
    
    return inv_popup;
}


void draw_inventory_popup(curw *inv_popup, inventory *inv, item_database *db, int selected_index, game_state *state) {
    if (!inv_popup || !inv || !db) return;
    
    WINDOW *win = inv_popup->overlay;
    werase(win);
    
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    
    // Заголовок с информацией
    mvwprintw(win, 0, 1, "Предметы [%d/%d]:", inv->count, inv->max_slots);
    
    inventory_node *current = inv->head;
    int line = 2;
    int index = 0;
    
    // Отображаем предметы
    while (current && line < max_y - 5) {
        item_template *template = itemdb_find_by_id(db, current->item_id);
        if (!template) {
            current = current->next;
            index++;
            continue;
        }
        
        // Выделение выбранного
        if (index == selected_index && state->current_mode == MODE_INVENTORY) {
            wattron(win, A_REVERSE);
        }
        
        // Иконка типа предмета
        char type_char = (current->type == ITEM_ARTIFACT) ? '⚔' : '⚗';
        
        // Для артефактов - статус экипировки
        if (current->type == ITEM_ARTIFACT) {
            char equip_char = current->state.artifact_state.is_equipped ? '✓' : ' ';
            mvwprintw(win, line, 2, "%c [%c] %-20s", type_char, equip_char, template->name);
        } else {
            mvwprintw(win, line, 2, "%c     %-20s x%d", 
                     type_char, template->name, current->state.consumable_state.quantity);
        }
        
        if (index == selected_index && state->current_mode == MODE_INVENTORY) {
            wattroff(win, A_REVERSE);
        }
        
        // Отображение описания для выбранного предмета
        if (index == selected_index && state->current_mode == MODE_INVENTORY) {
            mvwprintw(win, max_y - 2, 2, "%-60s", template->description);
        }
        
        current = current->next;
        line++;
        index++;
    }
    
    // Свободные слоты
    for (int i = inv->count; i < inv->max_slots && line < max_y - 5; i++) {
        mvwprintw(win, line, 2, "[  Свободный слот  ]");
        line++;
    }
    
    wrefresh(win);
}

// void display_inventory(inventory *inv, int selected_index) {
//     if (!inv || !inv->win) return;
    
//     werase(inv->win);
//     box(inv->win, 0, 0);
    
//     mvwprintw(inv->win, 0, 2, " ИНВЕНТАРЬ [%d/%d] ", inv->count, inv->max_slots);
    
//     inventory_node *current = inv->head;
//     int line = 1;
//     int index = 0;
    
//     while (current && line < getmaxy(inv->win) - 1) {
//         if (index == selected_index) {
//             wattron(inv->win, A_REVERSE);
//         }

//         if (current->type == ITEM_ARTIFACT) {
//             artifact *art = current->item.Art;
//             char equipped = art->is_equipped ? 'E' : ' ';
//             mvwprintw(inv->win, line, 2, "%c %s", equipped, art->name);
//         } else {
//             consumable *cons = current->item.Cons;
//             const char *type_name = "";
//             switch (cons->type) {
//                 case FOOD: type_name = "Еда"; break;
//                 case HELTH_POT: type_name = "Зелье здоровья"; break;
//                 case MANA_POT: type_name = "Зелье маны"; break;
//                 case STRENGHT_POT: type_name = "Зелье силы"; break;
//                 case DEXTERITY_POT: type_name = "Зелье ловкости"; break;
//                 case MAGIC_POT: type_name = "Зелье магии"; break;
//             }
//             mvwprintw(inv->win, line, 2, "  %s x%d", cons->name, cons->quantity);
//             mvwprintw(inv->win, line, getmaxx(inv->win) - 15, "%s", type_name);
//         }
        
//         if (index == selected_index) {
//             wattroff(inv->win, A_REVERSE);
//         }
        
//         current = current->next;
//         line++;
//         index++;
//     }
    
//     for (int i = inv->count; i < inv->max_slots && line < getmaxy(inv->win) - 1; i++) {
//         mvwprintw(inv->win, line, 2, "[Свободный слот]");
//         line++;
//     }
    
//     wrefresh(inv->win);
// }

// // Отображение статуса инвентаря
// void display_inventory_status(WINDOW *win, inventory *inv) {
//     if (!win || !inv) return;
    
//     int free_slots = inv->max_slots - inv->count;
    
//     mvwprintw(win, 0, 0, "Свободно слотов: %d", free_slots);
    
//     if (free_slots <= 2) {
//         wattron(win, COLOR_PAIR(1));  // Красный цвет
//         mvwprintw(win, 1, 0, "Мало места!");
//         wattroff(win, COLOR_PAIR(1));
//     }
// }