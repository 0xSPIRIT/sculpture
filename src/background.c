static Background background_init(void) {
    Background background = {0};
    background.surface = gs->surfaces.background;
    
    //background_setup_particles(background.particles, w, h);
    background_setup_star(&background.star);
    
    return background;
}

static void background_setup_star(Star *star) {
    int w = gs->gw;
    int h = gs->gh;
    
    star->sx = w/2;
    star->sy = h/2;
    star->x = star->sx;
    star->y = star->sy;
    
    star->length = 25+rand()%10;
    
    f32 angle = randf(M_PI);
    star->dir_x = cos(angle);
    star->dir_y = sin(angle);
}

// 0 <= x <= 1
static f32 star_function(f32 x) {
    return (3*x*x - 2*x*x*x) * (2*x - x*x);
}

/*
static void background_setup_particles(Background_Particle *particles, int w, int h) {
    int spiral_width = 6;
    
    for (int i = 0; i < BACKGROUND_PARTICLE_COUNT; i++) {
        int x, y;
        f32 time = (f32)i/256.f;
        
        x = w/2 - spiral_width/2 + (i%spiral_width);
        y = h/2 - i%spiral_width;
        
        particles[i] = (Background_Particle){1, x, y, x, y, time};
    }
}

static void particle_wrap(int *x, int *y, int w, int h) {
    while (*x <  0) (*x) += w;
    while (*x >= w) (*x) -= w;
    while (*y <  0) (*y) += h;
    while (*y >= h) (*y) -= h;
}


static f32 particle_function_cleaned_t(f32 t) {
    t = 1-t;
    t *= 6*M_PI;
    t -= 3.f*M_PI/2.f;
    return t;
}

// 0 <= t <= 1
static f32 particle_function_coefficient(f32 t) {
    f32 coeff = 22-t*22;
    return coeff;
}

// 0 <= t <= 1
static f32 particle_function_x(f32 t) {
    f32 orig_t = t;
    t = particle_function_cleaned_t(t);
    return cos(t) * particle_function_coefficient(orig_t);
}

// 0 <= t <= 1
static f32 particle_function_y(f32 t) {
    f32 orig_t = t;
    t = particle_function_cleaned_t(t);
    return sin(t) * particle_function_coefficient(orig_t);
}

static void background_draw_van_gogh(int target_enum, Background *bg) {
    int w = bg->surface->w, h = bg->surface->h;
    
    Assert(w == 128);
    Assert(h == 128);
    
    memset(bg->surface->pixels, 0, w*h*sizeof(Uint32));
    
    for (int i = 0; i < BACKGROUND_PARTICLE_COUNT; i++) {
        Background_Particle *p = &bg->particles[i];
        if (p->value == 0) continue;
        
        if (!gs->paused || gs->step_one) {
            // Tick
            
            p->timer += 1.f/360.f;
            if (p->timer >= 1) p->timer = 0;
            
            float time = p->timer;
            
            p->x = p->sx + round(0.5 * particle_function_x(time));
            p->y = p->sy + round(0.6 * particle_function_y(time));
        }
        
        // Draw
        
        Uint8 r = 255, g = 255, b = 0;
        f64 a = 0.5;
        
        // Get the target pixel.
        Uint32 *target = get_pointer_to_pixel(bg->surface, p->x, p->y);
        
        if (a == 1.0) {
            // Overwrite the old pixel with the new.
            Uint32 pixel = 0xFF000000 | (Uint8)b << 16 | (Uint8)g << 8 | (Uint8)r;
            *target = pixel;
        } else {
            // Disassemble the old pixel
            int a0, r0, g0, b0;
            
            r0 = (*target) & 0x000000FF;
            g0 = (*target) & 0x0000FF00 >> 8;
            b0 = (*target) & 0x00FF0000 >> 16;
            a0 = (*target) & 0xFF000000 >> 24;
            
            // Add in the new pixel according to the alpha.
            r0 = lerp(r0, r, a);
            g0 = lerp(g0, g, a);
            b0 = lerp(b0, b, a);
            a0 += 255*a; if (a0 > 255) a0 = 255;
            
            // Assemble and write it back to the target.
            Uint32 pixel = (Uint8)a0 << 24 | (Uint8)b0 << 16 | (Uint8)g0 << 8 | (Uint8)r0;
            *target = pixel;
        }
    }
    
    Texture texture = RenderCreateTextureFromSurface(bg->surface);
    RenderTextureAlphaMod(&texture, 255);
    RenderTexture(target_enum, &texture, null, &(SDL_Rect){ 64, 0, 128, 128 });
    RenderDestroyTexture(&texture);
}
*/

static void background_draw_star(int target, Background *bg) {
    bg->time += 1.0/60.0;
    if (bg->time > 1) {
        bg->time = 0;
        background_setup_star(&bg->star);
    }
    
    f32 x = bg->time;
    
    f32 f = star_function(x);
    
    f32 px = bg->star.x;
    f32 py = bg->star.y;
    
    f32 dx = bg->star.dir_x * f;
    f32 dy = bg->star.dir_y * f;
    
    bg->star.x = bg->star.sx + bg->star.length * dx;
    bg->star.y = bg->star.sy + bg->star.length * dy;
    
    f32 parabola = 4*(-x*x+x);
    
    Uint8 alpha = parabola*255;
    
    int d = 8;
    
    RenderColor(200, 200, 127, alpha);
    RenderLine(target, 64+px-dx*parabola*d, py-dy*parabola*d, 64+bg->star.x, bg->star.y);
}

static void background_draw(int target, Background *bg) {
    background_draw_star(target, bg);
    //background_draw_van_gogh(target, bg);
}

