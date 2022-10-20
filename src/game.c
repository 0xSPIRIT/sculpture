#include "shared.h"

// Include all files to compile in one translation unit for
// compilation speed's sake. ("Unity Build")
#include "util.c"
#include "overlay.c"
#include "grid.c"
#include "undo.c"
#include "chisel_blocker.c"
#include "spline.c"
#include "blocker.c"
#include "chisel.c"
#include "tooltip.c"
#include "placer.c"
#include "gui.c"
#include "overlay_interface.c"
#include "effects.c"
#include "grabber.c"
#include "deleter.c"
#include "popup.c"
#include "level.c"

export void game_init(struct Game_State *state, int level) {
    gs = state;
    levels_setup();

    goto_level(level);
}

export bool game_tick_event(struct Game_State *state, SDL_Event *event) {
    gs = state;
    gs->event = event;

    bool is_running = true;
    struct Input *input = &gs->input;

    // This is checked at game_run.
    /* gs->prev_tool = gs->current_tool; */

    if (event->type == SDL_QUIT) {
        is_running = false;
    }

    if (event->type == SDL_MOUSEWHEEL) {
        if (gs->current_tool == TOOL_PLACER) {
            struct Placer *placer = &gs->placers[gs->current_placer];
            if (input->keys[SDL_SCANCODE_LCTRL]) {
                placer->contains_type += event->wheel.y;
                if (placer->contains_type < CELL_NONE+1) placer->contains_type = CELL_NONE+1;
                if (placer->contains_type >= CELL_TYPE_COUNT) placer->contains_type = CELL_TYPE_COUNT-1;
            } else {
                placer->radius += event->wheel.y;
                placer->radius = clamp(placer->radius, 1, 5);
            }
        }
    }
           
    int selected_tool = 0;
    if (event->type == SDL_KEYDOWN && !gs->text_field.active) {
        switch (event->key.keysym.sym) {
        case SDLK_ESCAPE:
            if (gs->deleter.active) {
                deleter_stop(true);
            } else {
                is_running = false;
            }
            break;
        case SDLK_SPACE:
            gs->paused = !gs->paused;
            break;
        case SDLK_SEMICOLON:
            gui_message_stack_push("Test");
            break;
        case SDLK_n:
            gs->step_one = 1;
            break;
        case SDLK_r:
            if (gs->input.keys[SDL_SCANCODE_LCTRL]) {
                goto_level(gs->level_current);
            }
            break;
        case SDLK_b:
            gs->do_draw_blobs = !gs->do_draw_blobs;
            break;
        case SDLK_g:
            if (input->keys[SDL_SCANCODE_LCTRL]) {
	            set_text_field("Goto Level", "", goto_level_string_hook);
            }
            break;
        case SDLK_o:
            if (input->keys[SDL_SCANCODE_LCTRL]) {
                set_text_field("Output current grid to image:", "../", level_output_to_png);
            } else {
                gs->do_draw_objects = !gs->do_draw_objects;
            }
            break;
        case SDLK_u:
            objects_reevaluate();
            break;
        case SDLK_d:
            gs->debug_mode = 1;
            break;
        case SDLK_z:
            if (input->keys[SDL_SCANCODE_LCTRL]) {
                if (input->keys[SDL_SCANCODE_LSHIFT]) {
                    set_text_field("Go to State", "", set_state_to_string_hook);
                } else {
                    undo();
                }
            }
            break;
        case SDLK_q: {
            struct Cell *c;
            if (input->keys[SDL_SCANCODE_LSHIFT]) {
                c = &gs->fg_grid[input->mx+input->my*gs->gw];
            } else {
                c = &gs->grid[input->mx+input->my*gs->gw];
            }
            char name[256] = {0};
            get_name_from_type(c->type, name);

            int obj = c->object;
            if (obj == -1) obj = 0;

            Assert(obj != -1);

            printf("Cell %d, %d: Pos: (%f, %f), Type: %s, ID: %d, Rand: %d, Object: %d, Time: %d, Vx: %f, Vy: %f, Blob: %u\n",
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
        case SDLK_i:
            gs->grid_show_ghost = !gs->grid_show_ghost;
            break;
        case SDLK_1:
            gs->current_tool = TOOL_CHISEL_SMALL;
            gs->chisel = &gs->chisel_small;
            for (int i = 0; i < gs->object_count; i++)
                object_generate_blobs(i, 0);
            gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+2;
            selected_tool = 1;
            break;
        case SDLK_2:
            gs->current_tool = TOOL_CHISEL_MEDIUM;
            gs->chisel = &gs->chisel_medium;
            for (int i = 0; i < gs->object_count; i++)
                object_generate_blobs(i, 1);
            gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+4;
            selected_tool = 1;
            break;
        case SDLK_3:
            gs->current_tool = TOOL_CHISEL_LARGE;
            gs->chisel = &gs->chisel_large;
            for (int i = 0; i < gs->object_count; i++)
                object_generate_blobs(i, 2);
            gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) gs->chisel->w+4;
            selected_tool = 1;
            break;
        case SDLK_4:
            gs->current_tool = TOOL_DELETER;
            selected_tool = 1;
            break;
        case SDLK_5:
            gs->current_tool = TOOL_PLACER;
            selected_tool = 1;
            break;
        case SDLK_6:
            gs->current_tool = TOOL_GRABBER;
            selected_tool = 1;
            break;
        case SDLK_F1:
            gs->current_placer = 0;
            break;
        case SDLK_F2:
            gs->current_placer = 1;
            break;
        case SDLK_F3:
            gs->current_placer = 2;
            break;
        }
    }

    text_field_tick();

    if (selected_tool) {
        gs->gui.tool_buttons[gs->current_tool]->on_pressed(&gs->gui.tool_buttons[gs->current_tool]->index);
        gs->gui.tool_buttons[gs->current_tool]->activated = true;
    }

    return is_running;
}

void draw_intro() {
    gui_draw();

    SDL_Rect dst = {
        0, GUI_H,
        gs->window_width, gs->window_height-GUI_H
    };

    SDL_SetRenderTarget(gs->renderer, NULL);
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL), NULL, &dst);

    gui_popup_draw();
    all_converters_draw();

    if (gs->current_tool == TOOL_OVERLAY)
        overlay_interface_draw();

    tooltip_draw(&gs->gui.tooltip);

    gui_message_stack_tick_and_draw();

    text_field_draw();
}

void draw_outro(struct Level *level) {
    SDL_Rect rect = {gs->S*gs->gw/8, GUI_H + gs->S*gs->gh/2 - (gs->S*3*gs->gh/4)/2, gs->S*3*gs->gw/4, gs->S*3*gs->gh/4};
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(gs->renderer, &rect);

    const int margin = 36;

    { // Level name
        char string[256] = {0};
        sprintf(string, "Level %d - \"%s\"", gs->level_current+1, level->name);

        int x = rect.x + margin;
        int y = rect.y + margin;

        draw_text(gs->fonts.font, string, (SDL_Color){0,0,0,255}, 0, 0, x, y, NULL, NULL);
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

        draw_text(gs->fonts.font, string, (SDL_Color){0, 0, 0, 255}, 0, 0, dx, dy, NULL, NULL);

        for (int y = 0; y < gs->gh; y++) {
            for (int x = 0; x < gs->gw; x++) {
                SDL_Rect r;
                switch (i) {
                case 0: // Desired
                    if (!level->desired_grid[x+y*gs->gw].type) {
                        SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
                    }  else {
                        SDL_Color col = pixel_from_index(level->desired_grid, x+y*gs->gw);
                        SDL_SetRenderDrawColor(gs->renderer, col.r, col.g, col.b, 255); // 255 on this because desired_grid doesn't have depth set.
                    }
                    break;
                case 1: // Yours
                    if (!gs->grid[x+y*gs->gw].type) {
                        SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
                    }  else {
                        SDL_Color col = pixel_from_index(gs->grid, x+y*gs->gw);
                        SDL_SetRenderDrawColor(gs->renderer, col.r, col.g, col.b, col.a);
                    }
                    break;
                }
                r = (SDL_Rect){ 2*x + dx, 2*y + dy + 32, 2, 2 };
                SDL_RenderFillRect(gs->renderer, &r);
            }
        }
    }

    draw_text(gs->fonts.font,
              "Next Level [n]",
              (SDL_Color){0, 91, 0, 255},
              1, 1,
              rect.x + rect.w - margin,
              rect.y + rect.h - margin,
              NULL,
              NULL);
    draw_text(gs->fonts.font,
              "Close [f]",
              (SDL_Color){0, 91, 0, 255},
              0, 1,
              rect.x + margin,
              rect.y + rect.h - margin,
              NULL,
              NULL);
}

export void game_run(struct Game_State *state) {
    gs = state;

    struct Level *level = &gs->levels[gs->level_current];

    gui_tick();
    all_converters_tick();

    level_tick();
    level_draw();

    if (level->state == LEVEL_STATE_OUTRO) {
        SDL_Rect dst = {
            0, GUI_H,
            gs->window_width, gs->window_height-GUI_H
        };

        SDL_SetRenderTarget(gs->renderer, NULL);
        SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL), NULL, &dst);
        draw_outro(level);
    } else if (level->state != LEVEL_STATE_INTRO) {
        draw_intro();
    }
    SDL_RenderPresent(gs->renderer);

    gs->is_mouse_over_any_button = false;
}
