void chisel_blocker_init() {
    struct Chisel_Blocker *chisel_blocker = &gs->chisel_blocker;

    chisel_blocker->state = CHISEL_BLOCKER_OFF;

    chisel_blocker->control_points[0] = (SDL_Point){gs->gw/4, gs->gh/2};
    chisel_blocker->control_points[1] = (SDL_Point){gs->gw/2, gs->gh/2};
    chisel_blocker->control_points[2] = (SDL_Point){3*gs->gw/4, gs->gh/2};
    chisel_blocker->control_points[3] = (SDL_Point){10+3*gs->gw/4, gs->gh/2};
    chisel_blocker->point_count = 4;

    chisel_blocker->side = 1;

    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), SDL_BLENDMODE_BLEND);

    if (chisel_blocker->pixels == NULL) {
        chisel_blocker->pixels = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(Uint32));
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

void chisel_blocker_tick() {
    struct Chisel_Blocker *chisel_blocker = &gs->chisel_blocker;
    struct Input *input = &gs->input;

    if (chisel_blocker->state != CHISEL_BLOCKER_OFF && input->keys_pressed[SDL_SCANCODE_C]) {
        gs->chisel_blocker_mode = !gs->chisel_blocker_mode;
    }

    if (gs->current_tool == TOOL_CHISEL_MEDIUM) {
        if (input->keys_pressed[SDL_SCANCODE_L]) {
            chisel_blocker->state++;
            if (chisel_blocker->state > CHISEL_BLOCKER_CURVE_MODE)
                chisel_blocker->state = CHISEL_BLOCKER_OFF;
        }
    }

    if (!gs->chisel_blocker_mode || chisel_blocker->state == CHISEL_BLOCKER_OFF) return;

    /* SDL_ShowCursor(1); */
    if (SDL_GetCursor() != gs->grabber_cursor)
        /* SDL_SetCursor(gs->grabber_cursor); */

    if (input->mouse_pressed[SDL_BUTTON_RIGHT]) {
        chisel_blocker->side = (chisel_blocker->side == 1) ? 2 : 1;
    }

    if (input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            int found_point = 0;
            for (int i = 0; i < chisel_blocker->point_count; i++) {
                SDL_Point p = chisel_blocker->control_points[i];
                if (distance((f32)p.x, (f32)p.y, (f32)input->mx, (f32)input->my) <= 3) {
                    chisel_blocker->current_point = i;
                    found_point = 1;
                }
            }

            if (!found_point) {
                chisel_blocker->control_points[0].x = input->mx;
                chisel_blocker->control_points[0].y = input->my;
                chisel_blocker->control_points[1] = chisel_blocker->control_points[0];
                chisel_blocker->current_point = -1;
            }
        }
        if (chisel_blocker->current_point != -1) {
            SDL_Point *p = &chisel_blocker->control_points[chisel_blocker->current_point];
            p->x = input->mx;
            p->y = input->my;
        } else {
            SDL_Point p0 = chisel_blocker->control_points[0];
            SDL_Point p1 = chisel_blocker->control_points[1];
            if (p0.x == p1.x && p0.y == p1.y) {
                int xx = input->mx, yy = input->my;
                /* f32 angle = atan2f(xx-p0.x, yy-p0.y); */
                /* f32 dx = xx-p0.x; */
                /* f32 dy = yy-p0.y; */
                /* f32 len = sqrt(dx*dx + dy*dy); */

                /* angle = angle/(2*M_PI); */
                /* angle *= 360; */

                /* angle /= 45.; */
                /* angle = round(angle) * 45; */

                /* printf("Angle: %f\n", angle); */

                /* angle = 2*M_PI * angle/360.0; */

                /* f32 ux = sin(angle); */
                /* f32 uy = cos(angle); */

                /* xx = p0.x + ux * len; */
                /* yy = p0.y + uy * len; */

                chisel_blocker->control_points[chisel_blocker->point_count-1].x = xx;
                chisel_blocker->control_points[chisel_blocker->point_count-1].y = yy;
                chisel_blocker->control_points[chisel_blocker->point_count-2].x = xx;
                chisel_blocker->control_points[chisel_blocker->point_count-2].y = yy;
            }
        }
    } else {
        chisel_blocker->current_point = -1;

        SDL_Point p0 = chisel_blocker->control_points[0];
        SDL_Point p1 = chisel_blocker->control_points[1];
        SDL_Point p3 = chisel_blocker->control_points[3];

        SDL_Point p = (SDL_Point){(p0.x+p3.x)/2, (p0.y+p3.y)/2};

        if (p0.x == p1.x && p0.y == p1.y) {
            chisel_blocker->control_points[1].x = p.x;
            chisel_blocker->control_points[1].y = p.y;
            chisel_blocker->control_points[2].x = p.x;
            chisel_blocker->control_points[2].y = p.y;
        }
    }
}

// Finds tangent at point in cubic bezier curve.
// abcd = P0, P1, P2, P3
SDL_FPoint bezier_tangent(f32 t, SDL_Point a, SDL_Point b, SDL_Point c, SDL_Point d) {
    // To do this, we find the derivative.

    // Original bezier:

    // A = lerp(P0, P1, t)
    // B = lerp(P1, P2, t)
    // C = lerp(P2, P3, t)
    // D = lerp(A, B, t)
    // E = lerp(B, C, t)
    // P = lerp(D, E, t)

    // Replacing using math notation:

    // A = (1-t)P0 + P1t
    // B = (1-t)P1 + P2t
    // C = (1-t)P2 + P3t
    // D = (1-t)A + Bt
    // E = (1-t)B + Ct
    // P = (1-t)D + Et

    // Expand:

    // P(t) = P0(-t^3 + 3t^2 - 3t + 1) + P1(3t^3 - 6t^2 + 3t) + P2(-3t^3 + 3t^2) + P3(t^3)

    // Then, find derivative to get eqn. of tangent.
    // P'(t) = P0(-3t^2 + 6t - 3) + P1(9t^2 - 12t + 3) + P2(-9t^2 + 6t) + P3(3t^2)
    // replace with abcd:
    // P'(t) = A(-3t^2 + 6t - 3) + b(9t^2 - 12t + 3) + c(-9t^2 + 6t) + d(3t^2)

    f32 xp = a.x * (-3 * t*t + 6*t - 3) + b.x * (9*t*t - 12*t + 3) + c.x * (-9*t*t + 6*t) + d.x*(3*t*t);
    f32 yp = a.y * (-3 * t*t + 6*t - 3) + b.y * (9*t*t - 12*t + 3) + c.y * (-9*t*t + 6*t) + d.y*(3*t*t);

    SDL_FPoint p = {xp/gs->gw, yp/gs->gh};

    f32 len = (f32)sqrt(p.x*p.x + p.y*p.y);
    p.x /= len;
    p.y /= len;

    return p;
}

void chisel_blocker_draw() {
    struct Chisel_Blocker *chisel_blocker = &gs->chisel_blocker;

    if (chisel_blocker->state == CHISEL_BLOCKER_OFF)
        return;

    if (gs->chisel_blocker_mode) {
        SDL_SetRenderDrawColor(gs->renderer, 64, 64, 64, 64);
        SDL_RenderFillRect(gs->renderer, NULL);
    }

    SDL_Texture *prev_target = SDL_GetRenderTarget(gs->renderer);
    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), SDL_BLENDMODE_BLEND);

    Assert(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL_BLOCKER));

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);

    switch (chisel_blocker->state) {
    case CHISEL_BLOCKER_LINE_MODE: {
        for (int i = 0; i < 4-1; i++) {
            struct Line line = {
                chisel_blocker->control_points[i].x,
                chisel_blocker->control_points[i].y,
                chisel_blocker->control_points[i+1].x,
                chisel_blocker->control_points[i+1].y
            };
            SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(gs->renderer, line.x1, line.y1, line.x2, line.y2);
        }

        for (int i = 0; i < 2; i++) {
            struct Line a = (struct Line){
                chisel_blocker->control_points[i == 0 ? 0 : 3].x,
                chisel_blocker->control_points[i == 0 ? 0 : 3].y,
                chisel_blocker->control_points[i == 0 ? 1 : 2].x,
                chisel_blocker->control_points[i == 0 ? 1 : 2].y,
            };
            f32 dx = (f32) (a.x1 - a.x2);
            f32 dy = (f32) (a.y1 - a.y2);
            f32 len = (f32) sqrt(dx*dx + dy*dy);
            f32 ux = dx/len;
            f32 uy = dy/len;
            chisel_blocker->lines[i] = (struct Line){(int)a.x1, (int)a.y1, (int)(a.x1+ux*gs->gw), (int)(a.y1+uy*gs->gw)};
        }

        break;
    }
    case CHISEL_BLOCKER_CURVE_MODE: {
        f32 x = 0, y = 0;
        f32 px = 0, py = 0;

        // Bezier calculation.
        const f32 step = 0.02f;
        for (f32 t = 0.0; t < 1.0; t += step) {
            f32 m1x = lerp((f32) chisel_blocker->control_points[0].x, (f32) chisel_blocker->control_points[1].x, t);
            f32 m1y = lerp((f32) chisel_blocker->control_points[0].y, (f32) chisel_blocker->control_points[1].y, t);
            f32 m2x = lerp((f32) chisel_blocker->control_points[1].x, (f32) chisel_blocker->control_points[2].x, t);
            f32 m2y = lerp((f32) chisel_blocker->control_points[1].y, (f32) chisel_blocker->control_points[2].y, t);
            f32 m3x = lerp((f32) chisel_blocker->control_points[2].x, (f32) chisel_blocker->control_points[3].x, t);
            f32 m3y = lerp((f32) chisel_blocker->control_points[2].y, (f32) chisel_blocker->control_points[3].y, t);

            f32 pax = lerp(m1x, m2x, t);
            f32 pay = lerp(m1y, m2y, t);
            f32 pbx = lerp(m2x, m3x, t);
            f32 pby = lerp(m2y, m3y, t);

            if (t != 0.0) {
                px = x;
                py = y;
            }
            x = lerp(pax, pbx, t);
            y = lerp(pay, pby, t);

            if (t == 0.0) {
                px = x;
                py = y;
            }

            if ((int)x == (int)px && (int)y == (int)py) continue;

            SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(gs->renderer, (int)px, (int)py, (int)x, (int)y);

            SDL_Point p0 = chisel_blocker->control_points[0];
            SDL_Point p1 = chisel_blocker->control_points[1];
            if (!(p0.x == p1.x && p0.y == p1.y)) {
                {
                    SDL_FPoint start = bezier_tangent(0, chisel_blocker->control_points[0], chisel_blocker->control_points[1], chisel_blocker->control_points[2], chisel_blocker->control_points[3]);
                    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
                    SDL_Point point = chisel_blocker->control_points[0];
                    chisel_blocker->lines[0] = (struct Line){ (int)(point.x - start.x), (int)(point.y - start.y), (int)(point.x - gs->gw*start.x), (int)(point.y - gs->gw*start.y) };
                }

                {
                    SDL_FPoint end = bezier_tangent(1, chisel_blocker->control_points[0], chisel_blocker->control_points[1], chisel_blocker->control_points[2], chisel_blocker->control_points[3]);
                    SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 255);
                    SDL_Point point = chisel_blocker->control_points[3];
                    chisel_blocker->lines[1] = (struct Line){ (int)point.x, (int)point.y, (int)(point.x + gs->gw*end.x), (int)(point.y + gs->gw*end.y) };
                }

            }
        }
        break;
    }
    }

    for (int i = 0; i < 2; i++) {
        SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
        struct Line l = chisel_blocker->lines[i];
        SDL_RenderDrawLine(gs->renderer, l.x1, l.y1, l.x2, l.y2);
    }

    SDL_RenderReadPixels(gs->renderer, NULL, 0, chisel_blocker->pixels, 4*gs->gw);
    SDL_SetRenderTarget(gs->renderer, prev_target);

    int value = 1;
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (chisel_blocker->pixels[x+y*gs->gw] == 0) {
                flood_fill(chisel_blocker->pixels, x, y, value);
                value++;
            }
        }
    }

    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), SDL_BLENDMODE_BLEND);
    if (gs->chisel_blocker_mode)
        SDL_SetTextureAlphaMod(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), 255);
    else
        SDL_SetTextureAlphaMod(RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), 128);

    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL_BLOCKER), NULL, NULL);

    for (int i = 0; i < gs->gw*gs->gh; i++) {
        SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 16);
        if (chisel_blocker->pixels[i] != chisel_blocker->side) {
            SDL_RenderDrawPoint(gs->renderer, i%gs->gw, i/gs->gw);
        }
    }

    if (gs->chisel_blocker_mode) {
        for (int i = 0; i < chisel_blocker->point_count; i++) {
            SDL_SetRenderDrawColor(gs->renderer, 0, 255, 0, 100);
            SDL_RenderDrawPoint(gs->renderer, chisel_blocker->control_points[i].x, chisel_blocker->control_points[i].y);
            SDL_RenderDrawPoint(gs->renderer, chisel_blocker->control_points[i].x-1, chisel_blocker->control_points[i].y);
            SDL_RenderDrawPoint(gs->renderer, chisel_blocker->control_points[i].x+1, chisel_blocker->control_points[i].y);
            SDL_RenderDrawPoint(gs->renderer, chisel_blocker->control_points[i].x, chisel_blocker->control_points[i].y+1);
            SDL_RenderDrawPoint(gs->renderer, chisel_blocker->control_points[i].x, chisel_blocker->control_points[i].y-1);
        }
    }
}
