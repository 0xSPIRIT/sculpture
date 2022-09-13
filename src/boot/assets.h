#ifndef ASSETS_H_
#define ASSETS_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "../grid.h"

#define new_render_target() (SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, gh))

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *fp);

// Only contains textures!
struct Textures {
    SDL_Texture *point_knife,
        *placer,
        *knife,
        *popup,
        *blob_hammer,
        *converter_arrow;

    SDL_Texture *gui_target,
        *knife_render_target,
        *blob_hammer_render_target,
        *chisel_blocker_render_target,
        *chisel_render_target;

    SDL_Texture *chisel_outside[3],
        *chisel_face[3],
        *chisel_hammer;
        
    SDL_Texture *items[CELL_COUNT];
    SDL_Texture *tool_buttons[TOOL_COUNT];
    SDL_Texture *convert_button;

    SDL_Texture *render_texture;
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

void textures_init(SDL_Renderer *renderer, int gw, int gh, int S, struct Textures *textures);
void textures_deinit(struct Textures *textures);

void surfaces_init(struct Surfaces *surfaces);
void surfaces_deinit(struct Surfaces *surfaces);

void fonts_init(struct Game_State *gs);
void fonts_deinit(struct Game_State *gs);

#endif
