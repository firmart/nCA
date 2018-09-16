#include <stdlib.h>

#include "proto.h"

/* palette-related functions */

int rgb2xterm(int r, int g, int b) {

    int c[6] = {0, 95, 135, 175, 215, 255}; 
    int rlast = 255, glast = 255, blast = 255;
    int rindex = -1, gindex = -1, bindex = -1;

    for (int i = 0; i < 6; i++) {
        if (abs(r - c[i]) <= rlast) {
            rlast = abs(r - c[i]);
        } else if(rindex < 0) rindex = i - 1;
        

        if (abs(g - c[i]) <= glast) {
            glast = abs(g - c[i]);
        } else if(gindex < 0) gindex = i - 1;

        if (abs(b - c[i]) <= blast) {
            blast = abs(b - c[i]);
        } else if(bindex < 0) bindex = i - 1;

    }

    if (rindex == -1) rindex = 5;
    if (gindex == -1) gindex = 5;
    if (bindex == -1) bindex = 5;

    int cdist = dist_manhattan3d(r, g, b, c[rindex], c[gindex], c[bindex]);

    int temp = -1, vtemp = 255*3;
    for(int i = 0; i < 24; ++i) {
        int v = 8 + 10 * i;
        int vdist = dist_manhattan3d(r, g, b, v, v, v);
        if (vdist < cdist && vdist < vtemp) {
            temp = i;
            vtemp = vdist;
        }
    }
    if (temp != -1) return 232 + temp;

    return rindex * 36 + gindex * 6 + bindex + 1 + 15;
    
}

int* palette_create(int n_states) {
    int* palette = malloc(n_states * sizeof(int));
    int rl = 255, gl= 0, bl = 0; /* gradient left */
    int rr = 0, gr= 0, br = 0; /* gradient right */
    palette[0] = rgb2xterm(255, 255, 255) + 1; /* background/dead cell */

    if (n_states > 2) {
        for (int i = 1; i < n_states; ++i) {
            palette[i] = rgb2xterm(rl + (i - 1) * ((rr - rl)/(n_states -2)), gl + (i-1)*((gr - gl)/(n_states -2)), bl + (i-1)*((br - bl)/(n_states -2))) + 1;
        }
    } else {
        palette[1] = rgb2xterm(rr, gr, br) + 1;
    }

    return palette;
}

