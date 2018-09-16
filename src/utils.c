#include <stdlib.h>
#include <math.h>

#include "proto.h"

int dist_manhattan2d(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

int dist_manhattan3d(int x1, int y1, int z1, int x2, int y2, int z2) {
    return abs(x1 - x2) + abs(y1 - y2) + abs(z1 - z2);
}

int dist_euclidian(int x1, int y1, int x2, int y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    double dist = sqrt(dx*dx + dy*dy);
    return dist;
}
