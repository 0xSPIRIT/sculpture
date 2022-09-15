#include "blob_hammer.h"

#include <SDL2/SDL_image.h>
#include <math.h>

#include "chisel.h"
#include "grid.h"
#include "util.h"
#include "undo.h"
#include "game.h"

void blob_hammer_init() {
    struct Blob_Hammer *blob_hammer = &gs->blob_hammer;
    
    blob_hammer->state = HAMMER_STATE_IDLE;
    blob_hammer->timer = 0;
    blob_hammer->swing_direction = 1;

    blob_hammer->x = (float)gs->gw/2;
    blob_hammer->y = (float)gs->gh/2;
    blob_hammer->texture = gs->textures.blob_hammer;
    SDL_QueryTexture(blob_hammer->texture, NULL, NULL, &blob_hammer->w, &blob_hammer->h);
    blob_hammer->angle = 0;

    blob_hammer->render_texture = gs->textures.blob_hammer_render_target;
    blob_hammer->pixels = persist_alloc(gs->gw*gs->gh, sizeof(Uint32));
}

void blob_hammer_tick() {
    struct Blob_Hammer *blob_hammer = &gs->blob_hammer;
    struct Input *input = &gs->input;
    struct Chisel *chisel = gs->chisel;
    
    if (input->mouse_pressed[SDL_BUTTON_RIGHT]) {
        blob_hammer->swing_direction = (blob_hammer->swing_direction == 1) ? -1 : 1;
    }
    
    if (blob_hammer->state == HAMMER_STATE_IDLE && input->mouse_pressed[SDL_BUTTON_LEFT]) {
        blob_hammer->state = HAMMER_STATE_WINDUP;
        blob_hammer->timer = 10;
        blob_hammer->prev_angle = blob_hammer->angle;
    }

    blob_hammer->is_changing_angle = input->keys[SDL_SCANCODE_LCTRL];

    if (blob_hammer->is_changing_angle) {
        float dx = input->mx - blob_hammer->x;
        float dy = input->my - blob_hammer->y;
        
        float angle = (float)atan2(dy, dx);
        angle /= 2 * (float)M_PI;
        angle *= 360;
        angle += 270;
        angle = angle / 22.5f;
        angle = 22.5f * (int)angle;

        int int_angle = (int)angle;
        int_angle %= 360;

        blob_hammer->angle = (float)int_angle;
    } else {
        blob_hammer->x = (float)input->mx;
        blob_hammer->y = (float)input->my;
    }

    switch (blob_hammer->state) {
    case HAMMER_STATE_WINDUP:
        if (blob_hammer->timer > 0) {
            blob_hammer->timer--;
        } else {
            blob_hammer->state = HAMMER_STATE_SWING;
            blob_hammer->timer = 3;
            break;
        }

        const float speed = 5.0f;

        blob_hammer->angle += blob_hammer->swing_direction * speed;
        break;
    case HAMMER_STATE_SWING:
        if (blob_hammer->timer > 0) {
            blob_hammer->timer--;
        } else {
            blob_hammer->state = HAMMER_STATE_AFTERSWING;
            blob_hammer->timer = (int) minimum(4, (int)(blob_hammer->angle - blob_hammer->prev_angle));
            break;
        }

        blob_hammer->angle -= blob_hammer->swing_direction * 25;

        blob_hammer_update_texture();

        for (int y = 0; y < gs->gh; y++) {
            for (int x = 0; x < gs->gw; x++) {
                if (blob_hammer->pixels[x+y*gs->gw] == 0) continue;
                // Find the blob in this pixel,
                // then check if it's already in the list.
                // If not, add it.
                    
                Uint32 blob = gs->objects[0].blob_data[chisel->size].blobs[x+y*gs->gw];
                if (blob == 0) continue;

                int exists_in_array = 0;
                for (int i = 0; i < blob_hammer->blobs_touched_count; i++) {
                    if (blob_hammer->blobs_touched[i] == blob) {
                        exists_in_array = 1;
                        break;
                    }
                }

                if (!exists_in_array && blob_hammer->blobs_touched_count < MAX_BLOBS_TOUCHED) {
                    blob_hammer->blobs_touched[blob_hammer->blobs_touched_count++] = blob;
                }
            }
        }
        break;
    case HAMMER_STATE_AFTERSWING:
        if (blob_hammer->timer > 0) {
            blob_hammer->timer--;
        } else {
            blob_hammer->state = HAMMER_STATE_IDLE;
            blob_hammer->timer = 0;
            blob_hammer->angle = blob_hammer->prev_angle;

            for (int i = 0; i < blob_hammer->blobs_touched_count; i++) {
                switch_blob_to_array(gs->grid,
                                     gs->pickup_grid,
                                     gs->object_current,
                                     blob_hammer->blobs_touched[i],
                                     chisel->size);
                blob_hammer->blobs_touched[i] = 0;
            }
            blob_hammer->blobs_touched_count = 0;

            save_state_to_next();
            break;
        }

        blob_hammer->angle += blob_hammer->swing_direction;
        break;
    }
}

void blob_hammer_update_texture() {
    struct Blob_Hammer *blob_hammer = &gs->blob_hammer;
    
    SDL_Texture *prev_target = SDL_GetRenderTarget(gs->renderer);
    SDL_SetTextureBlendMode(blob_hammer->render_texture, SDL_BLENDMODE_BLEND);
    
    SDL_SetRenderTarget(gs->renderer, blob_hammer->render_texture);

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);

    // Actually render
    SDL_Rect dst = {
        (int)blob_hammer->x - blob_hammer->w/2,
        (int)blob_hammer->y - blob_hammer->h,
        blob_hammer->w, blob_hammer->h
    };

    SDL_Point center = { blob_hammer->w/2, blob_hammer->h };
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (blob_hammer->swing_direction == -1) {
        flip = SDL_FLIP_HORIZONTAL;
    }

    int angle = (int)blob_hammer->angle;
    while (angle < 0)
        angle += 360;

    SDL_RenderCopyEx(gs->renderer, blob_hammer->texture, NULL, &dst, angle, &center, flip);

    // Update pixels & reset render target.
    SDL_RenderReadPixels(gs->renderer, NULL, 0, blob_hammer->pixels, 4*gs->gw);

    SDL_SetRenderTarget(gs->renderer, prev_target);
}

void blob_hammer_draw() {
    struct Blob_Hammer *blob_hammer = &gs->blob_hammer;
    
    blob_hammer_update_texture();
    SDL_RenderCopy(gs->renderer, blob_hammer->render_texture, NULL, NULL);
}
