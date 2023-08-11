static void input_tick_keys_pressed(Input *in) {
    in->keys = (Uint8*) SDL_GetKeyboardState(null);
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

// This is pretty bad
static void input_tick_simulated(Game_State *state) {
    Input *in = &state->input;

    in->s_pmx = in->s_mx;
    in->s_pmy = in->s_my;
    in->mouse = (Uint32) SDL_GetMouseState(&in->s_mx, &in->s_my);

    int dx = in->s_mx - in->s_pmx;
    int dy = in->s_my - in->s_pmy;

    in->real_mx += dx;
    in->real_my += dy;

    in->mx = (in->real_mx+state->render.view.x)/state->S;
    in->my = (in->real_my+state->render.view.y)/state->S;

    // Hardcode
    if (gs->gw == 128) {
        in->mx += 32;
    }

    in->my -= GUI_H/state->S;

    input_tick_mouse_pressed(in);
    input_tick_keys_pressed(in);
}

static void input_tick_normal(Game_State *state) {
    Input *in = &state->input;

    in->pmx = in->mx;
    in->pmy = in->my;

    in->real_pmx = in->real_mx;
    in->real_pmy = in->real_my;

    in->mouse = (Uint32) SDL_GetMouseState(&in->real_mx, &in->real_my);

    in->real_mx -= gs->real_width/2 - gs->game_width/2;
    in->real_my -= gs->real_height/2 - gs->game_height/2;

    //in->mx = round(((f32)in->real_mx+state->render.view.x)/state->S);
    in->mx = (in->real_mx+state->render.view.x+32*state->S)/state->S;
    in->my = (in->real_my+state->render.view.y)/state->S;

    in->my -= round((f32)GUI_H/state->S);

    input_tick_mouse_pressed(in);
    input_tick_keys_pressed(in);
}

static void input_tick(Game_State *state) {
#if MOUSE_SIMULATED
    input_tick_simulated(state);
#else
    input_tick_normal(state);
#endif
}
