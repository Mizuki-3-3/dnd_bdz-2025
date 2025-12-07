#include "interface.h"

int main(){
    system("chcp 866 > nul");
    if (!ACS_HLINE) {
    // Терминал не поддерживает псевдографику
        printw("Warning: No line drawing support\n");
    }else{
        start_to_work();
        int y, x;
        getmaxyx(stdscr, y, x);
        curw *win = make_new_win(0, 0, y, x, "==Dungeons==");
        getch();
        endwin();
        return 0;
    }
}