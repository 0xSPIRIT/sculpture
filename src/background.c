static Background background_init(void) {
    Background background = {0};
    background.surface = gs->surfaces.background;
    
    int w = gs->gw;
    int h = gs->gh * 2;
    background.field = PushArray(gs->persistent_memory,
                                 w*h,
                                 sizeof(vec2));
    
    background_setup_particles(background.particles, w, h);
    
    return background;
}

static void background_setup_vector_field(vec2 *field, int w, int h) {
    f32 time = SDL_GetTicks()/900.f;
    f32 coeff = 1.0;
    
    for (int i = 0; i < w*h; i++) {
        f32 off = (f32)i/(w*h);
        off *= 2.5;
        field[i] = (vec2){ coeff * cosf(time + off) , coeff * sinf(time + off) };
    }
}

static void background_setup_particles(vec2 *particles, int w, int h) {
    for (int i = 0; i < BACKGROUND_PARTICLE_COUNT; i++)
        particles[i] = (vec2){rand()%w, rand()%h};
}

static void background_draw_van_gogh(int target, Background *bg) {
    int w = bg->surface->w, h = bg->surface->h;
    
    Assert(w == 128);
    Assert(h == 128);
    
    background_setup_vector_field(bg->field, w, h);
    
    memset(bg->surface->pixels, 0, w*h*sizeof(Uint32));
    
    for (int i = 0; i < BACKGROUND_PARTICLE_COUNT; i++) {
        vec2 *p = &bg->particles[i];
        
        if (!gs->paused || gs->step_one) {
            // Tick
            
            vec2 dir_at = bg->field[(int)p->x + (int)p->y * w];
            
            p->x += dir_at.x;
            p->y += dir_at.y;
        }
        
        // Wrap
        
        if (p->x >= w) p->x -= w;
        if (p->y >= h) p->y -= h;
        if (p->x <  0) p->x += w;
        if (p->y <  0) p->y += h;
        
        
        // Draw
        
        Uint8 r = 255, g = 255, b = 0;
        Uint32 pixel = 0xFF000000 | (Uint8)b << 16 | (Uint8)g << 8 | (Uint8)r;
        
        set_pixel(bg->surface, p->x, p->y, pixel);
    }

    Texture texture = RenderCreateTextureFromSurface(bg->surface);
    RenderTextureAlphaMod(&texture, 100 + 100 * NormalSine(SDL_GetTicks()/100.0));
    RenderTexture(target, &texture, null, &(SDL_Rect){ 64, 0, 128, 128 });
    RenderDestroyTexture(&texture);
    
}


static void background_draw_disco(int target, Background *bg) {
    int w = bg->surface->w, h = bg->surface->h;
    
    bg->time += 1.f/60.f;
    
    f64 rcoeff, gcoeff, bcoeff;
    
    rcoeff = NormalSine(bg->time);
    gcoeff = NormalSine(bg->time+M_PI);
    bcoeff = NormalSine(bg->time+M_PI/2);
    
    for (int i = 0; i < w*h; i++) {
        f64 r, g, b;
        
        r = 100*rcoeff;
        g = 100*gcoeff;
        b = 100*bcoeff;
        
        // Build pixel from rgb
        Uint32 pixel = 0xFF000000 | (Uint8)b << 16 | (Uint8)g << 8 | (Uint8)r;
        set_pixel(bg->surface, i%w, i/w, pixel);
    }
    
    Texture texture = RenderCreateTextureFromSurface(bg->surface);
    RenderTexture(target, &texture, null, null);
    RenderDestroyTexture(&texture);
}

static void background_draw(int target, Background *bg) {
    bool disco_mode = false;
    
    if (disco_mode) {
        background_draw_disco(target, bg);
    } else {
        background_draw_van_gogh(target, bg);
    }
}

