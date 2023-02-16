#define MAX_PREVIEW_STATES 12*60 // 12 seconds' worth

struct Preview_State {
    Uint8 *grid;
    Uint8 chisel_x, chisel_y; // lower size to reduce memory usage
    Uint16 angle; // 0 to 360
};

struct Preview {
    struct Preview_State states[MAX_PREVIEW_STATES];
    int length;
};

// 10 seconds of footage
// @ 60 fps
// 1 state per frame
// 
// 1 state = 4096 * 1 + 2 * 1 + 2 = 4100 bytes per state
//
// 10*60 = 600 frames
//
// 600 states = 4100 * 600 = 4100 * 600 = 
//