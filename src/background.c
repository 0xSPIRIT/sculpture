static Background background_init(void) {
    Background background = {0};
    background.surface = gs->surfaces.background;
    return background;
}

static void background_draw(int target, Background *bg, int xoff, int yoff) {
    int w = bg->surface->w, h = bg->surface->h;

    int background = TEXTURE_BG_0;

    int level = gs->level_current+1;

    if (level >= 4 && level <= 7) {
        background = TEXTURE_BG_2;
    } else if (level == 11) {
        background = TEXTURE_BG_1;
    }

    if (level == 9) background = 0;

    if (background == 0) return;

    Assert(w == 128);
    Assert(h == 96);
    Assert(bg->surface->format->BytesPerPixel == 4);

    // Draw teh background
    SDL_Rect dst = { xoff, yoff, w, h };
    RenderTexture(target, &GetTexture(background), null, &dst);
#if 0
    // Applying the lighting to the background.
    // Start with completely transparent surface
    memset_u32((u32*)bg->surface->pixels, 0x00000000, w*h);
    u32 *pixels = (u32*)bg->surface->pixels;

    // TODO: Bake in the backgroudn vignette. Don't do this surface crap.

    // Then apply the lighting pixel by pixel.
    for (int y = 0; y < bg->surface->h; y++) {
        for (int x = 0; x < bg->surface->w; x++) {
            u8 a;
            f64 cum_strength = 0;
            Lighting *lighting = &gs->lighting;

            for (int i = 0; i < lighting->light_count; i++) {
                if (lighting->lights[i].active)
                    cum_strength += get_light_strength_at_position(lighting->lights[i], x, y-32);
            }

            a = clamp((int)(cum_strength * 255), 0, 255);

            // We want the inverse of this.
            a = 255 - a;

            // Not too dark round the edges.
            a *= 0.75;

            pixels[x+y*bg->surface->w] |= ((u8)a << 24);
        }
    }

    Texture texture = RenderCreateTextureFromSurface(bg->surface);
    RenderTexture(target, &texture, null, &dst);
    RenderDestroyTexture(&texture);
#endif
}
