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
    
    f32 x, y;
    int w, h;

    bool is_changing_angle;
    int swing_direction;
    f32 angle, prev_angle;

    SDL_Texture *texture;

    Uint32 *pixels;

    Uint32 blobs_touched[MAX_BLOBS_TOUCHED];
    int blobs_touched_count;
};
