#ifndef CHISEL_BLOCKER_H
#define CHISEL_BLOCKER_H

#include <SDL2/SDL.h>

enum {
    CHISEL_BLOCKER_OFF,
    CHISEL_BLOCKER_LINE_MODE,
    CHISEL_BLOCKER_CURVE_MODE,
};

struct Line {
    int x1, y1, x2, y2;
};

struct ChiselBlocker {
    int state;
    
    SDL_Point control_points[4]; // The bezier or line control points.
    int point_count;

    struct Line lines[2]; // The lines that extend at the ends for the bezier.
    int current_point; // -1 for none
    int active; // Not to be confused with chisel_blocker_mode.

    Uint32 *pixels;
    SDL_Texture *render_tex;

    int side;
};

extern struct ChiselBlocker chisel_blocker;
extern int chisel_blocker_mode; // Edit mode for the points.

void chisel_blocker_init();
void chisel_blocker_tick();
void chisel_blocker_draw();

#endif // CHISEL_BLOCKER_H
