#ifndef KNIFE_H_
#define KNIFE_H_

#include <SDL2/SDL.h>

#include "grid.h"
#include "typedefs.h"

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
