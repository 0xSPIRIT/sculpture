#define INVENTORY_COLOR  0x2b2b2b
#define INVENTORY_COLOR2 0x000000

#define SLOT_COLOR 0x373737
#define SLOT_OUTLINE_COLOR 0xAAAAAA
#define SLOT_TEXT_COLOR SLOT_OUTLINE_COLOR

#define CONVERTER_NAME_COLOR SLOT_OUTLINE_COLOR
#define CONVERTER_LINE_COLOR 0x888888

#define Red(x) ((Uint8)((x >> 16) & 0xFF))
#define Green(x) ((Uint8)((x >> 8) & 0xFF))
#define Blue(x) ((Uint8)(x & 0xFF))

#define ColorFromInt(x) (SDL_Color){Red(x), Green(x), Blue(x), 255}

// When you want to change colors on the fly,
// uncomment the #define and comment the #undef.
// That makes it so that text updates every
// frame, which is really slow.

//#define MODIFYING_COLORS