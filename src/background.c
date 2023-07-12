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
    (void)w, h;

    bg->time += 1.f/60.f;

    f64 rcoeff, gcoeff, bcoeff;

    rcoeff = NormalSine(bg->time);
    gcoeff = NormalSine(bg->time+M_PI);
    bcoeff = NormalSine(bg->time+M_PI/2);
    
#if 0
    for (int i = 0; i < w*h; i++) {
        f64 r, g, b;

        r = 0;
        g = 0;
        b = 0;

        Uint32 pixel = 0xFF000000 | (Uint8)b << 16 | (Uint8)g << 8 | (Uint8)r;
        set_pixel(bg->surface, i%w, i/w, pixel);
    }
#endif

    Texture texture = RenderCreateTextureFromSurface(bg->surface);
    RenderTexture(target, &texture, null, null);
    RenderDestroyTexture(&texture);
}
