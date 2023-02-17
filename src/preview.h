#define MAX_PREVIEW_STATES 12*60 // 12 seconds' worth

struct Preview_State {
    Uint8 grid[4096];
    Uint8 tool; // Tool using
    Uint8 x, y; // Position of tool
    Uint16 angle; // 0 to 360
};

struct Preview {
    char name[64];
    struct Preview_State states[MAX_PREVIEW_STATES];
    int length;
    int index;
    bool recording, play;
};

void preview_draw(struct Preview *p, int dx, int dy, int scale);