#include "placer.h"

#include <SDL2/SDL_image.h>

#include "globals.h"
#include "grid.h"
#include "converter.h"
#include "gui.h"
#include "util.h"

struct Placer *placers[PLACER_COUNT];
int current_placer = 0;

void placer_init(int num) {
    SDL_Surface *surf = IMG_Load("../res/placer.png");

    placers[num] = calloc(1, sizeof(struct Placer));
    struct Placer *placer = placers[num];

    placer->state = PLACER_PLACE_CIRCLE_MODE;
    placer->index = num;
    placer->x = gw/2;
    placer->y = gh/2;
    placer->w = surf->w;
    placer->h = surf->h;
    placer->texture = SDL_CreateTextureFromSurface(renderer, surf);
    placer->object_index = -1;
    placer->did_click = 0;
    placer->contains_type = CELL_WATER;
    placer->contains_amount = 5000;
    placer->did_take_hard = 0;

    placer->rect.x = placer->rect.y = -1;

    if (num == 1) {
        placer->contains_type = 0;
        placer->contains_amount = 0;
    }

    SDL_FreeSurface(surf);
}

void placer_deinit(int i) {
    struct Placer *placer = placers[i];
    SDL_DestroyTexture(placer->texture);
    free(placer);
}

void placer_tick(struct Placer *placer) {
    int px = placer->x;
    int py = placer->y;

    placer->x = mx;
    placer->y = my;

    // TODO: Put fading text in the screen.
    static int p = 0;
    if (keys[SDL_SCANCODE_P]) {
        if (!p) {
            if (placer->state == PLACER_SUCK_MODE) {
                placer->state = PLACER_PLACE_CIRCLE_MODE;
            } else {
                placer->state = PLACER_SUCK_MODE;
            }
            p = 1;
        }
    } else p = 0;

    switch (placer->state) {
    case PLACER_PLACE_CIRCLE_MODE: {
        static int pressed = 0;
        
        if (placer->contains_amount > 0 && !gui.popup && mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (!pressed) {
                if (cell_is_hard(placer->contains_type)) {
                    placer->object_index = object_count++;
                }
                pressed = 1;
            }

            float dx = placer->x - px;
            float dy = placer->y - py;
            float len = sqrt(dx*dx + dy*dy);
            float ux = dx/len;
            float uy = dy/len;

            float fx = px;
            float fy = py;

            int did_set_object = 1;

            placer->did_set_new = 0;

            while ((len == 0 || sqrt((fx-px)*(fx-px) + (fy-py)*(fy-py)) < len) && placer->contains_amount > 0) {
                int radius = placer->radius;
                for (int y = -radius; y <= radius; y++) {
                    for (int x = -radius; x <= radius; x++) {
                        if (x*x + y*y > radius*radius) continue;
                        if (!is_in_boundsf(x+fx, y+fy)) continue;
                        if (placer->contains_amount <= 0) {
                            placer->contains_amount = 0;
                            goto end1;
                        }

                        if (cell_is_hard(placer->contains_type) && (grid[(int)(x+fx)+(int)(fy+y)*gw].type == 0 || grid[(int)(x+fx)+(int)(fy+y)*gw].object == placer->object_index)) {
                            placer->did_set_new = 1;
                        }

                        if (grid[(int)(x+fx)+(int)(fy+y)*gw].type) continue;
                        int object_index = placer->object_index;
                        if (!cell_is_hard(placer->contains_type)) {
                            object_index = -1;
                            did_set_object = 0;
                        }
                        set((int)(x+fx), (int)(y+fy), placer->contains_type, object_index);
                        placer->contains_amount--;
                    }
                }
            end1:
                if (len == 0) break;
                fx += ux;
                fy += uy;
            }

            placer->x = (int)fx;
            placer->y = (int)fy;

            // Stop drawing / reset everything if we stopped.
            if (!placer->did_set_new) {
                goto mouse_released;
            }

            placer->did_click = did_set_object;
        } else {
            if (placer->did_click) {
            mouse_released:
                if (placer->object_index != -1) {
                    object_set_blobs(placer->object_index, 0);
                    object_set_blobs(placer->object_index, 1);
                    object_set_blobs(placer->object_index, 2);
                }
                placer->did_click = 0;
            } 
            pressed = 0;
        }
        break;
    }
    case PLACER_PLACE_RECT_MODE: {
        static int pressed = 0;
        if (!gui.popup && mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (!pressed) {
                if (cell_is_hard(placer->contains_type)) {
                    placer->object_index = object_count++;
                }
                pressed = 1;
            }

            if (placer->rect.x == -1) {
                placer->rect.x = mx;
                placer->rect.y = my;
            }

            placer->rect.w = mx - placer->rect.x;
            placer->rect.h = my - placer->rect.y;

            int area = abs((placer->rect.w) * (placer->rect.h));
            if (area > placer->contains_amount) {
                placer->rect.h = clamp(placer->rect.h, -placer->contains_amount, placer->contains_amount);

                int placer_rect_h = placer->rect.h;
                if (placer_rect_h == 0) placer_rect_h = 1;
                int w = abs(placer->contains_amount / placer_rect_h);
                placer->rect.w = clamp(placer->rect.w, -w, w);

                int placer_rect_w = placer->rect.w;
                if (placer_rect_w == 0) placer_rect_w = 1;
                int h = abs(placer->contains_amount / placer_rect_w);
                placer->rect.h = clamp(placer->rect.h, -h, h);
            }
        } else {
            pressed = 0;
            if (!gui.popup && placer->rect.x != -1) {
                if (placer->rect.w < 0) {
                    placer->rect.x += placer->rect.w;
                }
                if (placer->rect.h < 0) {
                    placer->rect.y += placer->rect.h;
                }
                placer->rect.w = abs(placer->rect.w);
                placer->rect.h = abs(placer->rect.h);
            
                for (int y = placer->rect.y; y <= placer->rect.y+placer->rect.h; y++) {
                    for (int x = placer->rect.x; x <= placer->rect.x+placer->rect.w; x++) {
                        if (!is_in_bounds(x, y)) continue;
                        if (placer->contains_amount <= 0) {
                            placer->contains_amount = 0;
                            goto end2;
                        }
                        if (grid[x+y*gw].type == placer->contains_type) continue; // Don't overwrite anything.

                        int object_index = placer->object_index;
                        if (!cell_is_hard(placer->contains_type)) {
                            object_index = -1;
                        }

                        set(x, y, placer->contains_type, object_index);
                        placer->contains_amount--;
                    }
                }
            end2:             
                placer->rect.x = -1;
                placer->rect.y = -1;
                placer->rect.w = 0;
                placer->rect.h = 0;

                if (placer->object_index != -1) {
                    object_set_blobs(placer->object_index, 0);
                    object_set_blobs(placer->object_index, 1);
                    object_set_blobs(placer->object_index, 2);
                }
            }
        }
        break;
    }
    case PLACER_SUCK_MODE:;
        if (gui.popup) break;

        /* int index = clamp_to_grid(mx, my, 1, 0, 0, 0); */
        /* if (index != -1) { */
        /*     placer->x = index%gw; */
        /*     placer->y = index/gw; */
        /* } */

        int can_continue = 0;
        if (placer->did_take_hard) {
            static int pressed = 0;
            if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                if (!pressed) {
                    can_continue = 1;
                    pressed = 1;
                }
            } else {
                pressed = 0;
            }
        } else if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            can_continue = 1;
        }

        if (!can_continue) break;

        float x = px;
        float y = py;

        float dx = mx - x;
        float dy = my - y;
        float len = sqrt((dx*dx)+(dy*dy));
        float ux, uy;
        if (len == 0) {
            ux = 0;
            uy = 0;
            len = 1;
        } else {
            ux = dx/len;
            uy = dy/len;
        }

        while (distance(x, y, px, py) < len) {
            placer->did_take_hard = 0;

            // Suck up in a circle.
            const int r = placer->radius;
            for (int dy = -r; dy <= r; dy++) {
                for (int dx = -r; dx <= r; dx++) {
                    if (dx*dx + dy*dy > r*r)  continue;

                    int xx = x+dx;
                    int yy = y+dy;
                    if (!is_in_bounds(xx, yy)) continue;

                    int type = grid[xx+yy*gw].type;
                    if (type == 0) continue;

                    if (cell_is_hard(type)) {
                        placer->did_take_hard = 1;
                    }
                    if (placer->contains_type == type || placer->contains_type == 0 || placer->contains_amount == 0) {
                        placer->contains_type = type;
                        placer->contains_amount++;
                        set(xx, yy, 0, -1);
                    }
                }
            }

            x += ux;
            y += uy;
            if (ux == 0 && uy == 0) break;
        }

        objects_reevaluate();
        break;
    }

    gui.overlay = (struct Overlay){
        placer->x + placer->w/2 + 3, placer->y - placer->h
    };
    char string[256] = {0};
    overlay_get_string(placer->contains_type, placer->contains_amount, string);
    strcpy(gui.overlay.str[0], "Placer");
    if (placer->state == PLACER_PLACE_CIRCLE_MODE || placer->state == PLACER_PLACE_RECT_MODE) {
        strcpy(gui.overlay.str[1], "Mode: [PLACE]");
    } if (placer->state == PLACER_SUCK_MODE) {
        strcpy(gui.overlay.str[1], "Mode: [TAKE]");
    } 
    strcpy(gui.overlay.str[2], string);
}

void placer_draw(struct Placer *placer) {
    if (!gui.popup && (placer->state == PLACER_SUCK_MODE || placer->state == PLACER_PLACE_CIRCLE_MODE)) {
        int radius = placer->radius;
        int fx = placer->x;
        int fy = placer->y;
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y > radius*radius) continue;
                if (x+fx < 0 || x+fx >= gw || y+fy < 0 || y+fy >= gh) continue;
                if (placer->state == PLACER_SUCK_MODE) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 64);
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 64);
                }
                SDL_RenderDrawPoint(renderer, x+fx, y+fy);
            }
        }
    }

    SDL_Rect dst = {
        placer->x - placer->w/2, placer->y - placer->h,
        placer->w, placer->h
    };
    switch (placer->index) {
    case 0:
        SDL_SetTextureColorMod(placer->texture, 255, 0, 0);
        break;
    case 1:
        SDL_SetTextureColorMod(placer->texture, 0, 255, 0);
        break;
    case 2:
        SDL_SetTextureColorMod(placer->texture, 0, 0, 255);
        break;
    }
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (converter.placer_bottom == placer) {
        flip = SDL_FLIP_VERTICAL;
    }
    SDL_RenderCopyEx(renderer, placer->texture, NULL, &dst, 0, NULL, flip);

    if (placer->rect.x != -1) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &placer->rect);
    }
}
