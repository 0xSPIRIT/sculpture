#ifdef ALASKA_RELEASE_MODE
  #define RES_DIR "res/"
#else
  #define RES_DIR "../res/"
#endif

// Index into textures.render_targets[]
enum {
    RENDER_TARGET_GLOBAL, // The main render target
    RENDER_TARGET_GUI_TOOLBAR, // The render target showing the tool buttons
    RENDER_TARGET_CHISEL_BLOCKER,
    RENDER_TARGET_CHISEL, // Use the same render target for each chisel.
    RENDER_TARGET_DELETER, // TODO: Unused
    RENDER_TARGET_COUNT 
};

struct Audio {
    Mix_Music *music;
    Mix_Chunk *chisel;
};

// Only contains textures!
struct Textures {
    // List of render targets for each level
    // Index into this using enum.
    SDL_Texture *render_targets[LEVEL_COUNT][RENDER_TARGET_COUNT];
    
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
    // TODO: Perhaps use an array and streamline this process
    //       by only using calls to draw_text()

    SDL_Texture *slot_names[TOTAL_SLOT_COUNT];
    SDL_Texture *converter_names[CONVERTER_COUNT];
};

struct Surfaces {
    SDL_Surface *a;
    SDL_Surface *out_3d;
    
    SDL_Surface *bark_surface,
        *glass_surface,
        *wood_plank_surface,
        *marble_surface,
        *granite_surface,
        *diamond_surface,
        *ice_surface,
        *grass_surface,
        *triangle_blob_surface;

    // Any temp surfaces you might need to draw text or w/e goes here!
    SDL_Surface *slot_names[TOTAL_SLOT_COUNT];
    SDL_Surface *converter_names[CONVERTER_COUNT];
};

struct Fonts {
    TTF_Font *font,
        *font_consolas,
        *font_courier,
        *font_small,
        *font_bold_small,
        *font_title;
};
