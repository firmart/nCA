#include <stdlib.h>
#include <string.h>

#include "proto.h"


/* Rule-related functions */

rule_t *rule_create(char *rule_str) {
    rule_t *rule = malloc(sizeof(rule_t));
    sscanf(rule_str, "R%d,C%d,M%d,S%d..%d,B%d..%d,N%c", &rule->range, &rule->n_states, &rule->middle_included, &rule->s_min, &rule->s_max, &rule->b_min, &rule->b_max, &rule->neighborhood_type);

    if (rule->n_states < 2) {
        rule->n_states = 2;
    }

    return rule;
}


void rule_apply(config_t *config) {
    int size = 2 * config->radius + 1;

    /* create temporary config and terrain */
    config_t *tmp_config = malloc(sizeof(config_t));
    memcpy(tmp_config, config, sizeof(config_t));

    tmp_config->terrain = create_terrain(config->radius);

    /* copy terrain */
    for (int i = 0; i < size; i++) {
        memcpy(tmp_config->terrain[i], config->terrain[i], size * sizeof(int));
    }

    /* compute next generation */
    for (int i = -config->radius; i <= config->radius; ++i) {
        for (int j = -config->radius; j <= config->radius; ++j) {
            int nc = neighbor_count(config, i, j);
            int st = config_getyx(config, i, j);

            /* survive case */
            if (st) {
                if (nc >= config->rule->s_min && nc <= config->rule->s_max) {
                    continue;

                } else {
                    int new_st = (st + 1) % config->rule->n_states;
                    config_setyx(tmp_config, i, j, new_st);

                    if (new_st == 0) {
                        config->population--;
                    }

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
    free(config->terrain);
    config->terrain = tmp_config->terrain;
    config->generation++;
    free(tmp_config);
    //tmp_config->terrain = NULL;
    //free_config(tmp_config);
}
