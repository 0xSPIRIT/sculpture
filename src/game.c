#include <SDL2/SDL_image.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "shared.h"

// Include all files to compile in one translation unit for
// compilation speed's sake. ("Unity Build")
#include "blob_hammer.c"
#include "chisel_blocker.c"
#include "blocker.c"
#include "chisel.c"
#include "converter.c"
#include "effects.c"
#include "grabber.c"
#include "grid.c"
#include "gui.c"
#include "knife.c"
#include "level.c"
#include "placer.c"
#include "deleter.c"
#include "popup.c"
#include "undo.c"
#include "util.c"

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
        case SDLK_n:
            gs->step_one = 1;
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
        case SDLK_F5:
            view_save_state_linked_list();
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
            gs->current_tool = TOOL_KNIFE;
            selected_tool = 1;
            break;
        case SDLK_5:
            gs->current_tool = TOOL_DELETER;
            selected_tool = 1;
            break;
        case SDLK_6:
            gs->current_tool = TOOL_HAMMER;
            selected_tool = 1;
            break;
        case SDLK_7:
            gs->current_tool = TOOL_PLACER;
            selected_tool = 1;
            break;
        case SDLK_8:
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
        gs->gui.tool_buttons[gs->current_tool]->activated = 1;
    }

    return is_running;
}

export void game_run(struct Game_State *state) {
    gs = state;

    struct Level *level = &gs->levels[gs->level_current];

    level_tick();
    level_draw();

    if (level->state != LEVEL_STATE_INTRO) {
        gui_tick();
        gui_draw();

        SDL_Rect dst = {
            0, GUI_H,
            gs->window_width, gs->window_height-GUI_H
        };

        SDL_SetRenderTarget(gs->renderer, NULL);
        SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL), NULL, &dst);

        gui_popup_draw();
        tooltip_draw(&gs->gui.tooltip);
        text_field_draw();
    }
    
    SDL_RenderPresent(gs->renderer);
}
