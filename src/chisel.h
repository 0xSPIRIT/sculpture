#define CHISEL_TIME 0 /* The amount of frames the chisel moves. 0 = instant */
#define CHISEL_COOLDOWN 3
#define CHISEL_FORGIVING_AIM false // Do we check for surrounding cells?
#define DRAW_CHISEL_HIGHLIGHTS true

enum Hammer_State {
    HAMMER_STATE_IDLE,
    HAMMER_STATE_WINDUP,
    HAMMER_STATE_SWING,
    HAMMER_STATE_AFTERSWING // Unused for chisel hammer
};

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
    int click_cd;
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
    bool did_chisel_this_frame;
};
