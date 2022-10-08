#ifndef CHISEL_H_
#define CHISEL_H_

#define CHISEL_TIME 0 /* The amount of frames the chisel moves. 0 = instant */
#define CHISEL_COOLDOWN 3
#define CHISEL_FORGIVING_AIM false // Do we check for surrounding cells?
#define DRAW_CHISEL_HIGHLIGHTS false

struct Chisel_Hammer {
	int state;
    f32 x, y;
	f32 dist, normal_dist;
    int time;
    f32 angle;
    SDL_Texture *texture;
    int w, h;
};

struct Chisel {
    int size;
    f32 x, y;
    bool is_changing_angle;
    int click_cooldown;
	bool did_remove;
    bool face_mode;
    struct Line *line;
    f32 spd;
    f32 angle;
    Uint32 *pixels;
    SDL_Texture *texture, *outside_texture, *face_texture;
    int w, h;
    int face_w, face_h;
    int outside_w, outside_h;
    int *highlights; // List of indices into grid for highlighting the blobs.
    int highlight_count;
};

void chisel_hammer_init();
void chisel_hammer_tick();
void chisel_hammer_draw();

void chisel_init(struct Chisel *type);
void chisel_tick();
void chisel_draw();
Uint32 chisel_goto_blob(bool remove, f32 ux, f32 uy, f32 len);

void chisel_update_texture();

#endif /* CHISEL_H_ */
