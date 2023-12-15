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

static void background_intro_init(Intro_Background *bg) {
    memset(bg, 0, sizeof(Intro_Background));
}

static void background_intro_draw(int target, Intro_Background *bg) {
    int w = gs->game_width;
    int h = gs->game_height;

    if (gs->should_update) {
        f32 speed = 1;

        bg->x -= speed;
        bg->y -= speed * (f32)h/w;

        if (bg->x <= -w) bg->x += 2*w;
        if (bg->y <= -h) bg->y += 2*h;
    }

    Texture t = GetTexture(TEXTURE_PSYCHEDELIC);

    const bool clamp_to_grid = false;

    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 3; x++) {
            SDL_Rect dst;
            dst.x = bg->x + (x-1) * w;
            dst.y = bg->y + (y-1) * h;
            dst.w = w;
            dst.h = h;

            if (clamp_to_grid) {
                f32 xx = (f32)dst.x / (f32)w;
                xx *= 64;
                dst.x = (int)xx;
                dst.x *= w/64;

                f32 yy = (f32)dst.y / (f32)h;
                yy *= 64;
                dst.y = (int)yy;
                dst.y *= h/64;
            }

            RenderTexture(target, &t, null, &dst);
        }
    }
}