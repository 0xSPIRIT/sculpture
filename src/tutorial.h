#ifdef ALASKA_RELEASE_MODE
  #define SHOW_TUTORIAL 1
#else
  #define SHOW_TUTORIAL 1
#endif

#define MAX_TUTORIAL_LINES 10

#define TUTORIAL_CHISEL_STRING  \
    "Tutorial\n" \
    "--------\n\n" \
    "Use the chisels to create your sculpture!\n"\
    "  Left Click - Chisel\n"\
    "  Hold SHIFT & Move Mouse - Rotate Chisel\n\n"\
    "Click the checkmark when you're satisfied\n"\
    "with a level.\n"

#define TUTORIAL_OVERLAY_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Click the overlay button to show what Max\n"\
    "wants to create.\n\n" \
    "With the POINTER tool selected, you can\n"\
    "display the material of the cell by\n" \
    "hovering over the overlay.\n"

#define TUTORIAL_COMPLETE_LEVEL \
    "Tutorial\n" \
    "--------\n\n" \
    "For this first level, you must complete it\n"\
    "perfectly in order to continue.\n\n"\
    "However, most subsequent levels can be completed\n"\
    "without restrictions, except your own discretion.\n"\

#define TUTORIAL_COMPLETE_LEVEL_2 \
    "Tutorial\n" \
    "--------\n\n" \
    "For this level, you must complete it\n"\
    "perfectly in order to continue.\n"\

#define TUTORIAL_UNDO_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Press Z to undo if you make a mistake.\n" \
    "Press R to restart a level, losing all progress.\n"

#define TUTORIAL_CHISEL_ROTATE_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Hold SHIFT and move the cursor to rotate the chisel.\n"

#define TUTORIAL_PRESSURE_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Sometimes a chisel is unable to destroy a cell when\n" \
    "it's too far inside the sculpture. In these cases\n" \
    "you can use the DELETER to clear up material before\n" \
    "using the chisel again.\n"

#define TUTORIAL_PLACER_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Use the PLACER to take up the sand and place it into\n" \
    "your INVENTORY [TAB].\n\n" \
    "To place material, scroll the mouse for resizing,\n" \
    "then click.\n\n"\

#define TUTORIAL_PLACER_HARD \
    "Tutorial\n" \
    "--------\n\n" \
    "A placer can't take up stone material.\n"

#define TUTORIAL_PLACER_F_KEYS \
    "Tutorial\n" \
    "--------\n\n" \
    "- Use the F1-F5 keys to switch to a\n" \
    "  different Placer slot.\n"

#define TUTORIAL_INVENTORY_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "The slots in your inventory correspond to Placers 1-5.\n"

#define TUTORIAL_FUEL_CONVERTER_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "- The FUEL CONVERTER can be used to create fuel from\n" \
    "  existing material.\n" \
    "- You may use RIGHT CLICK to split an amount of\n"\
    "  material in two.\n" \

#define TUTORIAL_RECTANGLE_PLACE \
    "Tutorial\n" \
    "--------\n\n" \
    "To place material down, scroll the mouse, then click.\n"

// A rectangle containing text, and an "Okay" button.
typedef struct Tutorial_Rect {
    bool active;

    Font *font;
    //TTF_Font *font;
    //SDL_Texture *textures[MAX_TUTORIAL_LINES];
    //SDL_Rect texture_rects[MAX_TUTORIAL_LINES];

    int margin;

    char str[8*64];
    char lines[MAX_TUTORIAL_LINES][64];

    int line_count;

    f64 x, y; // 0 to 1
    SDL_Rect rect;

    Button *ok_button;

    struct Tutorial_Rect *next;
} Tutorial_Rect;

static Tutorial_Rect* tutorial_rect(const char *str, Tutorial_Rect *next);
static void tutorial_rect_close(void*);
