#include "knife.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdio.h>

#include "grid.h"
#include "util.h"
#include "game.h"

void knife_init() {
    struct Knife *knife = &gs->knife;

    knife->x = gs->gw/2;
    knife->y = gs->gh/2;

    knife->texture = gs->textures.knife;
    SDL_QueryTexture(knife->texture, NULL, NULL, &knife->w, &knife->h);
    
    knife->angle = 0;
    knife->pixels = push_memory(gs->persistent_memory, gs->gw*gs->gh, sizeof(Uint32));
}

void knife_tick() {
    struct Knife *knife = &gs->knife;
    struct Input *input = &gs->input;

    float px = knife->x;
    float py = knife->y;
    
    if (input->keys[SDL_SCANCODE_LCTRL]) {
        knife->angle = 180 + 360 * (atan2(input->my - knife->y, input->mx - knife->x)) / (2*M_PI);
        /* SDL_ShowCursor(1); */
    } else if (input->keys_released[SDL_SCANCODE_LCTRL]) {
        move_mouse_to_grid_position(knife->x, knife->y);
        input->mx = (int)knife->x;
        input->my = (int)knife->y;
        /* SDL_ShowCursor(0); */
    }

    if (!input->keys[SDL_SCANCODE_LCTRL]) {
        knife->x = input->mx;
        knife->y = input->my;
    }

    if (input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        float dx = knife->x - px;
        float dy = knife->y - py;
        float len = sqrt(dx*dx + dy*dy);
        float ux = dx/len;
        float uy = dy/len;

        knife->x = px;
        knife->y = py;

        while (sqrt((knife->x-px)*(knife->x-px) + (knife->y-py)*(knife->y-py)) < len) {
            for (int y = knife->y-knife->h/2; y < knife->y+knife->h/2; y++) {
                for (int x = knife->x-knife->h/2; x < knife->x+knife->h/2; x++) {
                    if (is_in_bounds(x, y)) continue;
                    if (knife->pixels[x+y*gs->gw] == 0xFFFFFF && gs->grid[x+y*gs->gw].type) {
                        gs->grid[x+y*gs->gw].depth = 256-64;
                    }
                }
            }
            knife->x += ux;
            knife->y += uy;
            knife_update_texture();
        }
        knife->x = (int)knife->x;
        knife->y = (int)knife->y;
    }
}

void knife_update_texture() {
    struct Knife *knife = &gs->knife;

    SDL_Texture *prev_target = SDL_GetRenderTarget(gs->renderer);
    SDL_SetTextureBlendMode(RenderTarget(gs, RENDER_TARGET_KNIFE), SDL_BLENDMODE_BLEND);
    
    Assert(RenderTarget(gs, RENDER_TARGET_KNIFE));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(gs, RENDER_TARGET_KNIFE));

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    
    const SDL_Rect dst = {
        knife->x, knife->y - knife->h/2,
        knife->w, knife->h
    };
    const SDL_Point center = { 0, knife->h/2 };

    SDL_RenderCopyEx(gs->renderer, knife->texture, NULL, &dst, knife->angle, &center, SDL_FLIP_NONE);

    SDL_RenderReadPixels(gs->renderer, NULL, 0, knife->pixels, 4*gs->gw);

    SDL_SetRenderTarget(gs->renderer, prev_target);
}

void knife_draw() {
    struct Knife *knife = &gs->knife;

    knife_update_texture();
    SDL_RenderCopy(gs->renderer, RenderTarget(gs, RENDER_TARGET_KNIFE), NULL, NULL);
}
