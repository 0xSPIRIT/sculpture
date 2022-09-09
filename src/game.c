#include "game.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include <stdio.h>

#include "globals.h"
#include "level.h"
#include "chisel.h"
#include "popup.h"
#include "grid.h"
#include "undo.h"
#include "util.h"
#include "placer.h"
#include "gui.h"
#include "grabber.h"
#include "cursor.h"

static void game_init_sdl(const char *window_title, int w, int h) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    
    window = SDL_CreateWindow(window_title,
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              w,
                              h,
                              0);

    // This function takes ~0.25 seconds.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

    render_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, window_width/S, window_height/S);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void game_init(void) {
    srand(time(0));

    game_init_sdl("Alaska", window_width, window_height);
    
    fonts_init();
    
    normal_cursor = SDL_GetCursor();
    grabber_cursor = init_system_cursor(arrow_cursor_data);
    placer_cursor = init_system_cursor(placer_cursor_data);

    item_init();
    levels_setup();

    goto_level(0);
}

void game_deinit() {
    levels_deinit();
    fonts_deinit();
    item_deinit();
    grid_deinit();

    SDL_DestroyTexture(render_tex);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void game_run(void) {
    while (running) {
        SDL_Event event;
        int prev_tool = current_tool;
        
        input_tick();

        while (SDL_PollEvent(&event)) {
            game_tick_event(&event);
        }

        if (prev_tool != current_tool) {
            overlay_reset(&gui.overlay);
        }

        level_tick();
        level_draw();
    }
}

void game_tick_event(SDL_Event *event) {
    if (event->type == SDL_QUIT) {
        running = false;
    }

    if (event->type == SDL_MOUSEWHEEL) {
        if (!gui.popup && current_tool == TOOL_PLACER) {
            struct Placer *placer = placers[current_placer];
            if (keys[SDL_SCANCODE_LCTRL]) {
                placer->contains_type += event->wheel.y;
                if (placer->contains_type < CELL_NONE+1) placer->contains_type = CELL_NONE+1;
                if (placer->contains_type >= CELL_COUNT) placer->contains_type = CELL_COUNT-1;
            } else {
                placer->radius += event->wheel.y;
                placer->radius = clamp(placer->radius, 1, 5);
            }
        }
    }
           
    int selected_tool = 0;
    if (event->type == SDL_KEYDOWN && !text_field.active) {
        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE:
            running = false;
            break;
        case SDLK_SPACE:
            paused = !paused;
            break;
        case SDLK_n:
            step_one = 1;
            break;
        case SDLK_b:
            do_draw_blobs = !do_draw_blobs;
            break;
        case SDLK_g:
            if (keys[SDL_SCANCODE_LCTRL]) {
                set_text_field("Goto Level", "", goto_level_string_hook);
            }
            break;
        case SDLK_o:
            if (keys[SDL_SCANCODE_LCTRL]) {
                set_text_field("Output current grid to image:", "../", level_output_to_png);
            } else {
                do_draw_objects = !do_draw_objects;
            }
            break;
        case SDLK_F5:
            view_save_state_linked_list();
            break;
        case SDLK_u:
            objects_reevaluate();
            break;
        case SDLK_d:
            debug_mode = 1;
            break;
        case SDLK_z:
            if (keys[SDL_SCANCODE_LCTRL]) {
                if (keys[SDL_SCANCODE_LSHIFT]) {
                    set_text_field("Go to State", "", set_state_to_string_hook);
                } else {
                    undo();
                }
            }
            break;
        case SDLK_q: {
            struct Cell *c;
            if (keys[SDL_SCANCODE_LSHIFT]) {
                c = &pickup_grid[mx+my*gw];
            } else {
                c = &grid[mx+my*gw];
            }
            char name[256] = {0};
            get_name_from_type(c->type, name);

            int obj = c->object;
            if (obj == -1) obj = 0;

            SDL_assert(obj != -1);

            printf("Cell %d, %d: Pos: (%f, %f), Type: %s, Rand: %d, Object: %d, Time: %d, Vx: %f, Vy: %f, Blob: %u\n",
                   mx,
                   my,
                   c->vx_acc,
                   c->vy_acc,
                   name,
                   c->rand,
                   c->object,
                   c->time,
                   c->vx,
                   c->vy,
                   objects[obj].blob_data[chisel->size].blobs[mx+my*gw]);
            fflush(stdout);
            break;
        }
        case SDLK_i:
            grid_show_ghost = !grid_show_ghost;
            break;
        case SDLK_1:
            current_tool = TOOL_CHISEL_SMALL;
            chisel = &chisel_small;
            for (int i = 0; i < object_count; i++)
                object_generate_blobs(i, 0);
            chisel_hammer.normal_dist = chisel_hammer.dist = chisel->w+2;
            selected_tool = 1;
            break;
        case SDLK_2:
            current_tool = TOOL_CHISEL_MEDIUM;
            chisel = &chisel_medium;
            for (int i = 0; i < object_count; i++)
                object_generate_blobs(i, 1);
            chisel_hammer.normal_dist = chisel_hammer.dist = chisel->w+4;
            selected_tool = 1;
            break;
        case SDLK_3:
            current_tool = TOOL_CHISEL_LARGE;
            chisel = &chisel_large;
            for (int i = 0; i < object_count; i++)
                object_generate_blobs(i, 2);
            chisel_hammer.normal_dist = chisel_hammer.dist = chisel->w+4;
            selected_tool = 1;
            break;
        case SDLK_4:
            current_tool = TOOL_KNIFE;
            selected_tool = 1;
            break;
        case SDLK_5:
            current_tool = TOOL_POINT_KNIFE;
            selected_tool = 1;
            break;
        case SDLK_6:
            current_tool = TOOL_HAMMER;
            selected_tool = 1;
            break;
        case SDLK_7:
            current_tool = TOOL_PLACER;
            selected_tool = 1;
            break;
        case SDLK_8:
            current_tool = TOOL_GRABBER;
            selected_tool = 1;
            break;
        case SDLK_F1:
            current_placer = 0;
            break;
        case SDLK_F2:
            current_placer = 1;
            break;
        case SDLK_F3:
            current_placer = 2;
            break;
        }
    }

    text_field_tick(event);

    if (selected_tool) {
        gui.tool_buttons[current_tool]->on_pressed(&gui.tool_buttons[current_tool]->index);
        gui.tool_buttons[current_tool]->activated = 1;
    }
}

void fonts_init(void) {
    font = TTF_OpenFont("../res/cour.ttf", 19);
    font_consolas = TTF_OpenFont("../res/consola.ttf", 24);
    small_font = TTF_OpenFont("../res/cour.ttf", 13);
    title_font = TTF_OpenFont("../res/cour.ttf", 45);
}

void fonts_deinit(void) {
    TTF_CloseFont(font);
    TTF_CloseFont(font_consolas);
    TTF_CloseFont(small_font);
    TTF_CloseFont(title_font);
}
