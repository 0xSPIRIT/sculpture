#define MAX_TOOLTIP_LEN 128
#define MAX_TOOLTIP_LINE_LEN 128
#define CLAMP_TOOLTIP 1

typedef enum {
    TOOLTIP_TYPE_OFF,    // Off state
    TOOLTIP_TYPE_BUTTON, // From GUI Buttons
    TOOLTIP_TYPE_ITEM,   // From hovering over items
    TOOLTIP_TYPE_PLACER  // From the placer
} Tooltip_Type;

typedef struct Tooltip {
    Tooltip_Type type;

    bool set_this_frame;

    f32 x, y;
    char str[MAX_TOOLTIP_LEN][MAX_TOOLTIP_LINE_LEN];
    int w, h;

    f32 alpha, to_alpha;
    int alpha_hang_timer;

    Preview *preview;
} Tooltip;

static void tooltip_reset(Tooltip *tooltip);
static void tooltip_set_position_to_cursor(Tooltip *tooltip, int type);
static void tooltip_set_position(Tooltip *tooltip, int x, int y, int type);
static void tooltip_draw_box(int target, Tooltip *tooltip, int w, int h);
static void tooltip_get_string(int type, int amt, char *out_str);
static void tooltip_draw(int target, Tooltip *tooltip);
static void tooltip_draw_deprectated(int target, Tooltip *tooltip);

