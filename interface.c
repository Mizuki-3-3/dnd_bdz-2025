#include "interface.h"
#include <string.h>
#include <locale.h>
#include "database.h"
#include "hero.h"
#include "inventory.h"

// Объявления внешних переменных (определены в game.c)
extern item_database global_db;
extern curw *narrative_win;

int start_to_work() {
    initscr(); // инициализируем библиотеку
    cbreak();  // Не использовать буфер для функции getch()
    raw();
    nonl();
    noecho(); // Не печатать на экране то, что набирает пользователь на клавиатуре
    curs_set(0); // Убрать курсор
    keypad(stdscr, TRUE); // Активировать специальные клавиши клавиатуры
    
    if (has_colors() == FALSE) {
        endwin();
        puts("\nYour terminal does not support color");
        return 1;
    }
    
    start_color(); // Активируем поддержку цвета
    init_color(COLOR_WHITE, 800, 800, 100);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_WHITE, COLOR_RED);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);

    int y, x;
    getmaxyx(stdscr, y, x);
    curw *win = make_new_win(0, 0, y, x, "==Dungeons==");
    
    return 0;
}

curw *make_new_win(int y, int x, int height, int width, char *label) {
    curw *new = malloc(sizeof(curw));
    if (!new) return NULL;
    
    // оформляем задник
    new->background = newwin(height, width, y, x);
    wbkgd(new->background, COLOR_PAIR(3));
    for (int i = width*0.1; i < width; i++)
        mvwaddch(new->background, height-1, i, '#');
    for (int i = height*0.2; i < height; i++)
        mvwaddch(new->background, i, width-1, '#');
    wattroff(new->background, COLOR_PAIR(3));
    
    // оформляем передник
    new->decoration = derwin(new->background, height-2, width-2, 1, 1);
    wbkgd(new->decoration, COLOR_PAIR(1));
    wborder(new->decoration, '|', '|', '-', '-', '+', '+', '+', '+');
    
    int xfd, yfd;
    getmaxyx(new->decoration, yfd, xfd);
    new->overlay = derwin(new->decoration, yfd-4, xfd-2, 3, 1); // рабочее дочернее окно
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
    int x = (maxx - len) / 2;
    if (x < 1) x = 1;
    
    mvwprintw(win, 1, x, "%s", label);
    for (int i = 1; i < maxx - 1; i++) {
        mvwaddch(win, 2, i, ACS_HLINE);
    }
    mvwaddch(win, 2, 0, ACS_LTEE);
    mvwaddch(win, 2, maxx - 1, ACS_RTEE);
}

void display_hero_stats(WINDOW *win, Hero *hero) {
    if (!win || !hero) return;
    
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    
    // Очищаем область
    for (int i = 0; i < 12; i++) {
        mvwhline(win, i, 0, ' ', max_x);
    }
    
    mvwprintw(win, 0, 1, "=== ХАРАКТЕРИСТИКИ ===");
    mvwprintw(win, 2, 1, "Уровень: %d", hero->level);
    mvwprintw(win, 3, 1, "Опыт: %d/%d", hero->exp, hero->exp_to_next);
    mvwprintw(win, 4, 1, "HP: %d/%d", hero->hp, hero->max_hp);
    mvwprintw(win, 5, 1, "MP: %d/%d", hero->mp, hero->max_mp);
    mvwprintw(win, 6, 1, "Сила: %d", hero->strength);
    mvwprintw(win, 7, 1, "Ловкость: %d", hero->dexterity);
    mvwprintw(win, 8, 1, "Магия: %d", hero->magic);
    mvwprintw(win, 9, 1, "Локация: %d", hero->current_location);
    
    // Показываем активные эффекты
    if (hero->effect_count > 0) {
        mvwprintw(win, 11, 1, "Эффекты:");
        int line = 12;
        for (int i = 0; i < hero->effect_count && line < max_y - 2; i++) {
            const char *effect_name = "";
            switch (hero->active_effects[i].type) {
                case STRENGTH_POT: effect_name = "Сила"; break;
                case DEXTERITY_POT: effect_name = "Ловкость"; break;
                case MAGIC_POT: effect_name = "Магия"; break;
                default: effect_name = "Другой";
            }
            mvwprintw(win, line++, 3, "%s +%d (%d хв.)", 
                     effect_name, hero->active_effects[i].power,
                     hero->active_effects[i].remaining_duration);
        }
    }
}

char show_item_menu(WINDOW *win, inventory_node *node, item_template *item) {
    if (!win || !node || !item) return 0;
    
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    
    WINDOW *menu_win = newwin(8, 40, max_y/2 - 4, max_x/2 - 20);
    box(menu_win, 0, 0);
    
    mvwprintw(menu_win, 1, 2, "%s", item->name);
    
    char desc[70];
    strncpy(desc, item->description, 59);
    desc[59] = '\0';
    mvwprintw(menu_win, 2, 2, "%s", desc);
    
    //доступные действия
    if (node->type == ITEM_ARTIFACT) {
        if (node->state.artifact_state.is_equipped) {
            mvwprintw(menu_win, 4, 2, "E - Снять");
        } else {
            mvwprintw(menu_win, 4, 2, "E - Надеть");
        }
    } else {
        mvwprintw(menu_win, 4, 2, "U - Использовать");
        mvwprintw(menu_win, 5, 2, "Количество: %d", node->state.consumable_state.quantity);
    }
    
    mvwprintw(menu_win, 6, 2, "D - Выбросить");
    mvwprintw(menu_win, 7, 2, "ESC - Назад");
    
    wrefresh(menu_win);
    
    int ch = getch();
    delwin(menu_win);
    
    return ch;
}

void show_location_description(int location_id) {
    item_template *loc = itemdb_find_by_id(&global_db, 400 + location_id);
    if (!loc) return;
    
    WINDOW *win = narrative_win->overlay;
    werase(win);
    
    mvwprintw(win, 1, 1, "=== %s ===", loc->name);
    
    // Создаем копию строки для strtok
    char desc_copy[MAX_DESC_LENGTH];
    strncpy(desc_copy, loc->template.location_template.description, MAX_DESC_LENGTH - 1);
    desc_copy[MAX_DESC_LENGTH - 1] = '\0';
    
    int line = 3;
    int col = 1;
    int max_x = getmaxx(win) - 2;
    
    char *word = strtok(desc_copy, " ");
    while (word && line < getmaxy(win) - 5) {
        int word_len = strlen(word);
        
        if (col + word_len + 1 > max_x) {
            line++;
            col = 1;
        }
        
        mvwprintw(win, line, col, "%s ", word);
        col += word_len + 1;
        
        word = strtok(NULL, " ");
    }
    
    wrefresh(win);
}