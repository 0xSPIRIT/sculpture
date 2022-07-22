#include "level.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdlib.h>

#include "globals.h"
#include "chisel.h"
#include "knife.h"
#include "point_knife.h"
#include "drill.h"
#include "placer.h"
#include "grabber.h"
#include "grid.h"
#include "gui.h"

struct Level levels[MAX_LEVELS];
int level_current = 0, level_count = 0;

static int level_add(const char *name, char *desired_image, char *initial_image) {
    struct Level *level = &levels[level_count++];
    level->index = level_count-1;
    strcpy(level->name, name);
    level->popup_time_current = 0;
    level->popup_time_max = 15;
    level->is_intro = 1;

    int w, h;
    level_get_cells_from_image(desired_image, &level->desired_grid, &level->w, &level->h);
    level_get_cells_from_image(initial_image, &level->initial_grid, &w, &h);

    SDL_assert(w == level->w);
    SDL_assert(h == level->h);

    return level->index;
}

void levels_setup() {
    level_add("Alaska",
              "../res/lvl/desired/level_1.png",
              "../res/lvl/initial/level_1.png");
    level_add("Conversion",
              "../res/lvl/desired/level_2.png",
              "../res/lvl/initial/level_2.png");
    level_add("Masonry",
              "../res/lvl/desired/level_1.png",
              "../res/lvl/initial/level_1.png");
    level_add("Remainder",
              "../res/lvl/desired/level_1.png",
              "../res/lvl/initial/level_1.png");
    level_add("Carbon Copy",
              "../res/lvl/desired/level_1.png",
              "../res/lvl/initial/level_1.png");
    level_add("Metamorphosis",
              "../res/lvl/desired/level_1.png",
              "../res/lvl/initial/level_1.png");
    level_add("Procedure Lullaby",
              "../res/lvl/desired/level_1.png",
              "../res/lvl/initial/level_1.png");
    level_add("Polished Turd",
              "../res/lvl/desired/level_1.png",
              "../res/lvl/initial/level_1.png");
    level_add("Showpiece",
              "../res/lvl/desired/level_1.png",
              "../res/lvl/initial/level_1.png");
    level_add("Glass Body",
              "../res/lvl/desired/level_1.png",
              "../res/lvl/initial/level_1.png");

    level_set_current(0);
}

void level_set_current(int lvl) {
    level_current = lvl;

    if (grid)
        grid_deinit();
    grid_init(levels[lvl].w, levels[lvl].h);

    SDL_DestroyTexture(render_tex);
    render_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, gh);

    S = window_width/gw;
    SDL_assert(S == (window_height/gh)); // Make sure W==H

    chisel_init(&chisel_small);
    chisel_init(&chisel_medium);
    chisel_init(&chisel_large);
    hammer_init();
    knife_init();
    point_knife_init();
    for (int i = 0; i < PLACER_COUNT; i++)
        placer_init(i);
    grabber_init();
    drill_init();
    gui_init();

    memcpy(grid, levels[lvl].desired_grid, sizeof(struct Cell)*gw*gh);
}

void levels_free() {
    for (int i = 0; i < level_count; i++) {
        free(levels[i].desired_grid);
        free(levels[i].initial_grid);
    }
}

void level_tick() {
    gui_tick();
    if (gui.popup) return;
    
    struct Level *level = &levels[level_current];
    if (!level->is_intro) {
        grid_tick();
    
        for (int i = 0; i < object_count; i++) {
            object_tick(i);
        }

        if (current_tool == TOOL_GRABBER) {
            if (SDL_GetCursor() != grabber_cursor) {
                SDL_ShowCursor(1);
                SDL_SetCursor(grabber_cursor);
            }
        } else if (SDL_GetCursor() != normal_cursor) {
            SDL_SetCursor(normal_cursor);
        }
    
        switch (current_tool) {
        case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE:
            chisel_tick();
            break;
        case TOOL_KNIFE:
            knife_tick();
            break;
        case TOOL_POINT_KNIFE:
            point_knife_tick();
            break;
        case TOOL_DRILL:
            drill_tick();
            break;
        case TOOL_PLACER:
            if (!gui.popup) // We'll handle updating it in the converter.c
                placer_tick(placers[current_placer]);
            break;
        case TOOL_GRABBER:
            grabber_tick();
            break;
        }
    } else {
        level->popup_time_current++;
        if (level->popup_time_current >= level->popup_time_max) {
            level->popup_time_current = 0;
            level->is_intro = 0;
            memset(level->desired_grid, 0, sizeof(struct Cell)*gw*gh);
            for (int i = 0; i < gw*gh; i++) {
                grid[i] = (struct Cell){.type = 0, .object = -1, .depth = 255};
            }
        }
    }
}

void level_draw() {
    struct Level *level = &levels[level_current];
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (level->is_intro) {
        level_draw_intro();
    } else {
        SDL_SetRenderTarget(renderer, render_tex);
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        grid_draw(1);

        switch (current_tool) {
        case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE:
            chisel_draw();
            break;
        case TOOL_KNIFE:
            knife_draw();
            break;
        case TOOL_POINT_KNIFE:
            point_knife_draw();
            break;
        case TOOL_DRILL:
            drill_draw();
            break;
        case TOOL_PLACER:
            if (!gui.popup) // We'll handle updating it in the converter.c
                placer_draw(placers[current_placer]);
            break;
        case TOOL_GRABBER:
            grabber_draw();
            break;
        }

        draw_blobs();
        draw_objects();

        gui_draw();

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, render_tex, NULL, NULL);
    }
        
    overlay_draw(&gui.overlay);
    
    SDL_RenderPresent(renderer);
}

void level_draw_intro() {
    struct Level *level = &levels[level_current];

    SDL_SetRenderTarget(renderer, render_tex);
        
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (level->desired_grid[x+y*gw].type == 0) continue;
            SDL_Color col = pixel_from_index(x+y*gw);
            SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, render_tex, NULL, NULL);

    char name[256] = {0};
    sprintf(name, "Level %d: %s", level->index+1, level->name);

    SDL_Surface *surf = TTF_RenderText_Solid(title_font, name, (SDL_Color){255,255,255,255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dst = {
        S*gw/2 - surf->w/2, S*gh/2 - surf->h/2,
        surf->w, surf->h
    };
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_FreeSurface(surf);
}

// Image must be formatted as Uint32 RGBA.
void level_get_cells_from_image(char *path, struct Cell **out, int *out_w, int *out_h) {
    SDL_Surface *surface = IMG_Load(path);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    int w = surface->w;
    int h = surface->h;

    *out_w = w;
    *out_h = h;

    *out = calloc(w*h, sizeof(struct Cell));

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Uint8 r, g, b;

            Uint32 pixel = ((Uint32*)surface->pixels)[x+y*w];
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            int cell = 0;

            if (r == 255 && g == 255 && b == 255) {
                cell = CELL_MARBLE;
            } else if (r == 0 && g == 255 && b == 0) {
                cell = CELL_LEAF;
            }

            (*out)[x+y*w].type = cell;
        }
    }

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
