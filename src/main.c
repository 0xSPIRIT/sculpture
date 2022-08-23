#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "grid.h"
#include "globals.h"
#include "util.h"
#include "chisel.h"
#include "knife.h"
#include "point_knife.h"
#include "blob_hammer.h"
#include "placer.h"
#include "grabber.h"
#include "level.h"
#include "cursor.h"
#include "gui.h"
#include "chisel_blocker.h"
#include "input.h"

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    
    font = TTF_OpenFont("../res/cour.ttf", 19);
    title_font = TTF_OpenFont("../res/cour.ttf", 45);
    
    srand(time(0));
    
    normal_cursor = SDL_GetCursor();
    grabber_cursor = init_system_cursor();
    
    window = SDL_CreateWindow("Alaska",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              window_width,
                              window_height,
                              0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    render_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, window_width/S, window_height/S);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    levels_setup();

    bool running = true;
    
    level_draw();

    while (running) {
        input_tick();

        SDL_Event event;
        int prev_tool = current_tool;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_MOUSEWHEEL) {
                if (gui.popup) {
                    if (is_point_in_rect((SDL_Point){mx, my}, (SDL_Rect){converter.x, converter.y, converter.w, converter.h})) {
                        if (event.wheel.y > 0) {
                            converter.heat_state = HEAT_HOT;
                        } else if (event.wheel.y < 0) {
                            converter.heat_state = HEAT_COLD;
                        }
                        converter.heat_state = clampf(converter.heat_state, -1, 1);
                    }
                } else if (current_tool == TOOL_PLACER) {
                    struct Placer *placer = placers[current_placer];
                    if (keys[SDL_SCANCODE_LCTRL]) {
                        placer->contains_type += event.wheel.y;
                        if (placer->contains_type < CELL_NONE+1) placer->contains_type = CELL_NONE+1;
                        if (placer->contains_type >= CELL_MAX) placer->contains_type = CELL_MAX-1;
                    } else {
                        placer->radius += event.wheel.y;
                        placer->radius = clamp(placer->radius, 1, 5);
                    }
                }
            }
           
            int selected_tool = 0;
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
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
                case SDLK_o:
                    do_draw_objects = !do_draw_objects;
                    break;
                case SDLK_u:
                    objects_reevaluate();
                    break;
                case SDLK_d:
                    debug_mode = 1;
                    break;
                case SDLK_0: {
                    print_blob_data(&objects[0], chisel->size);
                    break;
                };
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

                    printf("Cell %d, %d: Type: %s, Rand: %d, Object: %d, Time: %d, Vx: %f, Vy: %f, Blob: %u\n",
                           mx,
                           my,
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
            if (selected_tool) {
                gui.tool_buttons[current_tool]->on_pressed(gui.tool_buttons[current_tool]->index);
                gui.tool_buttons[current_tool]->activated = 1;
            }
        }
        
        if (prev_tool != current_tool) {
            gui.overlay.x = gui.overlay.y = -1;
        }

        level_tick();
        level_draw();
    }
    
    levels_deinit();
    
    TTF_CloseFont(font);
    TTF_CloseFont(title_font);
    
    grid_deinit();
    SDL_DestroyTexture(render_tex);
    
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
