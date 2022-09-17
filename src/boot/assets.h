#ifndef ASSETS_H_
#define ASSETS_H_

#define RENDER_TARGET_COUNT 7

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "../grid.h"

// Only contains textures!
struct Textures {
    SDL_Texture *point_knife,
        *placer,
        *knife,
        *popup,
        *blob_hammer,
        *converter_arrow;

    // Contains render unique targets for each unique type,
    // with new ones for every level because of the different sizes.
    SDL_Texture **render_targets;

    SDL_Texture *chisel_outside[3],
        *chisel_face[3],
        *chisel_hammer;
        
    SDL_Texture *items[CELL_COUNT];
    SDL_Texture *tool_buttons[TOOL_COUNT];
    SDL_Texture *convert_button;
};

struct Surfaces {
    SDL_Surface *bark_surface,
        *glass_surface,
        *wood_plank_surface,
        *diamond_surface,
        *ice_surface,
        *grass_surface,
        *triangle_blob_surface;
};

struct Fonts {
    TTF_Font *font,
        *font_consolas,
        *font_courier,
        *font_small,
        *font_bold_small,
        *font_title;
};

struct Game_State;
struct Level;

SDL_Texture *load_texture(SDL_Renderer *renderer, SDL_Window *window, const char *fp);

void textures_init(struct Game_State *state, int window_width, int gw, int gh, struct Textures *textures);
void textures_deinit(struct Textures *textures);

void render_targets_init(struct Game_State *state,
                         int width,
                         int height,
                         int amount,
                         struct Level *levels,
                         int level_count,
                         struct Textures *textures);

void surfaces_init(struct Surfaces *surfaces);
void surfaces_deinit(struct Surfaces *surfaces);

void fonts_init(struct Fonts *fonts);
void fonts_deinit(struct Fonts *fonts);

#endif
