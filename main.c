#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <ncurses.h>
#include <panel.h>


#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif


/* TODO
 *   - display legend panel when pressed a key
 *   - menu to choose neighborhood
 *   - insert mode
 *   - playback mode (forward/backward, speed)
 *   - parse http://www.mirekw.com/ca/ca_files_formats.html
 *   - palette
 *   - terrain : move around
 * */

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
}

void insertMode() {

}

void mainLoop() {
    int c, x, y;

    while ((c = getch()) != CTRL('q')) {

        getmaxyx(stdscr, y, x);
        move(0, 0);
        hline(' ', x);
        printw("%d, %d", y, x);

        switch (c) {
            case 'i':
                insertMode();

            default: /* TODO */
                break;
        }
    }
}


typedef struct state_t {
    int terrain_mode; /* 0 = continuous mode, 1 = wall mode, 2 = unlimited */
    int **terrain;
    int radius;
} state_t;

void setxy(state_t *state, int x, int y, int value) {
    int r = state->radius;

    if (abs(x) <= r && abs(y) <= r) {
        state->terrain[r + y - 1][r + x - 1] = value;
    }
}

void set_frontier(state_t *state, int fr) {
    for (int i = -fr; i <= fr; i++) {
        setxy(state, i, fr, fr);
        setxy(state, i, -fr, fr);
        setxy(state, fr, i, fr);
        setxy(state, -fr, i, fr);
    }
}

int getxy(state_t *state, int x, int y) {
    int r = state->radius;

    if (abs(x) <= r && abs(y) <= r) {
        return state->terrain[r + y - 1][r + x - 1];

    } else {
        return 0;
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

void print_state(WINDOW *win, int sy, int sx, state_t *state) {
    int size = state->radius * 2 - 1;

    wmove(win, sy, sx);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            mvwprintw(win, sy + i, sx + j,  "%d", state->terrain[i][j]);
        }
    }

}

void increase_size(state_t *state, int size) {
    int new_radius = state->radius + size;
    int new_side_len = new_radius * 2 - 1;
    int old_side_len = state->radius * 2 - 1;
    int **new_terrain = create_terrain(new_radius);

    for (int i = size; i < new_side_len - size; i++) {
        memcpy(&new_terrain[i][size], state->terrain[i - size], old_side_len * sizeof(int));
    }

    state->radius = new_radius;
    state->terrain = new_terrain;
}

//int main(int argc, char *argv[]) {
//    state_t* state = create_state(5);
//    setxy(state, 0, 0, 10);
//    for (int i = 0; i < 5; i++) {
//        set_frontier(state, i);
//    }
//    print_state(state);
//    increase_size(state, 2);
//    for (int i = 0; i < 7; i++) {
//        set_frontier(state, i);
//    }
//    print_state(state);
//    free_state(state);
//    return 0;
//}


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

    init_pair(1, COLOR_WHITE, -1);
    WINDOW *win_terrain = newwin(LINES - 1, LINES * 2, 0, 2);
    win_show(win_terrain, "Terrain window", 1);
    state_t *state = create_state(10);
    for (int i = 0; i < 10; i++) {
        set_frontier(state, i);
    }
    print_state(win_terrain, 3, 2, state);
    wrefresh(win_terrain);
    //mainLoop();

    getch();

    endwin();
    return 0;
}
