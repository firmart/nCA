/* TODO
 *   - display legend panel when pressed a key
 *   - menu to choose neighborhood
 *   - insert mode
 *   - playback mode (forward/backward, speed)
 *   - parse http://www.mirekw.com/ca/ca_files_formats.html
 *   - palette
 *   - terrain : move around
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <ncurses.h>
#include <panel.h>


#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif


typedef struct state_t {
    int terrain_mode; /* 0 = continuous mode, 1 = wall mode, 2 = unlimited */
    int **terrain;
    int radius;
} state_t;

WINDOW *win_info;

void init_colors() {

    init_pair(1, -1, COLOR_BLACK   );
    init_pair(2, -1, COLOR_RED     );
    init_pair(3, -1, COLOR_GREEN   );
    init_pair(4, -1, COLOR_YELLOW  );
    init_pair(5, -1, COLOR_BLUE    );
    init_pair(6, -1, COLOR_MAGENTA );
    init_pair(7, -1, COLOR_CYAN    );
    init_pair(8, -1, COLOR_WHITE   );

}

void init() {
    initscr();             /* ncurses init                        */
    refresh();             /* refresh stdscr so that
                              children windows could be displayed */
    start_color();         /* use colors                          */
    use_default_colors();  /* use terminal default colors when we
                              initialize color in pair with -1    */
    raw();                 /* raw mode                            */
    noecho();              /* do not echo user input              */
    keypad(stdscr, TRUE);  /* enable functional keys              */
    curs_set(0);           /* invisible cursor                    */

    /* other init functions */
    init_colors();
}


void insertMode() {

}

int state_getyx(state_t *state, int y, int x) {
    int r = state->radius;

    if (abs(x) < r && abs(y) < r) {
        return state->terrain[r + y - 1][r + x - 1];

    } else {
        return 0;
    }
}

void set_cell_attr(WINDOW *win, int value) {
    if (value > 0) {
        wattrset(win, COLOR_PAIR(7));

    } else {
        wstandend(win);
    }
}


void print_state(WINDOW *win, state_t *state, int cy, int cx) {

    int sy = 3, sx = 1, height = 0, width = 0;
    getmaxyx(win, height, width);
    wmove(win, sy, sx);

    for (int i = 0; i < height - 4; ++i) {
        mvwhline(win, sy + i, 1, ' ', width - 2);
        for (int j = 0; j < width - 2; ++j) {
            int value = state_getyx(state, - i - cy + (height - 3) / 2, j - cx - (width - 1) / 2) ;
            //set_cell_attr(win, value);
            mvwprintw(win, sy + i, sx + j,  "%d", value % 10);
            mvwchgat(win, sy + i, sx + j, 1, 0, value > 0 ? 7 : 0, NULL);
            //mvwprintw(win, sy + i, sx + j,  " ");
        }
    }

}

void mainLoop(WINDOW *win_terrain, state_t *state) {
    int c, cx = 0, cy = 0;

    print_state(win_terrain, state, cy, cx);
    wrefresh(win_terrain);

    while ((c = getch()) != CTRL('q')) {

        //mvhline(2, 1, ACS_HLINE, width - 2);
        switch (c) {
            case 'i':
                insertMode();
                break;

            case KEY_DOWN:
                print_state(win_terrain, state, ++cy, cx);
                wrefresh(win_terrain);
                break;

            case KEY_UP:
                print_state(win_terrain, state, --cy, cx);
                wrefresh(win_terrain);
                break;

            case KEY_LEFT:
                print_state(win_terrain, state, cy, ++cx);
                wrefresh(win_terrain);
                break;

            case KEY_RIGHT:
                print_state(win_terrain, state, cy, --cx);
                wrefresh(win_terrain);
                break;

            default:
                break;
        }

        int info_width, info_height;
        getmaxyx(win_info, info_height, info_width);
        mvwhline(win_info, 3, 1, ' ', info_width - 2);
        mvwprintw(win_info, 3, 2, "cursor : (%d, %d)", cx, cy);
        wrefresh(win_info);
    }
}



void state_setyx(state_t *state, int y, int x, int value) {
    int r = state->radius;

    if (abs(x) < r && abs(y) < r) {
        state->terrain[r + y - 1][r + x - 1] = value;
    }
}

void state_set_frontier(state_t *state, int fr) {
    for (int i = -fr; i <= fr; i++) {
        state_setyx(state, i, -fr, fr);
        state_setyx(state, i, fr, fr);
        state_setyx(state, fr, i, fr);
        state_setyx(state, -fr, i, fr);
    }
}


int **create_terrain(int radius) {
    int size = radius * 2 - 1;
    int **terrain = malloc(size * sizeof(int *));

    for (int i = 0; i < size; ++i) {
        terrain[i] = calloc(size, sizeof(int));
    }

    return terrain;
}

state_t *create_state(int radius) {
    state_t *state = malloc(sizeof(state_t));
    state->radius = radius;
    state->terrain = create_terrain(radius);
    return state;
}

void free_state(state_t *state) {
    int size = state->radius * 2 - 1;

    for (int i = 0; i < size; ++i) {
        free(state->terrain[i]);
    }

    free(state->terrain);
    free(state);
}


void increase_size(state_t *state, int size) {
    int new_radius = state->radius + size;
    int new_side_len = new_radius * 2 - 1;
    int old_side_len = state->radius * 2 - 1;
    int **new_terrain = create_terrain(new_radius);

    for (int i = size; i < new_side_len - size; i++) {
        memcpy(&new_terrain[i][size], state->terrain[i - size], old_side_len * sizeof(int));
    }

    free(state->terrain);

    state->radius = new_radius;
    state->terrain = new_terrain;
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color) {
    int length, x, y;
    float temp;

    if (win == NULL) {
        win = stdscr;
    }

    getyx(win, y, x);

    if (startx != 0) {
        x = startx;
    }

    if (starty != 0) {
        y = starty;
    }

    if (width == 0) {
        width = 80;
    }

    length = strlen(string);
    temp = (width - length) / 2;
    x = startx + (int)temp;
    wattron(win, color);
    mvwprintw(win, y, x, "%s", string);
    wattroff(win, color);
    wrefresh(win);
}

void win_show(WINDOW *win, char *label, int label_color) {
    int startx, starty, height, width;

    getbegyx(win, starty, startx);
    getmaxyx(win, height, width);

    box(win, 0, 0);
    mvwaddch(win, 2, 0, ACS_LTEE);
    mvwhline(win, 2, 1, ACS_HLINE, width - 2);
    mvwaddch(win, 2, width - 1, ACS_RTEE);

    print_in_middle(win, 1, 0, width, label, COLOR_PAIR(label_color));
}


int main(int argc, char *argv[]) {
    init();


    WINDOW *win_terrain = newwin(LINES - 1, LINES * 2, 0, 2);
    win_show(win_terrain, "Terrain navigation", 1);

    win_info = newwin(LINES/2, COLS/2, 0, LINES * 2 + 5);
    win_show(win_info, "Debug window", 1);

    state_t *state = create_state(25);

    for (int i = 0; i < 25; i++) {
        state_set_frontier(state, i);
    }

    mainLoop(win_terrain, state);
    endwin();
    return 0;
}
