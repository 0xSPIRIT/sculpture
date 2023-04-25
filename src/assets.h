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
#define Scale(x) ((f64)gs->window_width * (f64)x/768.0)

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
    TEXT_INDEX_COUNT = 128
};

// Index into textures.render_targets[]
enum {
    RENDER_TARGET_MASTER, // The final full-screen resolution target.
    RENDER_TARGET_GLOBAL, // The main pixel art render target
    RENDER_TARGET_GUI_TOOLBAR, // The render target showing the tool buttons
    RENDER_TARGET_CHISEL_BLOCKER,
    RENDER_TARGET_CHISEL, // Use the same render target for each chisel.
    RENDER_TARGET_GRID,
    RENDER_TARGET_3D,
    RENDER_TARGET_CONVERSION_PANEL,
    RENDER_TARGET_PREVIEW,
    RENDER_TARGET_OUTRO,
    RENDER_TARGET_COUNT 
};

struct Audio {
    Mix_Music *music_titlescreen;
    Mix_Music *music_creation;
    
    Mix_Chunk *stinger_a, *stinger_b;
    
    Mix_Chunk *accept;
    
    Mix_Chunk *pip;
    
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
    *font_title,
    *font_titlescreen;
};
