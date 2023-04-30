void timelapse_init() {
    struct Timelapse *tl = &gs->timelapse;
    tl->timer = 0;
    tl->timer_max = 5;
    tl->frame = 0;
}

bool timelapse_is_state_grids_same(int a, int b) {
    struct Save_State *sa = &gs->save_states[a];
    struct Save_State *sb = &gs->save_states[b];
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (sa->grid_layers[0][i].type != sb->grid_layers[0][i].type) return false;
    }
    
    return true;
}

void timelapse_tick_and_draw(int xx, int yy, int cw, int ch) {
    struct Timelapse *tl = &gs->timelapse;
    
    if (tl->frame < gs->save_state_count) {
        tl->timer++;
        
        if (tl->timer >= tl->timer_max) {
            tl->frame++;
            
            while (tl->frame < gs->save_state_count && timelapse_is_state_grids_same(tl->frame, tl->frame-1)) {
                tl->frame++;
            }
            if (tl->frame >= gs->save_state_count) {
                tl->sticky = 1;
            } 
            
            tl->timer = 0;
        }
    }
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        struct Save_State *state = NULL;
        struct Cell *grid = NULL;
        
        if (tl->sticky) {
            grid = gs->grid;
        } else {
            state = &gs->save_states[tl->frame];
            grid = state->grid_layers[0];
        }
        
        int x = i%gs->gw, y = i/gs->gw;
        
        SDL_Color c = pixel_from_index_grid(grid, grid[i].type, i);
        SDL_SetRenderDrawColor(gs->renderer, c.r, c.g, c.b, 255);
        SDL_Rect r = { xx + x*cw, yy + y*ch, cw, ch };
        SDL_RenderFillRect(gs->renderer, &r);
    }
}