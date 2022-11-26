void overlay_init(void) {
    struct Overlay *overlay = &gs->overlay;

    overlay->tool = OVERLAY_TOOL_BRUSH;

    if (overlay->grid == NULL) {
        overlay->grid = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(int));
        overlay->temp_grid = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(int));
    }

    memset(overlay->grid, 0, sizeof(int)*gs->gw*gs->gh);
    memset(overlay->temp_grid, 0, sizeof(int)*gs->gw*gs->gh);

    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            overlay->grid[x+y*gs->gw] =
                gs->levels[gs->level_current].desired_grid[x+y*gs->gw].type != 0;
        }
    }

    overlay->temp_x = -1;
    overlay->temp_y = -1;
    overlay->size = 3;

    overlay->show = false;

    overlay->eraser_mode = false;

    overlay->r.x = overlay->r.y = -1;
}

void overlay_set_circle(int x, int y, int r, int value) {
    struct Overlay *overlay = &gs->overlay;

    for (int yy = -r; yy <= r; yy++) {
        for (int xx = -r; xx <= r; xx++) {
            if (xx*xx + yy*yy > r*r) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;

            overlay->grid[x+xx+(y+yy)*gs->gw] = value;
        }
    }
}

void overlay_set_spline(void) {//int *grid, int x1, int y1, int x2, int y2, int value) {
    // TODO
}

void overlay_set_line(int *grid, int x1, int y1, int x2, int y2, int value) {
    f64 dx = x2-x1;
    f64 dy = y2-y1;

    const f64 clamp = 22.5;

    f64 angle = atan2f(dy, dx);
    angle /= 2 * M_PI;
    angle *= 360;
    angle = ((int)angle) % 360;
    angle = angle / clamp;
    angle = clamp * round(angle);

    angle /= 360;
    angle *= 2 * M_PI;

    f64 deg_angle = Degrees(angle);

    f64 len = distancei(x1, y1, x2, y2);

    f64 ux = 0, uy = 0;

    ux = cos(angle);
    uy = sin(angle);
    if (is_angle_45(deg_angle) || deg_angle == 90 || deg_angle == 180 || deg_angle == 270 || deg_angle == 0) {
        ux = round(ux);
        uy = round(uy);
    }

    if (is_angle_225(deg_angle)) {
        ux *= 2;
        ux = round(ux);
        ux /= 2.0;

        uy *= 2;
        uy = round(uy);
        uy /= 2.0;
    }

    f64 x = x1;
    f64 y = y1;

    f64 curr_dist = 0;
    while (curr_dist <= len) {
        int ix = ceil(x);
        int iy = ceil(y);
        grid[ix+iy*gs->gw] = value;
        x += ux;
        y += uy;

        curr_dist = distance(x, y, x1, y1);
    }
}

void overlay_flood_fill(int *grid, int x, int y, int value) {
    if (!is_in_bounds(x, y)) return;
    if (grid[x+y*gs->gw]) return;

    grid[x+y*gs->gw] = value;

    overlay_flood_fill(grid, x+1, y, value);
    overlay_flood_fill(grid, x, y+1, value);
    overlay_flood_fill(grid, x-1, y, value);
    overlay_flood_fill(grid, x, y-1, value);
}

void overlay_set_rectangle(int *grid, SDL_Rect r, int value) {
    if (r.w < 0) {
        r.w *= -1;
        r.x -= r.w;
    }
    if (r.h < 0) {
        r.h *= -1;
        r.y -= r.h;
    }

    for (int yy = r.y; yy <= r.y+r.h; yy++) {
        for (int xx = r.x; xx <= r.x+r.w; xx++) {
            if (is_in_bounds(xx, yy)) {
                grid[xx+yy*gs->gw] = value;
            }
        }
    }
}

void gui_message_stack_push(const char *str);

void overlay_tick(void) {
    struct Overlay *overlay = &gs->overlay;

    memset(overlay->temp_grid, 0, sizeof(int)*gs->gw*gs->gh);

    const f32 speed = 0.1f;
    if (gs->input.keys[SDL_SCANCODE_W]) {
        overlay->size += speed;
    } else if (gs->input.keys[SDL_SCANCODE_S]) {
        overlay->size -= speed;
    }
    if (overlay->size < 0) overlay->size = 0;

    if (gs->input.keys[SDL_SCANCODE_ESCAPE]) {
        gs->overlay.show = false;
    }

    if (gs->is_mouse_over_any_button) return;
    switch (overlay->tool) {
        case OVERLAY_TOOL_BRUSH: case OVERLAY_TOOL_ERASER_BRUSH: {
            int value = overlay->tool == OVERLAY_TOOL_BRUSH ? 1 : 0;

            if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {

                overlay_set_circle(gs->input.mx, gs->input.my, overlay->size, value);
            }
            break;
        }
        case OVERLAY_TOOL_RECTANGLE: case OVERLAY_TOOL_ERASER_RECTANGLE: {
            int value = overlay->tool == OVERLAY_TOOL_RECTANGLE ? 1 : 0;

            if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                if (overlay->r.x == -1) {
                    overlay->r.x = gs->input.mx;
                    overlay->r.y = gs->input.my;
                }
                overlay->r.w = gs->input.mx - overlay->r.x;
                overlay->r.h = gs->input.my - overlay->r.y;

                if (overlay->tool == OVERLAY_TOOL_ERASER_RECTANGLE) {
                    overlay_set_rectangle(overlay->temp_grid, overlay->r, 2);
                } else {
                    overlay_set_rectangle(overlay->temp_grid, overlay->r, value);
                }
            } else {
                if (overlay->r.x != -1) {
                    overlay_set_rectangle(overlay->grid, overlay->r, value);
                    overlay->r.x = overlay->r.y = -1;
                }
            }
            break;
        }
        case OVERLAY_TOOL_LINE: case OVERLAY_TOOL_SPLINE: {
            if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                if (overlay->temp_x == -1) {
                    overlay->temp_x = gs->input.mx;
                    overlay->temp_y = gs->input.my;
                }

                if (overlay->tool == OVERLAY_TOOL_LINE) {
                    overlay_set_line(overlay->temp_grid,
                                     overlay->temp_x,
                                     overlay->temp_y,
                                     gs->input.mx,
                                     gs->input.my,
                                     1);
                } else {
                    overlay_set_spline();
                }
            } else if (gs->input.mouse_released[SDL_BUTTON_LEFT]) {
                if (overlay->tool == OVERLAY_TOOL_LINE) {
                    overlay_set_line(overlay->grid,
                                     overlay->temp_x,
                                     overlay->temp_y,
                                     gs->input.mx,
                                     gs->input.my,
                                     1);
                } else {
                    overlay_set_spline();
                }

                overlay->temp_x = -1;
                overlay->temp_y = -1;
            }
            break;
        }
        case OVERLAY_TOOL_BUCKET: {
            if (gs->input.mouse_pressed[SDL_BUTTON_LEFT]) {
                overlay_flood_fill(overlay->grid, gs->input.mx, gs->input.my, 1);
            }
            break;
        }
    }
}

bool int_array_any_neighbours_free(int *array, int x, int y) {
    for (int xx = -1; xx <= 1; xx++) {
        for (int yy = -1; yy <= 1; yy++) {
            if (xx == 0 && yy == 0) continue;
            if (abs(xx)-abs(yy) == 0) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (array[x+xx+(y+yy)*gs->gw] == 0)
                return true;
        }
    }
    return false;
}

void overlay_draw(void) {
    struct Overlay *overlay = &gs->overlay;
    
    if (!overlay->show)
        return;
    
    const f32 strobe_speed = 3.5f;
    
    f32 alpha;
    alpha = 1 + sinf(strobe_speed * SDL_GetTicks()/1000.0);
    alpha /= 2;
    alpha *= 16;
    alpha += 180;
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (overlay->temp_grid[x+y*gs->gw] == 2) continue;
            
            if (!overlay->grid[x+y*gs->gw] && !overlay->temp_grid[x+y*gs->gw]) continue;
            
            if (!int_array_any_neighbours_free(overlay->grid, x, y)) {
                continue;
            }
            
            SDL_SetRenderDrawColor(gs->renderer, 0, 0, 240, (Uint8)alpha);

            SDL_RenderDrawPoint(gs->renderer, x, y);
        }
    }

    if (gs->current_tool == TOOL_OVERLAY && (overlay->tool == OVERLAY_TOOL_BRUSH || overlay->tool == OVERLAY_TOOL_ERASER_BRUSH)) {
        if (overlay->eraser_mode) {
            SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 64);
        } else {
            SDL_SetRenderDrawColor(gs->renderer, 0, 255, 255, 64);
        }
        fill_circle(gs->renderer, gs->input.mx, gs->input.my, overlay->size);
    }
}
