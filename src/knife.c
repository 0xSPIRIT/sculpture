#include "knife.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "globals.h"
#include "grid.h"
#include "util.h"

struct Knife knife;

void knife_init() {
    SDL_Surface *surf = IMG_Load("../res/knife.png");

    knife.x = gw/2;
    knife.y = gh/2;
    knife.w = surf->w;
    knife.h = surf->h;
    knife.texture = SDL_CreateTextureFromSurface(renderer, surf);
    knife.render_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, gh);
    knife.angle = 0;
    knife.pixels = calloc(gw*gh, sizeof(Uint32));

    SDL_FreeSurface(surf);
}

void knife_deinit() {
    SDL_DestroyTexture(knife.texture);
    SDL_DestroyTexture(knife.render_texture);
}

void knife_tick() {
    float px = knife.x;
    float py = knife.y;
    
    if (keys[SDL_SCANCODE_LCTRL]) {
        knife.angle = 180 + 360 * (atan2(my - knife.y, mx - knife.x)) / (2*M_PI);
        /* SDL_ShowCursor(1); */
    } else if (keys_released[SDL_SCANCODE_LCTRL]) {
        /* SDL_WarpMouseInWindow(window, (int)knife.x*S, GUI_H + (int)knife.y*S); */
        move_mouse_to_grid_position(knife.x, knife.y);
        mx = (int)knife.x;
        my = (int)knife.y;
        /* SDL_ShowCursor(0); */
    }

    if (!keys[SDL_SCANCODE_LCTRL]) {
        knife.x = mx;
        knife.y = my;
    }

    if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        float dx = knife.x - px;
        float dy = knife.y - py;
        float len = sqrt(dx*dx + dy*dy);
        float ux = dx/len;
        float uy = dy/len;

        knife.x = px;
        knife.y = py;

        while (sqrt((knife.x-px)*(knife.x-px) + (knife.y-py)*(knife.y-py)) < len) {
            for (int y = knife.y-knife.h/2; y < knife.y+knife.h/2; y++) {
                for (int x = knife.x-knife.h/2; x < knife.x+knife.h/2; x++) {
                    if (is_in_bounds(x, y)) continue;
                    if (knife.pixels[x+y*gw] == 0xFFFFFF && grid[x+y*gw].type) {
                        grid[x+y*gw].depth = 256-64;
                    }
                }
            }
            knife.x += ux;
            knife.y += uy;
            knife_update_texture();
        }
        knife.x = (int)knife.x;
        knife.y = (int)knife.y;
    }
}

void knife_update_texture() {
    SDL_Texture *prev_target = SDL_GetRenderTarget(renderer);
    SDL_SetTextureBlendMode(knife.render_texture, SDL_BLENDMODE_BLEND);
    
    SDL_SetRenderTarget(renderer, knife.render_texture);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    const SDL_Rect dst = {
        knife.x, knife.y - knife.h/2,
        knife.w, knife.h
    };
    const SDL_Point center = { 0, knife.h/2 };

    SDL_RenderCopyEx(renderer, knife.texture, NULL, &dst, knife.angle, &center, SDL_FLIP_NONE);

    SDL_RenderReadPixels(renderer, NULL, 0, knife.pixels, 4*gw);

    SDL_SetRenderTarget(renderer, prev_target);
}

void knife_draw() {
    knife_update_texture();
    SDL_RenderCopy(renderer, knife.render_texture, NULL, NULL);
}
