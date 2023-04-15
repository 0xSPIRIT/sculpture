#include "shared.h"

// Include all files to compile in one translation unit for
// compilation speed's sake. ("Unity Build")
#include "util.c"
#include "fades.c"
#include "overlay.c"
#include "conversions.c"
#include "dust.c"
#include "grid.c"
#include "credits.c"
#include "undo.c"
#include "chisel.c"
#include "preview.c"
#include "tooltip.c"
#include "placer.c"
#include "inventory.c"
#include "gui.c"
#include "tutorial.c"
#include "overlay_interface.c"
#include "effects.c"
#include "grabber.c"
#include "deleter.c"
#include "popup.c"
#include "timelapse.c"
#include "narrator.c"
#include "3d.c"
#include "level.c"
#include "titlescreen.c"

void game_resize(int h) {
    gs->window_height = h;
    gs->window_width = h;
    gs->S = round(6.0 * h/1080.0);
}

export void game_init(struct Game_State *state, int level) {
    gs = state;
    
    gs->view.x = 0;
    gs->view.y = 0;
    gs->view.w = gs->window_width;
    gs->view.h = gs->window_height-GUI_H;
    
    gs->show_tutorials = true;
    
    conversions_gui_init();
    
    levels_setup();
    
    previews_load();
    
    goto_level(level);
    
    titlescreen_init();
    
#ifndef ALASKA_RELEASE_MODE
    gs->gamestate = GAME_STATE_PLAY;
#else
    gs->gamestate = GAME_STATE_TITLESCREEN;
    if (Mix_PlayMusic(gs->audio.music_titlescreen, -1) == -1) {
        Log("%s\n", SDL_GetError());
        exit(1);
    }
#endif
}

export bool game_tick_event(struct Game_State *state, SDL_Event *event) {
    gs = state;
    gs->event = event;
    
    bool is_running = true;
    struct Input *input = &gs->input;
    
    if (event->type == SDL_QUIT) {
        is_running = false;
    }
    
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
        gs->real_width = event->window.data1;
        gs->real_height = event->window.data2;
        //game_resize(gs->real_height);
    }
    
    if (event->type == SDL_MOUSEWHEEL) {
        if (gs->current_tool == TOOL_PLACER) {
            struct Placer *placer = &gs->placers[gs->current_placer];
            if (input->keys[SDL_SCANCODE_LCTRL] && gs->creative_mode) {
                placer->contains->type += event->wheel.y;
                if (placer->contains->type < CELL_NONE+1) placer->contains->type = CELL_NONE+1;
                if (placer->contains->type >= CELL_TYPE_COUNT) placer->contains->type = CELL_TYPE_COUNT-1;
            }
        }
    }
    
    int selected_tool = 0;
    if (event->type == SDL_KEYDOWN && gs->gamestate == GAME_STATE_PLAY && !gs->text_field.active) {
        switch (event->key.keysym.sym) {
            case SDLK_ESCAPE: {
                struct Placer *placer = get_current_placer();
                if (gs->credits.state == CREDITS_SHOW) {
                    is_running = false;
                } else if (gs->tutorial.active) {
                    tutorial_rect_close(NULL);
                } else if (placer && placer->state == PLACER_PLACE_RECT_MODE && input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                    placer->escape_rect = true;
                    placer->rect.x = -1;
                    placer->rect.y = -1;
                    placer->rect.w = 0;
                    placer->rect.h = 0;
                } else {
#ifdef ALASKA_DEBUG
                    is_running = false; 
#endif
                }
                break;
            }
            case SDLK_F11: {
                if (!gs->fullscreen) {
                    SDL_SetWindowFullscreen(gs->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    gs->fullscreen = true;
                } else {
                    SDL_SetWindowFullscreen(gs->window, 0);
                    gs->fullscreen = false;
                }
                break;
            }
            case SDLK_F12: {
                int is_on = SDL_ShowCursor(SDL_QUERY);
                if (is_on == SDL_ENABLE) {
                    SDL_ShowCursor(SDL_DISABLE);
                } else {
                    SDL_ShowCursor(SDL_ENABLE);
                }
                break;
            }
            case SDLK_BACKQUOTE: {
#ifdef ALASKA_DEBUG
                gs->creative_mode = !gs->creative_mode;
                if (gs->creative_mode) {
                    gui_message_stack_push("Creative Mode: On");
                } else {
                    gui_message_stack_push("Creative Mode: Off");
                }
#endif
                break;
            }
            case SDLK_SPACE: {
                //gs->paused = !gs->paused;
                break;
            }
            case SDLK_n: {
                gs->step_one = 1;
                break;
            }
            case SDLK_SEMICOLON: {
                //gs->do_draw_objects = !gs->do_draw_objects;
                break;
            }
            case SDLK_r: {
                if (gs->input.keys[SDL_SCANCODE_LCTRL]) {
                    goto_level(gs->level_current);
                }
                break;
            }
#if ALASKA_DEBUG
            case SDLK_e: {
                if (gs->input.keys[SDL_SCANCODE_LCTRL]) {
                    for (int i = 0; i < gs->gw*gs->gh; i++)
                        gs->grid[i].type = gs->grid[i].object = 0;
                }
                break;
            }
#endif
            case SDLK_b: {
                gs->do_draw_blobs = !gs->do_draw_blobs;
                break;
            }
#if ALASKA_DEBUG
            case SDLK_g: {
                if (input->keys[SDL_SCANCODE_LCTRL]) {
                    set_text_field("Goto Level", "", goto_level_string_hook);
                } else {
                    //Mix_PlayChannel(-1, gs->audio.chisel[rand()%6], 0);
                }
                break;
            }
#endif
            case SDLK_o: {
                if (input->keys[SDL_SCANCODE_LCTRL]) {
                    set_text_field("Output current grid to image:", "../", level_output_to_png);
                } else {
                    gs->overlay.show = !gs->overlay.show;
                    gs->gui.tool_buttons[TOOL_OVERLAY]->highlighted = false;
                }
                break;
            }
            case SDLK_u: {
                objects_reevaluate();
                break;
            }
            case SDLK_z: {
                if (input->keys[SDL_SCANCODE_LCTRL]) {
                    if (input->keys[SDL_SCANCODE_LSHIFT]) {
                        set_text_field("Go to State", "", set_state_to_string_hook);
                    } else
                        undo();
                } else
                    undo();
                break;
            }
            case SDLK_q: {
                struct Cell *c;
                c = &gs->grid[input->mx+input->my*gs->gw];
                char name[256] = {0};
                get_name_from_type(c->type, name);
                
                int obj = c->object;
                if (obj == -1) obj = 0;
                
                Assert(obj != -1);
                
                
                Log("Cell %d, %d: Pos: (%f, %f), Type: %s, ID: %d, Rand: %d, Object: %d, Time: %d, Vx: %f, Vy: %f, Blob: %u\n",
                    input->mx,
                    input->my,
                    c->vx_acc,
                    c->vy_acc,
                    name,
                    c->id,
                    c->rand,
                    c->object,
                    c->time,
                    c->vx,
                    c->vy,
                    gs->objects[obj].blob_data[gs->chisel->size].blobs[input->mx+input->my*gs->gw]);
                break;
            }
            case SDLK_1: {
                if (!gs->gui.tool_buttons[TOOL_CHISEL_SMALL]->disabled) {
                    gs->current_tool = TOOL_CHISEL_SMALL;
                    gs->chisel = &gs->chisel_small;
                    for (int i = 0; i < gs->object_count; i++)
                        object_generate_blobs(i, 0);
                    gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+2;
                    selected_tool = 1;
                }
                break;
            }
            case SDLK_2: {
                if (!gs->gui.tool_buttons[TOOL_CHISEL_MEDIUM]->disabled) {
                    gs->current_tool = TOOL_CHISEL_MEDIUM;
                    gs->chisel = &gs->chisel_medium;
                    for (int i = 0; i < gs->object_count; i++)
                        object_generate_blobs(i, 1);
                    gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+4;
                    selected_tool = 1;
                }
                break;
            }
            case SDLK_3: {
                if (!gs->gui.tool_buttons[TOOL_CHISEL_LARGE]->disabled) {
                    gs->current_tool = TOOL_CHISEL_LARGE;
                    gs->chisel = &gs->chisel_large;
                    for (int i = 0; i < gs->object_count; i++)
                        object_generate_blobs(i, 2);
                    gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+4;
                    selected_tool = 1;
                }
                break;
            }
#if 0
            case SDLK_4: {
                gs->current_tool = TOOL_BLOCKER;
                selected_tool = 1;
                break;
            }
#endif
            case SDLK_4: {
                gs->current_tool = TOOL_OVERLAY;
                selected_tool = 1;
                break;
            }
            case SDLK_5: {
                gs->current_tool = TOOL_DELETER;
                selected_tool = 1;
                break;
            }
            case SDLK_6: {
                gs->current_tool = TOOL_PLACER;
                selected_tool = 1;
                break;
            }
            case SDLK_7: {
                gs->current_tool = TOOL_GRABBER;
                selected_tool = 1;
                break;
            }
            
            case SDLK_F1: case SDLK_F2: case SDLK_F3: case SDLK_F4: case SDLK_F5: {
                gs->current_placer = event->key.keysym.sym - SDLK_F1;
                break;
            }
        }
    }
    
    text_field_tick();
    
    if (selected_tool) {
        gs->gui.tool_buttons[gs->current_tool]->on_pressed(&gs->gui.tool_buttons[gs->current_tool]->index);
        gs->gui.tool_buttons[gs->current_tool]->active = true;
    }
    
    return is_running;
}

export void game_run(struct Game_State *state) {
    gs = state;
    
    gs->gui.tooltip.set_this_frame = false;
    
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_MASTER));
    
    switch (gs->gamestate) {
        case GAME_STATE_TITLESCREEN: {
            titlescreen_tick();
            titlescreen_draw();
            break;
        }
        case GAME_STATE_PLAY: {
            if (gs->obj.active) {
                SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
                SDL_RenderClear(gs->renderer);
                
                object_draw(&gs->obj);
                fade_draw();
                
                SDL_Rect dst = {
                    0, 0,
                    gs->gw*gs->S, GUI_H+SCALE_3D
                };
                
                SDL_SetTextureAlphaMod(RenderTarget(RENDER_TARGET_GUI_TOOLBAR), 255 - 255 * min(240,gs->obj.t) / 240.0);
                SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GUI_TOOLBAR), NULL, &dst);
            } else {
                //view_tick(&gs->view, &gs->input);
                
                gui_tick();
                conversions_gui_tick();
                inventory_tick();
                all_converters_tick();
                
                level_tick();
                level_draw();
                
                fade_draw();
                
                preview_tick();
                if (gs->current_preview.play)
                    preview_draw(&gs->current_preview, 0, 0, 6);
            }
            break;
        }
    }
    
    SDL_SetRenderTarget(gs->renderer, NULL);
    
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
    SDL_RenderClear(gs->renderer);
    
    SDL_Rect dst = {
        gs->real_width/2 - gs->window_width/2,
        gs->real_height/2 - gs->window_height/2,
        gs->window_width,
        gs->window_height
    };
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_MASTER), NULL, &dst);
    
    SDL_RenderPresent(gs->renderer);
    
    gs->is_mouse_over_any_button = false;
}
