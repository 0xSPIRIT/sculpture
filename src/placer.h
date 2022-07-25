#ifndef PLACER_H_
#define PLACER_H_

#include <SDL2/SDL.h>

#define PLACER_COUNT 3

enum {
    PLACER_SUCK_MODE,
    PLACER_PLACE_RECT_MODE,
    PLACER_PLACE_CIRCLE_MODE
};

struct Placer {
    int index;

    int state;

    SDL_Rect rect;
    
    int x, y;
    int w, h;
    SDL_Texture *texture;

    int radius;

    int object_index; // What object index to set the cells to.
    int did_click;    // Did we actually place down any material?

    int contains_type, contains_amount;
    int contains_current;

    int did_take_hard; // Did we pick up a hard cell last frame?
};

extern struct Placer *placers[PLACER_COUNT];
extern int current_placer;

void placer_init(int num);
void placer_deinit(int i);
void placer_tick(struct Placer *placer);
void placer_draw(struct Placer *placer);

#endif  /* PLACER_H_ */
