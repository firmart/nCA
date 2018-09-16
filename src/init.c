#include <stdlib.h>
#include <time.h>

#include "proto.h"

/* Init functions */

void init_colors() {

    for (int i = 0 ; i < COLORS; ++i) {
        init_pair(i + 1, -1, i);
    }

}

void init() {
    initscr();             /* ncurses init                        */
    refresh();             /* refresh stdscr so that
                              children windows could be displayed */
    start_color();         /* use colors                          */
    use_default_colors();  /* use terminal default colors when we
                              initialize color in pair with -1    */
    cbreak();
    //raw();                 /* raw mode                            */
    noecho();              /* do not echo user input              */
    keypad(stdscr, TRUE);  /* enable functional keys              */
    curs_set(0);           /* invisible cursor                    */

    /* other init functions */
    init_colors();
    srand(time(NULL));
}
