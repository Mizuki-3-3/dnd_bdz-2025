#include "interface.h"
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