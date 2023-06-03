static Background background_init(void) {
    Background background = {0};
    background.surface = gs->surfaces.background;
    return background;
}

static f64 NormalSine(f64 t) {
    return 0.5*(1 + sin(t));
}

static void background_draw(int target, Background *bg) {
    int w = bg->surface->w, h = bg->surface->h;
    
    bg->time += gs->dt;
    
    for (int i = 0; i < w*h; i++) {
        f64 r, g, b;
        
        r = g = b = 30*NormalSine(bg->time);
        
        Uint32 pixel = 0xFF000000 | (Uint8)b << 16 | (Uint8)g << 8 | (Uint8)r;
        set_pixel(bg->surface, i%w, i/w, pixel);
    }
    
    Texture texture = RenderCreateTextureFromSurface(bg->surface);
    RenderTexture(target, &texture, NULL, NULL);
    RenderDestroyTexture(&texture);
}