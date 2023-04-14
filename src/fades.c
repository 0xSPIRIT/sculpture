bool wait_for_fade(int id) {
    if (gs->fade.id == id)
        return !gs->fade.active;
    return false;
}

void set_fade(int id, float start, float end) {
    gs->fade.id = id;
    gs->fade.alpha = start;
    gs->fade.desired_alpha = end;
    gs->fade.active = true;
}

void reset_fade() {
    gs->fade.id = 0;
    gs->fade.alpha = 0;
    gs->fade.desired_alpha = 0;
    gs->fade.active = false;
}

void fade_draw() {
    gs->fade.alpha = lerp64(gs->fade.alpha, gs->fade.desired_alpha, FADE_T);
    if (abs(gs->fade.alpha - gs->fade.desired_alpha) < FADE_EPSILON) {
        gs->fade.alpha = gs->fade.desired_alpha;
        gs->fade.active = false;
    }
    
    SDL_SetRenderDrawColor(gs->renderer,
                           0, 0, 0, gs->fade.alpha);
    
    SDL_Rect rect = {
        0, 0,
        gs->window_width, gs->window_height
    };
    
    SDL_RenderFillRect(gs->renderer, &rect);
}