static bool confirm_popup_show_red_text(int level_index_plus_one) {
    return level_index_plus_one >= 8 && !compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY);
}

//~ End of level comfirmation popup callbacks

static void end_of_level_popup_confirm_run(int target) {
    Popup_Confirm *popup = &gs->gui.eol_popup_confirm;

    if (wait_for_fade(FADE_LEVEL_FINISH)) {
        reset_fade();
        goto_level(++gs->level_current);
        return;
    }


    if (!popup->active) return;

    bool next_level = can_goto_next_level();

    Popup_Confirm_Run_Data run_data = {0};
    if (!next_level) {
        popup->a->disabled = true;
        popup->a->texture = &GetTexture(TEXTURE_CONFIRM_X_BUTTON);
    } else {
        popup->a->disabled = false;
        popup->a->texture = &GetTexture(TEXTURE_CONFIRM_BUTTON);
    }

    popup->b->disabled = false;

    popup_confirm_base_tick_and_draw(&run_data, target, popup);

    if (!next_level || confirm_popup_show_red_text(gs->level_current+1)) {
        char comment[128]={0};

        if (gs->level_current+1 > 7) {
            if (next_level) {
                strcpy(comment, "This is sort of how I wanted it, I guess.");
            } else {
                strcpy(comment, "But this isn't how I wanted it.");
            }
        }

        int w, h;

        TTF_SizeText(gs->fonts.font_times->handle, comment, &w, &h);

        int xoff = 0;//get_glitched_offset();

        f64 button_x = popup->a->x + popup->a->w/2;
        f64 button_y = popup->a->y + popup->a->h/2;

        f64 norm_dist = sqrtf((button_x - gs->input.real_mx)*(button_x - gs->input.real_mx) + (button_y - gs->input.real_my)*(button_y - gs->input.real_my));
        norm_dist /= gs->game_width;
        norm_dist = 1 - norm_dist;

        SDL_Color color = (SDL_Color){180, 180, 180, 180};
        if (gs->level_current+1 > 7)
            color = COLOR_MAX_DIALOGUE;

        RenderTextQuick(target,
                        "Not good enough",
                        gs->fonts.font_times,
                        comment,
                        color,
                        xoff + popup->r.x + popup->r.w/2 - w/2,
                        popup->r.y + popup->r.h - 2.75*h,
                        null,
                        null,
                        false);
    }
}

static void end_of_level_popup_confirm_confirm(void *unused) {
    (void)unused;

    Popup_Confirm *c = &gs->gui.eol_popup_confirm;
    Level *level = &gs->levels[gs->level_current];

    c->active = false;

    if (can_goto_next_level()) {
        // See end_of_level_popup_confirm_run
        set_fade(FADE_LEVEL_FINISH, 0, 255);
    }

    level->off = false;
    level->desired_alpha = 0;
    level_set_state(gs->level_current, LEVEL_STATE_PLAY);
}

static void end_of_level_popup_confirm_cancel(void *unused) {
    (void)unused;

    Popup_Confirm *c = &gs->gui.eol_popup_confirm;
    Level *level = &gs->levels[gs->level_current];

    level->off = false;
    level->desired_alpha = 0;
    level_set_state(gs->level_current, LEVEL_STATE_PLAY);

    c->active = false;
}

//~ Reset level confirmation

void restart_popup_confirm_run(int target) {
    Popup_Confirm *popup = &gs->gui.restart_popup_confirm;
    if (!popup->active) return;

    Popup_Confirm_Run_Data run_data = {0};
    popup_confirm_base_tick_and_draw(&run_data, target, popup);
}

void restart_popup_confirm_confirm(void *unused) {
    (void)unused;
    Popup_Confirm *popup = &gs->gui.restart_popup_confirm;
    popup->active = false;
    goto_level(gs->level_current);
}

void restart_popup_confirm_cancel(void *unused) {
    (void)unused;
    Popup_Confirm *popup = &gs->gui.restart_popup_confirm;
    popup->active = false;
}

//~ Implementation

static Popup_Confirm popup_confirm_init(const char *string,
                                        int confirm_type, // Button_Type
                                        int cancel_type,  // Button_Type
                                        void (*on_confirm)(void*),
                                        void (*on_exit)(void*)) {
    Popup_Confirm result = {0};

    result.active = false;

    strcpy(result.text, string);

    result.a = button_allocate(confirm_type,
                               &GetTexture(TEXTURE_CONFIRM_BUTTON),
                               "",
                               on_confirm);
    result.b = button_allocate(cancel_type,
                               &GetTexture(TEXTURE_CANCEL_BUTTON),
                               "",
                               on_exit);

    return result;
}

static void popup_confirm_activate(Popup_Confirm *popup) {
    popup->active = true;
    Mix_HaltChannel(AUDIO_CHANNEL_GUI);
    Mix_PlayChannel(AUDIO_CHANNEL_GUI, gs->audio.accept, 0);
}

// You must call this in your own update callback.
static void popup_confirm_base_tick_and_draw(Popup_Confirm_Run_Data *data, int target, Popup_Confirm *popup) {
    if (!popup->active) return;

    popup->r = (SDL_Rect){
        gs->game_width/6,
        gs->game_height/3,
        2*gs->game_width/3,
        gs->game_height/4
    };

    RenderColor(0, 0, 0, 255);
    RenderFillRect(target, popup->r);

    RenderColor(255, 255, 255, 255);
    RenderDrawRect(target, popup->r);

    popup->a->x = 1*gs->game_width/5 + popup->b->w*1;
    popup->a->y = popup->r.y + popup->r.h - Scale(50);

    popup->b->x = 4*gs->game_width/5 - popup->a->w*2;
    popup->b->y = popup->r.y + popup->r.h - Scale(50);

    button_tick(popup->a, null);
    button_tick(popup->b, null);
    if (!popup->active) return;

    if (gs->input.keys_pressed[SDL_SCANCODE_RETURN]) {
        popup->a->on_pressed(null);
    } else if (gs->input.keys_pressed[SDL_SCANCODE_ESCAPE]) {
        popup->b->on_pressed(null);
    }

    button_draw(target, popup->a);
    button_draw(target, popup->b);

    SDL_Color col = (SDL_Color){175, 175, 175, 255};

    TTF_SizeText(gs->fonts.font_times->handle, popup->text, &data->text_width, &data->text_height);

    RenderTextQuick(target,
                    "confirm text",
                    gs->fonts.font_times,
                    "Confirmation",
                    col,
                    popup->r.x + Scale(16),
                    popup->r.y + Scale(10),
                    null,
                    null,
                    false);

    col = (SDL_Color){255, 255, 255, 255};
    RenderTextQuick(target,
                    "sdfsdf",
                    gs->fonts.font_times,
                    popup->text,
                    col,
                    popup->r.x + popup->r.w/2 - data->text_width/2,
                    popup->r.y + Scale(55),
                    null,
                    null,
                    false);
}
