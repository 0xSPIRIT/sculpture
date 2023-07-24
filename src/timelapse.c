static void timelapse_init() {
    Timelapse *tl = &gs->timelapse;
    tl->timer = 0;
    tl->timer_max = 5;
    tl->frame = 0;
}

static bool timelapse_is_state_grids_same(int a, int b) {
    Save_State *sa = &gs->save_states[a];
    Save_State *sb = &gs->save_states[b];

    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (sa->grid_layers[0][i].type != sb->grid_layers[0][i].type) return false;
    }

    return true;
}

static void timelapse_tick_and_draw(int xx, int yy, int cw, int ch) {
    Timelapse *tl = &gs->timelapse;
    
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
    
    Uint8 sin_value = (Uint8) (170 + 10 * sin(SDL_GetTicks()/1000.0));
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        Save_State *state = null;
        Cell *grid = null;
        
        Uint8 type = 0;
        
        if (tl->sticky) {
            grid = gs->grid;
            type = grid[i].type;
        } else {
            state = &gs->save_states[tl->frame];
            type = state->grid_layers[0][i].type;
        }
        
        if (!type) continue;
        
        int x = i%gs->gw, y = i/gs->gw;
        
        SDL_Color c = pixel_from_index_grid(gs->grid, type, i);
        c.a = 255;
        if (gs->gw == 128) {
            if (x < 32 || x > 32+64) {
                Uint8 v = (Uint8) (((int)(c.r+c.g+c.b))/3);
                c.r = c.g = c.b = v;
                c.a = sin_value;
            }
        }
        
        // Hardcode
        if (gs->gw == 128) {
            x -= 32;
        }

        RenderColor(c.r, c.g, c.b, c.a);
        SDL_Rect r = { xx + x*cw, yy + y*ch, cw, ch };
        SDL_RenderFillRect(gs->renderer, &r);
    }
}
