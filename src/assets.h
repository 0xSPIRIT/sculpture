#ifndef ASSETS_H_
#define ASSETS_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define new_render_target() (SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, gh))

inline void get_filename_from_type(int type, char *out) {
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

inline void get_file_from_tool(int type, char *out) {
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

inline void get_name_from_tool(int type, char *out) {
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

// In this file, we load / unload every single asset from file,
// as well as every single global texture or surface created
// which persists across frames.
//
// This is not necessary for textures created inside functions
// which are freed in the same function.
//
// This is included only in the platform/SDL layer.

inline void textures_init(SDL_Renderer *renderer, int gw, int gh, int S, struct Textures *textures) {
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

    // Point Knife
    {
        surf = IMG_Load("../res/point_knife.png");
        SDL_assert(surf);
        textures->point_knife = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        surf = NULL;
    }

    // Placers
    {
        surf = IMG_Load("../res/placer.png");
        SDL_assert(surf);
        textures->placer = SDL_CreateTextureFromSurface(renderer, surf);

        SDL_FreeSurface(surf);
        surf = NULL;
    }

    // Knife
    {
        surf = IMG_Load("../res/knife.png");
        SDL_assert(surf);
        textures->knife = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        textures->knife_render_target = new_render_target();

        surf = NULL;
    }

    // GUI
    {
        surf = IMG_Load("../res/popup.png");
        SDL_assert(surf);
        textures->popup = SDL_CreateTextureFromSurface(renderer, surf);
        textures->gui_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw*S, GUI_H);

        SDL_FreeSurface(surf);
        surf = NULL;
    }

    // Tool Buttons
    {
        for (int i = 0; i < TOOL_COUNT; i++) {
            char name[128] = {0};
            char filename[128] = {0};
            char path[128] = {0};

            get_name_from_tool(i, name);
            get_file_from_tool(i, filename);

            surf = IMG_Load(path);
            SDL_assert(surf);
            textures->tool_buttons[i] = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_assert(textures->tool_buttons[i]);
            SDL_FreeSurface(surf);
        }
        surf = NULL;
    }

    // Blob Hammer
    {
        surf = IMG_Load("../res/hammer.png");
        SDL_assert(surf);
        textures->blob_hammer = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        surf = NULL;

        textures->blob_hammer_render_target = new_render_target();
    }

    // Converter arrow
    {
        surf = IMG_Load("../res/arrow.png");
        SDL_assert(surf);
        textures->converter_arrow = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        surf = NULL;
    }

    // Convert Button
    {
        surf = IMG_Load("../res/converter_button.png");
        SDL_assert(surf);
        textures->convert_button = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        surf = NULL;
    }

    // Chisel Blocker
    {
        textures->chisel_blocker_render_target = new_render_target();
    }

    // Chisel
    {
        textures->chisel_render_target = new_render_target();

        const char *chisel_files[] = {
            "../res/chisel_small",
            "../res/chisel_medium",
            "../res/chisel_large",
        };
        for (int i = 0; i < 3; i++) {
            for (int face = 1; face != -1; face--) {
                char file[512] = {0};
                strcpy(file, chisel_files[i]);

                if (face)
                    strcat(file, "_face");

                strcat(file, ".png");

                surf = IMG_Load(file);
                SDL_assert(surf);

                if (face) {
                    textures->chisel_face[i] = SDL_CreateTextureFromSurface(renderer, surf);
                } else {
                    textures->chisel_outside[i] = SDL_CreateTextureFromSurface(renderer, surf);
                }
            }
        }
        surf = NULL;
    }

    // Chisel Hammer
    {
        surf = IMG_Load("../res/hammer.png");
        SDL_assert(surf);
        textures->chisel_hammer = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
}

/* inline void fonts_init(struct Game_State *gs) { */
/*     gs->font = TTF_OpenFont("../res/cour.ttf", 19); */
/*     gs->font_consolas = TTF_OpenFont("../res/consola.ttf", 24); */
/*     gs->font_courier = TTF_OpenFont("../res/cour.ttf", 20); */
/*     gs->small_font = TTF_OpenFont("../res/cour.ttf", 16); */
/*     gs->bold_small_font = TTF_OpenFont("../res/courbd.ttf", 16); */
/*     gs->title_font = TTF_OpenFont("../res/cour.ttf", 45); */
/* } */

/* inline void fonts_deinit(struct Game_State *gs) { */
/*     TTF_CloseFont(gs->font); */
/*     TTF_CloseFont(gs->font_consolas); */
/*     TTF_CloseFont(gs->font_courier); */
/*     TTF_CloseFont(gs->bold_small_font); */
/*     TTF_CloseFont(gs->small_font); */
/*     TTF_CloseFont(gs->title_font); */
/* } */

#endif
