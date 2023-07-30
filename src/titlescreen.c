static void titlescreen_init(void) {
    Titlescreen *t = &gs->titlescreen;
    effect_set(&t->effect,
               EFFECT_SNOW,
               true,
               0,
               0,
               gs->game_width,
               gs->game_height);

    //preview_load(&t->preview, RES_DIR "previews/test.bin");
}

static void titlescreen_goto_next(void) {
    gs->gamestate = GAME_STATE_PLAY;
}

static void titlescreen_tick(void) {
    Uint8 *keys = gs->input.keys;
    bool mouse_pressed = gs->input.mouse_pressed[SDL_BUTTON_LEFT];

    if (mouse_pressed ||
        keys[SDL_SCANCODE_RETURN] ||
        keys[SDL_SCANCODE_SPACE]  ||
        keys[SDL_SCANCODE_TAB])
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
                        "F11 - Fullscreen",
                        WHITE,
                        gs->game_width-w-8,
                        gs->game_height-h-8,
                        null,
                        null,
                        false);
    }
}
