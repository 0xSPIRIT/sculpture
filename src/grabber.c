#include "grabber.h"

#include <SDL2/SDL_image.h>

#include "grid.h"
#include "globals.h"
#include "undo.h"

struct Grabber grabber;

void grabber_init() {
    SDL_Surface *surf = IMG_Load("../res/pointer.png");
    grabber.texture = SDL_CreateTextureFromSurface(renderer, surf);
    grabber.w = surf->w;
    grabber.h = surf->h;
    grabber.object_holding = -1;
    SDL_FreeSurface(surf);
}

void grabber_deinit() {
    SDL_DestroyTexture(grabber.texture);
}

void grabber_tick() {
    float px = grabber.x, py = grabber.y;

    grabber.x = mx;
    grabber.y = my;

    if (!is_in_bounds(grabber.x, grabber.y)) return;

    if (grabber.object_holding != -1) {
        int dx = (int)grabber.x-px;
        int dy = (int)grabber.y-py;
        /* float len = sqrt(dx*dx + dy*dy); */
        /* float ux = dx/len; */
        /* float uy = dy/len; */

        /* float vx = 0, vy = 0; */

        /* int i = 0; */
        /* while (sqrt(vx*vx + vy*vy) < len) { */
        object_attempt_move(grabber.object_holding, dx, dy);
        /*     vx += ux; */
        /*     vy += uy; */
        /* } */
    }

    if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        // Find object at point, and attempt move to cursor pixel by pixel.
        int object = grid[(int)grabber.x+(int)grabber.y*gw].object;
        if (object != -1) grabber.object_holding = object;
    } else {
        grabber.object_holding = -1;
        /* if (mouse_released[SDL_BUTTON_LEFT]) */
        /*     save_state(); */
    }
}

void grabber_draw() {
    /* const SDL_Rect dst = { */
    /*     (int)grabber.x, (int)grabber.y, */
    /*     grabber.w, grabber.h */
    /* }; */
    /* SDL_RenderCopy(renderer, grabber.texture, NULL, &dst); */
}
