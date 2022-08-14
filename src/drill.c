#include "drill.h"

#include <SDL2/SDL_image.h>

#include "globals.h"
#include "grid.h"
#include "util.h"

// God, this tool is essentially useless.

struct Drill drill;

void drill_init() {
    SDL_Surface *surf = IMG_Load("../res/drill.png");

    drill.x = gw/2;
    drill.y = gh/2;
    drill.w = surf->w;
    drill.h = surf->h;
    drill.texture = SDL_CreateTextureFromSurface(renderer, surf);
    drill.angle = 0;

    SDL_FreeSurface(surf);
}

void drill_deinit() {
    SDL_DestroyTexture(drill.texture);
}

void drill_tick() {
    if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        float dx = cos(2*M_PI * ((drill.angle+180) / 360.0));
        float dy = sin(2*M_PI * ((drill.angle+180) / 360.0));
        drill.x += dx;
        drill.y += dy;
        /* SDL_WarpMouseInWindow(window, (int)(drill.x * S), GUI_H + (int)(drill.y * S)); */
        move_mouse_to_grid_position(drill.x, drill.y);
        mx = (int)drill.x;
        my = (int)drill.y;
    } else if (keys[SDL_SCANCODE_LCTRL]) {
        drill.angle = 180 + 360 * (atan2(my - drill.y, mx - drill.x)) / (2*M_PI);
    } else {
        drill.x = mx;
        drill.y = my;
    }

    /* float dx = drill.x - mx; */
    /* float dy = drill.y - my; */
    /* float dist = sqrt(dx*dx + dy*dy); */
    /* SDL_ShowCursor(dist > 2); */
}

void drill_draw() {
    SDL_Rect dst = {
        drill.x, drill.y - 1,
        drill.w, drill.h
    };
    const SDL_Point center = { 0, 1 };

    SDL_RendererFlip flip = SDL_FLIP_NONE;

    SDL_RenderCopyEx(renderer, drill.texture, NULL, &dst, drill.angle, &center, flip);
}
