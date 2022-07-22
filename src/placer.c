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

    placer->state = PLACER_PLACE_RECT_MODE;
    placer->index = num;
    placer->x = gw/2;
    placer->y = gh/2;
    placer->w = surf->w;
    placer->h = surf->h;
    placer->texture = SDL_CreateTextureFromSurface(renderer, surf);
    placer->object_index = -1;
    placer->did_click = 0;
    placer->contains_current = 0;
    placer->contains_type = CELL_COAL;
    placer->contains_amount = 2000;

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

            while ((len == 0 || sqrt((fx-px)*(fx-px) + (fy-py)*(fy-py)) < len) && placer->contains_amount > 0) {
                int radius = 3;
                for (int y = -radius; y <= radius; y++) {
                    for (int x = -radius; x <= radius; x++) {
                        if (x*x + y*y > radius*radius) continue;
                        if (x+fx < 0 || x+fx >= gw || y+fy < 0 || y+fy >= gh) continue;
                        if (placer->contains_amount <= 0) {
                            placer->contains_amount = 0;
                            goto end1;
                        }
                        if (grid[(int)(x+fx)+(int)(fy+y)*gw].type == placer->contains_type) continue; // Don't overwrite anything.
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
            placer->did_click = did_set_object;
        } else {
            if (placer->did_click) {
                object_set_blobs(placer->object_index, 0);
                object_set_blobs(placer->object_index, 1);
                object_set_blobs(placer->object_index, 2);
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
                    placer->object_index = object_count;
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

                /* area = abs((placer->rect.w) * (placer->rect.h)); */

                /* if (area < placer->contains_amount) { */
                /*     int diff = placer->contains_amount - area; */
                /*     placer->rect.w--; */
                /* } */
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

                object_count++;
                printf("Count: %d\n", object_count); fflush(stdout);

                object_set_blobs(placer->object_index, 0);
                object_set_blobs(placer->object_index, 1);
                object_set_blobs(placer->object_index, 2);
            }
        }
        break;
    }
    case PLACER_SUCK_MODE:;
        /* int px = placer->x, py = placer->y; */
        // Todo for later: make it a single line that it sucks up from
        // the bottom of the placer, and do it in for loop from prev
        // position to current position.

        /* int line_y = placer->y + placer->h; */
        break;
    }
}

void placer_draw(struct Placer *placer) {
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
