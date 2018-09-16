#include <stdlib.h>
#include <string.h>

#include "proto.h"

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
            mvwchgat(win, sy + i, sx + j, 1, 0, config->palette[value], NULL);
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
            if (dist_euclidian(x, y, i, j) <= r) {
                config_setyx(config, j, i, 1);
                config->population++;
            }
        }
    }
}

void config_set_random(config_t *config, double prob) {
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

config_t *create_config(int radius, char *rulestring) {
    config_t *config = malloc(sizeof(config_t));
    config->terrain_mode = 1; /* temporarily just support wall mode */
    config->rule_str = strdup(rulestring); /* temporarily just support Conway's game of life */
    config->rule = rule_create(config->rule_str);
    config->neighborhood = neighbor_create(config->rule);
    config->radius = radius;
    config->generation = 0;
    config->population = 0;
    config->terrain = create_terrain(radius);
    config->palette = palette_create(config->rule->n_states);
    return config;
}

void free_terrain(int **terrain, int radius) {
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
    free(config->palette);
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
