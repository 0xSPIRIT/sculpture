enum Chisel_Size {
    CHISEL_SMALL = 0,
    CHISEL_MEDIUM,
    CHISEL_LARGE,
};

enum Chisel_State {
    CHISEL_STATE_IDLE = 0,
    CHISEL_STATE_ROTATING,
    CHISEL_STATE_CHISELING
};

struct Chisel {
    enum Chisel_State state;
    enum Chisel_Size size;
    int x, y, w, h;
    f64 angle;
    SDL_Texture *texture;
    
    // Legacy members
    int did_chisel_this_frame;
    int num_times_chiseled;
};

void chisel_get_adjusted_positions(int angle, int size, int *x, int *y);
