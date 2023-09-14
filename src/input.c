static void input_tick_keys_pressed(Input *in) {
    in->keys = (u8*) SDL_GetKeyboardState(null);
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        in->keys_pressed[i] = in->keys[i] && !in->keys_previous[i];
        in->keys_released[i] = !in->keys[i] && in->keys_previous[i];
    }
    memcpy(in->keys_previous, in->keys, SDL_NUM_SCANCODES);
}

static void input_tick_mouse_pressed(Input *in) {
    for (int i = 0; i < MOUSE_BUTTONS; i++) {
        in->mouse_pressed[i] =
            in->mouse & SDL_BUTTON(i) && !(in->mouse_previous & SDL_BUTTON(i));
        in->mouse_released[i] =
            !(in->mouse & SDL_BUTTON(i)) && (in->mouse_previous & SDL_BUTTON(i));
    }

    in->mouse_previous = in->mouse;
}

static void input_set_locked(bool locked) {
#if SIMULATE_MOUSE
    gs->input.locked = locked;
    SDL_SetRelativeMouseMode(locked);
#else
    (void)locked;
#endif
}

// real_x and real_y are window positions, not grid positions.
static void input_set_mouse_position(Input *in, int real_x, int real_y) {
    in->real_pmx = in->real_mx;
    in->real_pmy = in->real_my;

    in->real_mx = real_x;
    in->real_my = real_y;

    // This is the problem area, you get a -ve real_mx.
#if !SIMULATE_MOUSE
    in->real_mx -= gs->real_width/2 - gs->game_width/2;
    in->real_my -= gs->real_height/2 - gs->game_height/2;
#endif

    if (in->real_mx < 0) { in->real_mx = 0; }
    if (in->real_my < 0) { in->real_my = 0; }
    if (in->real_mx >= gs->game_width) in->real_mx = gs->game_width-1;
    if (in->real_my >= gs->game_height) in->real_my = gs->game_height-1;

    //in->mx = round(((f32)in->real_mx+state->render.view.x)/state->S);
    in->mx = (in->real_mx+gs->render.view.x+32*gs->S)/gs->S;
    in->my = (in->real_my+gs->render.view.y)/gs->S;

    in->my -= round((f32)GUI_H/gs->S);
}

static void input_tick(Game_State *state) {
    Input *in = &state->input;

    if (in->locked) {
        in->hide_mouse = (state->obj.active);
    }

    if (!in->initted) {
        in->mouse = (u32) SDL_GetMouseState(&in->real_mx, &in->real_my);
        in->initted = true;
    } else {
#if SIMULATE_MOUSE
        in->mouse = (u32) SDL_GetMouseState(null, null);
#else
        int x, y;
        in->mouse = (u32) SDL_GetMouseState(&x, &y);
        input_set_mouse_position(in, x, y);
#endif
    }

    input_tick_mouse_pressed(in);
    input_tick_keys_pressed(in);
}

static void input_tick_mouse(Game_State *state, SDL_Event *event) {
    Input *in = &state->input;
    input_set_mouse_position(in, in->real_mx + event->motion.xrel, in->real_my + event->motion.yrel);
}
