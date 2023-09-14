static void titlescreen_init(void) {
    Titlescreen *t = &gs->titlescreen;
    effect_set(&t->effect,
               EFFECT_SNOW,
               true,
               0,
               0,
               gs->game_width,
               gs->game_height);

    //preview_load(&t->preview, DATA_DIR "previews/test.bin");
}

static void titlescreen_goto_next(void) {
    gs->gamestate = GAME_STATE_PLAY;
}

static void titlescreen_tick(void) {
    u8 *keys = gs->input.keys;
    bool mouse_pressed = gs->input.mouse_pressed[SDL_BUTTON_LEFT];

    bool can_goto = true;

#ifdef __EMSCRIPTEN__
    can_goto = gs->titlescreen.clicked_yet;
    if (!can_goto && mouse_pressed) {
        gs->titlescreen.clicked_yet = true;
    }
#endif

    if (can_goto &&
        (mouse_pressed ||
        keys[SDL_SCANCODE_RETURN] ||
        keys[SDL_SCANCODE_SPACE]  ||
         keys[SDL_SCANCODE_TAB]))
    {
        gs->titlescreen.stop = true;
        if (Mix_PlayingMusic()) {
            Mix_FadeOutMusic(1000);
            Mix_HookMusicFinished(titlescreen_goto_next);
        } else {
            titlescreen_goto_next();
        }
    }
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

static void titlescreen_draw(int target) {
    RenderColor(0, 0, 0, 255);
    RenderClear(target);

    if (gs->titlescreen.stop) return;

    int text_width, text_height;
    TTF_SizeText(gs->fonts.font_titlescreen->handle,
                 "Alaska",
                 &text_width,
                 &text_height);

    effect_draw(RENDER_TARGET_MASTER, &gs->titlescreen.effect, false);

    if (0) {
        SDL_Rect dst = preview_draw(target, &gs->titlescreen.preview, 0, 0, gs->S, true, true);
        Texture *t = &RenderTarget(RENDER_TARGET_PREVIEW)->texture;

        RenderTextureColorMod(t, 90, 90, 90);

        RenderTargetToTarget(target, RENDER_TARGET_PREVIEW, null, &dst);

        RenderTextureColorMod(t, 255, 255, 255);
    }

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

    f64 a = (1+sin(3*SDL_GetTicks()/1000.0))/2;
    a *= 255;

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

    if (gs->level_current > 0) {
        char text[64];
        sprintf(text, "Continue: Level %d", gs->level_current+1);

        Render_Text_Data data = {0};

        strcpy(data.identifier, "continuestr");
        data.font = gs->fonts.font_title_2;
        strcpy(data.str, text);
        data.x = gs->game_width/2;
        data.y = gs->game_height/2 + Scale(50);
        data.foreground = (SDL_Color){255, 0, 0, 200};
        data.alignment = ALIGNMENT_CENTER;

        RenderText(target, &data);
    }


#ifdef __EMSCRIPTEN__
    if (!gs->titlescreen.clicked_yet) {
        draw_focus(target);
    }
#endif
}
