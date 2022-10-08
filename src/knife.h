#ifndef KNIFE_H_
#define KNIFE_H_

#include "grid.h"

struct Knife {
    f32 x, y;
    int w, h;
    f32 angle;
    SDL_Texture *texture;
    Uint32 *pixels;
};

void knife_init();
void knife_tick();
void knife_update_texture();
void knife_draw();

#endif  /* KNIFE_H_ */
