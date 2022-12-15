void timelapse_init() {
    gs->timelapse.timer = 0;
    gs->timelapse.timer_max = 5;
    gs->timelapse.frame = 0;
}

void timelapse_tick_and_draw(int xx, int yy, int cw, int ch) {
    if (gs->timelapse.frame < gs->save_state_count) {
        gs->timelapse.timer++;
        if (gs->timelapse.timer >= gs->timelapse.timer_max) {
            gs->timelapse.frame++;
            if (gs->timelapse.frame >= gs->save_state_count) {
                gs->timelapse.frame = 0;
            }
            gs->timelapse.timer = 0;
        }
    }
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        struct Save_State *state = &gs->save_states[gs->timelapse.frame];
        struct Cell *grid = state->grid_layers[0];
        
        int x = i%gs->gw, y = i/gs->gw;
        
        SDL_Color c = pixel_from_index_grid(grid, grid[i].type, i);
        SDL_SetRenderDrawColor(gs->renderer, c.r, c.g, c.b, 255);
        SDL_Rect r = { xx + x*cw, yy + y*ch, cw, ch };
        SDL_RenderFillRect(gs->renderer, &r);
    }
}