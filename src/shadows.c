static void shadows_draw(int target) {
    Texture *texture = &RenderTarget(RENDER_TARGET_SHADOWS)->texture;
    u32 *pixels;
    int pitch;

    bool ok = (RenderLockTexture(texture, null, (void**)&pixels, &pitch) == 0);
    Assert(ok);

    memset(pixels, 0, pitch*gs->gh);

    for (int i = 0; i < gs->gw*gs->gh; i++) {
        u8 a, b, g, r;

        a = 255;
        b = 0;
        g = 0;
        r = 0;

        pixels[i] = ((u32) a << 24) | ((u32) b << 16) | ((u32) g << 8) | r;
    }

    RenderUnlockTexture(texture);

    SDL_Rect dst = {
        RenderTarget(target)->top_left.x,
        RenderTarget(target)->top_left.y,
        gs->gw, gs->gh
    };
    RenderTargetToTarget(target, RENDER_TARGET_SHADOWS, null, &dst);
}
