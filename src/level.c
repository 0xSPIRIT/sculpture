#include "level.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "globals.h"
#include "chisel.h"
#include "knife.h"
#include "point_knife.h"
#include "blob_hammer.h"
#include "placer.h"
#include "grabber.h"
#include "grid.h"
#include "gui.h"
#include "util.h"
#include "chisel_blocker.h"
#include "effects.h"
#include "cursor.h"
#include "popup.h"
#include "undo.h"

struct Level levels[MAX_LEVELS];
int level_current = 0, level_count = 0;

static int level_add(const char *name, char *desired_image, char *initial_image, int effect_type) {
    struct Level *level = &levels[level_count++];
    level->index = level_count-1;
    strcpy(level->name, name);
    level->popup_time_current = 0;
    level->popup_time_max = POPUP_TIME;
    level->state = LEVEL_STATE_INTRO;
    level->effect_type = effect_type;

    int w, h;
    level_get_cells_from_image(desired_image,
                               &level->desired_grid,
                               NULL,
                               NULL,
                               &level->w,
                               &level->h);
    level_get_cells_from_image(initial_image,
                               &level->initial_grid,
                               level->source_cell,
                               &level->source_cell_count,
                               &w,
                               &h);

    if (w != level->w || h != level->h) {
        fprintf(stderr, "%s and %s aren't the same size. Initial: %d, Desired: %d.\n", initial_image, desired_image, w, level->w);
        exit(1);
    }

    return level->index;
}

void levels_setup() {
    level_add("Alaska",
              "../res/lvl/desired/level 1.png",
              "../res/lvl/initial/level 1.png",
              EFFECT_SNOW);
    level_add("Masonry",
              "../res/lvl/desired/level 2.png",
              "../res/lvl/initial/level 2.png",
              EFFECT_NONE);
    level_add("Conversion",
              "../res/lvl/desired/level 3.png",
              "../res/lvl/initial/level 3.png",
              EFFECT_NONE);
    level_add("Remainder",
              "../res/lvl/desired/level 4.png",
              "../res/lvl/initial/level 4.png",
              EFFECT_RAIN);
    level_add("Carbon Copy",
              "../res/lvl/desired/level 5.png",
              "../res/lvl/initial/level 5.png",
              EFFECT_RAIN);
    level_add("Metamorphosis",
              "../res/lvl/desired/level 1.png",
              "../res/lvl/initial/level 1.png",
              EFFECT_NONE);
    level_add("Procedure Lullaby",
              "../res/lvl/desired/level 1.png",
              "../res/lvl/initial/level 1.png",
              EFFECT_NONE);
    level_add("Polished Turd",
              "../res/lvl/desired/level 1.png",
              "../res/lvl/initial/level 1.png",
              EFFECT_NONE);
    level_add("Showpiece",
              "../res/lvl/desired/level 1.png",
              "../res/lvl/initial/level 1.png",
              EFFECT_NONE);
    level_add("Glass Body",
              "../res/lvl/desired/level 1.png",
              "../res/lvl/initial/level 1.png",
              EFFECT_NONE);
}

void goto_level_string_hook(const char *string) {
    int lvl = atoi(string) - 1;

    if (lvl < 0) return;
    if (lvl >= MAX_LEVELS) return;

    goto_level(lvl);
}

void goto_level(int lvl) {
    level_current = lvl;

    // Deinit everything here. @Leak

    if (grid)
        grid_deinit();
    grid_init(levels[lvl].w, levels[lvl].h);

    SDL_DestroyTexture(render_tex);
    render_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, gh);

    S = window_width/gw;
    SDL_assert(gw==gh);

    chisel_init(&chisel_small);
    chisel_init(&chisel_medium);
    chisel_init(&chisel_large);
    chisel_blocker_init();
    blob_hammer_init();
    chisel_hammer_init();
    knife_init();
    point_knife_init();
    for (int i = 0; i < PLACER_COUNT; i++)
        placer_init(i);
    grabber_init();
    gui_init();
    all_converters_init();

    effect_set(levels[lvl].effect_type);

    memcpy(grid, levels[lvl].desired_grid, sizeof(struct Cell)*gw*gh);
}

void levels_deinit() {
    chisel_deinit(&chisel_small);
    chisel_deinit(&chisel_medium);
    chisel_deinit(&chisel_large);
    blob_hammer_deinit();
    chisel_hammer_deinit();
    knife_deinit();
    point_knife_deinit();
    gui_deinit();
    all_converters_deinit();

    effect_set(EFFECT_NONE);

    for (int i = 0; i < PLACER_COUNT; i++)
        placer_deinit(i);

    grabber_deinit();
    for (int i = 0; i < level_count; i++) {
        free(levels[i].desired_grid);
        free(levels[i].initial_grid);
    }

    undo_system_deinit();
}

void level_tick() {
    struct Level *level = &levels[level_current];

    if (text_field.active) return;

    if (level->state != LEVEL_STATE_INTRO) {
        gui_tick();
    }
    if (gui.popup) return;
    
    switch (level->state) {
    case LEVEL_STATE_INTRO:
        level->popup_time_current++;
        if (level->popup_time_current >= level->popup_time_max) {
            level->popup_time_current = 0;
            level->state = LEVEL_STATE_PLAY;
            srand(time(0));
            for (int i = 0; i < gw*gh; i++) {
                grid[i] = (struct Cell){.type = level->initial_grid[i].type, .rand = rand(), .object = -1, .depth = 255};
            }
            objects_reevaluate();

            undo_system_init();
        }
        break;
    case LEVEL_STATE_OUTRO:
        if (keys[SDL_SCANCODE_N]) {
            if (level_current+1 < 10) {
                goto_level(++level_current);
            }
        }

        if (keys_pressed[SDL_SCANCODE_F]) {
            levels[level_current].state = LEVEL_STATE_PLAY;
            keys[SDL_SCANCODE_F] = 0;
        }
        break;
    case LEVEL_STATE_PLAY:
        if (keys_pressed[SDL_SCANCODE_F]) {
            levels[level_current].state = LEVEL_STATE_OUTRO;
            keys[SDL_SCANCODE_F] = 0;
        }

        effect_tick(&current_effect);

        simulation_tick();
    
        if (!paused || step_one) {
            for (int i = 0; i < object_count; i++) {
                object_tick(i);
            }
        }

        if (step_one) {
            step_one = 0;
        }

        if (my < 0) { // If the mouse is in the GUI window...
            /* SDL_ShowCursor(1); */
            if (SDL_GetCursor() != normal_cursor) {
                set_cursor(normal_cursor);
            }
            break;
        } else if (current_tool == TOOL_GRABBER) {
            if (SDL_GetCursor() != grabber_cursor) {
                /* SDL_ShowCursor(1); */
                set_cursor(grabber_cursor);
            }
        } else if (SDL_GetCursor() != normal_cursor) {
            set_cursor(normal_cursor);
        }
    
        chisel_blocker_tick();
        if (chisel_blocker_mode) break;
        
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
        case TOOL_HAMMER:
            blob_hammer_tick();
            break;
        case TOOL_PLACER:
            if (!gui.popup) // We'll handle updating it in the converter.c
                placer_tick(placers[current_placer]);
            break;
        case TOOL_GRABBER:
            grabber_tick();
            break;
        }

        break;
    }
}

void level_draw() {
    struct Level *level = &levels[level_current];

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    switch (level->state) {
    case LEVEL_STATE_INTRO:
        level_draw_intro();
        break;
    case LEVEL_STATE_OUTRO: case LEVEL_STATE_PLAY:
        SDL_SetRenderTarget(renderer, render_tex);
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        grid_draw();

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
        case TOOL_HAMMER:
            blob_hammer_draw();
            break;
        case TOOL_PLACER:
            if (!gui.popup) // When gui.popup = true, we draw in converter.c
                placer_draw(placers[current_placer], false);
            break;
        case TOOL_GRABBER:
            grabber_draw();
            break;
        }

        chisel_blocker_draw();

        draw_blobs();
        draw_objects();

        effect_draw(&current_effect);

        gui_draw();

        SDL_Rect dst = {
            0, GUI_H,
            window_width, window_height-GUI_H
        };

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, render_tex, NULL, &dst);

        gui_popup_draw();
        
        break;
    }

    if (level->state == LEVEL_STATE_OUTRO) {
        SDL_Rect rect = {S*gw/8, GUI_H + S*gh/2 - (S*3*gh/4)/2, S*3*gw/4, S*3*gh/4};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);

        const int margin = 36;

        { // Level name
            char string[256] = {0};
            sprintf(string, "Level %d - \"%s\"", level_current+1, level->name);

            int x = rect.x + margin;
            int y = rect.y + margin;

            draw_text(font, string, (SDL_Color){0,0,0,255}, 0, 0, x, y, NULL, NULL);
        }

        // Desired and Your grid.
        for (int i = 0; i < 2; i++) {
            char string[256] = {0};
            if (!i) {
                strcpy(string, "What you intended");
            } else {
                strcpy(string, "The result");
            }

            int dx = rect.x + margin;
            int dy = rect.y + 100;
            if (i) { // If your grid, put it on the right
                dx += rect.w - margin - 2*level->w - margin;
            }

            draw_text(font, string, (SDL_Color){0, 0, 0, 255}, 0, 0, dx, dy, NULL, NULL);

            for (int y = 0; y < gh; y++) {
                for (int x = 0; x < gw; x++) {
                    SDL_Rect r;
                    switch (i) {
                    case 0: // Desired
                        if (!level->desired_grid[x+y*gw].type) {
                            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        }  else {
                            SDL_Color col = pixel_from_index(level->desired_grid, x+y*gw);
                            SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255); // 255 on this because desired_grid doesn't have depth set.
                        }
                        break;
                    case 1: // Yours
                        if (!grid[x+y*gw].type) {
                            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        }  else {
                            SDL_Color col = pixel_from_index(grid, x+y*gw);
                            SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
                        }
                        break;
                    }
                    r = (SDL_Rect){ 2*x + dx, 2*y + dy + 32, 2, 2 };
                    SDL_RenderFillRect(renderer, &r);
                }
            }
        }

        draw_text(font,
                  "Next Level [n]",
                  (SDL_Color){0, 91, 0, 255},
                  1, 1,
                  rect.x + rect.w - margin,
                  rect.y + rect.h - margin,
                  NULL,
                  NULL);
        draw_text(font,
                  "Close [f]",
                  (SDL_Color){0, 91, 0, 255},
                  0, 1,
                  rect.x + margin,
                  rect.y + rect.h - margin,
                  NULL,
                  NULL);
    }
    
    overlay_draw(&gui.overlay);

    text_field_draw();
    
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
            SDL_Color col = pixel_from_index(level->desired_grid, x+y*gw);
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
// Gets cells based on pixels in the image.
//
// path - path to the image
// out  - pointer to array of Cells (it's allocated in this function)
// source_cells - pointer to a bunch of source cells (NULL if don't want). Must not be heap allocated.
// out_source_cell_count - Pointer to the cell count. Updates in this func.
// out_w & out_h - width and height of the image.
void level_get_cells_from_image(char *path, struct Cell **out, struct Source_Cell *source_cells, int *out_source_cell_count, int *out_w, int *out_h) {
    SDL_Surface *surface = IMG_Load(path);
    SDL_assert(surface);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_assert(texture);

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

            int cell;

            if (r == 255 && g == 0 && b == 0) {
                struct Source_Cell *s = &source_cells[(*out_source_cell_count)++];
                s->x = x;
                s->y = y;
                s->type = CELL_STEAM;
            } else if (r == 255 && g == 255 && b == 255) {
                cell = CELL_MARBLE;
            } else if (r == 128 && g == 128 && b == 128) {
                cell = CELL_COBBLESTONE;
            } else if (r == 200 && r == 200 && g == 200) {
                cell = CELL_QUARTZ;
            } else if (r == 128 && g == 80 && b == 0) {
                cell = CELL_WOOD_LOG;
            } else if (r == 200 && g == 80 && b == 0) {
                cell = CELL_WOOD_PLANK;
            } else if (r == 200 && g == 0 && b == 0) {
                cell = CELL_DIRT;
            } else if (r == 255 && g == 255 && b == 0) {
                cell = CELL_SAND;
            } else if (r == 180 && g == 180 && b == 180) {
                cell = CELL_GLASS;
            } else if (r == 0 && g == 0 && b == 255) {
                cell = CELL_WATER;
            } else if (r == 50 && g == 50 && b == 50) {
                cell = CELL_COAL;
            } else if (0) {
                cell = CELL_STEAM;
            } else if (r == 150 && g == 200 && b == 200) {
                cell = CELL_DIAMOND;
            } else if (r == 188 && g == 255 && b == 255) {
                cell = CELL_ICE;
            } else if (r == 0 && g == 255 && b == 0) {
                cell = CELL_LEAF;
            } else if (0) {
                cell = CELL_SMOKE;
            } else if (0) {
                cell = CELL_DUST;
            } else if (r == 0 && g == 0 && b == 0) {
                cell = CELL_NONE;
            } else {
                fprintf(stderr, "Unknown color (%d, %d, %d) at (%d, %d) in file %s.\n", r, g, b, x, y, path);
                fflush(stderr);
                exit(1);
            }
            (*out)[x+y*w].type = cell;
        }
    }

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

Uint8 type_to_rgb_table[CELL_COUNT*4] = {
    // Type              R    G    B
    CELL_NONE,            0,   0,   0,
    CELL_MARBLE,        255, 255, 255,
    CELL_COBBLESTONE,   128, 128, 128,
    CELL_QUARTZ,        200, 200, 200,
    CELL_WOOD_LOG,      128,  80,   0,
    CELL_WOOD_PLANK,    200,  80,   0,
    CELL_DIRT,          200,   0,   0,
    CELL_SAND,          255, 255,   0,
    CELL_GLASS,         180, 180, 180,
    CELL_WATER,           0,   0, 255,
    CELL_COAL,           50,  50,  50,
    CELL_STEAM,         225, 225, 225,
    CELL_DIAMOND,       150, 200, 200,
    CELL_ICE,           188, 255, 255,
    CELL_LEAF,            0, 255,   0,
    CELL_SMOKE,         170, 170, 170,
    CELL_DUST,          150, 150, 150
};

SDL_Color type_to_rgb(int type) {
    SDL_assert(type < CELL_COUNT);
    return (SDL_Color){type_to_rgb_table[4*type+1], type_to_rgb_table[4*type+2], type_to_rgb_table[4*type+3], 255};
}

int rgb_to_type(Uint8 r, Uint8 g, Uint8 b) {
    for (int i = 0; i < CELL_COUNT; i++) {
        SDL_Color col = type_to_rgb(i);
        if (col.r == r && col.g == g && col.b == b) {
            return i;
        }
    }
    return CELL_NONE;
}

void level_output_to_png(const char *output_file) {
    SDL_Surface *surf = SDL_CreateRGBSurface(0, gw, gh, 32, 0, 0, 0, 0);

    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            int type = grid[x+y*gw].type;

            SDL_Color color = type_to_rgb(type);
            Uint32 pixel = SDL_MapRGB(surf->format, color.r, color.g, color.b);
            set_pixel(surf, x, y, pixel);
        }
    }

    SDL_assert(IMG_SavePNG(surf, output_file) == 0);
}
