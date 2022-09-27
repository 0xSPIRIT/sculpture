#ifndef GRABBER_H_
#define GRABBER_H_

#include <SDL2/SDL.h>

struct Grabber {
    float x, y;
    int w, h;
    SDL_Texture *texture;

    int cell_holding_id;
    int object_holding;
};

void grabber_init();
void grabber_tick();
void grabber_draw();

#endif  /* GRABBER_H_ */
