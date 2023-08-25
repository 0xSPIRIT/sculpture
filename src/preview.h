// Perhaps you should store a list of differences in the
// grid between frames instead of the entire grid...
//
// Only change this if it actually becomes a problem, though.

#define MAX_PREVIEW_STATES 120*60 // 120 seconds

#define PREVIEW_GRID_W 64 // all previews are hardcoded 64*64
#define PREVIEW_GRID_SIZE PREVIEW_GRID_W*PREVIEW_GRID_W

typedef struct {
    u8 grid[PREVIEW_GRID_SIZE];
    u8 tool; // Tool using
    u8 x, y; // Position of tool
    u16 data; // 0 to 360 for chisel angle
} Preview_State;

typedef struct Preview {
    char name[64];
    Preview_State states[MAX_PREVIEW_STATES];
    int overlay[PREVIEW_GRID_SIZE];
    int length;
    int index;
    bool recording, play;
    SDL_Rect placer_rect;
} Preview;
