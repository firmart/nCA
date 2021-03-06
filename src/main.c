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

#include <getopt.h>

#include "proto.h"

#define NCA_VERSION "0.0.1"

/* Globals */

WINDOW *win_info;

void mainLoop(WINDOW *win_terrain, config_t *config) {
    int c, cx = 0, cy = 0;

    config_print(win_terrain, config, cy, cx);
    wrefresh(win_terrain);

    while ((c = getch()) != CTRL('q')) {

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
        //TODO: add 'C' for circular
        mvwprintw(win_info, 13, 5, "neighborhood_type : %s", config->rule->neighborhood_type == 'M' ? "Moore" : "von Neumann");
        //mvwprintw(win_info, 14, 5, "xterm : %d %d %d %d", config->palette[0], config->palette[1], config->palette[2], config->palette[3]);
        //neighbor_print(win_info, config, 14, 2);
        wrefresh(win_info);
    }
}


int main(int argc, char *argv[]) {

    const char *optstring = "vhr:";
    int c;

    struct option opts[] = {
        {"version", 0, NULL, 'v'},
        {"help", 0, NULL, 'h'},
        {"usage", 0, NULL, 'u'},
        {"rule", 1, NULL, 'r'},
        {"color-start", 1, NULL, 'c'},
        {"color-end", 1, NULL, 'c'},
    };

    char *rule_str = NULL;

    /* options parsing */
    while ((c = getopt_long(argc, argv, optstring, opts, NULL)) != -1) {
        switch (c) {

            case 'r':
                rule_str = strdup(optarg);
                break;

            case 'v':
                printf("version is %s\n", NCA_VERSION);
                exit(EXIT_SUCCESS);
                break;

            case 'h':
                //print_help();
                break;

            case '?':
                printf("unknown option\n");
                break;

            case ':':
                break;

            case 0 :
                break;

            default:
                break;
        }
    }

    /* post-processing of options */

    /* Conway's game by default */
    if (!rule_str) {
        rule_str = strdup("R1,C2,M0,S2..3,B3..3,NM");
    }

    init();


    config_t *config = create_config(300, rule_str);
    //config_set_random(config, 0.95);
    config_set_ball(config, 0, 0, 16);

    WINDOW *win_terrain = newwin(LINES - 1, LINES * 2, 0, 2);
    win_show(win_terrain, "Terrain navigation", 1);

    win_info = newwin(LINES / 2, COLS / 2, 0, LINES * 2 + 5);
    win_show(win_info, "Debug window", 1);

    mainLoop(win_terrain, config);

    endwin();

    free_config(config);
    free(rule_str);

    return 0;
}
