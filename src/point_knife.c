#include "point_knife.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "grid.h"
#include "globals.h"
#include "game.h"

internal int array_clamped_to_grid(int px, int py, int outside, int on_edge, int *o_arr, int *o_count);

void point_knife_init() {
    struct Point_Knife *point_knife = &gs->point_knife;
    SDL_Surface *surf = IMG_Load("../res/point_knife.png");

    point_knife->x = gs->gw/2;
    point_knife->y = gs->gh/2;
    point_knife->texture = gs->textures.point_knife;
    SDL_QueryTexture(point_knife->texture, NULL, NULL, &point_knife->w, &point_knife->h);

    point_knife->face_mode = false;
    point_knife->highlights = persist_alloc(gs->gw*gs->gh, sizeof(int));

    point_knife->highlight_count = 0;
    memset(point_knife->highlights, 0, 50 * sizeof(int));

    SDL_FreeSurface(surf);
}

void point_knife_tick() {
    struct Point_Knife *point_knife = &gs->point_knife;
    struct Input *input = &gs->input;

    int index = clamp_to_grid(input->mx, input->my, !point_knife->face_mode, true, true, true);
    
    float px = point_knife->x;
    float py = point_knife->y;
    
    point_knife->x = index%gs->gw;
    point_knife->y = index/gs->gw;

    if (point_knife->x - px != 0 || point_knife->y - py != 0) {
        point_knife->highlight_count = array_clamped_to_grid(input->mx, input->my, !point_knife->face_mode, 1, point_knife->highlights, &point_knife->highlight_count);
    }

    /* float dx = point_knife->x - input->mx; */
    /* float dy = point_knife->y - input->my; */
    /* float dist = sqrt(dx*dx + dy*dy); */
    /* SDL_ShowCursor(dist > 2); */
    
    if (input->keys_pressed[SDL_SCANCODE_S]) {
        point_knife->face_mode = !point_knife->face_mode;
    }

    persist int cooldown = -1;

    if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
        if (point_knife->face_mode) {
            float dx = point_knife->x - px;
            float dy = point_knife->y - py;
            float len = sqrt(dx*dx + dy*dy);
            float ux = dx/len;
            float uy = dy/len;
            point_knife->x = px;
            point_knife->y = py;
            while (len == 0 || sqrt((point_knife->x-px)*(point_knife->x-px) + (point_knife->y-py)*(point_knife->y-py)) < len) {
                gs->grid[(int)point_knife->x + (int)point_knife->y*gs->gw].depth = 128;
                if (len == 0) break;
                point_knife->x += ux;
                point_knife->y += uy;
            }
            point_knife->x = (int)point_knife->x;
            point_knife->y = (int)point_knife->y;
        } else {
            if (cooldown == -1) {
                if (get_neighbour_count((int)point_knife->x, (int)point_knife->y, 2) <= 21) { /* Must be identical to statement in array_clamped_to_grid */
                    set((int)point_knife->x, (int)point_knife->y, 0, -1);
                }
                cooldown = 12;
                objects_reevaluate();
            }
        }
    }
    if (cooldown >= 0) cooldown--;
}

void point_knife_draw() {
    struct Point_Knife *point_knife = &gs->point_knife;

    if (!point_knife->face_mode) {
        for (int i = 0; i < point_knife->highlight_count; i++) {
            int index = point_knife->highlights[i];
            SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 128);
            SDL_RenderDrawPoint(gs->renderer, index%gs->gw, index/gs->gw);
        }
    }

    // One more thing...

    const SDL_Rect dst = {
        point_knife->x, point_knife->y,
        point_knife->w, point_knife->h
    };
    SDL_RenderCopy(gs->renderer, point_knife->texture, NULL, &dst);
}

internal int array_clamped_to_grid(int px, int py, int outside, int on_edge, int *o_arr, int *o_count) {
    int *array = temp_alloc(gs->gw*gs->gh, sizeof(int));
    int count = 0;
    
    struct Cell *grid_copy = temp_alloc(gs->gw*gs->gh, sizeof(struct Cell));
    memcpy(grid_copy, gs->grid, sizeof(struct Cell)*gs->gw*gs->gh);

    int x = 0;
    while (x < 17) {
        int i = clamp_to_grid(px, py, outside, on_edge, false, true);
        if (i == -1) break;
        if (get_neighbour_count(i%gs->gw, i/gs->gw, 2) <= 21) { /* Must be identical to the statement above */
            array[count++] = i;
            gs->grid[i].type = 0;
        }
        x++;
    }

    memcpy(gs->grid, grid_copy, sizeof(*grid_copy)*gs->gw*gs->gh);
    
    *o_count = count;
    memcpy(o_arr, array, sizeof(int) * count);

    temp_dealloc(array);
    temp_dealloc(grid_copy);

    return count;
}
