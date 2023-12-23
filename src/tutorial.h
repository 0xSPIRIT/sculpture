#ifdef ALASKA_RELEASE_MODE
  #define SHOW_TUTORIAL 1
#else
  #define SHOW_TUTORIAL 1
#endif

#define MAX_TUTORIAL_LINES 10

#define TUTORIAL_OVERLAY_STRING \
    "Tip\n" \
    "---\n\n" \
    "Click the OVERLAY BUTTON to show what Max\n"\
    "wants to create.\n\n" \
    "Select the POINTER TOOL and HOVER over\n"\
    "the overlay at any point to show the\n"\
    "material preferred for the sculpture, and\n" \
    "the material that is currently there.\n"

#define TUTORIAL_CHISEL_STRING  \
    "Tip\n" \
    "---\n\n" \
    "Use the chisels to create your sculpture!\n\n"\
    "Hold SHIFT or RIGHT CLICK and move your\n"\
    "mouse to rotate a chisel.\n\n" \

#define TUTORIAL_UNDO_STRING \
    "Tip\n" \
    "---\n\n" \
    "Z - Undo\n" \
    "R - Restart Level\n"

#define TUTORIAL_COMPLETE_LEVEL \
    "Tip\n" \
    "---\n\n" \
    "Most levels can be completed without\n"\
    "restrictions on completeness, except your\n"\
    "own discretion.\n"\

#define TUTORIAL_PLACER_STRING \
    "Tip\n" \
    "---\n\n" \
    "Use the PLACER to pick up the sand and place it into\n" \
    "your INVENTORY using the TAB key / button.\n\n"

#define TUTORIAL_PLACER_HARD \
    "Tip\n" \
    "---\n\n" \
    "A placer can't pick up stone material.\n"

#define TUTORIAL_PLACER_F_KEYS \
    "Tip\n" \
    "---\n\n" \
    "Use the F1-F5 keys to switch to a\n" \
    "different Placer slot.\n"

#define TUTORIAL_INVENTORY_STRING \
    "Tip\n" \
    "---\n\n" \
    "The slots in your inventory correspond to Placers 1-5.\n" \

#define TUTORIAL_FUEL_CONVERTER_STRING \
    "Tip\n" \
    "---\n\n" \
    "- The FUEL CONVERTER can be used to create fuel from\n" \
    "  existing material.\n" \
    "- You may use RIGHT CLICK to split an amount of\n"\
    "  material in two.\n" \

#define TUTORIAL_RECTANGLE_PLACE \
    "Tip\n" \
    "---\n\n" \
    "To place material down, SCROLL the mouse to change\n" \
    "the size of the rectangle, then CLICK.\n"

// A rectangle containing text, and an "Okay" button.
typedef struct Tutorial_Rect {
    bool active;

    f32 x, y; // 0 to 1
    SDL_Rect rect;

    Font *font;

    int margin;

    Button *ok_button;

    char str[8*64];
    char lines[MAX_TUTORIAL_LINES][64];

    int line_count;

    struct Tutorial_Rect *next;
} Tutorial_Rect;

static Tutorial_Rect* tutorial_rect(const char *str, Tutorial_Rect *next, bool button_active);
static void tutorial_rect_close(void*);
static void check_for_tutorial(bool info_button);