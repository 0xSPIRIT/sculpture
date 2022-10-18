void blocker_add_point(int x, int y) {
    struct Blocker *blocker = &gs->blocker;
    blocker->points[blocker->point_count].x = x;
    blocker->points[blocker->point_count].y = y;
    blocker->point_count++;
}

void blocker_init() {
    struct Blocker *blocker = &gs->blocker;

    blocker->state = BLOCKER_STATE_OFF;
    blocker->point_count = 0;
    memset(blocker->points, 0, BLOCKER_MAX_POINTS * sizeof(struct SDL_Point));

    blocker->angle = 0;

    if (blocker->pixels == NULL) {
        blocker->pixels = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(Uint32));
    }
}

void blocker_tick() {
    struct Blocker *blocker = &gs->blocker;
    struct Input *input = &gs->input;
    
    // if (input->keys_pressed[SDL_SCANCODE_F6]) {
    // blocker->state = BLOCKER_STATE_OFF;
    // }
    // if (input->keys_pressed[SDL_SCANCODE_F7]) {
    // blocker->state = BLOCKER_STATE_LINE;
    // }
    // if (input->keys_pressed[SDL_SCANCODE_F8]) {
    // blocker->state = BLOCKER_STATE_CURVE;
    // }
    
    if (blocker->state == BLOCKER_STATE_OFF) return;
    if (input->real_my < GUI_H) return;

    switch (blocker->state) {
    case BLOCKER_STATE_LINE:
        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            memset(blocker->points, 0, BLOCKER_MAX_POINTS * sizeof(SDL_Point));
            blocker->point_count = 0;

            blocker->points[0].x = input->mx;
            blocker->points[0].y = input->my;
            blocker->points[1].x = input->mx;
            blocker->points[1].y = input->my;
            blocker->point_count = 2;
        } else if (input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            blocker->points[1].x = input->mx;
            blocker->points[1].y = input->my;
            blocker->point_count = 2;

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
        }

        blocker->prev_state = blocker->state;
        break;
    case BLOCKER_STATE_CURVE:
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

void blocker_draw_line(f64 deg_angle) {
    struct Blocker *blocker = &gs->blocker;
    
    const int circle_size = 1;
    
    if (is_angle_225(deg_angle)) {
        int dir_x = sign(blocker->points[1].x - blocker->points[0].x);
        int dir_y = sign(blocker->points[1].y - blocker->points[0].y);
        
        int a = 0;
        int x = blocker->points[0].x;
        int y = blocker->points[0].y;

        f64 length = distance64((f64)blocker->points[0].x, (f64)blocker->points[1].y, (f64)x, (f64)y);
        f64 total_length = distance64_point(blocker->points[0], blocker->points[1]);

        f64 prev_length = 0;

        while (length < total_length) {
            bool break_out = false;
            
            a = 0;
            while (a < 2) {
                // SDL_RenderDrawPoint(gs->renderer, x, y);
                fill_circle(gs->renderer, x, y, circle_size);
                
                f64 f = fabs(deg_angle);

                if (f == 22.5 || f == 157.5) {
                    x += dir_x;
                } else {
                    y += dir_y;
                }
                
                ++a;
                length = distance64((f64)blocker->points[0].x, (f64)blocker->points[0].y, (f64)x, (f64)y);
                prev_length = length;
                
                if (length > total_length) {
                    break_out = true;
                    break;
                }
            }

            if (break_out) break;

            f64 f = fabs(deg_angle);

            if (f == 22.5 || f == 157.5) {
                y += dir_y;
            } else {
                x += dir_x;
            }
        }
    } else {
        SDL_Point prev = {-1, -1};
        for (int i = 0; i < blocker->point_count; i++) {
            if (prev.x != -1) {
                SDL_Point curr = blocker->points[i];
            
                f32 len = distancei(curr.x, curr.y, prev.x, prev.y);
                f32 dx = (f32) (curr.x - prev.x);
                f32 dy = (f32) (curr.y - prev.y);
            
                f32 ux, uy;
            
                ux = dx/len;
                uy = dy/len;
            
                SDL_FPoint c = {(f32)prev.x, (f32)prev.y};

                f32 curr_dist = 0;
                while (curr_dist < len) {
                    fill_circle(gs->renderer, (int)c.x, (int)c.y, circle_size);
                    c.x += ux;
                    c.y += uy;
            
                    curr_dist = distance(c.x, c.y, (f32)prev.x, (f32)prev.y);
                }
            }
        
            prev = blocker->points[i];
        }
    }
}

void blocker_draw_curve() {
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

void blocker_draw() {
    struct Blocker *blocker = &gs->blocker;

    if (!blocker->point_count) return;

    if (blocker->state != BLOCKER_STATE_OFF) {
        for (int i = 0; i < blocker->point_count; i++) {
            SDL_Point p = blocker->points[i];
            SDL_SetRenderDrawColor(gs->renderer, 0, 255, 0, 255);

            SDL_RenderDrawPoint(gs->renderer, p.x, p.y);
        }
    }

    SDL_Texture *prev_target = SDL_GetRenderTarget(gs->renderer);
    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), SDL_BLENDMODE_BLEND);

    Assert(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER));

    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL_BLOCKER));

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);

    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 0, 255);

    f64 deg_angle = Degrees(blocker->angle);

    switch (blocker->prev_state) {
    case BLOCKER_STATE_LINE:
        blocker_draw_line(deg_angle);
        break;
    case BLOCKER_STATE_CURVE:
        blocker_draw_curve();
        break;
    }

    // Do the lines at the two ends.

    /* // Starting line
     * if (blocker->point_count >= 2) {
     *     f32 dx = blocker->points[1].x - blocker->points[0].x;
     *     f32 dy = blocker->points[1].y - blocker->points[0].y;
     *     f32 dist = sqrtf(dx*dx* + dy*dy);
     *     f32 ux = dx/dist;
     *     f32 uy = dy/dist;
     * 
     *     struct Line a = {
     *         blocker->points[0].x,
     *         blocker->points[0].y,
     *         blocker->points[0].x - 1000*ux,
     *         blocker->points[0].y - 1000*uy
     *     };
     * 
     *     SDL_RenderDrawLine(gs->renderer, a.x1, a.y1, a.x2, a.y2);
     * } */

    SDL_RenderReadPixels(gs->renderer, NULL, 0, blocker->pixels, 4*gs->gw);

    SDL_SetRenderTarget(gs->renderer, prev_target);

    SDL_SetTextureAlphaMod(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), 128);
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), NULL, NULL);
}
