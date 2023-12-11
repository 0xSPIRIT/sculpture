#include "shared.h"

// Include all files to compile in one translation unit for
// compilation speed's sake. (Unity Build)

#include "input.c"
#include "util.c"
#include "lighting.c"
#include "render.c"
#include "fades.c"
#include "overlay.c"
#include "converter.c"
#include "dust.c"
#include "materials.c"
#include "grid.c"
#include "credits.c"
#include "undo.c"
#include "hammer.c"
#include "recipes.c"
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
#include "background.c"
#include "audio.c"
#include "wind.c"
#include "save.c"
#include "titlescreen.c"
#include "pause_menu.c"

static void game_resize(int h) {
    gs->gui.popup_y /= gs->gh*gs->S;

    gs->S = h / 72.0;

    gs->game_width = gs->S*64.0;
    gs->game_height = gs->S*64.0 + GUI_H;

    gs->gui.popup_y *= gs->gh*gs->S;

    gs->render.view.x = gs->render.to.x = 0;
    gs->render.view.y = gs->render.to.y = 0;
    gs->render.view.w = gs->game_width;
    gs->render.view.h = gs->game_height-GUI_H;

    for (int i = 0; i < FONT_COUNT; i++) {
        RenderSetFontSize(gs->fonts.fonts[i], Scale(font_sizes[i]));
    }

    gs->resized = true;
}

static void game_update_view(void) {
    if (gs->text_field.active)  return;
    if (gs->recipes.active) return;

    Input *input = &gs->input;
    SDL_FPoint *to = &gs->render.to;

    f64 amount = gs->game_width*0.25;

    bool changed = false;

    if (input->keys_pressed[SDL_SCANCODE_D] || input->keys_pressed[SDL_SCANCODE_RIGHT]) {
        to->x += amount;
        changed = true;
    }
    if (input->keys_pressed[SDL_SCANCODE_A] || input->keys_pressed[SDL_SCANCODE_LEFT]) {
        to->x -= amount;
        changed = true;
    }

    to->x = clampf(to->x, -gs->game_width*0.5, gs->game_width*0.5);

    if (input->keys_pressed[SDL_SCANCODE_S] || input->keys_pressed[SDL_SCANCODE_DOWN]) {
        to->y += amount;
        changed = true;
    }
    if (input->keys_pressed[SDL_SCANCODE_W] || input->keys_pressed[SDL_SCANCODE_UP]) {
        to->y -= amount;
        changed = true;
    }

    if (changed) {
        gs->wasd_popup_alpha--;
    }

    // NOTE: This is not a typo, they are supposed to be width, not height.
    to->y = clampf(to->y, -gs->game_width*0.5, gs->game_width*0.5);

    gs->render.view.x = lerp64(gs->render.view.x, gs->render.to.x, 0.2);
    gs->render.view.y = lerp64(gs->render.view.y, gs->render.to.y, 0.2);
    if (fabsf(gs->render.view.x - gs->render.to.x) <= 1) gs->render.view.x = gs->render.to.x;
    if (fabsf(gs->render.view.y - gs->render.to.y) <= 1) gs->render.view.y = gs->render.to.y;
}

static bool can_activate_pause_menu(void) {
    bool result;

    result = !gs->gui.popup;
    result &= gs->levels[gs->level_current].state != LEVEL_STATE_OUTRO;
    result &= !gs->gui.eol_popup_confirm.active;
    result &= !gs->gui.restart_popup_confirm.active;

    return result;
}

export void game_init(Game_State *state) {
    gs = state;

    load_game();
    setup_winds(&state->wind);

    audio_handler_init();

    pause_menu_init(&state->pause_menu);

    gs->border_color = (SDL_Color){0,0,0};

    gs->render.view.x = 0;
    gs->render.view.y = 0;
    gs->render.view.w = gs->game_width;
    gs->render.view.h = gs->game_height-GUI_H;

    gs->show_tutorials = true;
    gs->show_icons = true;

    levels_setup();
    previews_load();

    // If the save file was deleted on the titlescreen,
    // this will be called again.
    // The only reason we call it here as well is because
    // we want to initialize some important memory, notably
    // gs->gw and gs->gh to be used in render_targets_init().
    goto_level(gs->level_current);

    titlescreen_init();

#if SHOW_TITLESCREEN
    gs->gamestate = GAME_STATE_TITLESCREEN;

    Mix_VolumeMusic(AUDIO_TITLESCREEN_VOLUME);
    if (Mix_PlayMusic(gs->audio.music_titlescreen, -1))
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failure!", "Failed to play music!", null);
#else
    gs->gamestate = GAME_STATE_PLAY;
#endif
}

export bool game_handle_event(Game_State *state, SDL_Event *event) {
    gs = state;
    gs->event = event;

    bool is_running = true;
    Input *input = &gs->input;

    if (event->type == SDL_QUIT) {
        save_game();
        is_running = false;
    }

#if SIMULATE_MOUSE
    if (event->type == SDL_MOUSEMOTION) {
        input_tick_mouse(gs, event);
        gs->input.em_got_mouse_motion_event_this_frame = true;
        goto event_tick_end;
    }
#endif

    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
        gs->real_width = event->window.data1;
        gs->real_height = event->window.data2;
        if (gs->real_width == 0 || gs->real_height == 0) __debugbreak();
        game_resize(gs->real_height);
    }

    if (event->type == SDL_MOUSEWHEEL) {
        if (gs->recipes.active) {
            gs->recipes.y_to += Scale(50) * event->wheel.y;
            gs->recipes.y_to = clamp(gs->recipes.y_to,
                                     -gs->recipes.max_height,
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
        } else if (is_tool_chisel()) {
            gs->current_tool += event->wheel.y;
            gs->current_tool = clamp(gs->current_tool, TOOL_CHISEL_SMALL, TOOL_CHISEL_LARGE);

            gs->chisel = &gs->chisels[gs->current_tool - TOOL_CHISEL_SMALL];

            gs->gui.tool_buttons[gs->current_tool]->on_pressed(&gs->gui.tool_buttons[gs->current_tool]->index);
            gs->gui.tool_buttons[gs->current_tool]->active = true;

        }
    }

#ifndef __EMSCRIPTEN__
    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_F11) {
        if (!gs->fullscreen) {
//#ifdef __EMSCRIPTEN__
            //SDL_SetWindowFullscreen(gs->window, SDL_WINDOW_FULLSCREEN);
//#else
            SDL_SetWindowFullscreen(gs->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
//#endif
            gs->fullscreen = true;
        } else {
            SDL_SetWindowFullscreen(gs->window, 0);
            gs->fullscreen = false;
        }
    }
#endif

    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_BACKSPACE) {
            if (gs->input.keys[SDL_SCANCODE_LCTRL]) gs->draw_fps = !gs->draw_fps;
        }
    }

    int selected_tool = 0;
    if (event->type == SDL_KEYDOWN && gs->gamestate == GAME_STATE_PLAY && !gs->text_field.active) {
        switch (event->key.keysym.sym) {
            case SDLK_ESCAPE: {
                if (gs->credits.state == CREDITS_END) {
#ifndef __EMSCRIPTEN__
                    is_running = false;
#endif
                } else if (gs->tutorial.active) {
                    tutorial_rect_close(null);
                } else if (can_activate_pause_menu()) {
                    gs->pause_menu.active = !gs->pause_menu.active;
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
#ifndef __EMSCRIPTEN__
                if (gs->credits.state == CREDITS_END) is_running = false;
#endif
                break;
            }
            //case SDLK_MINUS: {
                //wind_stream_activate(&gs->wind);
            //} break;
            case SDLK_RETURN: {
#ifndef __EMSCRIPTEN__
                if (gs->credits.state == CREDITS_END) is_running = false;
#endif
            } break;
            case SDLK_TAB: {
#ifndef __EMSCRIPTEN__
                if (gs->credits.state == CREDITS_END) is_running = false;
#endif
            } break;
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
            case SDLK_i: {
                gs->show_icons = !gs->show_icons;
            } break;
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
                    //play_sound(-1, gs->audio.chisel[rand()%6], 0);
                }
                break;
            }
            case SDLK_h: { // TODO: Remove this on build
                textures_load_backgrounds(&gs->textures, false);
            } break;
            case SDLK_4: {
                if (input->keys[SDL_SCANCODE_LCTRL]) {
                    set_text_field("Output current grid to image:", "../", level_output_to_png);
                } else {
                    gui_click_overlay_button();
                    gs->gui.tool_buttons[TOOL_OVERLAY]->highlighted = false;
                }
                break;
            }
            case SDLK_z: {
                undo();
                break;
            }
            case SDLK_EQUALS: {
                Wind_Stream stream = load_wind_stream(DATA_DIR "wind1.png");
                Log("(SDL_Point){\n");
                for (int i = 0; i < stream.point_count; i++) {
                    Log("    { %d, %d }", stream.points[i].x, stream.points[i].y);
                    if (i < stream.point_count-1) Log(",");
                    Log("\n");
                }
                Log("}\nCount: %d\n", stream.point_count);
            } break;
            case SDLK_t: {
//#ifndef ALASKA_RELEASE_MODE
                grid_set_to_desired();
//#endif
            } break;
            case SDLK_q: {
                Cell *c;
                c = &gs->grid[input->mx+input->my*gs->gw];
                char name[256] = {0};
                get_name_from_type(c->type, name);

                int obj = c->object;
                if (obj == -1) obj = 0;

                Assert(obj != -1);


                Log("Cell %d, %d: Pos: (%f, %f), Type: %s, ID: %d, Rand: %d, Object: %d, Vx: %f, Vy: %f\n",
                    input->mx,
                    input->my,
                    c->vx_acc,
                    c->vy_acc,
                    name,
                    c->id,
                    c->rand,
                    c->object,
                    c->vx,
                    c->vy);
                break;
            }
            case SDLK_1: {
                if (!gs->gui.tool_buttons[TOOL_CHISEL_SMALL]->disabled) {
                    gs->current_tool = TOOL_CHISEL_SMALL;
                    gs->chisel = &gs->chisels[0];
                    //gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+2;
                    selected_tool = 1;
                }
                break;
            }
            case SDLK_2: {
                if (!gs->gui.tool_buttons[TOOL_CHISEL_MEDIUM]->disabled) {
                    gs->current_tool = TOOL_CHISEL_MEDIUM;
                    gs->chisel = &gs->chisels[1];
                    //gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+4;
                    selected_tool = 1;
                }
                break;
            }
            case SDLK_3: {
                if (!gs->gui.tool_buttons[TOOL_CHISEL_LARGE]->disabled) {
                    gs->current_tool = TOOL_CHISEL_LARGE;
                    gs->chisel = &gs->chisels[2];
                    //gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+4;
                    selected_tool = 1;
                }
                break;
            }
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

#ifdef __EMSCRIPTEN__
event_tick_end:
#endif
    text_field_tick();

    if (selected_tool) {
        gs->gui.tool_buttons[gs->current_tool]->on_pressed(&gs->gui.tool_buttons[gs->current_tool]->index);
        gs->gui.tool_buttons[gs->current_tool]->active = true;
    }

    return is_running;
}

void audio_setup_channel_volumes(void) {
    if (gs->audio_handler.channel_volumes[AUDIO_CHANNEL_CHISEL] == 0) {
        gs->audio_handler.channel_volumes[AUDIO_CHANNEL_CHISEL] = AUDIO_CHISEL_VOLUME;
        gs->audio_handler.channel_volumes[AUDIO_CHANNEL_GUI] = AUDIO_GUI_VOLUME;
        gs->audio_handler.channel_volumes[AUDIO_CHANNEL_MUSIC] = AUDIO_MUSIC_VOLUME;
        gs->audio_handler.channel_volumes[AUDIO_CHANNEL_AMBIENCE] = AUDIO_AMBIENCE_VOLUME;
    }

    assign_channel_volumes(&gs->pause_menu, &gs->audio_handler);
}

export void game_run(Game_State *state) {
    u64 start_frame = SDL_GetPerformanceCounter();

    //u64 aa = SDL_GetPerformanceCounter();

    gs = state;

    gs->gui.tooltip.set_this_frame = false;

    audio_setup_channel_volumes();

    gs->accum = 0;
    gs->amt = 0;

#ifdef __EMSCRIPTEN__
    if (!gs->input.em_got_mouse_motion_event_this_frame) {
        gs->input.real_pmx = gs->input.real_mx;
        gs->input.real_pmy = gs->input.real_my;
    }
#endif

    switch (gs->gamestate) {
        case GAME_STATE_TITLESCREEN: {
            titlescreen_draw(RENDER_TARGET_MASTER);
            break;
        }
        case GAME_STATE_PLAY: {
            game_update_view();

            audio_set_ambience_accordingly();
            audio_set_music_accordingly();

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

                u8 alpha = 255 - 255 * min(240,gs->obj.t) / 240.0;

                RenderTextureAlphaMod(&RenderTarget(RENDER_TARGET_GUI_TOOLBAR)->texture, alpha);
                RenderMaybeSwitchToTarget(RENDER_TARGET_MASTER);
                SDL_RenderCopy(gs->render.sdl,
                               RenderTarget(RENDER_TARGET_GUI_TOOLBAR)->texture.handle,
                               null,
                               &dst);
            } else {
                if (gs->pause_menu.active) {
                    RenderColor(0, 0, 0, 255);
                    RenderClear(RENDER_TARGET_MASTER);
                    pause_menu_draw(RENDER_TARGET_MASTER, &gs->pause_menu);
                } else {
                    //int y = 2*gs->game_height/3;

                    gui_tick();
                    inventory_tick();
                    all_converters_tick();
                    level_tick(&gs->levels[gs->level_current]);
                    level_draw(&gs->levels[gs->level_current]);
                    text_field_draw(RENDER_TARGET_MASTER);
                    fade_draw(RENDER_TARGET_MASTER);
                    preview_tick();

                    if (gs->current_preview.play) {
                        preview_draw(RENDER_TARGET_MASTER,
                                     &gs->current_preview,
                                     0,
                                     0,
                                     6,
                                     false,
                                     false);
                    }

                    if (gs->step_one) gs->step_one = false;
                }
            }
            break;
        }
    }

    //f64 frame_end = __end_timer(aa);

    {
        //char msg[64];
        //sprintf(msg, "Frame time took %.2fms", 1000*gs->dt);
        //RenderTextDebugPush(msg, 0, gs->game_height-35);
        //sprintf(msg, "SDL_SetRenderTarget took %.2fms | Count: %d", gs->accum, gs->amt);
        //RenderTextDebugPush(msg, 0, gs->game_height-65);
    }

    RenderTextDebug();

    if (gs->input.locked && !gs->input.hide_mouse) {
        Texture *t = &GetTexture(TEXTURE_CURSOR);
        RenderTexture(RENDER_TARGET_MASTER, t, null, &(SDL_Rect){gs->input.real_mx, gs->input.real_my, t->width, t->height});
    }

#ifdef __EMSCRIPTEN__
    if (SDL_GetMouseFocus() == null) {
        draw_focus(RENDER_TARGET_MASTER);
        gs->test = true;
    }

#endif

    if (gs->timer == 0) {
        gs->timer = 60;
        gs->highest_frametime = 0;
    }
    gs->timer--;
    if (1000*gs->dt > gs->highest_frametime)
        gs->highest_frametime = 1000*gs->dt;

    if (gs->draw_fps) {
        char str[128];
        sprintf(str, "Frametime: %.2f ms\n", 1000*gs->dt);
        SDL_Color color = {127,127,127,255};
        if (gs->dt*1000 >= 16.67)
            color = RED;

        RenderTextQuick(RENDER_TARGET_MASTER,
                        "fps",
                        gs->fonts.font,
                        str,
                        color,
                        100, 100,
                        null, null,
                        false);

        sprintf(str, "Max Local Frametime: %.2f ms\n", gs->highest_frametime);
        RenderTextQuick(RENDER_TARGET_MASTER,
                        "2fps",
                        gs->fonts.font,
                        str,
                        WHITE,
                        100, 140,
                        null, null,
                        false);
    }
    SDL_Color border_color_desired = {0};

    if (gs->level_current+1 == 11) {
        border_color_desired.r = 255;
        border_color_desired.g = 255;
        border_color_desired.b = 255;
#ifdef __EMSCRIPTEN__
        if (!gs->html_set_background_color_already) {
            html_set_background_color_white();
            gs->html_set_background_color_already = true;
        }
#endif
    } else {
#ifdef __EMSCRIPTEN__
        bool prev = gs->html_set_background_color_already;
        gs->html_set_background_color_already = false;
        if (prev) {
            html_set_background_color_black();
        }
#endif
    }

    gs->border_color.r = interpolate(gs->border_color.r, border_color_desired.r, 2);
    gs->border_color.g = interpolate(gs->border_color.g, border_color_desired.g, 2);
    gs->border_color.b = interpolate(gs->border_color.b, border_color_desired.b, 2);

    RenderColor(gs->border_color.r, gs->border_color.g, gs->border_color.b, 255);

    RenderClear(-1);

    RenderColor(255, 255, 255, 255);

    SDL_Rect src = {
        0, 0,
        gs->game_width, gs->game_height
    };

    SDL_Rect dst = {
        gs->real_width/2 - gs->game_width/2,
        gs->real_height/2 - gs->game_height/2,
        gs->game_width,
        gs->game_height
    };

#ifdef __EMSCRIPTEN__
    dst.x = dst.y = 0;
#endif

    SDL_RenderCopy(gs->render.sdl,
                   RenderTarget(RENDER_TARGET_MASTER)->texture.handle,
                   &src,
                   &dst);

    gs->dt = __end_timer(start_frame);

    SDL_RenderPresent(gs->renderer);

    gs->is_mouse_over_any_button = false;

    RenderCleanupTextCache(&gs->render.temp_text_cache);
}
