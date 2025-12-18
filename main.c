#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

int main(){

    if (start_to_work() != 0) {
        fprintf(stderr, "Ошибка инициализации ncurses\n");
        return 1;
    }
    game_loop();
    endwin();
    return 0;
}