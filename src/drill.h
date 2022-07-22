#ifndef DRILL_H_
#define DRILL_H_

#include <SDL2/SDL.h>

#include "grid.h"

struct Drill {
    float x, y;
    int w, h;
    float angle;
    SDL_Texture *texture;
};

extern struct Drill drill;

void drill_init();
void drill_deinit();
void drill_tick();
void drill_draw();

#endif  /* DRILL_H_ */
