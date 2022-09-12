#include "assets.h"

#include "game.h"

// In this file, we load / unload every single asset from file,
// as well as every single global texture or surface created
// which persists across frames.
//
// This is not necessary for textures created inside functions
// which are freed in the same function.
//
// This is included only in the platform/SDL layer.

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *fp) {
    SDL_Surface *surf = IMG_Load(fp);
    SDL_assert(surf);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_assert(texture);
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

// TODO: Deinit
void textures_init(SDL_Renderer *renderer, int gw, int gh, int S, struct Textures *textures) {
    SDL_Surface *surf = NULL;

    textures->render_texture = SDL_CreateTexture(renderer,
                                                 SDL_PIXELFORMAT_RGBA8888,
                                                 SDL_TEXTUREACCESS_TARGET,
                                                 gw,
                                                 gh);
    SDL_assert(textures->render_texture);


    // Converter Item Textures || previously item_init()
    for (int i = 0; i < CELL_COUNT; i++) {
        if (i == CELL_NONE) continue;
        
        char file[64] = {0};
        get_filename_from_type(i, file);
        surf = IMG_Load(file);
        SDL_assert(surf);
        textures->items[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        surf = NULL;
    }

    textures->point_knife = load_texture(renderer, "../res/point_knife.png");
    textures->placer = load_texture(renderer, "../res/placer.png");
    textures->knife = load_texture(renderer, "../res/knife.png");
    textures->knife_render_target = new_render_target();
    textures->popup = load_texture(renderer, "../res/popup.png");
    textures->gui_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw*S, GUI_H);

    for (int i = 0; i < TOOL_COUNT; i++) {
        char name[128] = {0};
        char filename[128] = {0};
        char path[128] = {0};

        get_name_from_tool(i, name);
        get_file_from_tool(i, filename);

        textures->tool_buttons[i] = load_texture(renderer, path);
    }

    textures->blob_hammer = load_texture(renderer, "../res/hammer.png");
    textures->blob_hammer_render_target = new_render_target();
    textures->converter_arrow = load_texture(renderer, "../res/arrow.png");
    textures->convert_button = load_texture(renderer, "../res/converter_button.png");
    textures->chisel_blocker_render_target = new_render_target();

    textures->chisel_render_target = new_render_target();

    const char *chisel_files[] = {
        "../res/chisel_small",
        "../res/chisel_medium",
        "../res/chisel_large",
    };
    textures->chisel_hammer = load_texture(renderer, "../res/hammer.png");
    for (int i = 0; i < 3; i++) {
        for (int face = 1; face != -1; face--) {
            char file[512] = {0};
            strcpy(file, chisel_files[i]);

            if (face)
                strcat(file, "_face");

            strcat(file, ".png");

            if (face) {
                textures->chisel_face[i] = load_texture(renderer, file);
            } else {
                textures->chisel_outside[i] = load_texture(renderer, file);
            }
        }
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
