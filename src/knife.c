#include "knife.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "globals.h"
#include "grid.h"

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
    
    static int did_lctrl = 0;
    if (keys[SDL_SCANCODE_LCTRL]) {
        knife.angle = 180 + 360 * (atan2(my - knife.y, mx - knife.x)) / (2*M_PI);
        did_lctrl = 1;
        SDL_ShowCursor(1);
    } else {
        if (did_lctrl) {
            SDL_WarpMouseInWindow(window, (int)knife.x*S, (int)knife.y*S);
            mx = (int)knife.x;
            my = (int)knife.y;
        }
        SDL_ShowCursor(0);

        knife.x = mx;
        knife.y = my;

        did_lctrl = 0;
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
