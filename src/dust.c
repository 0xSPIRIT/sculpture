static void dust_init(void) {
    memset(gs->dust, 0, MAX_DUST_COUNT*sizeof(Dust));
    gs->dust_count = 0;
}

static void emit_dust(enum Cell_Type type,
                      int x,
                      int y,
                      f64 vx,
                      f64 vy)
{
    if (gs->dust_count >= MAX_DUST_COUNT) return;
    
    gs->dust[gs->dust_count++] = (Dust){
        .type = type,
        .x = x,
        .y = y,
        .vx = vx,
        .vy = vy,
        .timer = 0,
        .timer2 = 0,
        .timer_max = rand()%35+40,
        .going_into_inventory = false
    };
}

static void emit_dust_explosion(enum Cell_Type type,
                                int x,
                                int y,
                                int count)
{
    for (int i = 0; i < count; i++) {
        f64 angle = randf(2*M_PI);
        f64 vx = cos(angle);
        f64 vy = sin(angle);
        emit_dust(type, x, y, vx, vy);
    }
}

static void dust_remove(int i) {
    gs->dust_count--;
    for (int j = i; j < gs->dust_count; j++) {
        gs->dust[j] = gs->dust[j+1];
    }
    i--;
}

static void dust_grid_run(int target) {
    for (int i = 0; i < gs->dust_count; i++) {
        Dust *dust = &gs->dust[i];
        
        if (distance(dust->x, dust->y, gs->gw/2, 0) < 3) {
            dust_remove(i);
            i--;
            continue;
        }
        
        if (dust->y >= gs->gh) {
            dust->y = gs->gh-1;
            dust->vy *= -0.5;
            dust->vx *= 0.75;
        }
        
        if (dust->timer == dust->timer_max) {
            if (gs->level_current+1 >= 4) {
                dust->going_into_inventory = true;
                dust->timer2 = 0;
            } else {
                dust_remove(i);
                i--;
                continue;
            }
        }
        
        dust->timer++;
        
        if (dust->going_into_inventory) {
            f64 dx = gs->gw / 2 - dust->x;
            f64 dy = -dust->y;
            f64 length = sqrt(dx*dx + dy*dy);
            dx /= length;
            dy /= length;
            
            // TODO: Make this go slower at the start,
            //       then increase the speed. Maybe use
            //       interpolate()
            
            f64 max = 30;
            dust->timer2 = min(dust->timer2, max);
            f64 coeff = dust->timer2 / max;
            
            dust->timer2++;
            
            dust->vx = coeff * 2 * dx;
            dust->vy = coeff * 2 * dy;
        } else {
            dust->vy += 0.2;
        }
        
        dust->x += dust->vx;
        dust->y += dust->vy;
        
        if (abs(dust->y-gs->gh-1) < 1 && abs(dust->vy) < 1) {
            dust->y = gs->gh-1;
            dust->vy = 0;
            dust->vx *= 0.25;
        }
        
        SDL_Color c = pixel_from_index_grid(gs->grid,
                                            dust->type,
                                            0);
        c.r *= 0.5;
        c.g *= 0.5;
        c.b *= 0.5;
        RenderColor(c.r, c.g, c.b, 200);
        RenderPointRelative(target, dust->x, dust->y);
    }
}
