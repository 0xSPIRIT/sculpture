#define HIGHLIGHT_MAX 64

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

typedef struct Chisel {
    enum Chisel_State state;
    enum Chisel_Size size;
    int x, y, w, h;
    
    int lookahead;
    bool is_calculating_highlight;
    
    int highlights[HIGHLIGHT_MAX]; // Stores indices of each highlight.
    int highlight_count;
    
    f64 angle;
    SDL_Texture *texture;
    
    int did_chisel_this_frame;
    int num_times_chiseled;
} Chisel;