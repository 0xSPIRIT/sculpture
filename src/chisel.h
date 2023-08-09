#define USE_PRESSURE false
#define HIGHLIGHT_MAX 64
#define CHISEL_FLASHING false

typedef enum ChiselSize {
    CHISEL_SMALL = 0,
    CHISEL_MEDIUM,
    CHISEL_LARGE,
} ChiselSize;

typedef enum ChiselState {
    CHISEL_STATE_IDLE = 0,
    CHISEL_STATE_ROTATING,
    CHISEL_STATE_CHISELING
} ChiselState;

typedef struct Chisel_Texture {
    Texture *straight;
    Texture *diagonal;
} Chisel_Texture;

typedef struct Chisel {
    ChiselState state;
    ChiselSize size;
    int x, y;
    
    int aa;

    f64 draw_x, draw_y;

    int temp_idx;

    int click_delay;
    bool repeated;

    int lookahead;
    bool is_calculating_highlight;

    int highlights[HIGHLIGHT_MAX]; // Stores indices of each highlight.
    int highlight_count;

    Uint8 *mask;

    f64 angle, prev_angle, draw_angle;
    Chisel_Texture textures;
    Texture *texture;

    f64 rotating_flash;

    int did_chisel_this_frame;
    int num_times_chiseled;
} Chisel;

// Called in grid_draw.

static Chisel_Texture get_chisel_texture(ChiselSize size);

static void chisel_draw_highlights(int target, int *highlights, int count, int xoff, int yoff);
static void chisel_get_adjusted_positions(int texture_height, bool diagonal, f32 *dx, f32 *dy);

static bool is_tool_chisel(void);