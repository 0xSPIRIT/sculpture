#ifndef GRABBER_H_
#define GRABBER_H_

#include <SDL2/SDL.h>

struct Grabber {
    float x, y;
    int w, h;

    int object_holding;
};

void grabber_init();
void grabber_tick();

#endif  /* GRABBER_H_ */
