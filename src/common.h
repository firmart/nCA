#ifndef COMMON_H
#define COMMON_H


/* Structures */

/* firstly implement LTL rules */
// http://golly.sourceforge.net/Help/Algorithms/Larger_than_Life.html

typedef enum tm_t {
    T_CONTINUOUS, /* Toroidal */
    T_WALL,
    T_UNLIMITED
} tm_t;

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
    int* palette;
    int **neighborhood;
} config_t;

typedef struct rgb_t {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb_t;

#endif /* ifndef COMMON_H */
