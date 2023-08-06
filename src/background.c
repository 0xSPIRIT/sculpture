static Background background_init(void) {
    Background background = {0};
    background.surface = gs->surfaces.background;
    return background;
}

static void background_draw_disco(int target, Background *bg) {
    int w = bg->surface->w, h = bg->surface->h;
    (void)w, h;

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
        // TODO the actual stuff
    }
}
    