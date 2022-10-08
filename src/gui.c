struct Button *button_allocate(enum Button_Type type, SDL_Texture *texture, const char *tooltip_text, void (*on_pressed)(void*)) {
    struct Button *b = arena_alloc(gs->persistent_memory, 1, sizeof(struct Button));
    b->type = type;
    b->texture = texture;
    SDL_QueryTexture(texture, NULL, NULL, &b->w, &b->h);

    strcpy(b->tooltip_text, tooltip_text);
    b->on_pressed = on_pressed;
    return b;
}

void click_gui_tool_button(void *type_ptr) {
    int type = *((int*)type_ptr);
    
    struct GUI *gui = &gs->gui;

    if (gui->popup) return;

    gs->current_tool = type;
    gs->chisel_blocker_mode = 0;
    switch (gs->current_tool) {
    case TOOL_CHISEL_SMALL:
        gs->chisel = &gs->chisel_small;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 0);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+2);
        break;
    case TOOL_CHISEL_MEDIUM:
        gs->chisel = &gs->chisel_medium;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 1);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+4);
        break;
    case TOOL_CHISEL_LARGE:
        gs->current_tool = TOOL_CHISEL_LARGE;
        gs->chisel = &gs->chisel_large;
        for (int i = 0; i < gs->object_count; i++)
            object_generate_blobs(i, 2);
        gs->chisel_hammer.normal_dist = gs->chisel_hammer.dist = (f32) (gs->chisel->w+4);
        break;
    }

    for (int i = 0; i < TOOL_COUNT; i++) {
        gui->tool_buttons[i]->activated = 0;
    }
    tooltip_reset(&gui->tooltip);
}

void button_tick(struct Button *b, void *data) {
    struct Input *input = &gs->input;
    struct GUI *gui = &gs->gui;

    int gui_input_mx = input->real_mx;
    int gui_input_my = input->real_my;

    // TODO: This is a temporary hack so that function pointers won't stop working
    //       upon reloading the DLL.
    switch (b->type) {
    case BUTTON_TYPE_CONVERTER:
        b->on_pressed = converter_begin_converting;
        break;
    case BUTTON_TYPE_TOOL_BAR:
        b->on_pressed = click_gui_tool_button;
        break;
    }

    if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
        gui_input_my >= b->y && gui_input_my < b->y+b->h) {

        tooltip_set_position_to_cursor(&gui->tooltip, TOOLTIP_TYPE_BUTTON);
        b->just_had_tooltip = true;

        if (strlen(b->tooltip_text))
            strcpy(gui->tooltip.str[0], b->tooltip_text);

        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            b->on_pressed(data);
            b->activated = true;
        }
    } else if (b->just_had_tooltip) {
        b->just_had_tooltip = false;
        tooltip_reset(&gui->tooltip);
    }

    if (!(input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        b->activated = false;
    }
}

void button_draw(struct Button *b) {
    struct Input *input = &gs->input;

    int gui_input_mx = input->real_mx;// / gs->S;
    int gui_input_my = input->real_my;// / gs->S;

    SDL_Rect dst = {
        b->x, b->y, b->w, b->h
    };
    if (b->activated) {
        SDL_SetTextureColorMod(b->texture, 200, 200, 200);
    } else if (gui_input_mx >= b->x && gui_input_mx < b->x+b->w &&
               gui_input_my >= b->y && gui_input_my < b->y+b->h) {
        SDL_SetTextureColorMod(b->texture, 230, 230, 230);
    } else {
        SDL_SetTextureColorMod(b->texture, 255, 255, 255);
    }
    SDL_RenderCopy(gs->renderer, b->texture, NULL, &dst);
}

void gui_init() {
    struct GUI *gui = &gs->gui;

    *gui = (struct GUI){ .popup_y = (f32) (gs->gh*gs->S), .popup_y_vel = 0, .popup_h = GUI_POPUP_H, .popup = 0 };
    gui->popup_texture = gs->textures.popup;

    tooltip_reset(&gui->tooltip);
    gui->is_placer_active = false;

    int cum = 0;

    for (int i = 0; i < TOOL_COUNT; i++) {
        char name[128] = {0};
        get_name_from_tool(i, name);

        gui->tool_buttons[i] = button_allocate(BUTTON_TYPE_TOOL_BAR, gs->textures.tool_buttons[i], name, click_gui_tool_button);
        gui->tool_buttons[i]->x = cum;
        gui->tool_buttons[i]->y = 0;
        gui->tool_buttons[i]->index = i;
        gui->tool_buttons[i]->activated = i == gs->current_tool;

        cum += gui->tool_buttons[i]->w;
    }
}

void gui_tick() {
    struct GUI *gui = &gs->gui;
    struct Input *input = &gs->input;

    if (input->keys_pressed[SDL_SCANCODE_TAB]) {
        gui->popup = !gui->popup;
        gui->popup_y_vel = 0;
        tooltip_reset(&gui->tooltip);

        gs->current_tool = gs->previous_tool;

        // Just in case the player had reset it.
        if (gs->current_placer == -1)
            gs->current_placer = 0;
    }

    const f32 speed = 3.0f;

    if (gui->popup) {
        if (SDL_GetCursor() != gs->grabber_cursor) {
            /* SDL_SetCursor(gs->grabber_cursor); */
            /* SDL_ShowCursor(1); */
        }

        if (gui->popup_y > gs->S*gs->gh-gui->popup_h) {
            gui->popup_y_vel -= speed;
        } else {
            gui->popup_y_vel = 0;
            gui->popup_y = gs->S*gs->gh-gui->popup_h;
        }

        int was_placer_active = gui->is_placer_active;

        gui->is_placer_active = input->keys[SDL_SCANCODE_LCTRL];

        if (was_placer_active && !gui->is_placer_active) {
            tooltip_reset(&gui->tooltip);
        } else if (!was_placer_active && gui->is_placer_active) {
            struct Placer *p = converter_get_current_placer();
            p->x = input->mx;
            p->y = input->my;
        }
    } else if (gui->popup_y < gs->S*gs->gh) {
        gui->popup_y_vel += speed;
    } else {
        gui->popup_y = (f32) (gs->S*gs->gh);
        gui->popup_y_vel = 0;
    }

    if (!gui->popup) {
        /* SDL_SetCursor(normal_cursor); */
        for (int i = 0; i < TOOL_COUNT; i++) {
            button_tick(gui->tool_buttons[i], &i);
        }
        if (input->real_my >= GUI_H) {
            tooltip_reset(&gui->tooltip);
        }
    }

    gui->popup_y += gui->popup_y_vel;
    gui->popup_y = (f32) clamp((int) gui->popup_y, (int) (gs->S*gs->gh - gui->popup_h), gs->window_height);
}

void gui_draw() {
    struct GUI *gui = &gs->gui;

    // Draw the toolbar buttons.
    SDL_Texture *old = SDL_GetRenderTarget(gs->renderer);

    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_GUI_TOOLBAR), SDL_BLENDMODE_BLEND);

    Assert(RenderTarget(RENDER_TARGET_GUI_TOOLBAR));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GUI_TOOLBAR));

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);

    SDL_SetRenderDrawColor(gs->renderer, 64, 64, 64, 255);
    SDL_Rect r = { 0, 0, gs->gw, GUI_H/gs->S };
    SDL_RenderFillRect(gs->renderer, &r);

    for (int i = 0; i < TOOL_COUNT; i++) {
        button_draw(gui->tool_buttons[i]);
    }

    SDL_Rect dst = {
        0, 0,
        gs->gw*gs->S, GUI_H
    };

    SDL_SetRenderTarget(gs->renderer, NULL);
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GUI_TOOLBAR), NULL, &dst);
    SDL_SetRenderTarget(gs->renderer, old);
}

void gui_popup_draw() {
    struct GUI *gui = &gs->gui;

    SDL_Rect popup = {
        0, (int)(GUI_H + gui->popup_y),
        gs->gw*gs->S, (int)gui->popup_h
    };

    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(gs->renderer, &popup);
}
