#define MAX_PREVIEW_STATES 30*60 // 30 seconds
#define PREVIEW_GRID_SIZE 4096 // 64*64

struct Preview_State {
    Uint8 grid[PREVIEW_GRID_SIZE];
    Uint8 tool; // Tool using
    Uint8 x, y; // Position of tool
    Uint16 angle; // 0 to 360
};

struct Preview {
    char name[64];
    struct Preview_State states[MAX_PREVIEW_STATES];
    int overlay[PREVIEW_GRID_SIZE];
    int length;
    int index;
    bool recording, play;
    SDL_Rect placer_rect;
};

void preview_draw(struct Preview *p, int dx, int dy, int scale);