void blocker_init() {
    struct Blocker *blocker = &gs->blocker;

    blocker->pixels = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(Uint32));
}

void blocker_tick() {
    struct Blocker *blocker = &gs->blocker;
    struct Input *input = &gs->input;
    
    if (input->keys_pressed[SDL_SCANCODE_F8]) {
        blocker->on = !blocker->on;
    }

    if (!blocker->on) return;

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

        if (input->keys[SDL_SCANCODE_LSHIFT]) {
            f64 dx = blocker->points[1].x - blocker->points[0].x;
            f64 dy = blocker->points[1].y - blocker->points[0].y;

            f64 length = distance64(blocker->points[0].x, blocker->points[0].y,
                                    blocker->points[1].x, blocker->points[1].y);
                        
            const f64 clamp = 22.5;

            f64 angle = atan2f(dy, dx);
            angle /= 2 * M_PI;
            angle *= 360;
            angle = ((int)angle) % 360;
            angle = angle / clamp;
            angle = clamp * round(angle);

            angle /= 360.f;
            angle *= 2 * M_PI;

            f64 ux = cos(angle);
            f64 uy = sin(angle);

            printf("%lf, %lf\n", ux, uy);

            blocker->points[1].x = round(blocker->points[0].x + ux*length);
            blocker->points[1].y = round(blocker->points[0].y + uy*length);
        }
    } else {
        /* memset(blocker->points, 0, BLOCKER_MAX_POINTS * sizeof(SDL_Point));
         * blocker->point_count = 0; */
    }
}

void blocker_draw() {
    struct Blocker *blocker = &gs->blocker;

    if (!blocker->point_count) return;

    SDL_Texture *prev_target = SDL_GetRenderTarget(gs->renderer);
    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), SDL_BLENDMODE_BLEND);

    Assert(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER));

    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL_BLOCKER));

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);

    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 0, 255);
    /* SDL_RenderDrawLine(gs->renderer, blocker->points[0].x, blocker->points[0].y, blocker->points[1].x, blocker->points[1].y); */

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

            const int circle_size = 2;

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

    SDL_RenderReadPixels(gs->renderer, NULL, 0, blocker->pixels, 4*gs->gw);

    SDL_SetRenderTarget(gs->renderer, prev_target);

    SDL_SetTextureAlphaMod(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), 128);
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), NULL, NULL);
}
