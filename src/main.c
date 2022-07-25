#include <stdio.h>
#include <stdbool.h>
#include <time.h>

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
#include "drill.h"
#include "placer.h"
#include "grabber.h"
#include "level.h"
#include "cursor.h"
#include "gui.h"

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    font = TTF_OpenFont("../res/cour.ttf", 19);
    title_font = TTF_OpenFont("../res/cour.ttf", 45);
    SDL_assert(font);
    SDL_assert(title_font);

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
    while (running) {
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
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                /* case SDLK_p: */
                /*     for (int y = 0; y < gh; y++) { */
                /*         for (int x = 0; x < gw; x++) { */
                /*             printf("%03d ", objects[0].blob_data[1].blobs[x+y*gw]); */
                /*         } */
                /*         printf("\n"); */
                /*     } */
                /*     fflush(stdout); */
                /*     break; */
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
                    printf("Updated!\n"); fflush(stdout);
                    break;
                case SDLK_q:
                    printf("Cell %d: Type: %d, Object: %d\n", mx+my*gw, grid[mx+my*gw].type, grid[mx+my*gw].object); fflush(stdout);
                    break;
                case SDLK_i:
                    grid_show_ghost = !grid_show_ghost;
                    printf("%d\n", grid_show_ghost); fflush(stdout);
                    break;
                case SDLK_f:
                    break;
                case SDLK_0:
                    current_tool = TOOL_CHISEL_SMALL;
                    chisel = &chisel_small;
                    for (int i = 0; i < object_count; i++)
                        object_set_blobs(i, 0);
                    hammer.normal_dist = hammer.dist = chisel->w+4;
                    break;
                case SDLK_1:
                    current_tool = TOOL_CHISEL_MEDIUM;
                    chisel = &chisel_medium;
                    for (int i = 0; i < object_count; i++)
                        object_set_blobs(i, 1);
                    hammer.normal_dist = hammer.dist = chisel->w+4;
                    break;
                case SDLK_2:
                    current_tool = TOOL_CHISEL_LARGE;
                    chisel = &chisel_large;
                    for (int i = 0; i < object_count; i++)
                        object_set_blobs(i, 2);
                    hammer.normal_dist = hammer.dist = chisel->w+4;
                    break;
                case SDLK_3:
                    current_tool = TOOL_KNIFE;
                    break;
                case SDLK_4:
                    current_tool = TOOL_POINT_KNIFE;
                    break;
                case SDLK_5:
                    current_tool = TOOL_DRILL;
                    break;
                case SDLK_6:
                    current_tool = TOOL_PLACER;
                    break;
                case SDLK_7:
                    current_tool = TOOL_GRABBER;
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
        }

        if (prev_tool != current_tool) {
            gui.overlay.x = gui.overlay.y = -1;
        }
        
        pmx = mx;
        pmy = my;
        
        mouse = (Uint32) SDL_GetMouseState(&real_mx, &real_my);
        keys = (Uint8*) SDL_GetKeyboardState(NULL);

        mx = real_mx/S;
        my = real_my/S;

        my -= GUI_H/S;

        level_tick();
        level_draw();
    }

    levels_deinit();
    gui_deinit();
    drill_deinit();
    chisel_deinit(&chisel_small);
    chisel_deinit(&chisel_medium);
    chisel_deinit(&chisel_large);
    hammer_deinit();
    knife_deinit();
    point_knife_deinit();
    for (int i = 0; i < PLACER_COUNT; i++)
        placer_deinit(i);
    grabber_deinit();
    
    TTF_CloseFont(font);
    TTF_CloseFont(title_font);

    grid_deinit();
    SDL_DestroyTexture(render_tex);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
