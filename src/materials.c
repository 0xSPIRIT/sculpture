static inline SDL_Color glass(int idx) {
    SDL_Color result;

    result = get_pixel(gs->surfaces.glass_surface, idx%gs->gw, idx/gs->gw);
    result.a = 255;

    f32 t = -0.10 * gs->frames;

    int x = idx%gs->gw;
    int y = idx/gs->gw;

    f32 amplitude = (1+sin(y+x+t/5))*10;

    f32 width = 0.65;

    int v = 32+sin(width * (t+x+y))*amplitude;

    int r, g, b;

    r = g = b = v;

    result.r += r;
    result.g += g;
    result.b += b;

    return result;
}

static inline SDL_Color ice(int idx) {
    SDL_Color result;

    int x = idx % gs->gw;
    int y = idx / gs->gw;

    result = get_pixel(gs->surfaces.ice_surface, x, y);
    result.r *= 0.80;
    result.g *= 0.80;

    f32 t = -0.07 * gs->frames;

    f32 amplitude = (1+sin(y-x+t/5))*10;

    f32 width = 0.44;

    int v = 10+sin(width * (t+x-y*2))*amplitude;

    int r, g, b;

    r = g = b = v;

    result.r = clamp((int)result.r + r, 0, 255);
    result.g = clamp((int)result.g + g, 0, 255);
    result.b = clamp((int)result.b + b, 0, 255);

    return result;
}

static inline SDL_Color marble(int idx) {
    SDL_Color result;
    
    int id = gs->grid[idx].id*2;
    result = get_pixel(gs->surfaces.marble_surface, id%gs->gw, id/gs->gw);
    
    int gray = result.r + result.g + result.b;
    gray /= 3;
    
    f64 t = 0.1*gs->frames;
        
    int threshold = 222;
    if (gray > threshold) {
        int amplitude = 15;
        int r = amplitude+amplitude*sin(0.25*(t+id));
        int g = r;
        int b = r;
        
        result.r = clamp(result.r+r, 0, 255);
        result.g = clamp(result.g+g, 0, 255);
        result.b = clamp(result.b+b, 0, 255);
    }
    
    return result;
}

static inline SDL_Color granite(int idx) {
    SDL_Color result;
    
    result = get_pixel(gs->surfaces.granite_surface, idx%gs->gw, idx/gs->gw);
    
    int gray = result.r + result.g + result.b;
    gray /= 3;
    
    f64 t = 0.2*gs->frames;
        
    int threshold = 150;
    if (gray < threshold) {
        int amplitude = 15;
        int r = amplitude+amplitude*sin(0.25*(t+idx));
        int g = r;
        int b = r;
        
        result.r = clamp(result.r+r, 0, 255);
        result.g = clamp(result.g+g, 0, 255);
        result.b = clamp(result.b+b, 0, 255);
    }
    
    return result;
}