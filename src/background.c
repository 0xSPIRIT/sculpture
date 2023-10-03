static Background background_init(void) {
    Background background = {0};
    background.surface = gs->surfaces.background;
    return background;
}

static void background_draw(int target, Background *bg, int xoff, int yoff) {
    int w = bg->surface->w, h = bg->surface->h;

    int background = TEXTURE_BG_0;

    int level = gs->level_current+1;

    if (level == 11) {
        background = TEXTURE_BG_1;
    } else if (level >= 5) {
        background = TEXTURE_BG_2;
    }

    if (background == 0) return;

    Assert(w == 128);
    Assert(h == 96);
    Assert(bg->surface->format->BytesPerPixel == 4);

    memset_u32((u32*)bg->surface->pixels, 0xff100000, w*h);
    
    SDL_Rect dst = { xoff, yoff, w, h };

    Texture texture = RenderCreateTextureFromSurface(bg->surface);
    RenderTexture(target, &texture, null, &dst);
    RenderTexture(target, &GetTexture(background), null, &dst);
    RenderDestroyTexture(&texture);
}
