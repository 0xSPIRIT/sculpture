#include "point_knife.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "grid.h"
#include "globals.h"

// This tool is pretty much useless now.
// Maybe find another use for it?

struct PointKnife point_knife;

static int array_clamped_to_grid(int px, int py, int outside, int on_edge, int *o_arr, int *o_count);

void point_knife_init() {
    SDL_Surface *surf = IMG_Load("../res/point_knife.png");

    point_knife.x = gw/2;
    point_knife.y = gh/2;
    point_knife.w = surf->w;
    point_knife.h = surf->h;
    point_knife.texture = SDL_CreateTextureFromSurface(renderer, surf);
    point_knife.face_mode = false;
    point_knife.highlights = calloc(gw*gh, sizeof(int));

    point_knife.highlight_count = 0;
    memset(point_knife.highlights, 0, 50 * sizeof(int));

    SDL_FreeSurface(surf);
}

void point_knife_deinit() {
    SDL_DestroyTexture(point_knife.texture);
}

void point_knife_tick() {
    int index = clamp_to_grid(mx, my, !point_knife.face_mode, true, true, true);
    
    float px = point_knife.x;
    float py = point_knife.y;
    
    point_knife.x = index%gw;
    point_knife.y = index/gw;

    if (point_knife.x - px != 0 || point_knife.y - py != 0) {
        point_knife.highlight_count = array_clamped_to_grid(mx, my, !point_knife.face_mode, 1, point_knife.highlights, &point_knife.highlight_count);
    }

    /* float dx = point_knife.x - mx; */
    /* float dy = point_knife.y - my; */
    /* float dist = sqrt(dx*dx + dy*dy); */
    /* SDL_ShowCursor(dist > 2); */

    static int pressed_f = 0;
    if (keys[SDL_SCANCODE_S]) {
        if (!pressed_f) {
            point_knife.face_mode = !point_knife.face_mode;
            pressed_f = 1;
        }
    } else {
        pressed_f = 0;
    }

    static int cooldown = -1;
    static int pressed = 0;

    if (point_knife.face_mode && mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (!pressed) {
            float dx = point_knife.x - px;
            float dy = point_knife.y - py;
            float len = sqrt(dx*dx + dy*dy);
            float ux = dx/len;
            float uy = dy/len;
            point_knife.x = px;
            point_knife.y = py;
            while (len == 0 || sqrt((point_knife.x-px)*(point_knife.x-px) + (point_knife.y-py)*(point_knife.y-py)) < len) {
                grid[(int)point_knife.x + (int)point_knife.y*gw].depth = 128;
                if (len == 0) break;
                point_knife.x += ux;
                point_knife.y += uy;
            }
            point_knife.x = (int)point_knife.x;
            point_knife.y = (int)point_knife.y;
            
            pressed = 1;
        }
    } else if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (!pressed) {
            if (cooldown == -1) {
                if (get_neighbour_count((int)point_knife.x, (int)point_knife.y, 2) <= 21) { /* Must be identical to statement in array_clamped_to_grid */
                    set((int)point_knife.x, (int)point_knife.y, 0, -1);
                }
                cooldown = 12;
                objects_reevaluate();
            }
            pressed = 1;
        }
    } else {
        pressed = 0;
    }
    if (cooldown >= 0) cooldown--;
}

void point_knife_draw() {
    if (!point_knife.face_mode) {
        for (int i = 0; i < point_knife.highlight_count; i++) {
            int index = point_knife.highlights[i];
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128);
            SDL_RenderDrawPoint(renderer, index%gw, index/gw);
        }
    }

    // One more thing...

    const SDL_Rect dst = {
        point_knife.x, point_knife.y,
        point_knife.w, point_knife.h
    };
    SDL_RenderCopy(renderer, point_knife.texture, NULL, &dst);
}

static int array_clamped_to_grid(int px, int py, int outside, int on_edge, int *o_arr, int *o_count) {
    int *array = calloc(gw*gh, sizeof(int));
    int count = 0;
    
    struct Cell *grid_copy = calloc(gw*gh, sizeof(struct Cell));
    memcpy(grid_copy, grid, sizeof(struct Cell)*gw*gh);

    int x = 0;
    while (x < 17) {
        int i = clamp_to_grid(px, py, outside, on_edge, false, true);
        if (i == -1) break;
        if (get_neighbour_count(i%gw, i/gw, 2) <= 21) { /* Must be identical to the statement above */
            array[count++] = i;
            grid[i].type = 0;
        }
        x++;
    }

    memcpy(grid, grid_copy, sizeof(*grid_copy)*gw*gh);
    
    *o_count = count;
    memcpy(o_arr, array, sizeof(int) * count);

    free(array);
    free(grid_copy);

    return count;
}
