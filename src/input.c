static void input_tick(Game_State *state) {
    Input *in = &state->input;

    in->pmx = in->mx;
    in->pmy = in->my;

    in->real_pmx = in->real_mx;
    in->real_pmy = in->real_my;
    
//#if MOUSE_SIMULATED
//#else
    in->mouse = (Uint32) SDL_GetMouseState(&in->real_mx, &in->real_my);
//#endif

    in->real_mx -= gs->real_width/2 - gs->game_width/2;
    in->real_my -= gs->real_height/2 - gs->game_height/2;

    in->keys = (Uint8*) SDL_GetKeyboardState(null);

    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        in->keys_pressed[i] = in->keys[i] && !in->keys_previous[i];
        in->keys_released[i] = !in->keys[i] && in->keys_previous[i];
    }
    for (int i = 0; i < MOUSE_BUTTONS; i++) {
        in->mouse_pressed[i] =
            in->mouse & SDL_BUTTON(i) && !(in->mouse_previous & SDL_BUTTON(i));
        in->mouse_released[i] =
            !(in->mouse & SDL_BUTTON(i)) && (in->mouse_previous & SDL_BUTTON(i));
    }

    in->mx = (in->real_mx+state->render.view.x)/state->S;
    in->my = (in->real_my+state->render.view.y)/state->S;

    in->my -= GUI_H/state->S;

    memcpy(in->keys_previous, in->keys, SDL_NUM_SCANCODES);
    in->mouse_previous = in->mouse;
}
