#define MAX_PREVIEW_STATES 30*60 // 30 seconds
#define PREVIEW_GRID_SIZE 4096 // all previews are hardcoded 64*64

typedef struct Preview_State {
    Uint8 grid[PREVIEW_GRID_SIZE];
    Uint8 tool; // Tool using
    Uint8 x, y; // Position of tool
    Uint16 angle; // 0 to 360
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

static void preview_draw(int target, Preview *p, int dx, int dy, int scale);
