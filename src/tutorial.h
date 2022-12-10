// Tutorial Strings

#define MAX_TUTORIAL_LINES 10

#define TUTORIAL_CHISEL_STRING  \
    "Tutorial\n" \
    "========\n\n" \
    "Use the chisels to create your sculpture!\n" \
    "Press F when you're satisfied with a level."

#define TUTORIAL_OVERLAY_STRING \
    "Tutorial\n" \
    "========\n\n" \
    "Click the overlay button to show what you want to create.\n\n" \
    "With the POINTER tool selected, show the material type of\n" \
    "a cell by hovering over the overlay."

#define TUTORIAL_UNDO_STRING \
    "Tutorial\n" \
    "========\n\n" \
    "Use Ctrl+Z to undo if you make a mistake!"

#define TUTORIAL_CHISEL_ROTATE_STRING \
    "Tutorial\n" \
    "========\n\n" \
    "Hold SHIFT and move the cursor to rotate the chisel."

#define TUTORIAL_PRESSURE_STRING \
    "Tutorial\n" \
    "========\n\n" \
    "Sometimes a chisel is unable to destroy a cell\n" \
    "when it's too far inside the sculpture.\n" \
    "In these cases, you can use the DELETER."

#define TUTORIAL_PLACER_STRING \
    "Tutorial\n" \
    "========\n\n" \
    "Use the PLACER to take up existing non-solid material,\n" \
    "and place it into your INVENTORY [TAB].\n" \
    "You may use F1-F5 to switch to a different Placer slot."

#define TUTORIAL_INVENTORY_STRING \
    "Tutorial\n" \
    "========\n\n" \
    "The slots in your inventory correspond to Placers 1-5."

#define TUTORIAL_FUEL_CONVERTER_STRING \
    "Tutorial\n" \
    "========\n\n" \
    "The FUEL CONVERTER can be used to create fuel from\n" \
    "existing material.\n" \
    "You may use RIGHT CLICK to split an amount of material in two." \

#define TUTORIAL_TEXT_FILE_STRING \
    "Tutorial\n" \
    "========\n\n" \
    "To get a full list of conversions, visit the text file\n" \
    "\"layout_converter.txt\" in the game's top directory."

#define TUTORIAL_CAREFUL_STRING \
    "Tutorial\n" \
    "========\n\n" \
    "Ensure you be careful with the amount of material you're\n" \
    "converting; You might end up with not enough of a certain\n" \
    "type!"

// A rectangle containing text, and an "Okay" button.
struct Tutorial_Rect {
    bool active;
    
    TTF_Font *font;
    SDL_Texture *textures[MAX_TUTORIAL_LINES];
    
    int margin;
    
    char str[8*64];
    char lines[MAX_TUTORIAL_LINES][64];
    
    int line_count;
    
    SDL_Rect rect;
    
    struct Button *ok_button;
    
    struct Tutorial_Rect *next;
};

struct Tutorial_Rect* tutorial_rect(const char *str,
                                    int x,
                                    int y,
                                    struct Tutorial_Rect *next);
void tutorial_rect_close(void*);
