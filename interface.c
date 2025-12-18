#include "interface.h"
#include <string.h>
#include <locale.h>
#include "database.h"
#include "hero.h"
#include "inventory.h"

extern item_database global_db;
extern curw *narrative_win;
extern curw *inventory_win;

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
    init_color(COLOR_WHITE, 100, 100, 100);
    init_pair(1, COLOR_WHITE, COLOR_GREEN);
    init_pair(2, COLOR_WHITE, COLOR_RED);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);

    int y, x;
    getmaxyx(stdscr, y, x);
    return 0;
}

curw *make_new_win(int y, int x, int height, int width, char *label) {
    curw *new = malloc(sizeof(curw));
    if (!new) return NULL;
    
    // оформляем задник
    new->background = newwin(height, width, y, x);
    wbkgd(new->background, COLOR_PAIR(3));
    wattron(new->background, COLOR_PAIR(3));
    for (int i = width*0.1; i < width; i++)
        mvwaddch(new->background, height-1, i, ACS_BLOCK);
    for (int i = height*0.2; i < height; i++)
        mvwaddch(new->background, i, width-1, ACS_BLOCK);
    wattroff(new->background, COLOR_PAIR(3));
    
    // оформляем передник
    new->decoration = derwin(new->background, height-2, width-2, 1, 1);
    wbkgd(new->decoration, COLOR_PAIR(1));
    box(new->decoration,0,0);
    
    tui_win_label(new->decoration, label, 0);

    int xfd, yfd;
    getmaxyx(new->decoration, yfd, xfd);
    new->overlay = derwin(new->decoration, yfd-4, xfd-2, 3, 1); // рабочее дочернее окно
    wbkgd(new->overlay, COLOR_PAIR(1));
    new->panel = new_panel(new->background);
    
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

void print_wrapped_text(WINDOW *win, const char *text) {
    int win_height, win_width;
    getmaxyx(win, win_height, win_width);
    
    int y, x;
    getyx(win, y, x); // Текущая позиция курсора
    
    char *text_copy = strdup(text);
    char *line = strtok(text_copy, "\n");
    
    while (line != NULL) {
        int line_len = strlen(line);
        int start_pos = 0;
        
        while (start_pos < line_len) {
            if (y >= win_height) {
                free(text_copy);
                return;
            }
            
            int max_chars = win_width - x;
            int chars_to_print;
            
            if (max_chars > 0) {
                if (line_len - start_pos <= max_chars) {
                    chars_to_print = line_len - start_pos;
                } else {
                    chars_to_print = max_chars;
                    while (chars_to_print > 0 && 
                           line[start_pos + chars_to_print] != ' ' &&
                           line[start_pos + chars_to_print - 1] != ' ') {
                        chars_to_print--;
                    }
                    
                    if (chars_to_print == 0) {
                        chars_to_print = max_chars;
                    }
                }
                
                mvwaddnstr(win, y, x, line + start_pos, chars_to_print);
                start_pos += chars_to_print;
                
                if (start_pos < line_len && line[start_pos] == ' ') {
                    start_pos++;
                }
                
                y++;
                x = 0;
                
                if (chars_to_print > 0 && 
                    start_pos > 0 && 
                    start_pos < line_len &&
                    line[start_pos] != ' ' &&
                    line[start_pos - 1] != ' ') {
                }
            } else {
                y++;
                x = 0;
                max_chars = win_width;
            }
        }
        
        line = strtok(NULL, "\n");
        if (line != NULL) {
            y++;
            x = 0;
        }
    }
    free(text_copy);
    wmove(win, y, x); //устанавка курсора в конечную позицию
    wrefresh(win);
}

