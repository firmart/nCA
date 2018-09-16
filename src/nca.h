#ifndef NCA_H
#define NCA_H

#include <ncurses.h>
#include <panel.h>

#include "common.h"


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

#endif /* ifndef NCA_H

 */
