#include "chisel.h"

#include <math.h>
#include <SDL2/SDL_image.h>

#include "globals.h"
#include "grid.h"
#include "vectorizer.h"
#include "util.h"
#include "grid.h"
#include "blob_hammer.h" // For hammer state enum

struct Chisel *chisel = &chisel_medium;
struct Chisel chisel_small, chisel_medium, chisel_large;

struct ChiselHammer chisel_hammer;

void chisel_init(struct Chisel *type) {
    SDL_Surface *surf;

    chisel = type;

    for (int face = 1; face != -1; face--) {
        char file[512] = {0};
        if (chisel == &chisel_small) {
            chisel->size = 0;
            strcpy(file, "../res/chisel_small");
        } else if (chisel == &chisel_medium) {
            chisel->size = 1;
            strcpy(file, "../res/chisel_medium");
        } else if (chisel == &chisel_large) {
            chisel->size = 2;
            strcpy(file, "../res/chisel_large");
        }
        if (face)
            strcat(file, "_face");

        strcat(file, ".png");

        surf = IMG_Load(file);
        SDL_assert(surf);

        if (face) {
            chisel->face_texture = SDL_CreateTextureFromSurface(renderer, surf);
            chisel->face_w = surf->w;
            chisel->face_h = surf->h;
        } else {
            chisel->outside_texture = SDL_CreateTextureFromSurface(renderer, surf);
            chisel->outside_w = surf->w;
            chisel->outside_h = surf->h;
        }
    }

    chisel->click_cooldown = 0;
    chisel->line = NULL;
    chisel->face_mode = false;

    chisel->render_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, gh);
    chisel->pixels = calloc(gw*gh, sizeof(Uint32));

    chisel->texture = chisel->outside_texture;
    chisel->w = chisel->outside_w;
    chisel->h = chisel->outside_h;

    chisel->highlights = calloc(gw*gh, sizeof(int));
    chisel->highlight_count = 0;

    chisel->spd = 3.;

    SDL_FreeSurface(surf);
}

void chisel_deinit(struct Chisel *type) {
    SDL_DestroyTexture(type->texture);
    SDL_DestroyTexture(type->face_texture);
    SDL_DestroyTexture(type->render_texture);
}

static void chisel_set_depth() {
    switch (chisel->size) {
    case 0:
        grid[(int)chisel->x + ((int)chisel->y)*gw].depth = 128;
        break;
    case 1:
        for (int y = 0; y < gh; y++) {
            for (int x = 0; x < gw; x++) {
                if (chisel->pixels[x+y*gw] == 0x9B9B9B) {
                    grid[x+y*gw].depth = 128;
                }
            }
        }
        break;
    }
}

void chisel_tick() {
    bool prev_changing_angle = chisel->is_changing_angle;

    chisel->is_changing_angle = keys[SDL_SCANCODE_LCTRL];

    if (prev_changing_angle && !chisel->is_changing_angle) {
        /* SDL_WarpMouseInWindow(window, (int)chisel->x*S, GUI_H + (int)chisel->y*S); */
        move_mouse_to_grid_position(chisel->x, chisel->y);
        mx = (int)chisel->x;
        my = (int)chisel->y;
    }

    if (chisel_hammer.state == HAMMER_STATE_IDLE && !chisel->is_changing_angle && !chisel->click_cooldown) {
        int index = clamp_to_grid(mx, my, !chisel->face_mode, false, true, true);
        if (index != -1) {
            chisel->x = index%gw;
            chisel->y = index/gw;

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

            int blob_highlight = chisel_goto_blob(false, ux, uy, len);

            *chisel = copy;

            if (blob_highlight != -1) {
                memset(chisel->highlights, 0, chisel->highlight_count);
                chisel->highlight_count = 0;

                for (int i = 0; i < gw*gh; i++) {
                    Uint32 b = objects[object_current].blob_data[chisel->size].blobs[i];
                    if (b == blob_highlight) {
                        chisel->highlights[chisel->highlight_count++] = i;
                    }
                }
            }
        }
    }

    if (chisel->is_changing_angle) {
        float rmx = (float)real_mx / (float)S;
        float rmy = (float)(real_my-GUI_H) / (float)S;
        chisel->angle = 180 + 360 * (atan2(rmy - chisel->y, rmx - chisel->x)) / (2*M_PI);

        float step = 45.0;
        if (chisel->face_mode) {
            step = 22.5;
        }
        chisel->angle /= step;
        chisel->angle = ((int)chisel->angle) * step;
        /* SDL_ShowCursor(1); */
    }/*  else { */
    /*     float dx = chisel->x - mx; */
    /*     float dy = chisel->y - my; */
    /*     float dist = sqrt(dx*dx + dy*dy); */
    /*     SDL_ShowCursor(dist > 1); */
    /* } */

    if (keys_pressed[SDL_SCANCODE_S]) {
        chisel->face_mode = !chisel->face_mode;
        chisel->w = chisel->face_mode ? chisel->face_w : chisel->outside_w;
        chisel->h = chisel->face_mode ? chisel->face_h : chisel->outside_h;
        chisel->texture = chisel->face_mode ? chisel->face_texture : chisel->outside_texture;
    }    

    if (mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
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
            } else if (!chisel->did_remove && object_current != -1) {
                chisel_goto_blob(true, ux, uy, len);
            }
            /* SDL_WarpMouseInWindow(window, (int)(chisel->x * S), GUI_H + (int)(chisel->y * S)); */
            move_mouse_to_grid_position(chisel->x, chisel->y);
            mx = chisel->x;
            my = chisel->y;
        }
        chisel->click_cooldown--;
        if (chisel->click_cooldown == 0) {
            chisel->line = NULL;
            int index = clamp_to_grid(mx, my, !chisel->face_mode, false, true, true);
            chisel->x = index%gw;
            chisel->y = index/gw;
        }
    }
    chisel_hammer_tick();
}

void chisel_update_texture() {
    SDL_Texture *prev_target = SDL_GetRenderTarget(renderer);
    SDL_SetTextureBlendMode(chisel->render_texture, SDL_BLENDMODE_BLEND);
    
    SDL_SetRenderTarget(renderer, chisel->render_texture);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    
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

    SDL_RenderCopyEx(renderer, chisel->texture, NULL, &dst, chisel->angle, &center, SDL_FLIP_NONE);

    chisel_hammer_draw();

    if (!chisel->face_mode) {
        SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
        SDL_RenderDrawPoint(renderer, chisel->x, chisel->y);
    }

    SDL_RenderReadPixels(renderer, NULL, 0, chisel->pixels, 4*gw);

    SDL_SetRenderTarget(renderer, prev_target);
}

void chisel_draw() {
    chisel_update_texture();
    SDL_RenderCopy(renderer, chisel->render_texture, NULL, NULL);

    // Draw the highlights for blobs now.
    /* for (int i = 0; i < chisel->highlight_count; i++) { */
    /*     SDL_SetRenderDrawColor(renderer, 255, 0, 0, 64); */
    /*     SDL_RenderDrawPoint(renderer, chisel->highlights[i]%gw, chisel->highlights[i]/gw); */
    /* } */
}

// if remove == 1, delete the blob
// else, highlight it and don't change anything about the chisel's state.
// ux, uy = unit vector for the direction of chisel.
// px, py = initial positions.
// Returns the blob it reaches only if remove == 0.
int chisel_goto_blob(bool remove, float ux, float uy, float len) {
    float px = chisel->x, py = chisel->y;

    if (object_current == -1) return -1;

    while (sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y)) < len) {
        // If we come into contact with a cell, locate its blob
        // then remove it. We only remove one blob per chisel,
        // so we stop our speed right here.

        Uint32 b = objects[object_current].blob_data[chisel->size].blobs[(int)chisel->x + ((int)chisel->y)*gw];
        if (b > 0 && !remove) {
            if (grid[(int)chisel->x + ((int)chisel->y)*gw].type == 0) b = -1;
            return b;
        } else if (remove && b > 0 && !chisel->did_remove) {
            // We want to have the chisel end up right at the edge of the
            // blob itself, to make it seem like it really did knock that
            // entire thing out.
            // So, we continue at our current direction until we reach
            // another blob, and we backtrack one.

            while (objects[object_current].blob_data[chisel->size].blobs[(int)chisel->x + ((int)chisel->y)*gw] == b) {
                chisel->x += ux;
                chisel->y += uy;
            }

            chisel->x -= ux;
            chisel->y -= uy;

            if (blob_can_destroy(object_current, chisel->size, b)) {
                object_remove_blob(object_current, b, chisel->size, 1);

                /* SDL_WarpMouseInWindow(window, (int)(chisel->x * S), GUI_H + (int)(chisel->y * S)); */
                move_mouse_to_grid_position(chisel->x, chisel->y);
            }

            chisel->did_remove = true;

            chisel->click_cooldown = CHISEL_COOLDOWN-CHISEL_TIME-1;
            break;
        }

        chisel->x += ux;
        chisel->y += uy;
    }
    Uint32 b = objects[object_current].blob_data[chisel->size].blobs[(int)chisel->x + ((int)chisel->y)*gw];
    if (!remove) {
        if (grid[(int)chisel->x + ((int)chisel->y)*gw].type == 0) b = -1;
        return b;
    }

    // remove=true from here on out.

    // Do a last-ditch effort if a blob is around a certain radius in order for the player
    // not to feel frustrated if it's a small blob or it's one pixel away.
    if (CHISEL_FORGIVING_AIM && !chisel->did_remove) {
        const float r = 1;
        for (int y = -r; y <= r; y++) {
            for (int x = -r; x <= r; x++) {
                if (x*x + y*y > r*r) continue;
                int xx = chisel->x + x;
                int yy = chisel->y + y;
                Uint32 blob = objects[object_current].blob_data[chisel->size].blobs[xx+yy*gw];
                if (blob > 0) {
                    object_remove_blob(object_current, blob, chisel->size, 1);
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
        for (int i = 0; i < object_count; i++) {
            if (objects[i].cell_count <= 4) {
                convert_object_to_dust(i);
                did_remove = true;
            }
        }
        if (did_remove) {
            objects_reevaluate();
        }
    }

    return -1;
}

void chisel_hammer_init() {
    chisel_hammer.x = chisel->x;
    chisel_hammer.y = chisel->y;
    chisel_hammer.normal_dist = chisel->w+4;
    chisel_hammer.dist = chisel_hammer.normal_dist;
    chisel_hammer.time = 0;
    chisel_hammer.angle = 0;

    SDL_Surface *surf = IMG_Load("../res/hammer.png");
    chisel_hammer.w = surf->w;
    chisel_hammer.h = surf->h;
    chisel_hammer.texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
}

void chisel_hammer_deinit() {
    SDL_DestroyTexture(chisel_hammer.texture);
}

void chisel_hammer_tick() {
    chisel_hammer.angle = chisel->angle;

    float rad = (chisel_hammer.angle) / 360.0;
    rad *= 2 * M_PI;

    const float off = 6;

    int dir = 1;

    if (chisel_hammer.angle > 90 && chisel_hammer.angle < 270) {
        dir = -1;
    }
    
    chisel_hammer.x = chisel->x + chisel_hammer.dist * cos(rad) - dir * off * sin(rad);
    chisel_hammer.y = chisel->y + chisel_hammer.dist * sin(rad) + dir * off * cos(rad);

    const int stop = 24;
    
    switch (chisel_hammer.state) {
    case HAMMER_STATE_WINDUP:
        chisel_hammer.time++;
        if (chisel_hammer.time < 3) {
            chisel_hammer.dist += chisel_hammer.time * 4;
        } else {
            chisel_hammer.time = 0;
            chisel_hammer.state = HAMMER_STATE_SWING;
        }
        break;
    case HAMMER_STATE_SWING:
        chisel_hammer.dist -= 8;
        if (chisel_hammer.dist < stop) {
            chisel_hammer.dist = stop;
            // Activate the chisel.
            if (chisel->click_cooldown == 0) {
                chisel->click_cooldown = CHISEL_COOLDOWN;
                chisel->spd = 3.;
            }
            chisel->did_remove = false;
            chisel_hammer.state = HAMMER_STATE_IDLE;
        }
        break;
    case HAMMER_STATE_IDLE:
        chisel_hammer.dist = chisel_hammer.normal_dist;
        if (mouse_pressed[SDL_BUTTON_LEFT]) {
            chisel_hammer.state = HAMMER_STATE_WINDUP;
        }
        break;
    }
}

void chisel_hammer_draw() {
    const SDL_Rect dst = {
        chisel_hammer.x, chisel_hammer.y - chisel_hammer.h/2,
        chisel_hammer.w, chisel_hammer.h
    };
    const SDL_Point center = { 0, chisel_hammer.h/2 };

    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (chisel_hammer.angle > 90 && chisel_hammer.angle < 270) {
        flip = SDL_FLIP_VERTICAL;
    }

    SDL_RenderCopyEx(renderer, chisel_hammer.texture, NULL, &dst, chisel_hammer.angle, &center, flip);
}
