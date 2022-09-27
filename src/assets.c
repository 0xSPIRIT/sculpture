#include "assets.h"

#include "shared.h"
#include <SDL2/SDL_ttf.h>

#define new_render_target(width, height) (SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height))

//
// In this file, we load / unload every single asset from file,
// as well as every single global texture or surface created
// which persists across frames.
//
// This is not necessary for textures created inside functions
// which are freed in the same function.
//
// This is included only in the platform/SDL layer.
//

SDL_Texture *load_texture(SDL_Renderer *renderer, SDL_Window *window, const char *fp) {
    SDL_Surface *surf = IMG_Load(fp);
    Assert(surf);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    Assert(texture);
    return texture;
}

internal void get_filename_from_type(int type, char *out) {
    switch (type) {
    case CELL_NONE:        strcpy(out, "nothing"); break;
    case CELL_DIRT:        strcpy(out, RES_DIR "/items/dirt.png"); break;
    case CELL_SAND:        strcpy(out, RES_DIR "/items/sand.png"); break;
        
    case CELL_WATER:       strcpy(out, RES_DIR "/items/water.png"); break;
    case CELL_ICE:         strcpy(out, RES_DIR "/items/ice.png"); break;
    case CELL_STEAM:       strcpy(out, RES_DIR "/items/steam.png"); break;
    
    case CELL_WOOD_LOG:    strcpy(out, RES_DIR "/items/wood_log.png"); break;
    case CELL_WOOD_PLANK:  strcpy(out, RES_DIR "/items/wood_plank.png"); break;
    
    case CELL_COBBLESTONE: strcpy(out, RES_DIR "/items/cobblestone.png"); break;
    case CELL_MARBLE:      strcpy(out, RES_DIR "/items/marble.png"); break;
    case CELL_SANDSTONE:   strcpy(out, RES_DIR "/items/sandstone.png"); break;
    
    case CELL_CEMENT:      strcpy(out, RES_DIR "/items/cement.png"); break;
    case CELL_CONCRETE:    strcpy(out, RES_DIR "/items/concrete.png"); break;
    
    case CELL_QUARTZ:      strcpy(out, RES_DIR "/items/quartz.png"); break;
    case CELL_GLASS:       strcpy(out, RES_DIR "/items/glass.png"); break;
    
    case CELL_GRANITE:     strcpy(out, RES_DIR "/items/granite.png"); break;
    case CELL_BASALT:      strcpy(out, RES_DIR "/items/basalt.png"); break;
    case CELL_DIAMOND:     strcpy(out, RES_DIR "/items/diamond.png"); break;
    
    case CELL_UNREFINED_COAL: strcpy(out, RES_DIR "/items/coal.png"); break;
    case CELL_REFINED_COAL:   strcpy(out, RES_DIR "/items/coal.png"); break;
    case CELL_LAVA:           strcpy(out, RES_DIR "/items/lava.png"); break;
    
    case CELL_SMOKE:       strcpy(out, RES_DIR "/items/smoke.png"); break;
    case CELL_DUST:        strcpy(out, RES_DIR "/items/dust.png"); break;
    }
}

internal void get_file_from_tool(int type, char *out) {
    switch (type) {
    case TOOL_CHISEL_SMALL:  strcpy(out, "chisel_small.png"); break;
    case TOOL_CHISEL_MEDIUM: strcpy(out, "chisel_medium.png"); break;
    case TOOL_CHISEL_LARGE:  strcpy(out, "chisel_large.png"); break;
    case TOOL_KNIFE:         strcpy(out, "knife.png"); break;
    case TOOL_POINT_KNIFE:   strcpy(out, "point_knife.png"); break;
    case TOOL_HAMMER:        strcpy(out, "hammer.png"); break;
    case TOOL_PLACER:        strcpy(out, "placer.png"); break;
    case TOOL_GRABBER:       strcpy(out, "pointer.png"); break;
    }
}

internal void get_name_from_tool(int type, char *out) {
    switch (type) {
    case TOOL_CHISEL_SMALL:  strcpy(out, "Small Chisel"); break;
    case TOOL_CHISEL_MEDIUM: strcpy(out, "Medium Chisel"); break;
    case TOOL_CHISEL_LARGE:  strcpy(out, "Large Chisel"); break;
    case TOOL_KNIFE:         strcpy(out, "Knife"); break;
    case TOOL_POINT_KNIFE:   strcpy(out, "Point Knife"); break;
    case TOOL_HAMMER:        strcpy(out, "Hammer"); break;
    case TOOL_PLACER:        strcpy(out, "Placer"); break;
    case TOOL_GRABBER:       strcpy(out, "Grabber"); break;
    }
}

// Creates all render targets for all the levels,
// then sets all the tools equal to them.
void render_targets_init(SDL_Window *window,
                         SDL_Renderer *renderer,
                         int width, // In screen coords, not game coords.
                         struct Level *levels,
                         struct Textures *textures) {
    for (int lvl = 0; lvl < LEVEL_COUNT; lvl++) {
        struct Level *l = &levels[lvl];
        for (int i = 0; i < RENDER_TARGET_COUNT; i++) {
            if (i == 1) {
                textures->render_targets[lvl][i] = new_render_target(width, GUI_H);
                continue;
            }

            Assert(l->w != 0 && l->h != 0);
            textures->render_targets[lvl][i] = new_render_target(l->w, l->h);
            Assert(textures->render_targets[lvl][i]);
        }
    }
}

void textures_init(SDL_Window *window,
                   SDL_Renderer *renderer,
                   struct Level *levels,
                   int window_width,
                   int gw,
                   int gh,
                   struct Textures *textures) {
    SDL_Surface *surf = NULL;

    // Converter Item Textures || previously item_init()
    for (int i = 0; i < CELL_TYPE_COUNT; i++) {
        if (i == CELL_NONE) continue;
        
        char file[64] = {0};
        get_filename_from_type(i, file);

        surf = IMG_Load(file);
        Assert(surf);

        textures->items[i] = SDL_CreateTextureFromSurface(renderer, surf);
        Assert(textures->items[i]);

        SDL_FreeSurface(surf);
        surf = NULL;
    }

    textures->point_knife = load_texture(renderer, window,  RES_DIR "/point_knife.png");
    textures->placer = load_texture(renderer, window,  RES_DIR "/placer.png");
    textures->knife = load_texture(renderer, window,  RES_DIR "/knife.png");
    textures->popup = load_texture(renderer, window,  RES_DIR "/popup.png");

    for (int i = 0; i < TOOL_COUNT; i++) {
        char filename[128] = {0};
        char path[128] = {0};

        get_file_from_tool(i, filename);
        sprintf(path, RES_DIR "/buttons/%s", filename);

        textures->tool_buttons[i] = load_texture(renderer, window, path);
        Assert(textures->tool_buttons[i]);
    }

    textures->blob_hammer = load_texture(renderer, window,  RES_DIR "/hammer.png");
    textures->converter_arrow = load_texture(renderer, window,  RES_DIR "/arrow.png");
    textures->convert_button = load_texture(renderer, window,  RES_DIR "/buttons/convert.png");

    const char *chisel_files[] = {
        RES_DIR "/chisel_small",
        RES_DIR "/chisel_medium",
        RES_DIR "/chisel_large",
    };
    textures->chisel_hammer = load_texture(renderer, window,  RES_DIR "/hammer.png");

    // Loop through all chisels
    for (int i = 0; i < 3; i++) {
        // Alternate through face mode
        for (int face = 1; face != -1; face--) {
            char file[512] = {0};
            strcpy(file, chisel_files[i]);

            if (face)
                strcat(file, "_face");

            strcat(file, ".png");

            if (face) {
                textures->chisel_face[i] = load_texture(renderer, window,  file);
                Assert(textures->chisel_face[i]);
            } else {
                textures->chisel_outside[i] = load_texture(renderer, window,  file);
                Assert(textures->chisel_outside[i]);
            }
        }
    }
}

void textures_deinit(struct Textures *textures) {
    SDL_Texture **texs = (SDL_Texture**) textures;
    size_t tex_count = sizeof(struct Textures)/sizeof(SDL_Texture*);

    for (size_t i = 0; i < tex_count; i++) {
        if (texs[i])
            SDL_DestroyTexture(texs[i]);
    }
}

void surfaces_init(struct Surfaces *surfaces) {
    surfaces->bark_surface = IMG_Load(RES_DIR "/bark.png");
    surfaces->glass_surface = IMG_Load(RES_DIR "/glass.png");
    surfaces->wood_plank_surface = IMG_Load(RES_DIR "/plank.png");
    surfaces->diamond_surface = IMG_Load(RES_DIR "/diamond.png");
    surfaces->ice_surface = IMG_Load(RES_DIR "/ice.png");
    surfaces->grass_surface = IMG_Load(RES_DIR "/grass.png");
    surfaces->triangle_blob_surface = IMG_Load(RES_DIR "/triangle_blob.png");
}

void surfaces_deinit(struct Surfaces *surfaces) {
    SDL_Surface **surfs = (SDL_Surface**) surfaces;
    size_t surf_count = sizeof(struct Surfaces)/sizeof(SDL_Surface*);

    for (size_t i = 0; i < surf_count; i++) {
        if (surfs[i])
            SDL_FreeSurface(surfs[i]);
    }
}

void fonts_init(struct Fonts *fonts) {
    fonts->font = TTF_OpenFont(RES_DIR "/cour.ttf", 19);
    fonts->font_consolas = TTF_OpenFont(RES_DIR "/consola.ttf", 24);
    fonts->font_courier = TTF_OpenFont(RES_DIR "/cour.ttf", 20);
    fonts->font_small = TTF_OpenFont(RES_DIR "/cour.ttf", 16);
    fonts->font_bold_small = TTF_OpenFont(RES_DIR "/courbd.ttf", 16);
    fonts->font_title = TTF_OpenFont(RES_DIR "/cour.ttf", 45);

    TTF_Font **ttf_fonts = (TTF_Font**) fonts;
    size_t font_count = sizeof(struct Fonts)/sizeof(TTF_Font*);

    for (size_t i = 0; i < font_count; i++) {
        TTF_SetFontHinting(ttf_fonts[i], TTF_HINTING_LIGHT_SUBPIXEL);
    }
}

void fonts_deinit(struct Fonts *fonts) {
    TTF_Font **ttf_fonts = (TTF_Font**) fonts;
    size_t font_count = sizeof(struct Fonts)/sizeof(TTF_Font*);

    for (size_t i = 0; i < font_count; i++) {
        TTF_CloseFont(ttf_fonts[i]);
    }
}
