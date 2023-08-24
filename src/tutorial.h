#ifdef ALASKA_RELEASE_MODE
  #define SHOW_TUTORIAL 1
#else
  #define SHOW_TUTORIAL 0
#endif

#define MAX_TUTORIAL_LINES 10

#define TUTORIAL_OVERLAY_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Click the overlay button to show what Max\n"\
    "wants to create.\n\n" \
    "With the POINTER tool selected, you can\n"\
    "display the desired material of any cell by\n" \
    "hovering your cursor over the overlay.\n"

#define TUTORIAL_CHISEL_STRING  \
    "Tutorial\n" \
    "--------\n\n" \
    "Use the chisels to create your sculpture!\n\n"\
    "Hold SHIFT and move your mouse to rotate\n"\
    "chisel.\n\n" \
    "Click the checkmark when you're satisfied\n"\
    "with a level.\n"

#define TUTORIAL_UNDO_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Z - Undo\n" \
    "R - Restart Level\n"

#define TUTORIAL_COMPLETE_LEVEL \
    "Tutorial\n" \
    "--------\n\n" \
    "Most levels can be completed without\n"\
    "restrictions on completeness, except your\n"\
    "own discretion.\n"\

#define TUTORIAL_CHISEL_ROTATE_STRING \
    "Tutorial\n" \
    "--------\n\n" \

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

    f32 x, y; // 0 to 1
    SDL_Rect rect;

    Font *font;
    //TTF_Font *font;
    //SDL_Texture *textures[MAX_TUTORIAL_LINES];
    //SDL_Rect texture_rects[MAX_TUTORIAL_LINES];

    int margin;

    Button *ok_button;
    
    char str[8*64];
    char lines[MAX_TUTORIAL_LINES][64];

    int line_count;

    struct Tutorial_Rect *next;
} Tutorial_Rect;

static Tutorial_Rect* tutorial_rect(const char *str, Tutorial_Rect *next);
static void tutorial_rect_close(void*);
