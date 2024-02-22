static void titlescreen_init(void) {
    Titlescreen *t = &gs->titlescreen;
    effect_set(&t->effect,
               EFFECT_SNOW,
               true,
               0,
               0,
               gs->game_width,
               gs->game_height);

    sprintf(t->goto_level_field.text_input, "%d", gs->level_current+1);
    t->goto_level_field.active = true;
}

static void titlescreen_goto_next(void) {
    Titlescreen_Goto_Level *field = &gs->titlescreen.goto_level_field;

    if (field->active && field->text_input[0]) {
        int value = atoi(field->text_input)-1;
        if (value < 0) value = 0;
        if (value > 10) value = 10;
        gs->level_current = value;
    }

    gs->gamestate = GAME_STATE_PLAY;
    goto_level(gs->level_current);
}

static void draw_focus(int target) {
    RenderColor(0,0,0,128);
    RenderFillRect(0, (SDL_Rect){0, 0, gs->game_width, gs->game_height});

    Render_Text_Data data = {0};
    strcpy(data.identifier, "thing");
    data.font = gs->fonts.font_title_2;
    strcpy(data.str, "Click to focus");
    data.x = gs->game_width/2;
    data.y = gs->game_height/2;
    data.foreground = WHITE;
    data.alignment = ALIGNMENT_CENTER;
    data.render_type = TEXT_RENDER_BLENDED;

    RenderText(target, &data);
}

static bool titlescreen_input_next(void) {
    u8 *keys = gs->input.keys;
    bool mouse_pressed = gs->input.mouse_pressed[SDL_BUTTON_LEFT];

    return (mouse_pressed ||
            keys[SDL_SCANCODE_RETURN] ||
            keys[SDL_SCANCODE_SPACE]  ||
            keys[SDL_SCANCODE_TAB]);
}

static void titlescreen_tick_goto_level(Titlescreen_Goto_Level *field) {
    if (!field->active) return;
    if (gs->titlescreen.state != 1) return;

    switch (gs->event->type) {
        case SDL_TEXTINPUT: {
            strcat(field->text_input, gs->event->text.text);
        } break;
        case SDL_KEYDOWN: {
            if (gs->event->key.keysym.sym == SDLK_BACKSPACE) {
                u64 length = strlen(field->text_input);
                if (length > 0) {
                    field->text_input[length-1] = 0;
                }
            }
        } break;
    }
}

static void titlescreen_draw_goto_level(int target, Titlescreen_Goto_Level *field) {
    if (!field->active) return;

    Render_Text_Data data = {0};

    strcpy(data.identifier, "sdlf");
    data.font = gs->fonts.font_times;
    strcpy(data.str, "Begin game at level (1 to 11):");
    data.x = gs->game_width/2;
    data.y = gs->game_height/2;
    data.foreground = WHITE;
    data.alignment = ALIGNMENT_CENTER;
    data.render_type = TEXT_RENDER_BLENDED;

    RenderText(target, &data);

    int width = 0, height = 0;

    TTF_SizeText(gs->fonts.font_times->handle, field->text_input, &width, &height);
    int pad = Scale(6);

    RenderColor(0, 0, 0, 255);
    SDL_Rect r = {
        data.x + data.texture.width/2 + Scale(20) - pad,
        data.y - data.texture.height/2,
        width + pad*2,
        height,
    };

    RenderFillRect(target, r);
    RenderColor(255, 255, 255, 255);
    RenderDrawRect(target, r);

    Render_Text_Data data2 = (Render_Text_Data){0};
    strcpy(data2.identifier, "ceasdf");
    strcpy(data2.str, field->text_input);
    data2.font = gs->fonts.font_times;
    data2.x = data.x + data.texture.width/2 + Scale(20);
    data2.y = data.y - data.texture.height/2;
    data2.foreground = WHITE;
    data2.render_type = TEXT_RENDER_BLENDED;

    RenderText(target, &data2);

    // draw caret

    if (field->caret_timer <= 30) {
        RenderColor(255, 255, 255, 255);

        RenderLine(target, data2.x + width, data2.y, data2.x + width, data2.y + height);
    }
    field->caret_timer++;
    if (field->caret_timer >= 60) field->caret_timer = 0;
}

static void titlescreen_state_2(void) {
    gs->titlescreen.state = 1;
    set_fade(FADE_TITLESCREEN, 255, 0);
}

static void titlescreen_draw(int target) {
    RenderColor(0, 0, 0, 255);
    RenderClear(target);

    if (gs->titlescreen.stop) return;

    bool pressed_delete_save = false;

    int text_width, text_height;
    TTF_SizeText(gs->fonts.font_titlescreen->handle,
                 "Alaska",
                 &text_width,
                 &text_height);

    effect_draw(RENDER_TARGET_MASTER, &gs->titlescreen.effect, false);

    if (gs->titlescreen.state == 0) {
        RenderTextQuick(target,
                        "titlescreen",
                        gs->fonts.font_titlescreen,
                        "Alaska",
                        WHITE,
                        gs->game_width/2 - text_width/2,
                        gs->game_height/4 - text_height/2,
                        null,
                        null,
                        false);

        TTF_SizeText(gs->fonts.font_times->handle,
                     "by spiritwolf",
                     &text_width,
                     &text_height);
        RenderTextQuick(target,
                        "spirit",
                        gs->fonts.font_times,
                        "by spiritwolf",
                        (SDL_Color){200,200,200,255},
                        gs->game_width/2 - text_width/2,
                        gs->game_height/2 - Scale(150),
                        null, null, false);

#ifdef __EMSCRIPTEN__
        RenderTextQuick(target,
                        "23123",
                        gs->fonts.font_times,
                        "Consider desktop version for superior performance!",
                        (SDL_Color){100,100,100,255},
                        Scale(16),
                        gs->game_height - Scale(45),
                        null,
                        null,
                        false);
#endif

#ifndef __EMSCRIPTEN__
        {
            const char *text = "F11 - Fullscreen";
            int w, h;
            TTF_SizeText(gs->fonts.font_times->handle,
                         text,
                         &w,
                         &h);
            RenderTextQuick(target,
                            "another",
                            gs->fonts.font_times,
                            text,
                            WHITE,
                            gs->game_width-w-Scale(8),
                            gs->game_height-h-Scale(8),
                            null,
                            null,
                            false);
        }
#endif

        f64 a = (1+sin(3*SDL_GetTicks()/1000.0))/2;
        a *= 255;

        char text[64];
        if (gs->level_current > 0) {
            sprintf(text, "Continue: Level %d", gs->level_current+1);
        } else {
            sprintf(text, "Click to begin");
        }

        Render_Text_Data data = {0};

        strcpy(data.identifier, "continuestr");
        data.font = gs->fonts.font_title_2;
        strcpy(data.str, text);
        data.x = gs->game_width/2;
        data.y = gs->game_height/2 + Scale(150);
        data.foreground = RGBA(255, 255, 255, 180);
        data.alignment = ALIGNMENT_CENTER;

        RenderText(target, &data);

#ifndef __EMSCRIPTEN__
        pressed_delete_save = draw_text_button(target,
                                               "delete",
                                               "Delete Save Data",
                                               Scale(8+4),
                                               gs->game_height - Scale(8+4),
                                               gs->fonts.font,
                                               WHITE,
                                               ALIGNMENT_BOTTOM_LEFT,
                                               TEXT_RENDER_BLENDED);
        if (pressed_delete_save) {
            gs->level_previous = 0;
            gs->level_current = 0;
            sprintf(gs->titlescreen.goto_level_field.text_input, "%d", gs->level_current+1);
            save_game();
        }
#endif

        if (pressed_delete_save) return;

        bool can_goto = true;

#ifdef __EMSCRIPTEN__
        bool mouse_pressed = gs->input.mouse_pressed[SDL_BUTTON_LEFT];
        can_goto = gs->titlescreen.clicked_yet;
        if (!can_goto && mouse_pressed) {
            gs->titlescreen.clicked_yet = true;
        }
#endif

        Titlescreen_Goto_Level *field = &gs->titlescreen.goto_level_field;

        if (can_goto && titlescreen_input_next()) {
            set_fade(FADE_TITLESCREEN_3, 0, 255);
        }

        if (wait_for_fade(FADE_TITLESCREEN_3)) {
            if (!field->active) {
                titlescreen_goto_next();
            } else {
                titlescreen_state_2();
            }
        }
    } else {
        titlescreen_draw_goto_level(target, &gs->titlescreen.goto_level_field);
        if (titlescreen_input_next() && !gs->titlescreen.fading) {
            set_fade(FADE_TITLESCREEN_2, 0, 255);
            Mix_FadeOutMusic(1000);
            Mix_HookMusicFinished(titlescreen_goto_next);
            gs->titlescreen.fading = true;
        }
    }

    fade_draw(target);
}
