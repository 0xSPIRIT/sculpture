#ifndef CHISEL_H_
#define CHISEL_H_

#include <SDL2/SDL.h>
#include "grid.h"

#define CHISEL_TIME 0           /* The amount of frames the chisel moves. 0 = instant */
#define CHISEL_COOLDOWN 3
#define CHISEL_FORGIVING_AIM 0

enum {
	HAMMER_STATE_IDLE,
	HAMMER_STATE_WINDUP,
	HAMMER_STATE_SWING
};

struct Hammer {
	int state;
    float x, y;
	float dist, normal_dist;
    int time;
    float angle;
    SDL_Texture *texture;
    int w, h;
};

struct Chisel {
    int size;
    float x, y;
    int changing_angle;
    int click_cooldown;
	int did_remove;
    int face_mode;
    struct Line *line;
    float spd;
    float angle;
    Uint32 *pixels;
    SDL_Texture *texture, *outside_texture, *face_texture, *render_texture;
    int w, h;
    int face_w, face_h;
    int outside_w, outside_h;
    int *highlights; // List of indices into grid for highlighting the blobs.
    int highlight_count;
};

extern struct Chisel *chisel;
extern struct Chisel chisel_small, chisel_medium, chisel_large;
extern struct Hammer hammer;

void hammer_init();
void hammer_deinit();
void hammer_tick();
void hammer_draw();

void chisel_init(struct Chisel *type);
void chisel_deinit();
void chisel_tick();
void chisel_draw();
int chisel_goto_blob(int remove, float ux, float uy, float len);

void chisel_update_texture();

#endif
