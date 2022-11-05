void deleter_init(void) {
    struct Deleter *deleter = &gs->deleter;

    deleter->highlight_count = 0;

    if (deleter->highlights == NULL) {
        deleter->highlights = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(int));
    }

    if (deleter->pixels == NULL) {
        deleter->pixels = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(Uint32));
    }

    deleter->texture = gs->textures.deleter;
    SDL_QueryTexture(deleter->texture, NULL, NULL, &deleter->w, &deleter->h);
}

void take_point(void) {
    struct Deleter *deleter = &gs->deleter;

    if (deleter->point_count == DELETER_POINT_COUNT-1) {
        return;
    }

    SDL_Point mouse = (SDL_Point){(int)gs->deleter.x, (int)gs->deleter.y};

    // memcmp returns zero if the buffers are the same.
    if (0 != memcmp(&mouse, &deleter->points[deleter->point_count-1], sizeof(SDL_Point))) {
        deleter->points[deleter->point_count++] = mouse;
    }
}

// A special number marking a pixel as marked in deleter->pixels.
const Uint32 marked = 1234567890;
// TODO: Account for different color formats, this assumes ABGR
#define red 0xFF0000FF

void deleter_fill_neighbours(int x, int y) {
    struct Deleter *deleter = &gs->deleter;

    if (!is_in_bounds(x, y))              return;
    if (deleter->pixels[x+y*gs->gw] != 0) return;

    deleter->pixels[x+y*gs->gw] = marked;

    deleter_fill_neighbours(x+1, y);
    deleter_fill_neighbours(x-1, y);
    deleter_fill_neighbours(x, y+1);
    deleter_fill_neighbours(x, y-1);
}

void deleter_delete(void) {
    struct Deleter *deleter = &gs->deleter;
    
    save_state_to_next();
    
    deleter_fill_neighbours(0, 0);
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            Uint32 color = deleter->pixels[x+y*gs->gw];
            
            if (color == 0 || color == red) {
                set(x, y, 0, -1);
            } 
            
            if (color != 0 && color != red && color != marked) {
                Log("%x\n", color);
            }
        }
    }

    objects_reevaluate();
}

void deleter_stop(bool cancel) {
    struct Deleter *deleter = &gs->deleter;
    
    if (!cancel) {
        deleter_delete();
    }

    memset(deleter->points, 0, sizeof(struct SDL_Point)*DELETER_POINT_COUNT);
    deleter->point_count = 0;
    deleter->active = false;

    if (cancel)
        deleter->was_active = false; // To prevent deletion the second time around if you cancel.
}

void deleter_tick(void) {
    struct Deleter *deleter = &gs->deleter;
    struct Input *input = &gs->input;

    if (gs->is_mouse_over_any_button) return;

    deleter->x = (f32) input->mx;
    deleter->y = (f32) input->my;

    if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
        deleter->active = true;
    } else if (!(input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        deleter->active = false;
    }
    
    const int min_distance = 8;
    
    if (deleter->active) {
        if (deleter->point_count == 0) {
            take_point();
        } else if (distancei(deleter->points[deleter->point_count-1].x,
                             deleter->points[deleter->point_count-1].y,
                             deleter->x,
                             deleter->y) >= min_distance) {
            take_point();
        }
    } else if (deleter->was_active) {
        deleter_stop(false);
    }                                   

    deleter->was_active = deleter->active;
}

void deleter_draw(void) {
    struct Deleter *deleter = &gs->deleter;

    const SDL_Rect dst = {
        (int) deleter->x, (int) deleter->y,
        deleter->w, deleter->h
    };

    if (deleter->active) {
        SDL_SetTextureColorMod(deleter->texture, 127, 127, 127);
    } else {
        SDL_SetTextureColorMod(deleter->texture, 255, 255, 255);
    }

    SDL_RenderCopy(gs->renderer, deleter->texture, NULL, &dst);

    SDL_Texture *prev_target = SDL_GetRenderTarget(gs->renderer);
    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_DELETER), SDL_BLENDMODE_BLEND);
    
    Assert(RenderTarget(RENDER_TARGET_DELETER));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_DELETER));

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);

    if (deleter->point_count) {
        SDL_Point prev = {-1, -1};

        SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 255);

        for (int i = 0; i < DELETER_POINT_COUNT; i++) {
            if (deleter->points[i].x == 0 && deleter->points[i].y == 0) continue;
        
            if (prev.x != -1) {
                SDL_RenderDrawLine(gs->renderer, 
                                   prev.x, 
                                   prev.y, 
                                   deleter->points[i].x,
                                   deleter->points[i].y);
            } else {
                SDL_RenderDrawPoint(gs->renderer, 
                                    deleter->points[i].x, 
                                    deleter->points[i].y);
            }

            prev = deleter->points[i];
        }

        SDL_RenderDrawLine(gs->renderer, deleter->points[0].x, deleter->points[0].y,
                           deleter->points[deleter->point_count-1].x,
                           deleter->points[deleter->point_count-1].y);
    }

    SDL_RenderReadPixels(gs->renderer, NULL, 0, deleter->pixels, 4*gs->gw);

    SDL_SetRenderTarget(gs->renderer, prev_target);

    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_DELETER), NULL, NULL);
    
    if (deleter->point_count > 0) {
        SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 100);
        SDL_RenderDrawLine(gs->renderer,
                           deleter->points[deleter->point_count-1].x,
                           deleter->points[deleter->point_count-1].y,
                           deleter->x,
                           deleter->y);
        SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    }
}
