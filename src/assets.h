#ifdef ALASKA_RELEASE_MODE
#define DATA_DIR "data/"
#else
#define DATA_DIR "../data/"
#endif

#define ALASKA_PIXELFORMAT SDL_PIXELFORMAT_ABGR8888

#define ITEM_COUNT 32

// 768x768 is the benchmark resolution I used so when
// converting to new resolutions, I just put Scale(...)
// around all the values working with the old 768x768 res.
#define Scale(x) ((f64)gs->game_width * (f64)(x)/768.0)
#define NormX(x) ((f64)(x)/768.0)
#define NormY(x) (NormX(x))

typedef enum {
    RENDER_TARGET_MASTER, // The final full-screen resolution target. Bounds: { 0, 0, gs->game_width, gs->game_height }
    RENDER_TARGET_PIXELGRID, // The main pixel art render target Bounds: {0, 0, gs->gw*2, gs->gh*2} where the default bounds is {gs->gw/2, gs->gh/2, gs->gw*2, gs->gh*2}
    RENDER_TARGET_GUI_TOOLBAR, // The render target showing the tool buttons. Used only for the ending.
    RENDER_TARGET_CHISEL_BLOCKER,
    RENDER_TARGET_CHISEL, // Use the same render target for each chisel.
    RENDER_TARGET_HAMMER,
    RENDER_TARGET_HAMMER2,
    RENDER_TARGET_TOOLTIP,
    RENDER_TARGET_GRID, // Temp render target used by 3d.c
    RENDER_TARGET_EFFECTS,
    RENDER_TARGET_GLOW, // Pixel grid
    RENDER_TARGET_SHADOWS,
    RENDER_TARGET_CONVERSION_PANEL,
    RENDER_TARGET_GUI_CONVERSIONS,
    RENDER_TARGET_PREVIEW,
    RENDER_TARGET_OUTRO,
    RENDER_TARGET_COUNT
} RenderTargetType;

#pragma pack(push, 1)
typedef struct Audio {
    // Music
    Mix_Music *music_titlescreen;

    // Chunks
    Sound ambience_rain;
    Sound ambience_rain_reversed;
    Sound ambience1, ambience_wind;

    Sound place;
    Sound suck;

    Sound music0;
    Sound music1;
    Sound music2;
    Sound music3;

    Sound undo;

    Sound sprinkle, macabre;
    Sound accept;
    Sound pip;
    Sound medium_chisel[6];
    Sound ice_chisel[7];

    Sound glass_chisel[2];
    Sound small_glass_chisel[3];
    Sound small_chisel;

    //Sound woosh[4];
} Audio;
#pragma pack(pop)

enum {
    TEXTURE_LEVEL_BACKGROUNDS,
    TEXTURE_LEVEL_BACKGROUNDS_END = TEXTURE_LEVEL_BACKGROUNDS+LEVEL_COUNT,
    //TEXTURE_TEXT,
    //TEXTURE_TEXT_END = TEXTURE_TEXT + TEXT_INDEX_COUNT,
    TEXTURE_TAB,
    TEXTURE_SPEAKER,
    TEXTURE_PLANK,

    TEXTURE_CHISEL_SMALL,
    TEXTURE_CHISEL_SMALL_DIAGONAL,

    TEXTURE_CHISEL_MEDIUM,
    TEXTURE_CHISEL_MEDIUM_DIAGONAL,

    TEXTURE_CHISEL_LARGE,
    TEXTURE_CHISEL_LARGE_DIAGONAL,

    TEXTURE_CURSOR,

    TEXTURE_CONVERTER_BG,

    TEXTURE_BG_0,
    TEXTURE_BG_1,
    TEXTURE_BG_2,
    TEXTURE_BG_3,

    TEXTURE_CHISEL_HAMMER,
    TEXTURE_W_KEY,
    TEXTURE_A_KEY,
    TEXTURE_S_KEY,
    TEXTURE_D_KEY,
    TEXTURE_ITEMS,
    TEXTURE_ITEMS_END = TEXTURE_ITEMS+CELL_TYPE_COUNT,
    TEXTURE_TOOL_BUTTONS,
    TEXTURE_TOOL_BUTTONS_END = TEXTURE_TOOL_BUTTONS+TOOL_COUNT,
    TEXTURE_CONVERT_BUTTON,
    TEXTURE_ALTERNATE_BUTTON,
    TEXTURE_RECIPE_BOOK_BUTTON,
    TEXTURE_OK_BUTTON,
    TEXTURE_SLOT_NAMES,
    TEXTURE_SLOT_NAMES_END=TEXTURE_SLOT_NAMES+TOTAL_SLOT_COUNT,
    TEXTURE_CONVERTER_NAMES,
    TEXTURE_CONVERTER_NAMES_END=TEXTURE_CONVERTER_NAMES+CONVERTER_COUNT,
    TEXTURE_ITEM_NUMS,
    TEXTURE_ITEM_NUMS_END = TEXTURE_ITEM_NUMS+ITEM_COUNT,
    TEXTURE_NARRATOR,
    TEXTURE_NARRATOR_LINE,
    TEXTURE_NARRATOR_LINE_END=TEXTURE_NARRATOR_LINE + 10,
    TEXTURE_CONFIRM_BUTTON,
    TEXTURE_CONFIRM_X_BUTTON,
    TEXTURE_CANCEL_BUTTON,
    TEXTURE_BACKGROUND,
    TEXTURE_COUNT
};

#define GetTexture(x) (gs->textures.texs[x])

typedef struct Textures {
    Texture texs[TEXTURE_COUNT];
} Textures;


// A bit nasty...
#define SURFACE_COUNT (55+TOTAL_SLOT_COUNT+CONVERTER_COUNT)

typedef union Surfaces {
    SDL_Surface *surfaces[SURFACE_COUNT];

    struct {
        SDL_Surface *renderer_3d,
        *background,
        *a,
        *glass_surface,
        *wood_plank_surface,
        *marble_surface,
        *granite_surface,
        *diamond_surface,
        *ice_surface;

        SDL_Surface *item_nums[ITEM_COUNT];

        // Any temp surfaces you might need to draw text or w/e goes here!
        SDL_Surface *slot_names[TOTAL_SLOT_COUNT];
        SDL_Surface *converter_names[CONVERTER_COUNT];
        SDL_Surface *narrator_line[10];

        SDL_Surface *narrator;
    };
} Surfaces;

#define FONT_COUNT 10

typedef union Fonts {
    Font *fonts[FONT_COUNT];
    struct {
        Font
        *font,
        *font_times,
        *font_courier,
        *font_small,
        *font_bold_small,
        *font_title,
        *font_title_2,
        *font_titlescreen,
        *font_converter_gui,
        *font_times_small;
    };
} Fonts;

int font_sizes[FONT_COUNT] = {
    20,
    30,
    20,
    16,
    16,
    100,
    80,
    175,
    24,
    20,
};