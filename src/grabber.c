#include "grabber.h"

#include <SDL2/SDL_image.h>

#include "grid.h"
#include "game.h"
#include "undo.h"

void grabber_init() {
    gs->grabber.texture = 0;
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
        object_attempt_move(grabber->object_holding, dx, dy);
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

void grabber_draw() {
    /* const SDL_Rect dst = { */
    /*     (int)grabber->x, (int)grabber->y, */
    /*     grabber->w, grabber->h */
    /* }; */
    /* SDL_RenderCopy(gs->renderer, grabber->texture, NULL, &dst); */
}
