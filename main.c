#include <ncurses.h>
#include <panel.h>

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif


/* TODO
 *   - display legend panel when pressed a key
 *   - menu to choose neighborhood
 *   - mode to insert
 *   - mode for playback
 * */

void init() {
    initscr();             /* ncurses init           */
    raw();                 /* raw mode               */
    noecho();              /* do not echo user input */
    keypad(stdscr, TRUE);  /* enable functional keys */
}

void insertMode() {
}

void mainLoop() {
    int c;
    while((c = getch()) != CTRL('q')) {
        switch(c) {
            case 'i': 
                insertMode();
            
            default: /* TODO */
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    init();

    mainLoop(); 

    endwin();
    return 0;
}
