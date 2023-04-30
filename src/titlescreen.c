void titlescreen_init(void) {
    TTF_SizeText(gs->fonts.font_titlescreen, "Alaska", &gs->titlescreen.text_width, NULL);
}

void titlescreen_goto_next(void) {
    gs->gamestate = GAME_STATE_PLAY;
}

void titlescreen_tick(void) {
    if (gs->input.keys[SDL_SCANCODE_RETURN]) {
        gs->titlescreen.stop = true;
        Mix_FadeOutMusic(1000);
        Mix_HookMusicFinished(titlescreen_goto_next);
    }
}

void titlescreen_draw(void) {
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    SDL_RenderClear(gs->renderer);
    
    if (gs->titlescreen.stop) return;
    
    draw_text_blended_indexed(TEXT_TITLESCREEN,
                              gs->fonts.font_titlescreen,
                              "Alaska",
                              BLACK,
                              false,
                              false,
                              gs->window_width/2 - gs->titlescreen.text_width/2,
                              gs->window_height/7,
                              NULL,
                              NULL);
    
    char *string = "Press RETURN";
    int w;
    
    f64 a = (1+sin(3*SDL_GetTicks()/1000.0))/2;
    a *= 255;
    
    TTF_SizeText(gs->fonts.font_times, string, &w, NULL);
    draw_text( gs->fonts.font_times,
              string,
              (SDL_Color){a, a, a, 255},
              (SDL_Color){255, 255, 255, 255},
              false,
              false,
              gs->window_width/2 - w/2,
              2*gs->window_height/3,
              NULL,
              NULL);
    draw_text(gs->fonts.font_times,
              "F11 - Fullscreen",
              BLACK,
              WHITE,
              true,
              true,
              gs->window_width-8,
              gs->window_height-8,
              NULL,
              NULL);
}