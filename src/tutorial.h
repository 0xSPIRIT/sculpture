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
    "  Hold SHIFT & Move Mouse - Rotate\n\n"\
    "Click the checkmark when you're satisfied with a level.\n"

#define TUTORIAL_OVERLAY_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Click the overlay button to show what you want to create.\n\n" \
    "With the POINTER tool selected, show the material type of\n" \
    "a cell by hovering over the overlay.\n"

#define TUTORIAL_COMPLETE_LEVEL \
    "Tutorial\n" \
    "--------\n\n" \
    "For this first level, you must complete it\n"\
    "perfectly in order to continue.\n\n"\
    "Subsequent levels can be completed\n"\
    "at any point, with no restrictions\n"\
    "except your own discretion.\n"\

#define TUTORIAL_COMPLETE_LEVEL_2 \
    "Tutorial\n" \
    "--------\n\n" \
    "For this level, you must complete it\n"\
    "perfectly in order to continue.\n"\

#define TUTORIAL_UNDO_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Use Z to undo if you make a mistake!\n"

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
    "- Use the PLACER to take up the sand and place it into\n" \
    "your INVENTORY [TAB].\n\n" \
    "- To place material, scroll the mouse, then click.\n\n"\
    "- You may use F1-F5 to switch to a different Placer slot.\n"

#define TUTORIAL_INVENTORY_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "The slots in your inventory correspond to Placers 1-5.\n"

#define TUTORIAL_FUEL_CONVERTER_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "The FUEL CONVERTER can be used to create fuel from\n" \
    "existing material.\n" \
    "You may use RIGHT CLICK to split an amount of material in two.\n" \

#define TUTORIAL_TEXT_FILE_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "To get a full list of converter, press I to toggle the\n" \
"conversion recipe book.\n"\

#define TUTORIAL_CAREFUL_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Ensure you be careful with the amount of material you're\n" \
    "converting; You might end up with not enough of a certain\n" \
    "type!\n"

#define TUTORIAL_RECTANGLE_PLACE \
    "Tutorial\n" \
    "--------\n\n" \
    "To place material down, click and drag a rectangle.\n"

#define TUTORIAL_CHISEL_INVENTORY_STRING \
    "Tutorial\n" \
    "--------\n\n" \
    "Upon chiseling, the broken material gets placed into\n" \
    "your inventory.\n"

// A rectangle containing text, and an "Okay" button.
typedef struct Tutorial_Rect {
    bool active;

    TTF_Font *font;
    SDL_Texture *textures[MAX_TUTORIAL_LINES];
    SDL_Rect texture_rects[MAX_TUTORIAL_LINES];

    int margin;

    char str[8*64];
    char lines[MAX_TUTORIAL_LINES][64];

    int line_count;

    f64 x, y; // 0 to 1
    SDL_Rect rect;

    Button *ok_button;

    struct Tutorial_Rect *next;
} Tutorial_Rect;

static Tutorial_Rect* tutorial_rect(const char *str,
                             f64 x,
                             f64 y,
                             Tutorial_Rect *next);
static void tutorial_rect_close(void*);
