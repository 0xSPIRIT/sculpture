#ifndef ASSETS_H_
#define ASSETS_H_

#define RENDER_TARGET_COUNT 7
#define RES_DIR "../res" 

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "grid.h"
#include "level.h"
#include "converter.h" // for CONVERTER_COUNT

// Index into textures.render_targets[]
enum {
    RENDER_TARGET_GLOBAL, // The main render target
    RENDER_TARGET_GUI_TOOLBAR, // The render target showing the tool buttons
    RENDER_TARGET_KNIFE,
    RENDER_TARGET_BLOB_HAMMER,
    RENDER_TARGET_CHISEL_BLOCKER,
    RENDER_TARGET_CHISEL // Use the same render target for each chisel.
};

// Only contains textures!
struct Textures {
    // List of render targets for each level
    // Index into this using enum.
    SDL_Texture *render_targets[LEVEL_COUNT][RENDER_TARGET_COUNT];

    SDL_Texture *point_knife,
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
    SDL_Texture *convert_button;

    // Temp textures for drawing text goes here!
    // TODO: Perhaps use an array and streamline this process
    //       by only using calls to draw_text()

    SDL_Texture *slot_names[SLOT_MAX_COUNT * CONVERTER_COUNT];
    SDL_Texture *converter_names[CONVERTER_COUNT];
};

struct Surfaces {
    SDL_Surface *bark_surface,
        *glass_surface,
        *wood_plank_surface,
        *diamond_surface,
        *ice_surface,
        *grass_surface,
        *triangle_blob_surface;

    // Any temp surfaces you might need to draw text or w/e goes here!
    SDL_Surface *slot_names[SLOT_MAX_COUNT * CONVERTER_COUNT];
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

SDL_Texture *load_texture(SDL_Renderer *renderer, SDL_Window *window, const char *fp);

void textures_init(SDL_Window *window,
                   SDL_Renderer *renderer,
                   struct Level *levels,
                   int window_width,
                   int gw,
                   int gh,
                   struct Textures *textures);
void textures_deinit(struct Textures *textures);

void render_targets_init(SDL_Window *window,
                         SDL_Renderer *renderer,
                         int width, // In screen coords, not game coords.
                         struct Level *levels,
                         struct Textures *textures);

void surfaces_init(struct Surfaces *surfaces);
void surfaces_deinit(struct Surfaces *surfaces);

void fonts_init(struct Fonts *fonts);
void fonts_deinit(struct Fonts *fonts);

#endif
