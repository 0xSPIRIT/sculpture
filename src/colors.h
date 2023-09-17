#define COLOR_MAX_DIALOGUE 0xa36a6a
#define INVENTORY_COLOR  0x1b1b1b
#define INVENTORY_COLOR2 0x222222

#define SLOT_COLOR 0x202020
#define SLOT_OUTLINE_COLOR 0xAAAAAA
#define SLOT_OUTLINE_SELECTED_COLOR 0xffe65a
#define SLOT_TEXT_COLOR SLOT_OUTLINE_COLOR

#define CONVERTER_NAME_COLOR SLOT_OUTLINE_COLOR
#define CONVERTER_LINE_COLOR 0x000000

#define Red(x) ((u8)((x >> 16) & 0xFF))
#define Green(x) ((u8)((x >> 8) & 0xFF))
#define Blue(x) ((u8)(x & 0xFF))

#define ColorFromInt(x) (SDL_Color){Red(x), Green(x), Blue(x), 255}

//                               RRGGBBAA
#define CONVERSION_PANEL_COLOR 0x000000c8

// When you want to change colors on the fly,
// uncomment the #define and comment the #undef.
// That makes it so that text updates every
// frame, which is really slow.

#define MODIFYING_COLORS
