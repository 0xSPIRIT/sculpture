// Perhaps you should store a list of differences in the
// grid between frames instead of the entire grid...
//
// Only change this if it actually becomes a problem, though.

#define MAX_PREVIEW_STATES 30*60 // 30 seconds
#define PREVIEW_GRID_SIZE 4096 // all previews are hardcoded 64*64

typedef struct {
    Uint8 grid[PREVIEW_GRID_SIZE];
    Uint8 tool; // Tool using
    Uint8 x, y; // Position of tool
    Uint16 data; // 0 to 360
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

static void preview_draw(int target, Preview *p, int dx, int dy, int scale, bool alpha_background);