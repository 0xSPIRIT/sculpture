#ifndef PLACER_H_
#define PLACER_H_

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "converter.h"

#define PLACER_COUNT 3

#define MAX_PLACE_SOLID_TIME 45 // Frames until cut off for solids.

enum {
    PLACER_SUCK_MODE,
    PLACER_PLACE_RECT_MODE,
    PLACER_PLACE_CIRCLE_MODE
};

struct Placer {
    int index;

    int state;

    SDL_Rect rect;
    
    int x, y, px, py;
    int w, h;
    SDL_Texture *texture;

    int radius;

    int object_index; // What object index to set the cells to.
    int did_click;    // Did we actually place down any material?
    int did_set_new;

    int contains_type, contains_amount;

    int placing_solid_time;
};

extern struct Placer *placers[PLACER_COUNT];
extern int current_placer;

void placer_init(int num);
void placer_deinit(int i);
void placer_tick(struct Placer *placer);
void placer_draw(struct Placer *placer, bool full_size);

bool is_mouse_in_placer(struct Placer *placer);
/* SDL_RendererFlip get_placer_flip(struct Converter *converter, int placer_socket); */

#endif  /* PLACER_H_ */
