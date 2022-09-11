#ifndef POINT_KNIFE_H_
#define POINT_KNIFE_H_

#include <SDL2/SDL.h>

#include "grid.h"

struct Point_Knife {
    float x, y;
    int w, h;
    SDL_Texture *texture;

    bool face_mode;

    int *highlights;
    int highlight_count;
};

void point_knife_init(gs);
void point_knife_deinit(gs);
void point_knife_tick();
void point_knife_draw();

#endif  /* POINT_KNIFE_H_ */
