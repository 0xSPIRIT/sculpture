#ifndef KNIFE_H_
#define KNIFE_H_

#include <SDL2/SDL.h>

#include "grid.h"

struct Knife {
    float x, y;
    int w, h;
    float angle;
    SDL_Texture *texture, *render_texture;
    Uint32 *pixels;
};

void knife_init();
void knife_deinit();
void knife_tick();
void knife_update_texture();
void knife_draw();

#endif  /* KNIFE_H_ */
