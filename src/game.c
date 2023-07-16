#include "shared.h"

// Include all files to compile in one translation unit for
// compilation speed's sake. ("Unity Build")
#include "util.c"
#include "render.c"
#include "fades.c"
#include "overlay.c"
#include "converter.c"
#include "dust.c"
#include "grid.c"
#include "credits.c"
#include "undo.c"
#include "hammer.c"
#include "converter_gui.c"
#include "chisel.c"
#include "preview.c"
#include "tooltip.c"
#include "placer.c"
#include "inventory.c"
#include "gui.c"
#include "confirm_popup.c"
#include "tutorial.c"
#include "overlay_interface.c"
#include "effects.c"
#include "grabber.c"
#include "popup.c"
#include "timelapse.c"
#include "narration.c"
#include "3d.c"
#include "level.c"
#include "titlescreen.c"
#include "background.c"
#include "audio.c"

static void game_resize(int h) {
    gs->gui.popup_y /= gs->gh*gs->S;

    gs->S = h / 72.0;

    gs->window_width = gs->S*64.0;
    gs->window_height = gs->S*64.0 + GUI_H;

    gs->gui.popup_y *= gs->gh*gs->S;

    gs->render.view.x = gs->render.to.x = 0;
    gs->render.view.y = gs->render.to.y = 0;
    gs->render.view.w = gs->window_width;
    gs->render.view.h = gs->window_height-GUI_H;

    gs->real_top_left.x = gs->window_width/4;
    gs->real_top_left.y = gs->window_height/4;

    for (int i = 0; i < FONT_COUNT; i++) {
        RenderSetFontSize(gs->fonts.fonts[i], Scale(font_sizes[i]));
    }

    gs->resized = true;

    SDL_DestroyTexture(RenderTarget(RENDER_TARGET_3D)->texture.handle);
    RenderTarget(RENDER_TARGET_3D)->texture.handle = SDL_CreateTexture(gs->renderer,
                                                                       ALASKA_PIXELFORMAT,
                                                                       SDL_TEXTUREACCESS_STREAMING,
                                                                       SCALE_3D*gs->window_width,
                                                                       SCALE_3D*gs->window_width);
    RenderTarget(RENDER_TARGET_3D)->texture.width = SCALE_3D * gs->window_width;
    RenderTarget(RENDER_TARGET_3D)->texture.height = SCALE_3D * gs->window_width;

    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_3D)->texture.handle, SDL_BLENDMODE_BLEND);
}

static void game_update_view(void) {
    Input *input = &gs->input;
    SDL_FPoint *to = &gs->render.to;

    if (input->keys_pressed[SDL_SCANCODE_D] || input->keys_pressed[SDL_SCANCODE_RIGHT])
        to->x += gs->window_width*0.25;
    if (input->keys_pressed[SDL_SCANCODE_A] || input->keys_pressed[SDL_SCANCODE_LEFT])
        to->x -= gs->window_width*0.25;
    to->x = clampf(to->x, -gs->window_width*0.5, gs->window_width*0.5);

    // NOTE: This is not a typo, they are supposed to be width, not height.
    if (input->keys_pressed[SDL_SCANCODE_S] || input->keys_pressed[SDL_SCANCODE_DOWN])
        to->y += gs->window_width*0.25;
    if (input->keys_pressed[SDL_SCANCODE_W] || input->keys_pressed[SDL_SCANCODE_UP])
        to->y -= gs->window_width*0.25;
    to->y = clampf(to->y, -gs->window_width*0.5, gs->window_width*0.5);

    gs->render.view.x = lerp64(gs->render.view.x, gs->render.to.x, 0.2);
    gs->render.view.y = lerp64(gs->render.view.y, gs->render.to.y, 0.2);
    if (abs(gs->render.view.x - gs->render.to.x) <= 1) gs->render.view.x = gs->render.to.x;
    if (abs(gs->render.view.y - gs->render.to.y) <= 1) gs->render.view.y = gs->render.to.y;
}

export void game_init(Game_State *state, int level) {
    gs = state;

    gs->render.view.x = 0;
    gs->render.view.y = 0;
    gs->render.view.w = gs->window_width;
    gs->render.view.h = gs->window_height-GUI_H;

    gs->real_top_left.x = gs->window_width/4;
    gs->real_top_left.y = gs->window_height/4;

    gs->show_tutorials = true;

    levels_setup();
    previews_load();
    goto_level(level);
    titlescreen_init();
    
#ifdef ALASKA_RELEASE_MODE
    #define SHOW_TITLESCREEN 1
#else
    #define SHOW_TITLESCREEN 0
#endif

#if SHOW_TITLESCREEN
    gs->gamestate = GAME_STATE_TITLESCREEN;

    Mix_VolumeMusic(AUDIO_TITLESCREEN_VOLUME);
    #ifdef ALASKA_RELEASE_MODE
    Assert(Mix_PlayMusic(gs->audio.music_titlescreen, -1) != -1);
    #endif
#else
    gs->gamestate = GAME_STATE_PLAY;
#endif
}

export bool game_tick_event(Game_State *state, SDL_Event *event) {
    gs = state;
    gs->event = event;

    bool is_running = true;
    Input *input = &gs->input;

    if (event->type == SDL_QUIT) {
        is_running = false;
    }

    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
        gs->real_width = event->window.data1;
        gs->real_height = event->window.data2;
        game_resize(gs->real_height);
    }

    if (event->type == SDL_MOUSEWHEEL) {
        if (gs->conversions.active) {
            gs->conversions.y_to += Scale(50) * event->wheel.y;
            gs->conversions.y_to = clamp(gs->conversions.y_to,
                                         -gs->conversions.max_height,
                                         0);
        } else if (gs->current_tool == TOOL_PLACER) {
            Placer *placer = &gs->placers[gs->current_placer];
            if (input->keys[SDL_SCANCODE_LCTRL] && gs->creative_mode) {
                placer->contains->type += event->wheel.y;
                if (placer->contains->type < CELL_NONE+1) placer->contains->type = CELL_NONE+1;
                if (placer->contains->type >= CELL_TYPE_COUNT) placer->contains->type = CELL_TYPE_COUNT-1;
            } else {
                placer->place_width += event->wheel.y;
                placer->place_width = clamp(placer->place_width, 8, 20);
                placer->place_height = placer->place_width * placer->place_aspect;
            }
        }
    }

    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_F11 && !gs->obj.active) {
        if (!gs->fullscreen) {
            SDL_SetWindowFullscreen(gs->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            gs->fullscreen = true;
        } else {
            SDL_SetWindowFullscreen(gs->window, 0);
            gs->fullscreen = false;
        }
    }

    int selected_tool = 0;
    if (event->type == SDL_KEYDOWN && gs->gamestate == GAME_STATE_PLAY && !gs->text_field.active) {
        switch (event->key.keysym.sym) {
            case SDLK_ESCAPE: {
                Placer *placer = get_current_placer();
                if (gs->credits.state == CREDITS_SHOW) {
                    is_running = false;
                } else if (gs->tutorial.active) {
                    tutorial_rect_close(null);
                } else if (placer && placer->state == PLACER_PLACE_RECT_MODE && input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                    placer->escape_rect = true;
                    placer->rect.x = -1;
                    placer->rect.y = -1;
                    placer->rect.w = 0;
                    placer->rect.h = 0;
                } else {
#ifndef ALASKA_RELEASE_MODE
                    is_running = false;
#endif
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
#ifndef ALASKA_RELEASE_MODE
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
#ifndef ALASKA_RELEASE_MODE
                if (gs->input.keys[SDL_SCANCODE_LSHIFT])
                    gs->paused = !gs->paused;
#endif
                break;
            }
            case SDLK_n: {
                gs->step_one = 1;
                break;
            }
            case SDLK_SEMICOLON: {
                gs->do_draw_objects = !gs->do_draw_objects;
                break;
            }
            case SDLK_r: {
                popup_confirm_activate(&gs->gui.restart_popup_confirm);
                break;
            }
#ifndef ALASKA_RELEASE_MODE
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
            case SDLK_g: {
                if (input->keys[SDL_SCANCODE_LCTRL] || input->keys[SDL_SCANCODE_RCTRL]) {
                    set_text_field("Goto Level", "", goto_level_string_hook);
                } else {
                    //Mix_PlayChannel(-1, gs->audio.chisel[rand()%6], 0);
                }
                break;
            }
            case SDLK_4: case SDLK_o: {
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
                Cell *c;
                c = &gs->grid[input->mx+input->my*gs->gw];
                char name[256] = {0};
                get_name_from_type(c->type, name);

                int obj = c->object;
                if (obj == -1) obj = 0;

                Assert(obj != -1);


                Log("Cell %d, %d: Pos: (%f, %f), Type: %s, ID: %d, Rand: %d, Object: %d, Time: %d, Vx: %f, Vy: %f\n",
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
                    c->vy);
                break;
            }
            case SDLK_1: {
                if (!gs->gui.tool_buttons[TOOL_CHISEL_SMALL]->disabled) {
                    gs->current_tool = TOOL_CHISEL_SMALL;
                    gs->chisel = &gs->chisel_small;
                    //gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+2;
                    selected_tool = 1;
                }
                break;
            }
            case SDLK_2: {
                if (!gs->gui.tool_buttons[TOOL_CHISEL_MEDIUM]->disabled) {
                    gs->current_tool = TOOL_CHISEL_MEDIUM;
                    gs->chisel = &gs->chisel_medium;
                    //gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+4;
                    selected_tool = 1;
                }
                break;
            }
            case SDLK_3: {
                if (!gs->gui.tool_buttons[TOOL_CHISEL_LARGE]->disabled) {
                    gs->current_tool = TOOL_CHISEL_LARGE;
                    gs->chisel = &gs->chisel_large;
                    //gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+4;
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
            case SDLK_5: {
                gs->current_tool = TOOL_GRABBER;
                selected_tool = 1;
                break;
            }
            case SDLK_6: {
                gs->current_tool = TOOL_PLACER;
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

export void game_run(Game_State *state) {
    LARGE_INTEGER frequency, time_start, time_end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&time_start);

    gs = state;

    gs->gui.tooltip.set_this_frame = false;

    switch (gs->gamestate) {
        case GAME_STATE_TITLESCREEN: {
            titlescreen_tick();
            titlescreen_draw(RENDER_TARGET_MASTER);
            break;
        }
        case GAME_STATE_PLAY: {
            game_update_view();

            if (gs->obj.active) {
                RenderColor(255, 255, 255, 255);
                RenderClear(RENDER_TARGET_MASTER);

                object_draw(&gs->obj);
                fade_draw(RENDER_TARGET_MASTER);

                SDL_Rect dst = {
                    0, 0,
                    RenderTarget(RENDER_TARGET_GUI_TOOLBAR)->working_width,
                    RenderTarget(RENDER_TARGET_GUI_TOOLBAR)->working_height
                };

                Uint8 alpha = 255 - 255 * min(240,gs->obj.t) / 240.0;
                RenderTextureAlphaMod(&RenderTarget(RENDER_TARGET_GUI_TOOLBAR)->texture, alpha);
                RenderMaybeSwitchToTarget(RENDER_TARGET_MASTER);
                SDL_RenderCopy(gs->render.sdl,
                               RenderTarget(RENDER_TARGET_GUI_TOOLBAR)->texture.handle,
                               null,
                               &dst);
            } else {
                //view_tick(&gs->view, &gs->input);

                gui_tick();
                inventory_tick();
                all_converters_tick();
                
                audio_set_ambience_accordingly();
                
                level_tick(&gs->levels[gs->level_current]);
                level_draw(&gs->levels[gs->level_current]);
                text_field_draw(RENDER_TARGET_MASTER);

                fade_draw(RENDER_TARGET_MASTER);

                preview_tick();
                if (gs->current_preview.play)
                    preview_draw(RENDER_TARGET_MASTER,
                                 &gs->current_preview,
                                 0,
                                 0,
                                 6);

                if (gs->step_one) gs->step_one = false;
            }
            break;
        }
    }

    RenderColor(0, 0, 0, 255);
    RenderClear(-1);

    SDL_Rect src = {
        0, 0,
        gs->window_width, gs->window_height
    };

    SDL_Rect dst = {
        gs->real_width/2 - gs->window_width/2,
        gs->real_height/2 - gs->window_height/2,
        gs->window_width,
        gs->window_height
    };

    SDL_RenderCopy(gs->render.sdl,
                   RenderTarget(RENDER_TARGET_MASTER)->texture.handle,
                   &src,
                   &dst);

    QueryPerformanceCounter(&time_end);
    Uint64 delta = time_end.QuadPart - time_start.QuadPart;
    f64 d = (f64)delta / (f64)frequency.QuadPart;
    gs->dt = d;

    SDL_RenderPresent(gs->renderer);

    gs->is_mouse_over_any_button = false;

    RenderCleanupTextCache(&gs->render.temp_text_cache);
}
