void dust_init(void) {
    if (!gs->dust_data.xs) {
        gs->dust_data.xs = PushArray(gs->persistent_memory,
                                     gs->gw*gs->gh,
                                     sizeof(f64));
        gs->dust_data.ys = PushArray(gs->persistent_memory,
                                     gs->gw*gs->gh,
                                     sizeof(f64));
    }
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        gs->dust_data.xs[i] = i % gs->gw;
        gs->dust_data.ys[i] = i / gs->gw;
    }
}

void emit_dust(enum Cell_Type type, int x, int y) {
    struct Cell *c = &gs->dust_grid[x+y*gs->gw];
    
    c->type = type;
    c->vx = 1.f;
    c->vy = 0.f;
}

void dust_grid_tick(void) {
    struct Cell *grid = gs->dust_grid;
    
    for (int i = 0; i < gs->gw * gs->gh; i++) {
        if (grid[i].type == 0) continue;
        if (grid[i].vx == 0 && grid[i].vy == 0) continue;
        
        // TODO: Finish adding the code for moving cells here.
        //       You use the Dust_Data as position data and
        //       round to int.
    }
}