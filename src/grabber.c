#include "grabber.h"

#include <SDL2/SDL_image.h>

#include "grid.h"
#include "game.h"
#include "undo.h"

void grabber_init() {
    gs->grabber.object_holding = -1;
}

void grabber_tick() {
    struct Grabber *grabber = &gs->grabber;

    float px = grabber->x, py = grabber->y;

    grabber->x = gs->input.mx;
    grabber->y = gs->input.my;

    if (!is_in_bounds(grabber->x, grabber->y)) return;

    if (grabber->object_holding != -1) {
        int dx = (int)grabber->x-px;
        int dy = (int)grabber->y-py;
        /* float len = sqrt(dx*dx + dy*dy); */
        /* float ux = dx/len; */
        /* float uy = dy/len; */

        /* float vx = 0, vy = 0; */

        /* int i = 0; */
        /* while (sqrt(vx*vx + vy*vy) < len) { */
        object_attempt_move(grabber->object_holding, dx, dy);
        /*     vx += ux; */
        /*     vy += uy; */
        /* } */
    }

    if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        // Find object at point, and attempt move to cursor pixel by pixel.
        int object = gs->grid[(int)grabber->x+(int)grabber->y*gs->gw].object;
        if (object != -1) grabber->object_holding = object;
    } else {
        grabber->object_holding = -1;
        /* if (mouse_released[SDL_BUTTON_LEFT]) */
        /*     save_state(); */
    }
}
