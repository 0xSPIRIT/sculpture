int find_blocker_side(f32 x, f32 y) {
    struct Blocker *blocker = &gs->blocker;
    Uint32 *pixels = blocker->pixels;
    
    int v = pixels[(int)x+(int)y*gs->gw];
    
    if (v != 1 && v != 2) { // We're on the blocker itself.
        v = -1;
    }
    
    return v;
}

void blocker_add_point(int x, int y) {
    struct Blocker *blocker = &gs->blocker;
    blocker->points[blocker->point_count].x = x;
    blocker->points[blocker->point_count].y = y;
    blocker->point_count++;
}

void blocker_init(void) {
    struct Blocker *blocker = &gs->blocker;
    
    blocker->state = BLOCKER_STATE_LINE;
    blocker->point_count = 0;
    memset(blocker->points, 0, BLOCKER_MAX_POINTS * sizeof(struct SDL_Point));
    
    blocker->angle = 0;
    blocker->circle_size = 1;
    blocker->active = false;
    
    if (blocker->pixels == NULL) {
        blocker->pixels = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(Uint32));
    }
}

void blocker_tick(void) {
    struct Blocker *blocker = &gs->blocker;
    struct Input *input = &gs->input;
    
    if (gs->is_mouse_over_any_button) return;
    
    if (input->keys_pressed[SDL_SCANCODE_C]) {
        blocker->state = (blocker->state == BLOCKER_STATE_LINE) ?
            BLOCKER_STATE_CURVE : BLOCKER_STATE_LINE;
    }
    
    switch (blocker->state) {
        case BLOCKER_STATE_LINE: {
            if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
                memset(blocker->points, 0, BLOCKER_MAX_POINTS * sizeof(SDL_Point));
                blocker->point_count = 0;
                
                blocker->active = true;
                
                blocker->points[0].x = input->mx;
                blocker->points[0].y = input->my;
                blocker->points[1].x = input->mx;
                blocker->points[1].y = input->my;
                blocker->point_count = 2;
            } else if (input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                blocker->points[1].x = input->mx;
                blocker->points[1].y = input->my;
                blocker->point_count = 2;
                
                blocker->active = true;
                
                f64 dx = blocker->points[1].x - blocker->points[0].x;
                f64 dy = blocker->points[1].y - blocker->points[0].y;
                
                f64 length = distance64(blocker->points[0].x, blocker->points[0].y,
                                        blocker->points[1].x, blocker->points[1].y);
                
                const f64 clamp = 22.5;
                
                blocker->angle = atan2f(dy, dx);
                
                if (input->keys[SDL_SCANCODE_LSHIFT]) {
                    blocker->angle /= 2 * M_PI;
                    blocker->angle *= 360;
                    blocker->angle = ((int)blocker->angle) % 360;
                    blocker->angle = blocker->angle / clamp;
                    blocker->angle = clamp * round(blocker->angle);
                    
                    blocker->angle /= 360.f;
                    blocker->angle *= 2 * M_PI;
                    
                    f64 ux = cos(blocker->angle);
                    f64 uy = sin(blocker->angle);
                    
                    blocker->points[1].x = round(blocker->points[0].x + ux*length);
                    blocker->points[1].y = round(blocker->points[0].y + uy*length);
                }
            } else if (input->mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                memset(blocker->pixels, 0, sizeof(Uint32)*gs->gw*gs->gh);
                memset(blocker->points, 0, sizeof(SDL_Point)*BLOCKER_MAX_POINTS);
                blocker->point_count = 0;
                blocker->active = false;
            }
            
            blocker->prev_state = blocker->state;
            break;
        }
        case BLOCKER_STATE_CURVE: {
            if (input->mouse_pressed[SDL_BUTTON_RIGHT]) {
                memset(blocker->points, 0, BLOCKER_MAX_POINTS * sizeof(SDL_Point));
                blocker->point_count = 0;
                
                blocker_add_point(input->mx, input->my);
                blocker_add_point(input->mx, input->my);
            }
            
            if (input->mouse_pressed[SDL_BUTTON_LEFT] && blocker->point_count < BLOCKER_MAX_POINTS) {
                blocker_add_point(input->mx, input->my);
            }
            
            blocker->prev_state = blocker->state;
            break;
        }
    }
}

void blocker_draw_line(f64 deg_angle) {
    struct Blocker *blocker = &gs->blocker;
    
    if (is_angle_225(deg_angle)) {
        SDL_Point a = blocker->points[0];
        SDL_Point b = blocker->points[1];
        
        draw_line_225(deg_angle, a, b, blocker->circle_size, true);
        
        int dx = sign(a.x - b.x);
        int dy = sign(a.y - b.y);
        
        b.x += dx*gs->gw;
        b.y += dy*gs->gw*2;
        draw_line_225(deg_angle, a, b, blocker->circle_size, false);
    } else {
        SDL_Point prev = {-1, -1};
        for (int i = 0; i < blocker->point_count; i++) {
            if (prev.x != -1) {
                SDL_Point curr = blocker->points[i];
                
                SDL_Point a = curr;
                SDL_Point b = prev;
                
                draw_line(a, b, blocker->circle_size, true);
                
                int dx = (a.x - b.x);
                int dy = (a.y - b.y);
                
                b.x += dx*gs->gw;
                b.y += dy*gs->gw;
                
                draw_line(a, b, blocker->circle_size, false);
            }
            
            prev = blocker->points[i];
        }
    }
}

void blocker_draw_curve(void) {
    struct Blocker *blocker = &gs->blocker;
    if (!blocker->point_count) return;
    
    SDL_Point p = get_spline_point(blocker->points, 0);
    
    for (f32 t = 0.0f; t < blocker->point_count-3.f; t += 0.005f) {
        p = get_spline_point(blocker->points, t);
        
        p.x /= 2;
        p.y /= 2;
        
        SDL_RenderDrawPoint(gs->renderer, p.x-1, p.y);
        SDL_RenderDrawPoint(gs->renderer, p.x+1, p.y);
        SDL_RenderDrawPoint(gs->renderer, p.x, p.y-1);
        SDL_RenderDrawPoint(gs->renderer, p.x, p.y+1);
        SDL_RenderDrawPoint(gs->renderer, p.x, p.y);
    }
}

void flood_fill(Uint32 *pixels, int x, int y, Uint32 value) {
    if (x < 0 || y < 0 || x >= gs->gw || y >= gs->gh || pixels[x+y*gs->gw] != 0) {
        return;
    }
    
    pixels[x+y*gs->gw] = value;
    
    flood_fill(pixels, x+1, y, value);
    flood_fill(pixels, x-1, y, value);
    flood_fill(pixels, x, y+1, value);
    flood_fill(pixels, x, y-1, value);
}

void blocker_draw(void) {
    struct Blocker *blocker = &gs->blocker;
    
    if (!blocker->point_count) return;
    
    SDL_Texture *prev_target = SDL_GetRenderTarget(gs->renderer);
    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), SDL_BLENDMODE_BLEND);
    
    Assert(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER));
    
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL_BLOCKER));
    
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 255);
    
    f64 deg_angle = Degrees(blocker->angle);
    
    switch (blocker->prev_state) {
        case BLOCKER_STATE_LINE: {
            blocker_draw_line(deg_angle);
            break;
        }
        case BLOCKER_STATE_CURVE: {
            blocker_draw_curve();
            break;
        }
    }
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    
    // Do the lines at the two ends.
    
    SDL_RenderReadPixels(gs->renderer, NULL, 0, blocker->pixels, 4*gs->gw);
    
    int value = 1;
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (blocker->pixels[x+y*gs->gw] == 0) {
                flood_fill(blocker->pixels, x, y, value);
                value++;
            }
        }
    }
    
    SDL_SetRenderTarget(gs->renderer, prev_target);
    
    SDL_SetTextureAlphaMod(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), 128);
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), NULL, NULL);
}
