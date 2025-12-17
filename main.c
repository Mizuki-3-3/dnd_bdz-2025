#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "game.h"

int main(){

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    setlocale(LC_ALL, "ru_RU.UTF-8");
    if (start_to_work() != 0) {
        fprintf(stderr, "Ошибка инициализации ncurses\n");
        return 1;
    }
    game_loop();
    endwin();
    return 0;
}