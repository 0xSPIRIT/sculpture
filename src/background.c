static Background background_init(void) {
    Background background = {0};
    background.surface = gs->surfaces.background;
    return background;
}

static void background_draw(int target, Background *bg, int xoff, int yoff) {
    int w = bg->surface->w, h = bg->surface->h;

    int background = TEXTURE_BG_0;

    int level = gs->level_current+1;

    if (level >= 8 && level <= 10) {
        background = TEXTURE_BG_3;
    } else if (level >= 4 && level <= 7) {
        background = TEXTURE_BG_2;
    } else if (level == 11) {
        background = TEXTURE_BG_1;
    }

    if (background == 0) return;

    SDL_Rect dst = { xoff, yoff, w, h };
    RenderTexture(target, &GetTexture(background), null, &dst);
}
