#include "chisel.h"

#include <math.h>
#include <SDL2/SDL_image.h>

#include "globals.h"

#include "grid.h"
#include "util.h"
#include "grid.h"
#include "blob_hammer.h" // For hammer state enum
#include "chisel_blocker.h"
#include "undo.h"
#include "game.h"

void chisel_init(struct Chisel *type) {
    struct Chisel *chisel = gs->chisel;

    struct Chisel *chisel_small = &gs->chisel_small;
    struct Chisel *chisel_medium = &gs->chisel_medium;
    struct Chisel *chisel_large = &gs->chisel_large;

    SDL_Surface *surf;

    chisel = type;

    for (int face = 1; face != -1; face--) {
        /* char file[512] = {0}; */
        /* if (chisel == chisel_small) { */
        /*     chisel->size = 0; */
        /*     strcpy(file, "../res/chisel_small"); */
        /* } else if (chisel == chisel_medium) { */
        /*     chisel->size = 1; */
        /*     strcpy(file, "../res/chisel_medium"); */
        /* } else if (chisel == chisel_large) { */
        /*     chisel->size = 2; */
        /*     strcpy(file, "../res/chisel_large"); */
        /* } */
        /* if (face) */
        /*     strcat(file, "_face"); */

        /* strcat(file, ".png"); */

        /* surf = IMG_Load(file); */
        /* SDL_assert(surf); */

        if (face) {
            chisel->face_texture = SDL_CreateTextureFromSurface(gs->renderer, surf);
            chisel->face_w = surf->w;
            chisel->face_h = surf->h;
        } else {
            chisel->outside_texture = SDL_CreateTextureFromSurface(gs->renderer, surf);
            chisel->outside_w = surf->w;
            chisel->outside_h = surf->h;
        }
    }

    chisel->click_cooldown = 0;
    chisel->line = NULL;
    chisel->face_mode = false;

    chisel->render_texture = SDL_CreateTexture(gs->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gs->gw, gs->gh);
    chisel->pixels = calloc(gs->gw*gs->gh, sizeof(Uint32));

    chisel->texture = chisel->outside_texture;
    chisel->w = chisel->outside_w;
    chisel->h = chisel->outside_h;

    chisel->highlights = calloc(gs->gw*gs->gh, sizeof(int));
    chisel->highlight_count = 0;

    chisel->spd = 3.;

    SDL_FreeSurface(surf);
}

void chisel_deinit(struct Chisel *type) {
    SDL_DestroyTexture(type->texture);
    SDL_DestroyTexture(type->face_texture);
    SDL_DestroyTexture(type->render_texture);
}

internal void chisel_set_depth() {
    struct Chisel *chisel = gs->chisel;
    struct Cell *grid = gs->grid;

    switch (chisel->size) {
    case 0:
        grid[(int)chisel->x + ((int)chisel->y)*gs->gw].depth = 127;
        break;
    case 1:
        for (int y = 0; y < gs->gh; y++) {
            for (int x = 0; x < gs->gw; x++) {
                if (chisel->pixels[x+y*gs->gw] == 0x9B9B9B) {
                    const int amt = 127;
                    if (grid[x+y*gs->gw].depth > amt)
                        grid[x+y*gs->gw].depth -= amt;
                }
            }
        }
        break;
    }
}

void chisel_tick() {
    struct Chisel *chisel = gs->chisel;
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;
    struct Input *input = &gs->input;
    struct Object *objects = gs->objects;

    bool prev_changing_angle = chisel->is_changing_angle;

    chisel->is_changing_angle = input->keys[SDL_SCANCODE_LSHIFT];

    if (prev_changing_angle && !chisel->is_changing_angle) {
        /* SDL_WarpMouseInWindow(gs->window, (int)chisel->x*gs->S, GUI_H + (int)chisel->y*gs->S); */
        move_mouse_to_grid_position(chisel->x, chisel->y);
        input->mx = (int)chisel->x;
        input->my = (int)chisel->y;
    }

    if (hammer->state == HAMMER_STATE_IDLE && !chisel->is_changing_angle && !chisel->click_cooldown) {
        int index = clamp_to_grid(input->mx, input->my, !chisel->face_mode, false, true, true);
        if (index != -1) {
            chisel->x = index%gs->gw;
            chisel->y = index/gs->gw;

            // Highlight the current blob.
            // This is a fake chiseling- we're resetting position afterwards.
            float chisel_dx = cos(2*M_PI * ((chisel->angle+180) / 360.0));
            float chisel_dy = sin(2*M_PI * ((chisel->angle+180) / 360.0));
            float dx = chisel->spd * chisel_dx;
            float dy = chisel->spd * chisel_dy;
            float len = sqrt(dx*dx + dy*dy);
            float ux = dx/len;
            float uy = dy/len;

            struct Chisel copy = *chisel;

            Uint32 blob_highlight = chisel_goto_blob(false, ux, uy, len);

            *chisel = copy;

            if (blob_highlight > 0) {
                memset(chisel->highlights, 0, chisel->highlight_count);
                chisel->highlight_count = 0;

                if (blob_can_destroy(gs->object_current, chisel->size, blob_highlight)) {
                    for (int i = 0; i < gs->gw*gs->gh; i++) {
                        Uint32 b = objects[gs->object_current].blob_data[chisel->size].blobs[i];
                        if (b == blob_highlight) {
                            chisel->highlights[chisel->highlight_count++] = i;
                        }
                    }
                }
            }
        }
    }

    if (chisel->is_changing_angle) {
        float rmx = (float)input->real_mx / (float)gs->S;
        float rmy = (float)(input->real_my-GUI_H) / (float)gs->S;
        chisel->angle = 180 + 360 * (atan2(rmy - chisel->y, rmx - chisel->x)) / (2*M_PI);

        float step = 45.0;
        if (chisel->face_mode) {
            step = 22.5;
        }
        chisel->angle /= step;
        chisel->angle = ((int)chisel->angle) * step;
        /* SDL_ShowCursor(1); */
    }/*  else { */
    /*     float dx = chisel->x - input->mx; */
    /*     float dy = chisel->y - input->my; */
    /*     float dist = sqrt(dx*dx + dy*dy); */
    /*     SDL_ShowCursor(dist > 1); */
    /* } */

    if (input->keys_pressed[SDL_SCANCODE_S]) {
        chisel->face_mode = !chisel->face_mode;
        chisel->w = chisel->face_mode ? chisel->face_w : chisel->outside_w;
        chisel->h = chisel->face_mode ? chisel->face_h : chisel->outside_h;
        chisel->texture = chisel->face_mode ? chisel->face_texture : chisel->outside_texture;
    }    

    if (input->mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        chisel->is_changing_angle = 0;
    }

    if (chisel->click_cooldown) {
        if (chisel->click_cooldown >= CHISEL_COOLDOWN-CHISEL_TIME) {
            // Cut out the stone.
            float px = chisel->x;
            float py = chisel->y;
            float ux = cos(2*M_PI * ((chisel->angle+180) / 360.0));
            float uy = sin(2*M_PI * ((chisel->angle+180) / 360.0));
            float len = chisel->spd;

            switch ((int)chisel->angle) {
            case 135:
                ux = 1;
                uy = -1;
                break;
            case 225:
                ux = 1;
                uy = 1;
                break;
            case 270:
                ux = 0;
                uy = 1;
                break;
            case 315:
                ux = -1;
                uy = 1;
                break;
            }
            
            if (chisel->face_mode) {
                while (sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y)) < len) {
                    chisel->x += ux;
                    chisel->y += uy;
                    chisel_set_depth(chisel);
                    chisel_update_texture();
                }
            } else if (!chisel->did_remove && gs->object_current != -1) {
                chisel_goto_blob(true, ux, uy, len);
            }
            /* SDL_WarpMouseInWindow(gs->window, (int)(chisel->x * gs->S), GUI_H + (int)(chisel->y * gs->S)); */
            move_mouse_to_grid_position(chisel->x, chisel->y);
            input->mx = chisel->x;
            input->my = chisel->y;
        }
        chisel->click_cooldown--;
        if (chisel->click_cooldown == 0) {
            chisel->line = NULL;
            int index = clamp_to_grid(input->mx, input->my, !chisel->face_mode, false, true, true);
            chisel->x = index%gs->gw;
            chisel->y = index/gs->gw;
        }
    }
    chisel_hammer_tick();
}

void chisel_update_texture() {
    struct Chisel *chisel = gs->chisel;

    SDL_Texture *prev_target = SDL_GetRenderTarget(gs->renderer);
    SDL_SetTextureBlendMode(chisel->render_texture, SDL_BLENDMODE_BLEND);
    
    SDL_SetRenderTarget(gs->renderer, chisel->render_texture);

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);
    
    int x = chisel->x;
    int y = chisel->y;

    // Disgusting hardcoding to adjust the weird rotation SDL does.
    if (!chisel->face_mode) {
        if (chisel->size == 0 || chisel->size == 1) {
            if (chisel->angle == 225) {
                x += 1;
                y += 2;
            } else if (chisel->angle == 180) {
                x++;
                y++;
            } else if (chisel->angle == 90 || chisel->angle == 45) {
                x++;
            } else if (chisel->angle == 135) {
                x += 2;
            }
        } else {
            if (chisel->angle == 0) {
                y--;
            } else if (chisel->angle == 270) {
                y++;
                x--;
            } else if (chisel->angle == 225) {
                y += 2;
            } else if (chisel->angle == 180) {
                x++;
                y += 2;
            } else if (chisel->angle == 90) {
                x += 2;
            } else if (chisel->angle == 45) {
                y -= 2;
            } else if (chisel->angle == 135) {
                x += 2;
                y++;
            } else if (chisel->angle == 315) {
                x--;
            }
        }
    }

    const SDL_Rect dst = {
        x, y - chisel->h/2,
        chisel->w, chisel->h
    };
    const SDL_Point center = { 0, chisel->h/2 };

    SDL_RenderCopyEx(gs->renderer, chisel->texture, NULL, &dst, chisel->angle, &center, SDL_FLIP_NONE);

    chisel_hammer_draw();

    if (!chisel->face_mode) {
        SDL_SetRenderDrawColor(gs->renderer, 127, 127, 127, 255);
        SDL_RenderDrawPoint(gs->renderer, chisel->x, chisel->y);
    }

    SDL_RenderReadPixels(gs->renderer, NULL, 0, chisel->pixels, 4*gs->gw);

    SDL_SetRenderTarget(gs->renderer, prev_target);
}

void chisel_draw() {
    struct Chisel *chisel = gs->chisel;

    chisel_update_texture();
    SDL_RenderCopy(gs->renderer, chisel->render_texture, NULL, NULL);

    // Draw the highlights for blobs now.
    if (DRAW_CHISEL_HIGHLIGHTS) {
        for (int i = 0; i < chisel->highlight_count; i++) {
            SDL_SetRenderDrawColor(gs->renderer, 234, 103, 93, 64);
            SDL_RenderDrawPoint(gs->renderer, chisel->highlights[i]%gs->gw, chisel->highlights[i]/gs->gw);
        }
    }
}

// if remove, delete the blob
// else, highlight it and don't change anything about the chisel's state.
// ux, uy = unit vector for the direction of chisel.
// px, py = initial positions.
// Returns the blob it reaches only if remove == 0.
Uint32 chisel_goto_blob(bool remove, float ux, float uy, float len) {
    struct Chisel *chisel = gs->chisel;
    struct Object *objects = gs->objects;

    bool did_hit_blocker = false;
    float px = chisel->x, py = chisel->y;

    if (gs->object_current == -1) return 0;

    while (sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y)) < len) {
        // If we hit the chisel blocker, keep that in mind for later.
        if (gs->current_tool == TOOL_CHISEL_MEDIUM && gs->chisel_blocker.state != CHISEL_BLOCKER_OFF && gs->chisel_blocker.pixels[(int)chisel->x + ((int)chisel->y)*gs->gw] != gs->chisel_blocker.side) {
            did_hit_blocker = true;
        }

        // If we come into contact with a cell, locate its blob
        // then remove it. We only remove one blob per chisel,
        // so we stop our speed right here.
        Uint32 b = objects[gs->object_current].blob_data[chisel->size].blobs[(int)chisel->x + ((int)chisel->y)*gs->gw];
        if (b > 0 && !remove) {
            if (gs->grid[(int)chisel->x + ((int)chisel->y)*gs->gw].type == 0) b = -1;
            return b;
        } else if (remove && b > 0 && !chisel->did_remove) {
            // We want to have the chisel end up right at the edge of the
            // blob itself, to make it seem like it really did knock that
            // entire thing out.
            // So, we continue at our current direction until we reach
            // another blob, and we backtrack one.

            while (objects[gs->object_current].blob_data[chisel->size].blobs[(int)chisel->x + ((int)chisel->y)*gs->gw] == b) {
                chisel->x += ux;
                chisel->y += uy;
            }

            chisel->x -= ux;
            chisel->y -= uy;

            if (blob_can_destroy(gs->object_current, chisel->size, b)) {
                object_remove_blob(gs->object_current, b, chisel->size, true);

                /* SDL_WarpMouseInWindow(gs->window, (int)(chisel->x * gs->S), GUI_H + (int)(chisel->y * gs->S)); */
                move_mouse_to_grid_position(chisel->x, chisel->y);
                chisel->did_remove = true;
            }

            chisel->click_cooldown = CHISEL_COOLDOWN-CHISEL_TIME-1;
            break;
        }

        chisel->x += ux;
        chisel->y += uy;
    }

    Uint32 b = objects[gs->object_current].blob_data[chisel->size].blobs[(int)chisel->x + ((int)chisel->y)*gs->gw];
    if (!remove) {
        if (gs->grid[(int)chisel->x + ((int)chisel->y)*gs->gw].type == 0) b = -1;
        return b;
    }

    // remove=true from here on out.

    // Do a last-ditch effort if a blob is around a certain radius in order for the player
    // not to feel frustrated if it's a small blob or it's one pixel away.
    if (did_hit_blocker || (CHISEL_FORGIVING_AIM && !chisel->did_remove)) {
        const float r = 1;
        for (int y = -r; y <= r; y++) {
            for (int x = -r; x <= r; x++) {
                if (x*x + y*y > r*r) continue;
                int xx = chisel->x + x;
                int yy = chisel->y + y;
                Uint32 blob = objects[gs->object_current].blob_data[chisel->size].blobs[xx+yy*gs->gw];

                if (blob > 0) {
                    object_remove_blob(gs->object_current, blob, chisel->size, true);
                    chisel->did_remove = true;
                    goto chisel_did_remove;
                }
            }
        }
    }

 chisel_did_remove:
    if (chisel->did_remove) {
        objects_reevaluate();
        // Here, we check for all the small objects that pop up from repeated
        // chiseling that make chiseling an annoyance. We convert that to
        // dust particles in order to get out of the player's way.
        
        bool did_remove = false;
        for (int i = 0; i < gs->object_count; i++) {
            if (objects[i].cell_count <= 4) {
                convert_object_to_dust(i);
                did_remove = true;
            }
        }
        if (did_remove) {
            objects_reevaluate();
        }
    }

    return 0;
}

void chisel_hammer_init() {
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;
    struct Chisel *chisel = gs->chisel;

    hammer->x = chisel->x;
    hammer->y = chisel->y;
    hammer->normal_dist = chisel->w+4;
    hammer->dist = hammer->normal_dist;
    hammer->time = 0;
    hammer->angle = 0;

    SDL_Surface *surf = IMG_Load("../res/hammer.png");
    hammer->w = surf->w;
    hammer->h = surf->h;
    hammer->texture = SDL_CreateTextureFromSurface(gs->renderer, surf);
    SDL_FreeSurface(surf);
}

void chisel_hammer_deinit() {
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;

    SDL_DestroyTexture(hammer->texture);
}

void chisel_hammer_tick() {
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;
    struct Input *input = &gs->input;
    struct Chisel *chisel = gs->chisel;

    hammer->angle = chisel->angle;

    float rad = (hammer->angle) / 360.0;
    rad *= 2 * M_PI;

    const float off = 6;

    int dir = 1;

    if (hammer->angle > 90 && hammer->angle < 270) {
        dir = -1;
    }
    
    hammer->x = chisel->x + hammer->dist * cos(rad) - dir * off * sin(rad);
    hammer->y = chisel->y + hammer->dist * sin(rad) + dir * off * cos(rad);

    const int stop = 24;
    
    switch (hammer->state) {
    case HAMMER_STATE_WINDUP:
        hammer->time++;
        if (hammer->time < 3) {
            hammer->dist += hammer->time * 4;
        } else {
            hammer->time = 0;
            hammer->state = HAMMER_STATE_SWING;
        }
        break;
    case HAMMER_STATE_SWING:
        hammer->dist -= 8;
        if (hammer->dist < stop) {
            hammer->dist = stop;
            // Activate the chisel.
            if (chisel->click_cooldown == 0) {
                chisel->click_cooldown = CHISEL_COOLDOWN;
                chisel->spd = 3.;
            }
            chisel->did_remove = false;
            hammer->state = HAMMER_STATE_IDLE;
        }
        break;
    case HAMMER_STATE_IDLE:
        hammer->dist = hammer->normal_dist;
        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            hammer->state = HAMMER_STATE_WINDUP;
            save_state_to_next();
        }
        break;
    }
}

void chisel_hammer_draw() {
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;

    const SDL_Rect dst = {
        hammer->x, hammer->y - hammer->h/2,
        hammer->w, hammer->h
    };
    const SDL_Point center = { 0, hammer->h/2 };

    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (hammer->angle > 90 && hammer->angle < 270) {
        flip = SDL_FLIP_VERTICAL;
    }

    SDL_RenderCopyEx(gs->renderer, hammer->texture, NULL, &dst, hammer->angle, &center, flip);
}
