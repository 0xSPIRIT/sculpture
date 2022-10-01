#ifndef POINT_KNIFE_H_
#define POINT_KNIFE_H_

#include <SDL2/SDL.h>

#include "grid.h"
#include "typedefs.h"

struct Point_Knife {
    f32 x, y;
    int w, h;
    SDL_Texture *texture;

    bool face_mode;

    int *highlights;
    int highlight_count;

    int cooldown;
};

void point_knife_init();
void point_knife_tick();
void point_knife_draw();

#endif  /* POINT_KNIFE_H_ */
