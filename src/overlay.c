void overlay_init() {
    struct Overlay *overlay = &gs->overlay;

    overlay->tool = OVERLAY_TOOL_NONE;

    if (overlay->grid == NULL) {
        overlay->grid = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(int));
    }
}

void overlay_set_circle(int x, int y, int r, int value) {
    struct Overlay *overlay = &gs->overlay;
    
    for (int yy = -r; yy <= r; yy++) {
        for (int xx = -r; xx <= r; xx++) {
            if (xx*xx + yy*yy > r*r) continue;

            overlay->grid[x+xx+(y+yy)*gs->gw] = value;
        }
    }
}

void overlay_tick() {
    struct Overlay *overlay = &gs->overlay;

    if (gs->input.keys_pressed[SDL_SCANCODE_F9]) {
        overlay->tool = OVERLAY_TOOL_BRUSH;
    }
    if (gs->input.keys_pressed[SDL_SCANCODE_F10]) {
        overlay->tool = OVERLAY_TOOL_LINE;
    }
    if (gs->input.keys_pressed[SDL_SCANCODE_F11]) {
        overlay->tool = OVERLAY_TOOL_ERASER;
    }
    if (gs->input.keys_pressed[SDL_SCANCODE_F12]) {
        overlay->tool = OVERLAY_TOOL_NONE;
    }

    if (overlay->tool == OVERLAY_TOOL_NONE) return;

    switch (overlay->tool) {
    case OVERLAY_TOOL_BRUSH:
        const int r = 3;
        if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            overlay_set_circle(gs->input.mx, gs->input.my, r, 1);
        } else if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            overlay_set_circle(gs->input.mx, gs->input.my, r, 0);
        }
        break;
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
            if (!overlay->grid[x+y*gs->gw]) continue;

            SDL_SetRenderDrawColor(gs->renderer, 0, 255, 0, (Uint8)alpha);
            SDL_RenderDrawPoint(gs->renderer, x, y);
        }
    }
}
