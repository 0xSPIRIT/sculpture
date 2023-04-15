bool wait_for_fade(int id) {
    if (gs->fade.id == id)
        return !gs->fade.active;
    return false;
}

void set_fade(int id, f64 start, f64 end) {
    gs->fade.id = id;
    gs->fade.alpha = gs->fade.start_alpha = start;
    gs->fade.desired_alpha = end;
    gs->fade.active = true;
}

void reset_fade() {
    memset(&gs->fade, 0, sizeof(struct Fade));
}

void fade_draw() {
    if (!gs->fade.active) return;
    
#if 0
    f64 a = gs->fade.start_alpha;
    f64 b = gs->fade.desired_alpha;
    gs->fade.alpha = interpolate64(a, b, gs->fade.time);
    
    gs->fade.time += 2.0/60.0;
#endif
    
    gs->fade.alpha = interpolate64(gs->fade.alpha, gs->fade.desired_alpha, FADE_T);
    
    //gs->fade.alpha = lerp64(gs->fade.alpha, gs->fade.desired_alpha, FADE_T);
    if (abs(gs->fade.alpha - gs->fade.desired_alpha) < FADE_EPSILON) {
        gs->fade.alpha = gs->fade.desired_alpha;
        gs->fade.time = 0;
        gs->fade.active = false;
    }
    
    if (gs->obj.active) return; // Hack for ending. We need the fade to be active for other things to occur, but we don't want to draw the fade.
    
    SDL_SetRenderDrawColor(gs->renderer,
                           0, 0, 0, gs->fade.alpha);
    
    SDL_Rect rect = {
        0, 0,
        gs->window_width, gs->window_height
    };
    
    SDL_RenderFillRect(gs->renderer, &rect);
}