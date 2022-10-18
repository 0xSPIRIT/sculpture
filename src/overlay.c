void overlay_init() {
    struct Overlay *overlay = &gs->overlay;

    overlay->tool = OVERLAY_TOOL_BRUSH;

    if (overlay->grid == NULL) {
        overlay->grid = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(int));
        overlay->temp_grid = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(int));
    }

    overlay->temp_x = -1;
    overlay->temp_y = -1;
    overlay->size = 3;

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
    while (curr_dist < len) {
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

void overlay_tick() {
    struct Overlay *overlay = &gs->overlay;

    memset(overlay->temp_grid, 0, sizeof(int)*gs->gw*gs->gh);

    if (gs->input.keys_pressed[SDL_SCANCODE_RIGHTBRACKET]) {
        overlay->size++;
    } else if (gs->input.keys_pressed[SDL_SCANCODE_LEFTBRACKET]) {
        overlay->size--;
    }
    overlay->size = clamp(overlay->size, 1, 8);

    if (gs->input.keys_pressed[SDL_SCANCODE_F7]) {
        overlay->tool = OVERLAY_TOOL_RECTANGLE;
        gui_message_stack_push("Overlay Tool: Rectangle");
    }
    if (gs->input.keys_pressed[SDL_SCANCODE_F8]) {
        overlay->tool = OVERLAY_TOOL_BRUSH;
        gui_message_stack_push("Overlay Tool: Brush");
    }
    if (gs->input.keys_pressed[SDL_SCANCODE_F9]) {
        overlay->tool = OVERLAY_TOOL_LINE;
        gui_message_stack_push("Overlay Tool: Line");
    }
    if (gs->input.keys_pressed[SDL_SCANCODE_F10]) {
        if (overlay->tool != OVERLAY_TOOL_ERASER_BRUSH) {
            overlay->tool = OVERLAY_TOOL_ERASER_BRUSH;
            gui_message_stack_push("Overlay Tool: Eraser (Brush)");
        } else {
            overlay->tool = OVERLAY_TOOL_ERASER_RECTANGLE;
            gui_message_stack_push("Overlay Tool: Eraser (Rectangle)");
        }
    }
    if (gs->input.keys_pressed[SDL_SCANCODE_F11]) {
        overlay->tool = OVERLAY_TOOL_BUCKET;
        gui_message_stack_push("Overlay Tool: Bucket");
    }

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
    case OVERLAY_TOOL_LINE: {
        if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (overlay->temp_x == -1) {
                overlay->temp_x = gs->input.mx;
                overlay->temp_y = gs->input.my;
            }

            overlay_set_line(
                overlay->temp_grid,
                overlay->temp_x,
                overlay->temp_y,
                gs->input.mx,
                gs->input.my,
                1
            );
        } else if (gs->input.mouse_released[SDL_BUTTON_LEFT]) {
            overlay_set_line(
                overlay->grid,
                overlay->temp_x,
                overlay->temp_y,
                gs->input.mx,
                gs->input.my,
                1
            );

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

void overlay_draw() {
    struct Overlay *overlay = &gs->overlay;

    const f32 strobe_speed = 3.5f;

    f32 alpha;
    alpha = 1 + sinf(strobe_speed * SDL_GetTicks()/1000.0);
    alpha /= 2;
    alpha *= 16;
    alpha += 128;
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (overlay->temp_grid[x+y*gs->gw] == 2) continue;

            if (!overlay->grid[x+y*gs->gw] && !overlay->temp_grid[x+y*gs->gw]) continue;

            SDL_SetRenderDrawColor(gs->renderer, 0, 255, 0, (Uint8)alpha);
            SDL_RenderDrawPoint(gs->renderer, x, y);
        }
    }

    if (gs->current_tool == TOOL_OVERLAY && (overlay->tool == OVERLAY_TOOL_BRUSH || overlay->tool == OVERLAY_TOOL_ERASER_BRUSH)) {
        SDL_SetRenderDrawColor(gs->renderer, 0, 255, 255, 64);
        fill_circle(gs->renderer, gs->input.mx, gs->input.my, overlay->size);
    }
}