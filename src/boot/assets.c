#include "assets.h"

#include "../game.h"

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
    Assert(window, surf);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    Assert(window, texture);
    return texture;
}

static void get_filename_from_type(int type, char *out) {
    switch (type) {
    case CELL_NONE:        strcpy(out, "nothing"); break;
    case CELL_MARBLE:      strcpy(out, "../res/items/marble.png"); break;
    case CELL_COBBLESTONE: strcpy(out, "../res/items/cobblestone.png"); break;
    case CELL_QUARTZ:      strcpy(out, "../res/items/quartz.png"); break;
    case CELL_GRANITE:     strcpy(out, "../res/items/quartz.png"); break;
    case CELL_BASALT:      strcpy(out, "../res/items/quartz.png"); break;
    case CELL_WOOD_LOG:    strcpy(out, "../res/items/wood_log.png"); break;
    case CELL_WOOD_PLANK:  strcpy(out, "../res/items/wood_plank.png"); break;
    case CELL_DIRT:        strcpy(out, "../res/items/dirt.png"); break;
    case CELL_SAND:        strcpy(out, "../res/items/sand.png"); break;
    case CELL_GLASS:       strcpy(out, "../res/items/glass.png"); break;
    case CELL_WATER:       strcpy(out, "../res/items/water.png"); break;
    case CELL_COAL:        strcpy(out, "../res/items/coal.png"); break;
    case CELL_STEAM:       strcpy(out, "../res/items/steam.png"); break;
    case CELL_DIAMOND:     strcpy(out, "../res/items/diamond.png"); break;
    case CELL_ICE:         strcpy(out, "../res/items/ice.png"); break;
    case CELL_LEAF:        strcpy(out, "../res/items/leaf.png"); break;
    case CELL_SMOKE:       strcpy(out, "../res/items/smoke.png"); break;
    case CELL_DUST:        strcpy(out, "../res/items/dust.png"); break;
    case CELL_LAVA:        strcpy(out, "../res/items/quartz.png"); break;
    }
}

static void get_file_from_tool(int type, char *out) {
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

static void get_name_from_tool(int type, char *out) {
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

void render_targets_init(SDL_Renderer *renderer, SDL_Window *window, int gw, int gh, struct Textures *textures) {
    textures->chisel_render_target = new_render_target();
    textures->chisel_blocker_render_target = new_render_target();
    textures->blob_hammer_render_target = new_render_target();
    textures->knife_render_target = new_render_target();
    textures->render_texture = new_render_target();
}

void textures_init(SDL_Renderer *renderer, SDL_Window *window, int window_width, int gw, int gh, struct Textures *textures) {
    SDL_Surface *surf = NULL;

    // Converter Item Textures || previously item_init()
    for (int i = 0; i < CELL_COUNT; i++) {
        if (i == CELL_NONE) continue;
        
        char file[64] = {0};
        get_filename_from_type(i, file);

        surf = IMG_Load(file);
        Assert(window, surf);

        textures->items[i] = SDL_CreateTextureFromSurface(renderer, surf);
        Assert(window, textures->items[i]);

        SDL_FreeSurface(surf);
        surf = NULL;
    }

    textures->point_knife = load_texture(renderer, window,  "../res/point_knife.png");
    textures->placer = load_texture(renderer, window,  "../res/placer.png");
    textures->knife = load_texture(renderer, window,  "../res/knife.png");
    textures->popup = load_texture(renderer, window,  "../res/popup.png");
    textures->gui_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, window_width, GUI_H);

    for (int i = 0; i < TOOL_COUNT; i++) {
        char filename[128] = {0};
        char path[128] = {0};

        get_file_from_tool(i, filename);
        sprintf(path, "../res/buttons/%s", filename);

        textures->tool_buttons[i] = load_texture(renderer, window, path);
        Assert(window, textures->tool_buttons[i]);
    }

    textures->blob_hammer = load_texture(renderer, window,  "../res/hammer.png");
    textures->converter_arrow = load_texture(renderer, window,  "../res/arrow.png");
    textures->convert_button = load_texture(renderer, window,  "../res/buttons/convert.png");

    const char *chisel_files[] = {
        "../res/chisel_small",
        "../res/chisel_medium",
        "../res/chisel_large",
    };
    textures->chisel_hammer = load_texture(renderer, window,  "../res/hammer.png");

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
                Assert(window, textures->chisel_face[i]);
            } else {
                textures->chisel_outside[i] = load_texture(renderer, window,  file);
                Assert(window, textures->chisel_outside[i]);
            }
        }
    }

    render_targets_init(renderer, window, gw, gh, textures);
}

void textures_deinit(struct Textures *textures) {
    SDL_Texture **texs = (SDL_Texture**) textures;
    size_t tex_count = sizeof(struct Textures)/sizeof(SDL_Texture*);

    for (size_t i = 0; i < tex_count; i++) {
        SDL_DestroyTexture(texs[i]);
    }
}

void surfaces_init(struct Surfaces *surfaces) {
    surfaces->bark_surface = IMG_Load("../res/bark.png");
    surfaces->glass_surface = IMG_Load("../res/glass.png");
    surfaces->wood_plank_surface = IMG_Load("../res/plank.png");
    surfaces->diamond_surface = IMG_Load("../res/diamond.png");
    surfaces->ice_surface = IMG_Load("../res/ice.png");
    surfaces->grass_surface = IMG_Load("../res/grass.png");
    surfaces->triangle_blob_surface = IMG_Load("../res/triangle_blob.png");
}

void surfaces_deinit(struct Surfaces *surfaces) {
    SDL_Surface **surfs = (SDL_Surface**) surfaces;
    size_t surf_count = sizeof(struct Surfaces)/sizeof(SDL_Surface*);

    for (size_t i = 0; i < surf_count; i++) {
        SDL_FreeSurface(surfs[i]);
    }
}

void fonts_init(struct Game_State *gs) {
    gs->font = TTF_OpenFont("../res/cour.ttf", 19);
    gs->font_consolas = TTF_OpenFont("../res/consola.ttf", 24);
    gs->font_courier = TTF_OpenFont("../res/cour.ttf", 20);
    gs->small_font = TTF_OpenFont("../res/cour.ttf", 16);
    gs->bold_small_font = TTF_OpenFont("../res/courbd.ttf", 16);
    gs->title_font = TTF_OpenFont("../res/cour.ttf", 45);
}

void fonts_deinit(struct Game_State *gs) {
    TTF_CloseFont(gs->font);
    TTF_CloseFont(gs->font_consolas);
    TTF_CloseFont(gs->font_courier);
    TTF_CloseFont(gs->bold_small_font);
    TTF_CloseFont(gs->small_font);
    TTF_CloseFont(gs->title_font);
}
