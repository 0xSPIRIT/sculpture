void input_tick(struct Game_State *state) {
    struct Input *in = &state->input;
    
    in->pmx = in->mx;
    in->pmy = in->my;

    in->real_pmx = in->real_mx;
    in->real_pmy = in->real_my;

    persist Uint8 keys_previous[SDL_NUM_SCANCODES] = {0};
    persist Uint32 mouse_previous = {0};

    in->mouse = (Uint32) SDL_GetMouseState(&in->real_mx, &in->real_my);
    in->keys = (Uint8*) SDL_GetKeyboardState(NULL);

    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        in->keys_pressed[i] = in->keys[i] && !keys_previous[i];
        in->keys_released[i] = !in->keys[i] && keys_previous[i];
    }
    for (int i = 0; i < MOUSE_BUTTONS; i++) {
        in->mouse_pressed[i] = in->mouse & SDL_BUTTON(i) && !(mouse_previous & SDL_BUTTON(i));
        in->mouse_released[i] = !(in->mouse & SDL_BUTTON(i)) && (mouse_previous & SDL_BUTTON(i));
    }

    in->mx = in->real_mx/state->S;
    in->my = in->real_my/state->S;
        
    in->my -= GUI_H/state->S;

    memcpy(keys_previous, in->keys, SDL_NUM_SCANCODES);
    mouse_previous = in->mouse;
}
