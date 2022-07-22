#ifndef LEVEL_H_
#define LEVEL_H_

#define MAX_LEVELS 16

#include "grid.h"

struct Level {
    int index;
    char name[256];
    struct Cell *desired_grid; // What the inspiration is
    struct Cell *initial_grid; // Starting state of grid
    int w, h;
    int is_intro;
    int popup_time_current, popup_time_max;
};

extern struct Level levels[MAX_LEVELS];
extern int level_current, level_count;

void levels_setup();
void level_set_current(int lvl);
void levels_free();
void level_tick();
void level_draw();
void level_draw_intro();
void level_get_cells_from_image(char *path, struct Cell **out, int *out_w, int *out_h);

#endif
