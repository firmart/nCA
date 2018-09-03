/* TODO
 *   - display legend panel when pressed a key
 *   - menu to choose neighborhood
 *   - insert mode
 *   - playback mode (forward/backward, speed)
 *   - parse http://www.mirekw.com/ca/ca_files_formats.html
 *   - palette
 *   - terrain : move around
 *   - HashLife Algorithm
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <ncurses.h>
#include <panel.h>

/* Definitions */

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif


#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

typedef enum tm_t {
    T_CONTINUOUS, /* Toroidal */
    T_WALL,
    T_UNLIMITED
} tm_t;


/* Structures */

/* firstly implement LTL rules */
// http://golly.sourceforge.net/Help/Algorithms/Larger_than_Life.html

typedef struct rule_t {
    int range;
    int n_states;
    int middle_included;
    int s_min;
    int s_max;
    int b_min;
    int b_max;
    char neighborhood_type;
} rule_t;

typedef struct state_t {
    tm_t terrain_mode; /* 0 = continuous mode, 1 = wall mode, 2 = unlimited */
    int **terrain;
    int radius;
    int generation;
    int population;
    char *rule_str;
    rule_t *rule;
    int **neighborhood;
} state_t;

/* Prototypes */

int state_getyx(state_t *state, int y, int x) ;
void state_setyx(state_t *state, int y, int x, int value) ;
int **create_terrain(int radius) ;
void free_terrain(int** terrain, int radius) ;

/* Globals */

WINDOW *win_info;

/* Init functions */

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
    srand(time(NULL));
}

/* Neighborhood-related functions */

int dist_manhattan(int x1, int y1, int x2, int y2) {
    return abs(y1 - y2) + abs(x1 - x2);
}

int **neighbor_create(rule_t *rule) {
    int size = 2 * rule->range + 1;
    int **neighborhood = malloc(size * sizeof(int *));

    for (int i = 0; i < size; ++i) {
        neighborhood[i] = calloc(size, sizeof(int));
    }

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {

            if (i == rule->range && j == rule->range) {
                if (rule->middle_included) {
                    neighborhood[i][j] = 1;

                } else {
                    continue;
                }
            }

            if (rule->neighborhood_type == 'N' && dist_manhattan(i, j, rule->range, rule->range) <= rule->range) {
                neighborhood[i][j] = 1;

            } else if (rule->neighborhood_type == 'M') {
                neighborhood[i][j] = 1;
            }
        }
    }

    return neighborhood;
}

/* TODO - Only support wall mode for the momment.
 *        Implement
 *          - unlimited mode
 *          - continuous mode
 */

int neighbor_count(state_t *state, int y, int x) {
    int counter = 0;
    int **neighbor = state->neighborhood;

    if (state->terrain_mode == T_WALL) {
        for (int k = -state->rule->range; k <= state->rule->range; ++k) {
            for (int l = -state->rule->range; l <= state->rule->range; ++l) {
                counter += state_getyx(state, y + k, x + l) * state->neighborhood[k + state->rule->range][l + state->rule->range];
            }
        }
    }

    return counter;
}


void neighbor_print(WINDOW *win, state_t* state, int sy, int sx) {

    int height = 0, width = 0;
    getmaxyx(win, height, width);
    wmove(win, sy, sx);

    for (int i = 0; i < min(height - 4, 2 * state->rule->range + 1); ++i) {
        mvwhline(win, sy + i, 1, ' ', width - 2);

        for (int j = 0; j < min(width - 2, 2 * state->rule->range + 1); ++j) {
            mvwprintw(win, sy + i, sx + j,  "%d", state->neighborhood[i][j]);
        }
    }

}

/* Rule-related functions */

rule_t *rule_create(char *rule_str) {
    rule_t *rule = malloc(sizeof(rule_t));
    sscanf(rule_str, "R%d,C%d,M%d,S%d..%d,B%d..%d,N%c", &rule->range, &rule->n_states, &rule->middle_included, &rule->s_min, &rule->s_max, &rule->b_min, &rule->b_max, &rule->neighborhood_type);
    if (rule->n_states < 2) rule->n_states = 2;
    return rule;
}


void rule_apply(state_t *state) {
    int size = 2 * state->radius + 1;

    /* create temporary state and terrain */
    state_t* tmp_state = malloc(sizeof(state_t));
    memcpy(tmp_state, state, sizeof(state_t));

    tmp_state->terrain = create_terrain(state->radius);

    /* copy terrain */
    for (int i = 0; i < size; i++) {
        memcpy(tmp_state->terrain[i], state->terrain[i], size * sizeof(int));
    }

    /* compute next generation */
    for(int i = -state->radius; i <= state->radius; ++i) {
        for(int j = -state->radius; j <= state->radius; ++j) {
            int nc = neighbor_count(state, i, j);
            int st = state_getyx(state, i, j);

            /* survive case */
            if (st) {
                if (nc >= state->rule->s_min && nc <= state->rule->s_max) {
                    continue;
                } else {
                    int new_st = (st + 1) % state->rule->n_states;
                    state_setyx(tmp_state, i, j, new_st);
                    if (new_st == 0) state->population--;

                }
            } 
            /* dead case */
            else {
                if (nc >= state->rule->b_min && nc <= state->rule->b_max) {
                    /* get born */
                    state_setyx(tmp_state, i, j, 1);
                    state->population++;
                }
            }
        }
    }

    free_terrain(state->terrain, state->radius);
    state->terrain = tmp_state->terrain;
    state->generation++;
}

/* State-related functions */


/* TODO: - consider terrain mode
 *          - continuous/wall mode : draw limit
 * */
void state_print(WINDOW *win, state_t *state, int cy, int cx) {

    int sy = 3, sx = 1, height = 0, width = 0;
    getmaxyx(win, height, width);
    wmove(win, sy, sx);

    for (int i = 0; i < height - 4; ++i) {
        mvwhline(win, sy + i, 1, ' ', width - 2);

        for (int j = 0; j < width - 2; ++j) {
            int value = state_getyx(state, - i - cy + (height - 3) / 2, j - cx - (width - 1) / 2) ;
            mvwprintw(win, sy + i, sx + j,  "%d", value % 10);
            mvwchgat(win, sy + i, sx + j, 1, 0, value > 0 ? 5 : 0, NULL);
        }
    }

}

int state_getyx(state_t *state, int y, int x) {
    int r = state->radius;

    if (abs(x) <= r && abs(y) <= r) {
        return state->terrain[r + y][r + x];

    } else {
        return 0;
    }
}

void state_setyx(state_t *state, int y, int x, int value) {
    int r = state->radius;

    if (abs(x) <= r && abs(y) <= r) {
        state->terrain[r + y][r + x] = value;
    }
}

void state_set_frontier(state_t *state, int fr) {
    for (int i = -fr; i <= fr; i++) {
        state_setyx(state, i, -fr, 1);
        state_setyx(state, i, fr, 1);
        state_setyx(state, fr, i, 1);
        state_setyx(state, -fr, i, 1);
    }
    state->population = fr * 8 ;
}

void state_set_ball(state_t *state, int y, int x, int r) {
    for (int i = x - r; i <= x + r; ++i) {
        for (int j = y - r; j <= y + r; ++j) {
            if (dist_manhattan(x, y, i, j) <= r) {
                state_setyx(state, j, i, 1);
                state->population++;
            }
        }
    }
}

void state_set_random(state_t* state, double prob) {
    for (int i = -state->radius; i <= state->radius; i++) {
        for (int j = -state->radius; j <= state->radius; j++) {
            double rd = (double) rand() / RAND_MAX;
            if (rd > prob) { 
                state_setyx(state, i, j, (rand() % (state->rule->n_states - 1)) + 1);
                state->population++;
            }
        }
    }
}


int **create_terrain(int radius) {
    int size = 2 * radius + 1;
    int **terrain = malloc(size * sizeof(int *));

    for (int i = 0; i < size; ++i) {
        terrain[i] = calloc(size, sizeof(int));
    }

    return terrain;
}

state_t *create_state(int radius, char* rulestring) {
    state_t *state = malloc(sizeof(state_t));
    state->terrain_mode = 1; /* temporarily just support wall mode */
    state->rule_str = strdup(rulestring); /* temporarily just support Conway's game of life */
    state->rule = rule_create(state->rule_str);
    state->neighborhood = neighbor_create(state->rule);
    state->radius = radius;
    state->generation = 0;
    state->population = 0;
    state->terrain = create_terrain(radius);
    return state;
}

void free_terrain(int** terrain, int radius) {
    int size = 2 * radius + 1;
    for (int i = 0; i < size; ++i) {
        free(terrain[i]);
    }
    
}

void free_state(state_t *state) {


    //for (int i = 0; i < size; ++i) {
    //    free(state->neighborhood[i]);
    //}

    free_terrain(state->terrain, state->radius);
    free(state->terrain);
    free(state->rule_str);
    free(state->rule);
    free(state);
}


void increase_size(state_t *state, int size) {
    int new_radius = state->radius + size;
    int new_side_len = 2 * new_radius + 1;
    int old_side_len = 2 * state->radius + 1;
    int **new_terrain = create_terrain(new_radius);

    for (int i = size; i < new_side_len - size; i++) {
        memcpy(&new_terrain[i][size], state->terrain[i - size], old_side_len * sizeof(int));
    }

    free(state->terrain);

    state->radius = new_radius;
    state->terrain = new_terrain;
}


void mainLoop(WINDOW *win_terrain, state_t *state) {
    int c, cx = 0, cy = 0;

    state_print(win_terrain, state, cy, cx);
    wrefresh(win_terrain);

    while ((c = getch()) != CTRL('q')) {

        switch (c) {

            case 'n' :
                rule_apply(state);
                state_print(win_terrain, state, cy, cx);
                wrefresh(win_terrain);
                break;

            case KEY_DOWN:
            case 'j':
                state_print(win_terrain, state, ++cy, cx);
                wrefresh(win_terrain);
                break;

            case KEY_UP:
            case 'k':
                state_print(win_terrain, state, --cy, cx);
                wrefresh(win_terrain);
                break;

            case KEY_LEFT:
            case 'h':
                state_print(win_terrain, state, cy, ++cx);
                wrefresh(win_terrain);
                break;

            case KEY_RIGHT:
            case 'l':
                state_print(win_terrain, state, cy, --cx);
                wrefresh(win_terrain);
                break;

            default:
                break;
        }

        int info_width, info_height;
        getmaxyx(win_info, info_height, info_width);

        for (int i = 3; i <= 11; ++i) {
            mvwhline(win_info, i, 1, ' ', info_width - 2);
        }

        mvwprintw(win_info, 3, 2, "cursor : (%d, %d)", cx, cy);
        mvwprintw(win_info, 4, 2, "neighbor count : %d", neighbor_count(state, cy, cx));
        mvwprintw(win_info, 5, 2, "generation : %d", state->generation);
        mvwprintw(win_info, 6, 2, "population : %d", state->population);
        mvwprintw(win_info, 7, 2, "rule string : %s", state->rule_str);
        mvwprintw(win_info, 8, 5, "range : %d", state->rule->range);
        mvwprintw(win_info, 9, 5, "number of states : %d", state->rule->n_states);
        mvwprintw(win_info, 10, 5, "middle included : %s", state->rule->middle_included ? "true" : "false");
        mvwprintw(win_info, 11, 5, "survive : %d to %d", state->rule->s_min, state->rule->s_max);
        mvwprintw(win_info, 12, 5, "born : %d to %d", state->rule->b_min, state->rule->b_max);
        mvwprintw(win_info, 13, 5, "neighborhood_type : %s", state->rule->neighborhood_type == 'M' ? "Moore" : "von Neumann");
        neighbor_print(win_info, state, 13, 2);
        wrefresh(win_info);
    }
}


/* Utils functions */

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

    win_info = newwin(LINES / 2, COLS / 2, 0, LINES * 2 + 5);
    win_show(win_info, "Debug window", 1);

    state_t *state = create_state(1000, "R1,C2,M1,S3..8,B3..8,NM");
    //state_set_random(state, 0.95);

    state_set_ball(state, 0, 0, 10);

    mainLoop(win_terrain, state);
    endwin();
    return 0;
}
