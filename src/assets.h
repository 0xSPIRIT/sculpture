#ifdef ALASKA_RELEASE_MODE
  #define RES_DIR "res/"
#else
  #define RES_DIR "../res/"
#endif

#define ALASKA_PIXELFORMAT SDL_PIXELFORMAT_ABGR8888

#define ITEM_COUNT 32

enum {
    TEXT_OUTRO_LEVEL_NAME,
    TEXT_OUTRO_INTENDED,
    TEXT_OUTRO_RESULT,
    TEXT_OUTRO_NEXT_LEVEL,
    TEXT_OUTRO_PREV_LEVEL,
    TEXT_CONVERTER_CHART_START,
    TEXT_CONVERTER_REQUIRED_START,
    TEXT_INDEX_COUNT = 128
};

// Index into textures.render_targets[]
enum {
    RENDER_TARGET_GLOBAL, // The main render target
    RENDER_TARGET_GUI_TOOLBAR, // The render target showing the tool buttons
    RENDER_TARGET_CHISEL_BLOCKER,
    RENDER_TARGET_CHISEL, // Use the same render target for each chisel.
    RENDER_TARGET_GRID,
    RENDER_TARGET_3D,
    RENDER_TARGET_CONVERSION_PANEL,
    RENDER_TARGET_COUNT 
};

struct Audio {
    Mix_Music *music;
    Mix_Chunk *medium_chisel[6];
    Mix_Chunk *small_chisel, *large_chisel;
};

// Only contains textures!
struct Textures {
    // List of render targets for each level
    // Index into this using enum.
    SDL_Texture *render_targets[LEVEL_COUNT][RENDER_TARGET_COUNT];
    
    SDL_Texture *text[TEXT_INDEX_COUNT];
    
    SDL_Texture *tab;
    
    SDL_Texture *deleter,
        *placer,
        *knife,
        *popup,
        *blob_hammer,
        *converter_arrow;

    SDL_Texture *chisel_outside[3],
        *chisel_face[3],
        *chisel_hammer;

    SDL_Texture *items[CELL_TYPE_COUNT];
    
    SDL_Texture *tool_buttons[TOOL_COUNT];
    SDL_Texture *convert_button, *tutorial_ok_button;

    // Temp textures for drawing text goes here!

    SDL_Texture *slot_names[TOTAL_SLOT_COUNT];
    SDL_Texture *converter_names[CONVERTER_COUNT];
    
    SDL_Texture *item_nums[ITEM_COUNT]; // Refers to the number text on items.
    
    SDL_Texture *narrator;
    
    SDL_Texture *text_arrow;
};

struct Surfaces {
    SDL_Surface *renderer_3d;
    
    SDL_Surface *text[TEXT_INDEX_COUNT];
    
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
    
    SDL_Surface *narrator;
};

struct Fonts {
    TTF_Font *font,
        *font_times,
        *font_consolas,
        *font_courier,
        *font_small,
        *font_bold_small,
        *font_title;
};
