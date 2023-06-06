static void dust_init(void) {
    if (!gs->dust_data.xs) {
        gs->dust_data.types = PushArray(gs->persistent_memory,
                                        gs->gw*gs->gh,
                                        sizeof(enum Cell_Type));
        gs->dust_data.xs = PushArray(gs->persistent_memory,
                                     gs->gw*gs->gh,
                                     sizeof(f64));
        gs->dust_data.ys = PushArray(gs->persistent_memory,
                                     gs->gw*gs->gh,
                                     sizeof(f64));
        gs->dust_data.vxs = PushArray(gs->persistent_memory,
                                     gs->gw*gs->gh,
                                     sizeof(f64));
        gs->dust_data.vys = PushArray(gs->persistent_memory,
                                     gs->gw*gs->gh,
                                     sizeof(f64));
        gs->dust_data.timers = PushArray(gs->persistent_memory,
                                         gs->gw*gs->gh,
                                         sizeof(int));
    }
    
    memset(gs->dust_data.types, 0, sizeof(enum Cell_Type)*gs->gw*gs->gh);
    memset(gs->dust_data.xs, 0, sizeof(f64)*gs->gw*gs->gh);
    memset(gs->dust_data.ys, 0, sizeof(f64)*gs->gw*gs->gh);
    memset(gs->dust_data.vxs, 0, sizeof(f64)*gs->gw*gs->gh);
    memset(gs->dust_data.vys, 0, sizeof(f64)*gs->gw*gs->gh);
    memset(gs->dust_data.timers, 0, sizeof(int)*gs->gw*gs->gh);
    gs->dust_data.count = 0;
}

static void emit_dust(enum Cell_Type type, int x, int y, f64 vx, f64 vy) {
    gs->dust_data.types[gs->dust_data.count] = type;
    gs->dust_data.xs[gs->dust_data.count] = x;
    gs->dust_data.ys[gs->dust_data.count] = y;
    gs->dust_data.vxs[gs->dust_data.count] = vx;
    gs->dust_data.vys[gs->dust_data.count] = vy;
    gs->dust_data.timers[gs->dust_data.count] = 0;
    gs->dust_data.count++;
}

static void emit_dust_explosion(enum Cell_Type type, int x, int y, int count) {
    for (int i = 0; i < count; i++) {
        f64 vx = randf(2.0)-1;
        f64 vy = randf(2.0)-1;
        emit_dust(type, x, y, vx, vy);
    }
}

static void swap_array(Cell *arr, int x1, int y1, int x2, int y2);

static void dust_grid_tick(void) {
    Dust_Data *data = &gs->dust_data;
    
    for (int i = 0; i < data->count; i++) {
        const f64 gravity = 0.2f;
        data->vys[i] += gravity;
        
        data->xs[i] += data->vxs[i];
        data->ys[i] += data->vys[i];
        data->timers[i]++;
        
        const int total = 30;
        if (data->timers[i] >= total || !is_in_boundsf(data->xs[i], data->ys[i])) {
            data->count--;
            for (int j = i; j < data->count; j++) {
                data->xs[j] = data->xs[j+1];
                data->ys[j] = data->ys[j+1];
                data->vxs[j] = data->vxs[j+1];
                data->vys[j] = data->vys[j+1];
                data->timers[j] = data->timers[j+1];
            }
            i--;
        }
    }
}

static SDL_Color pixel_from_index(enum Cell_Type type, int i);

static void dust_grid_draw(int target) {
    Dust_Data *data = &gs->dust_data;
    
    for (int i = 0; i < data->count; i++) {
        SDL_Color c = pixel_from_index(data->types[i], (int)data->xs[i] + (int)data->ys[i]*gs->gw);
        const f64 coeff = 0.5;
        RenderColor(c.r*coeff, c.g*coeff, c.b*coeff, c.a);
        RenderPointRelative(target, data->xs[i], data->ys[i]);
    }
}
