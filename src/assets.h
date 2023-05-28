#ifdef ALASKA_RELEASE_MODE
#define RES_DIR "res/"
#else
#define RES_DIR "../res/"
#endif

#define ALASKA_PIXELFORMAT SDL_PIXELFORMAT_ABGR8888

#define ITEM_COUNT 32

#define Volume(x) ((int) ((x) * MIX_MAX_VOLUME))

enum {
    AUDIO_CHANNEL_CHISEL,
    AUDIO_CHANNEL_NARRATOR,
    AUDIO_CHANNEL_GUI
};

// 768x768 is the benchmark resolution I used so when
// converting to new resolutions, I just put Scale(...)
// around all the values working with the old 768x768 res.
#define Scale(x) ((f64)gs->window_width * (f64)(x)/768.0)
#define NormX(x) ((f64)(x)/768.0)
#define NormY(x) (NormX(x))

// Not the best method, but it's too much work for too little gain
// when adding a new text index. Just add another guy on here
// and use him in the draw call.
enum {
    TEXT_OUTRO_LEVEL_NAME,
    TEXT_OUTRO_INTENDED,
    TEXT_OUTRO_RESULT,
    TEXT_OUTRO_NEXT_LEVEL,
    TEXT_OUTRO_PREV_LEVEL,
    TEXT_CONVERTER_CHART_START,
    TEXT_CONVERTER_REQUIRED_START,
    TEXT_TITLESCREEN,
    TEXT_NOT_GOOD_ENOUGH,
    TEXT_CONFIRM,
    TEXT_INDEX_COUNT = 128
};

// Index into textures.render_targets[]
enum {
    RENDER_TARGET_MASTER, // The final full-screen resolution target.
    RENDER_TARGET_GLOBAL, // The main pixel art render target
    RENDER_TARGET_GUI_TOOLBAR, // The render target showing the tool buttons
    RENDER_TARGET_CHISEL_BLOCKER,
    RENDER_TARGET_CHISEL, // Use the same render target for each chisel.
    RENDER_TARGET_HAMMER,
    RENDER_TARGET_HAMMER2,
    RENDER_TARGET_GRID,
    RENDER_TARGET_3D,
    RENDER_TARGET_CONVERSION_PANEL,
    RENDER_TARGET_PREVIEW,
    RENDER_TARGET_OUTRO,
    RENDER_TARGET_COUNT 
};

typedef struct Audio {
    Mix_Music *music_titlescreen;
    Mix_Music *music_creation;
    
    Mix_Chunk *stinger_a, *stinger_b;
    
    Mix_Chunk *accept;
    
    Mix_Chunk *pip;
    
    Mix_Chunk *medium_chisel[6];
    Mix_Chunk *small_chisel, *large_chisel;
} Audio;

enum {
    TEXTURE_RENDER_TARGETS = 0,
    TEXTURE_RENDER_TARGET_END = LEVEL_COUNT*RENDER_TARGET_COUNT,
    TEXTURE_LEVEL_BACKGROUNDS,
    TEXTURE_LEVEL_BACKGROUNDS_END = TEXTURE_LEVEL_BACKGROUNDS+LEVEL_COUNT,
    TEXTURE_TEXT,
    TEXTURE_TEXT_END = TEXTURE_TEXT + TEXT_INDEX_COUNT,
    TEXTURE_TAB,
    TEXTURE_DELETER,
    TEXTURE_PLACER,
    TEXTURE_KNIFE,
    TEXTURE_POPUP,
    TEXTURE_BLOB_HAMMER,
    TEXTURE_CONVERTER_ARROW,
    TEXTURE_CHISEL,
    TEXTURE_CHISEL_END = TEXTURE_CHISEL+3,
    TEXTURE_CHISEL_HAMMER,
    TEXTURE_ITEMS,
    TEXTURE_ITEMS_END = TEXTURE_ITEMS+CELL_TYPE_COUNT,
    TEXTURE_TOOL_BUTTONS,
    TEXTURE_TOOL_BUTTONS_END = TEXTURE_TOOL_BUTTONS+TOOL_COUNT,
    TEXTURE_CONVERT_BUTTON,
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
    TEXTURE_TEXT_ARROW,
    TEXTURE_CONFIRM_BUTTON,
    TEXTURE_CANCEL_BUTTON,
    TEXTURE_BACKGROUND,
    TEXTURE_COUNT
};

#define Texture(x) (gs->textures.texs[x])

typedef struct Textures {
    SDL_Texture *texs[TEXTURE_COUNT];
} Textures;

typedef struct Surfaces {
    SDL_Surface *renderer_3d;
    
    SDL_Surface *text[TEXT_INDEX_COUNT];
    
    SDL_Surface *background;
    
    SDL_Surface *a;
    SDL_Surface *bark_surface,
    *glass_surface,
    *wood_plank_surface,
    *marble_surface,
    *granite_surface,
    *diamond_surface,
    *ice_surface,
    *grass_surface,
    *triangle_blob_surface;
    
    SDL_Surface *item_nums[ITEM_COUNT];
    
    // Any temp surfaces you might need to draw text or w/e goes here!
    SDL_Surface *slot_names[TOTAL_SLOT_COUNT];
    SDL_Surface *converter_names[CONVERTER_COUNT];
    SDL_Surface *narrator_line[10];
    
    SDL_Surface *narrator;
} Surfaces;

typedef struct Fonts {
    TTF_Font *font,
    *font_times,
    *font_consolas,
    *font_courier,
    *font_small,
    *font_bold_small,
    *font_title,
    *font_title_2,
    *font_titlescreen;
} Fonts;

int font_sizes[32] = {
    20,
    30,
    24,
    20,
    16,
    16,
    100,
    80,
    135
};

