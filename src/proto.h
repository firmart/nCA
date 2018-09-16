#ifndef PROTO_H
#define PROTO_H 

#include "nca.h"

/* neighborhood functions */
int neighbor_count(config_t *config, int y, int x);
int** neighbor_create(rule_t *rule);
void neighbor_print(WINDOW *win, config_t *config, int sy, int sx);

int config_getyx(config_t *config, int y, int x);
void config_print(WINDOW *win, config_t *config, int cy, int cx);
void config_set_ball(config_t *config, int y, int x, int r);
void config_set_frontier(config_t *config, int fr);
void config_set_random(config_t *config, double prob);
void config_setyx(config_t *config, int y, int x, int value);
config_t* create_config(int radius, char *rulestring);
int** create_terrain(int radius);
int dist_euclidian(int x1, int y1, int x2, int y2);
int dist_manhattan2d(int x1, int y1, int x2, int y2);
int dist_manhattan3d(int x1, int y1, int z1, int x2, int y2, int z2);
void free_config(config_t *config);
void free_terrain(int **terrain, int radius);
void increase_size(config_t *config, int size);
void init();
void init_colors();
int main(int argc, char *argv[]);
void mainLoop(WINDOW *win_terrain, config_t *config);
int* palette_create(int n_states);
void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
int rgb2xterm(int r, int g, int b);
void rule_apply(config_t *config);
rule_t* rule_create(char *rule_str);
void win_show(WINDOW *win, char *label, int label_color);
#endif /* ifndef PROTO_H */
