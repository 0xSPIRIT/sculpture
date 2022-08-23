#include "blob_hammer.h"

#include <SDL2/SDL_image.h>
#include <math.h>

#include "chisel.h"
#include "globals.h"
#include "grid.h"
#include "util.h"

struct Blob_Hammer blob_hammer;

void blob_hammer_init() {
    SDL_Surface *surf = IMG_Load("../res/hammer.png");

    blob_hammer.state = HAMMER_STATE_IDLE;
    blob_hammer.timer = 0;
    blob_hammer.swing_direction = 1;

    blob_hammer.x = gw/2;
    blob_hammer.y = gh/2;
    blob_hammer.w = surf->w;
    blob_hammer.h = surf->h;
    blob_hammer.texture = SDL_CreateTextureFromSurface(renderer, surf);
    blob_hammer.angle = 0;

    blob_hammer.render_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gw, gh);
    blob_hammer.pixels = calloc(gw*gh, sizeof(Uint32));

    SDL_FreeSurface(surf);
}

void blob_hammer_deinit() {
    SDL_DestroyTexture(blob_hammer.texture);
}

void blob_hammer_tick() {
    if (mouse_pressed[SDL_BUTTON_RIGHT]) {
        blob_hammer.swing_direction = (blob_hammer.swing_direction == 1) ? -1 : 1;
    }
    
    if (blob_hammer.state == HAMMER_STATE_IDLE && mouse_pressed[SDL_BUTTON_LEFT]) {
        blob_hammer.state = HAMMER_STATE_WINDUP;
        blob_hammer.timer = 10;
        blob_hammer.prev_angle = blob_hammer.angle;
    }

    blob_hammer.is_changing_angle = keys[SDL_SCANCODE_LCTRL];

    if (blob_hammer.is_changing_angle) {
        float dx = mx - blob_hammer.x;
        float dy = my - blob_hammer.y;
        
        float angle = atan2(dy, dx);
        angle /= 2 * M_PI;
        angle *= 360;
        angle += 270;
        angle = angle / 22.5;
        angle = 22.5 * (int)angle;

        int int_angle = (int)angle;
        int_angle %= 360;

        blob_hammer.angle = int_angle;
    } else {
        blob_hammer.x = mx;
        blob_hammer.y = my;
    }

    switch (blob_hammer.state) {
    case HAMMER_STATE_WINDUP:
        if (blob_hammer.timer > 0) {
            blob_hammer.timer--;
        } else {
            blob_hammer.state = HAMMER_STATE_SWING;
            blob_hammer.timer = 3;
            break;
        }

        const float speed = 5.0f;

        blob_hammer.angle += blob_hammer.swing_direction * speed;
        break;
    case HAMMER_STATE_SWING:
        if (blob_hammer.timer > 0) {
            blob_hammer.timer--;
        } else {
            blob_hammer.state = HAMMER_STATE_AFTERSWING;
            blob_hammer.timer = min(4, blob_hammer.angle - blob_hammer.prev_angle);
            break;
        }

        blob_hammer.angle -= blob_hammer.swing_direction * 25;

        blob_hammer_update_texture();

        for (int y = 0; y < gh; y++) {
            for (int x = 0; x < gw; x++) {
                if (blob_hammer.pixels[x+y*gw] == 0) continue;
                // Find the blob in this pixel,
                // then check if it's already in the list.
                // If not, add it.
                    
                Uint32 blob = objects[0].blob_data[chisel->size].blobs[x+y*gw];
                if (blob == 0) continue;

                int exists_in_array = 0;
                for (int i = 0; i < blob_hammer.blobs_touched_count; i++) {
                    if (blob_hammer.blobs_touched[i] == blob) {
                        exists_in_array = 1;
                        break;
                    }
                }

                if (!exists_in_array && blob_hammer.blobs_touched_count < MAX_BLOBS_TOUCHED) {
                    blob_hammer.blobs_touched[blob_hammer.blobs_touched_count++] = blob;
                }
            }
        }
        break;
    case HAMMER_STATE_AFTERSWING:
        if (blob_hammer.timer > 0) {
            blob_hammer.timer--;
        } else {
            blob_hammer.state = HAMMER_STATE_IDLE;
            blob_hammer.timer = 0;
            blob_hammer.angle = blob_hammer.prev_angle;

            for (int i = 0; i < blob_hammer.blobs_touched_count; i++) {
                switch_blob_to_array(grid, pickup_grid, object_current, blob_hammer.blobs_touched[i], chisel->size);
                blob_hammer.blobs_touched[i] = 0;
            }
            blob_hammer.blobs_touched_count = 0;
            break;
        }

        blob_hammer.angle += blob_hammer.swing_direction;
        break;
    }
}

void blob_hammer_update_texture() {
    SDL_Texture *prev_target = SDL_GetRenderTarget(renderer);
    SDL_SetTextureBlendMode(blob_hammer.render_texture, SDL_BLENDMODE_BLEND);
    
    SDL_SetRenderTarget(renderer, blob_hammer.render_texture);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Actually render
    SDL_Rect dst = {
        blob_hammer.x - blob_hammer.w/2, blob_hammer.y - blob_hammer.h,
        blob_hammer.w, blob_hammer.h
    };

    SDL_Point center = { blob_hammer.w/2, blob_hammer.h };
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (blob_hammer.swing_direction == -1) {
        flip = SDL_FLIP_HORIZONTAL;
    }

    int angle = blob_hammer.angle;
    while (angle < 0)
        angle += 360;

    SDL_RenderCopyEx(renderer, blob_hammer.texture, NULL, &dst, angle, &center, flip);

    // Update pixels & reset render target.
    SDL_RenderReadPixels(renderer, NULL, 0, blob_hammer.pixels, 4*gw);

    SDL_SetRenderTarget(renderer, prev_target);
}

void blob_hammer_draw() {
    blob_hammer_update_texture();
    SDL_RenderCopy(renderer, blob_hammer.render_texture, NULL, NULL);
}
