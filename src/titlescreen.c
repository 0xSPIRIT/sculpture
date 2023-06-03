static void titlescreen_init(void) {
}

static void titlescreen_goto_next(void) {
    gs->gamestate = GAME_STATE_PLAY;
}

static void titlescreen_tick(void) {
    if (gs->input.keys[SDL_SCANCODE_RETURN]) {
        gs->titlescreen.stop = true;
        Mix_FadeOutMusic(1000);
        Mix_HookMusicFinished(titlescreen_goto_next);
    }
}

static void titlescreen_draw(void) {
    RenderColor(255, 255, 255, 255);
    SDL_RenderClear(gs->renderer);
    
    if (gs->titlescreen.stop) return;
    
    TTF_SizeText(gs->fonts.font_titlescreen->handle, "Alaska", &gs->titlescreen.text_width, NULL);
    RenderDrawTextQuick(RENDER_TARGET_GLOBAL,
                        "titlescreen",
                        gs->fonts.font_titlescreen,
                        "Alaska",
                        BLACK,
                        gs->window_width/2 - gs->titlescreen.text_width/2,
                        gs->window_height/7,
                        255,
                        NULL,
                        NULL,
                        false);
    
    char *string = "Press RETURN";
    int w;
    
    f64 a = (1+sin(3*SDL_GetTicks()/1000.0))/2;
    a *= 255;
    
    TTF_SizeText(gs->fonts.font_times->handle, string, &w, NULL);
    RenderDrawTextQuick(RENDER_TARGET_GLOBAL, //TODO
                        "something else",
                        gs->fonts.font_times,
                        string,
                        (SDL_Color){a, a, a, 255},
                        gs->window_width/2 - w/2,
                        2*gs->window_height/3,
                        255,
                        NULL,
                        NULL,
                        false);
    RenderDrawTextQuick(RENDER_TARGET_GLOBAL,
                        "another",
                        gs->fonts.font_times,
                        "F11 - Fullscreen",
                        BLACK,
                        gs->window_width-8,
                        gs->window_height-8,
                        255,
                        NULL,
                        NULL,
                        false);
}