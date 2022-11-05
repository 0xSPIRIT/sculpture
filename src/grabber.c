void grabber_init(void) {
    gs->grabber.texture = 0;
    gs->grabber.object_holding = -1;
}

void grabber_tick(void) {
    struct Grabber *grabber = &gs->grabber;

    f32 px = grabber->x, py = grabber->y;

    grabber->x = (f32) gs->input.mx;
    grabber->y = (f32) gs->input.my;

    if (!is_in_bounds((int)grabber->x, (int)grabber->y)) return;
    if (gs->is_mouse_over_any_button) return;
    
    if (grabber->object_holding != -1) {
        int dx = (int) (grabber->x-px);
        int dy = (int) (grabber->y-py);
        object_attempt_move(grabber->object_holding, dx, dy);
    }

    if (gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (gs->input.mouse_pressed[SDL_BUTTON_LEFT]) {
            save_state_to_next();
        }
        // Find object at point, and attempt move to cursor pixel by pixel.
        int object = gs->grid[(int)grabber->x+(int)grabber->y*gs->gw].object;
        if (object != -1) grabber->object_holding = object;
    } else {
        grabber->object_holding = -1;
    }
}
