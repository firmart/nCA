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

typedef struct config_t {
    tm_t terrain_mode; /* 0 = continuous mode, 1 = wall mode, 2 = unlimited */
    int **terrain;
    int radius;
    int generation;
    int population;
    char *rule_str;
    rule_t *rule;
    int **neighborhood;
} config_t;

/* Prototypes */

int config_getyx(config_t *config, int y, int x) ;
void config_setyx(config_t *config, int y, int x, int value) ;
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
    cbreak();              
    //raw();                 /* raw mode                            */
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

int neighbor_count(config_t *config, int y, int x) {
    int counter = 0;
    int **neighbor = config->neighborhood;

    if (config->terrain_mode == T_WALL) {
        for (int k = -config->rule->range; k <= config->rule->range; ++k) {
            for (int l = -config->rule->range; l <= config->rule->range; ++l) {
                counter += config_getyx(config, y + k, x + l) * config->neighborhood[k + config->rule->range][l + config->rule->range];
            }
        }
    }

    return counter;
}


void neighbor_print(WINDOW *win, config_t* config, int sy, int sx) {

    int height = 0, width = 0;
    getmaxyx(win, height, width);
    wmove(win, sy, sx);

    for (int i = 0; i < min(height - 4, 2 * config->rule->range + 1); ++i) {
        mvwhline(win, sy + i, 1, ' ', width - 2);

        for (int j = 0; j < min(width - 2, 2 * config->rule->range + 1); ++j) {
            mvwprintw(win, sy + i, sx + j,  "%d", config->neighborhood[i][j]);
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


void rule_apply(config_t *config) {
    int size = 2 * config->radius + 1;

    /* create temporary config and terrain */
    config_t* tmp_config = malloc(sizeof(config_t));
    memcpy(tmp_config, config, sizeof(config_t));

    tmp_config->terrain = create_terrain(config->radius);

    /* copy terrain */
    for (int i = 0; i < size; i++) {
        memcpy(tmp_config->terrain[i], config->terrain[i], size * sizeof(int));
    }

    /* compute next generation */
    for(int i = -config->radius; i <= config->radius; ++i) {
        for(int j = -config->radius; j <= config->radius; ++j) {
            int nc = neighbor_count(config, i, j);
            int st = config_getyx(config, i, j);

            /* survive case */
            if (st) {
                if (nc >= config->rule->s_min && nc <= config->rule->s_max) {
                    continue;
                } else {
                    int new_st = (st + 1) % config->rule->n_states;
                    config_setyx(tmp_config, i, j, new_st);
                    if (new_st == 0) config->population--;

                }
            } 
            /* dead case */
            else {
                if (nc >= config->rule->b_min && nc <= config->rule->b_max) {
                    /* get born */
                    config_setyx(tmp_config, i, j, 1);
                    config->population++;
                }
            }
        }
    }

    free_terrain(config->terrain, config->radius);
    config->terrain = tmp_config->terrain;
    config->generation++;
}

/* State-related functions */


/* TODO: - consider terrain mode
 *          - continuous/wall mode : draw limit
 * */
void config_print(WINDOW *win, config_t *config, int cy, int cx) {

    int sy = 3, sx = 1, height = 0, width = 0;
    getmaxyx(win, height, width);
    wmove(win, sy, sx);

    for (int i = 0; i < height - 4; ++i) {
        mvwhline(win, sy + i, 1, ' ', width - 2);

        for (int j = 0; j < width - 2; ++j) {
            int value = config_getyx(config, - i - cy + (height - 3) / 2, j - cx - (width - 1) / 2) ;
            mvwprintw(win, sy + i, sx + j,  "%d", value % 10);
            mvwchgat(win, sy + i, sx + j, 1, 0, value > 0 ? 5 : 0, NULL);
        }
    }

}

int config_getyx(config_t *config, int y, int x) {
    int r = config->radius;

    if (abs(x) <= r && abs(y) <= r) {
        return config->terrain[r + y][r + x];

    } else {
        return 0;
    }
}

void config_setyx(config_t *config, int y, int x, int value) {
    int r = config->radius;

    if (abs(x) <= r && abs(y) <= r) {
        config->terrain[r + y][r + x] = value;
    }
}

void config_set_frontier(config_t *config, int fr) {
    for (int i = -fr; i <= fr; i++) {
        config_setyx(config, i, -fr, 1);
        config_setyx(config, i, fr, 1);
        config_setyx(config, fr, i, 1);
        config_setyx(config, -fr, i, 1);
    }
    config->population = fr * 8 ;
}

void config_set_ball(config_t *config, int y, int x, int r) {
    for (int i = x - r; i <= x + r; ++i) {
        for (int j = y - r; j <= y + r; ++j) {
            if (dist_manhattan(x, y, i, j) <= r) {
                config_setyx(config, j, i, 1);
                config->population++;
            }
        }
    }
}

void config_set_random(config_t* config, double prob) {
    for (int i = -config->radius; i <= config->radius; i++) {
        for (int j = -config->radius; j <= config->radius; j++) {
            double rd = (double) rand() / RAND_MAX;
            if (rd > prob) { 
                config_setyx(config, i, j, (rand() % (config->rule->n_states - 1)) + 1);
                config->population++;
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

config_t *create_config(int radius, char* rulestring) {
    config_t *config = malloc(sizeof(config_t));
    config->terrain_mode = 1; /* temporarily just support wall mode */
    config->rule_str = strdup(rulestring); /* temporarily just support Conway's game of life */
    config->rule = rule_create(config->rule_str);
    config->neighborhood = neighbor_create(config->rule);
    config->radius = radius;
    config->generation = 0;
    config->population = 0;
    config->terrain = create_terrain(radius);
    return config;
}

void free_terrain(int** terrain, int radius) {
    int size = 2 * radius + 1;
    for (int i = 0; i < size; ++i) {
        free(terrain[i]);
    }
    
}

void free_config(config_t *config) {


    //for (int i = 0; i < size; ++i) {
    //    free(config->neighborhood[i]);
    //}

    free_terrain(config->terrain, config->radius);
    free(config->terrain);
    free(config->rule_str);
    free(config->rule);
    free(config);
}


void increase_size(config_t *config, int size) {
    int new_radius = config->radius + size;
    int new_side_len = 2 * new_radius + 1;
    int old_side_len = 2 * config->radius + 1;
    int **new_terrain = create_terrain(new_radius);

    for (int i = size; i < new_side_len - size; i++) {
        memcpy(&new_terrain[i][size], config->terrain[i - size], old_side_len * sizeof(int));
    }

    free(config->terrain);

    config->radius = new_radius;
    config->terrain = new_terrain;
}


void mainLoop(WINDOW *win_terrain, config_t *config) {
    int c, cx = 0, cy = 0;

    config_print(win_terrain, config, cy, cx);
    wrefresh(win_terrain);

    while ((c = getch()) != 'q') {

        switch (c) {

            case 'n' :
                rule_apply(config);
                config_print(win_terrain, config, cy, cx);
                wrefresh(win_terrain);
                break;

            case KEY_DOWN:
            case 'j':
                config_print(win_terrain, config, ++cy, cx);
                wrefresh(win_terrain);
                break;

            case KEY_UP:
            case 'k':
                config_print(win_terrain, config, --cy, cx);
                wrefresh(win_terrain);
                break;

            case KEY_LEFT:
            case 'h':
                config_print(win_terrain, config, cy, ++cx);
                wrefresh(win_terrain);
                break;

            case KEY_RIGHT:
            case 'l':
                config_print(win_terrain, config, cy, --cx);
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
        mvwprintw(win_info, 4, 2, "neighbor count : %d", neighbor_count(config, cy, cx));
        mvwprintw(win_info, 5, 2, "generation : %d", config->generation);
        mvwprintw(win_info, 6, 2, "population : %d", config->population);
        mvwprintw(win_info, 7, 2, "rule string : %s", config->rule_str);
        mvwprintw(win_info, 8, 5, "range : %d", config->rule->range);
        mvwprintw(win_info, 9, 5, "number of states : %d", config->rule->n_states);
        mvwprintw(win_info, 10, 5, "middle included : %s", config->rule->middle_included ? "true" : "false");
        mvwprintw(win_info, 11, 5, "survive : %d to %d", config->rule->s_min, config->rule->s_max);
        mvwprintw(win_info, 12, 5, "born : %d to %d", config->rule->b_min, config->rule->b_max);
        mvwprintw(win_info, 13, 5, "neighborhood_type : %s", config->rule->neighborhood_type == 'M' ? "Moore" : "von Neumann");
        neighbor_print(win_info, config, 13, 2);
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

    config_t *config = create_config(1000, "R1,C2,M1,S4..8,B4..8,NM");
    //config_set_random(config, 0.95);

    config_set_ball(config, 0, 0, 10);

    mainLoop(win_terrain, config);
    endwin();
    return 0;
}
