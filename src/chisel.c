#include "chisel.h"

#include <math.h>
#include <SDL2/SDL_image.h>

#include "globals.h"
#include "grid.h"
#include "vectorizer.h"
#include "util.h"
#include "grid.h"

struct Chisel *chisel = &chisel_medium;
struct Chisel chisel_small, chisel_medium, chisel_large;

struct Hammer hammer;

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
    chisel->face_mode = 0;

    chisel->render_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, gh);
    chisel->pixels = calloc(gw*gh, sizeof(Uint32));

    chisel->texture = chisel->outside_texture;
    chisel->w = chisel->outside_w;
    chisel->h = chisel->outside_h;

    SDL_FreeSurface(surf);
}

void chisel_deinit(struct Chisel *type) {
    SDL_DestroyTexture(type->texture);
    SDL_DestroyTexture(type->face_texture);
    SDL_DestroyTexture(type->render_texture);
}

void chisel_tick() {
    int p = chisel->changing_angle;
    chisel->changing_angle = keys[SDL_SCANCODE_LCTRL];
    if (p && !chisel->changing_angle) {
        SDL_WarpMouseInWindow(window, (int)chisel->x*S, (int)chisel->y*S);
        mx = (int)chisel->x;
        my = (int)chisel->y;
    }

    if (hammer.state == HAMMER_STATE_IDLE && !chisel->changing_angle && !chisel->click_cooldown) {
        int index = clamp_to_grid(mx, my, !chisel->face_mode, 0, 1);
        if (index != -1) {
            chisel->x = index%gw;
            chisel->y = index/gw;
        }
    }

    if (chisel->changing_angle) {
        chisel->angle = 180 + 360 * (atan2(my - chisel->y, mx - chisel->x)) / (2*M_PI);
        /* if (!chisel->face_mode) { */
            chisel->angle /= 45;
            chisel->angle = ((int)chisel->angle) * 45;
        /* } */
        SDL_ShowCursor(1);
    } else {
        float dx = chisel->x - mx;
        float dy = chisel->y - my;
        float dist = sqrt(dx*dx + dy*dy);
        SDL_ShowCursor(dist > 1);
    }

    static int pressed_f = 0;
    if (keys[SDL_SCANCODE_F]) {
        if (!pressed_f) {
            chisel->face_mode = !chisel->face_mode;
            chisel->w = chisel->face_mode ? chisel->face_w : chisel->outside_w;
            chisel->h = chisel->face_mode ? chisel->face_h : chisel->outside_h;
            chisel->texture = chisel->face_mode ? chisel->face_texture : chisel->outside_texture;
            pressed_f = 1;
        }
    } else {
        pressed_f = 0;
    }
    
    if (mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        chisel->changing_angle = 0;
    }

    if (chisel->click_cooldown) {
        if (chisel->click_cooldown >= CHISEL_COOLDOWN-CHISEL_TIME) {
            // Cut out the stone.
            float px = chisel->x;
            float py = chisel->y;
            float chisel_dx = cos(2*M_PI * ((chisel->angle+180) / 360.0));
            float chisel_dy = sin(2*M_PI * ((chisel->angle+180) / 360.0));
            float dx = chisel->spd * chisel_dx;
            float dy = chisel->spd * chisel_dy;
            float len = sqrt(dx*dx + dy*dy);
            float ux = dx/len;
            float uy = dy/len;
            
            if (chisel->face_mode) {
                while (sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y)) < len) {
                    for (int y = 0; y < gh; y++) {
                        for (int x = 0; x < gw; x++) {
                            if (chisel->pixels[x+y*gw] == 0x9B9B9B) {
                                if (grid[x+y*gw].object != -1) grid[x+y*gw].depth = 128;
                            }
                        }
                    }
                    chisel->x += ux;
                    chisel->y += uy;
                    chisel_update_texture();
                }
                SDL_WarpMouseInWindow(window, (int)(chisel->x * S), (int)(chisel->y * S));
                mx = chisel->x;
                my = chisel->y;
            } else if (object_current != -1) {
                while (sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y)) < len) {
                    // If we come into contact with a cell, locate its blob
                    // then remove it. We only remove one blob per chisel,
                    // so we stop our speed right here.

                    int b = objects[object_current].blob_data[chisel->size].blobs[(int)chisel->x + ((int)chisel->y)*gw];
                    if (b > 0 && !chisel->did_remove) {
                        // We want to have the chisel end up right at the edge of the
                        // blob itself, to make it seem like it really did knock that
                        // entire thing out.
                        // So, we continue at our current direction until we reach
                        // another blob, and we backtrack one.

                        while (objects[object_current].blob_data[chisel->size].blobs[(int)chisel->x + ((int)chisel->y)*gw] == b) {
                            chisel->x += ux;
                            chisel->y += uy;
                            /* if (sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y)) >= len) break; */
                        }

                        chisel->x -= ux;
                        chisel->y -= uy;

                        if ((float)objects[object_current].blob_data[chisel->size].blob_pressures[b] / MAX_PRESSURE < 0.75) {
                            object_remove_blob(object_current, b, chisel->size);
                            SDL_WarpMouseInWindow(window, (int)(chisel->x * S), (int)(chisel->y * S));
                        }

                        chisel->did_remove = 1;

                        chisel->click_cooldown = CHISEL_COOLDOWN-CHISEL_TIME-1;
                        break;
                    }

                    chisel->x += ux;
                    chisel->y += uy;
                }
                int b = objects[object_current].blob_data[chisel->size].blobs[(int)chisel->x + ((int)chisel->y)*gw];
                if (b > 0 && !chisel->did_remove) {
                    if ((float)objects[object_current].blob_data[chisel->size].blob_pressures[b] / MAX_PRESSURE < 0.75) {
                        object_remove_blob(object_current, b, chisel->size);
                    }
                }

                objects_reevalute();
            }
        }
        chisel->click_cooldown--;
        if (chisel->click_cooldown == 0) {
            chisel->line = NULL;
            int index = clamp_to_grid(mx, my, !chisel->face_mode, 0, 1);
            chisel->x = index%gw;
            chisel->y = index/gw;
        }
    }
    hammer_tick();
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
    
    const SDL_Rect dst = {
        x, y - chisel->h/2,
        chisel->w, chisel->h
    };
    const SDL_Point center = { 0, chisel->h/2 };

    SDL_RenderCopyEx(renderer, chisel->texture, NULL, &dst, chisel->angle, &center, SDL_FLIP_NONE);

    hammer_draw();

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
}

void hammer_init() {
    hammer.x = chisel->x;
    hammer.y = chisel->y;
    hammer.normal_dist = chisel->w+4;
    hammer.dist = hammer.normal_dist;
    hammer.time = 0;
    hammer.angle = 0;

    SDL_Surface *surf = IMG_Load("../res/hammer.png");
    hammer.w = surf->w;
    hammer.h = surf->h;
    hammer.texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
}

void hammer_deinit() {
    SDL_DestroyTexture(hammer.texture);
}

void hammer_tick() {
    hammer.angle = chisel->angle;

    float rad = (hammer.angle) / 360.0;
    rad *= 2 * M_PI;

    const float off = 6;

    int dir = 1;

    if (hammer.angle > 90 && hammer.angle < 270) {
        dir = -1;
    }
    
    hammer.x = chisel->x + hammer.dist * cos(rad) - dir * off * sin(rad);
    hammer.y = chisel->y + hammer.dist * sin(rad) + dir * off * cos(rad);

    static int prevclicked = 0;

    const int stop = 24;
    
    switch (hammer.state) {
    case HAMMER_STATE_WINDUP:
        hammer.time++;
        if (hammer.time < 3) {
            hammer.dist += hammer.time * 4;
        } else {
            hammer.time = 0;
            hammer.state = HAMMER_STATE_SWING;
        }
        break;
    case HAMMER_STATE_SWING:
        hammer.dist -= 8;
        if (hammer.dist < stop) {
            hammer.dist = stop;
            // Activate the chisel.
            if (chisel->click_cooldown == 0) {
                chisel->click_cooldown = CHISEL_COOLDOWN;
                chisel->spd = 3.;
            }
            chisel->did_remove = 0;
            hammer.state = HAMMER_STATE_IDLE;
        }
        break;
    case HAMMER_STATE_IDLE:
        hammer.dist = hammer.normal_dist;
        if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (!prevclicked) {
                hammer.state = HAMMER_STATE_WINDUP;
                prevclicked = 1;
            }
        } else {
            prevclicked = 0;
        }
        break;
    }
}

void hammer_draw() {
    const SDL_Rect dst = {
        hammer.x, hammer.y - hammer.h/2,
        hammer.w, hammer.h
    };
    const SDL_Point center = { 0, hammer.h/2 };

    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (hammer.angle > 90 && hammer.angle < 270) {
        flip = SDL_FLIP_VERTICAL;
    }

    SDL_RenderCopyEx(renderer, hammer.texture, NULL, &dst, hammer.angle, &center, flip);
}
