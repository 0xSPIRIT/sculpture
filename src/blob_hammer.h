#ifndef HAMMER_H_
#define HAMMER_H_

#include <SDL2/SDL.h>
#include <stdbool.h>

#include "grid.h"

#define MAX_BLOBS_TOUCHED 32

// Used in chisel.c as well for the chisel's hammer state.
enum Hammer_State {
    HAMMER_STATE_IDLE,
    HAMMER_STATE_WINDUP,
    HAMMER_STATE_SWING,
    HAMMER_STATE_AFTERSWING // Unused for chisel hammer
};

struct Blob_Hammer {
    int state;
    int timer;
    
    float x, y;
    int w, h;

    bool is_changing_angle;
    int swing_direction;
    float angle, prev_angle;

    SDL_Texture *texture;

    SDL_Texture *render_texture;
    Uint32 *pixels;

    Uint32 blobs_touched[MAX_BLOBS_TOUCHED];
    int blobs_touched_count;
};

void blob_hammer_init();
void blob_hammer_deinit();
void blob_hammer_tick();
void blob_hammer_update_texture();
void blob_hammer_draw();

#endif  /* HAMMER_H_ */
