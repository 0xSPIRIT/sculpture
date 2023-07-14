#define USE_PRESSURE false
#define HIGHLIGHT_MAX 64
#define CHISEL_FLASHING false

typedef enum Chisel_Size {
    CHISEL_SMALL = 0,
    CHISEL_MEDIUM,
    CHISEL_LARGE,
} Chisel_Size;

typedef enum Chisel_State {
    CHISEL_STATE_IDLE = 0,
    CHISEL_STATE_ROTATING,
    CHISEL_STATE_CHISELING
} Chisel_State;

typedef struct Chisel {
    Chisel_State state;
    Chisel_Size size;
    int x, y;
    
    int temp_idx;

    int click_delay;
    bool repeated;
    
    int lookahead;
    bool is_calculating_highlight;

    int highlights[HIGHLIGHT_MAX]; // Stores indices of each highlight.
    int highlight_count;

    Uint8 *mask;

    f64 angle;
    Texture *texture;
    
    f64 rotating_flash;

    int did_chisel_this_frame;
    int num_times_chiseled;
} Chisel;

// Called in grid_draw.
static void chisel_draw_highlights(int target, int *highlights, int count, int xoff, int yoff);
static void chisel_get_adjusted_positions(int angle, int size, int *x, int *y);

static bool is_tool_chisel(void);