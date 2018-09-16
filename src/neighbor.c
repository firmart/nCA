#include <stdlib.h>

#include "proto.h"

/* Neighborhood-related functions */

//TODO: add Euclidian distance


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

            if (rule->neighborhood_type == 'N' && dist_manhattan2d(i, j, rule->range, rule->range) <= rule->range) {
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


void neighbor_print(WINDOW *win, config_t *config, int sy, int sx) {

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
