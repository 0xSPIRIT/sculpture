#ifndef DELETER_H_
#define DELETER_H_

#include <SDL2/SDL.h>

#include "grid.h"
#include "typedefs.h"

#define POINT_COUNT 50

struct Deleter {
    f32 x, y;
    int w, h;
    SDL_Texture *texture;

    Uint32 *pixels;

    bool active, was_active;

    int *highlights;
    int highlight_count;

    SDL_Point points[POINT_COUNT];
    int point_count;

    int cooldown;
};

void deleter_init();
void deleter_tick();
void deleter_draw();
void deleter_stop(bool cancel);
void deleter_delete();

#endif  /* DELETER_H_ */