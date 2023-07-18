//
// In this file, we load / unload every single asset from file,
// as well as every single global texture or surface created
// which persists across frames.
//
// This is not necessary for textures created inside functions
// which are freed in the same function.
//
// This is included only in the platform/SDL layer.
//

// Creates all render targets for all the levels.
static void render_targets_init(void) {
    int width = gs->desktop_w;
    int height = gs->desktop_h;

    for (int i = 0; i < RENDER_TARGET_COUNT; i++) {
        switch (i) {
            case RENDER_TARGET_MASTER: {
                gs->render.render_targets[i] = RenderMakeTarget(width, height, VIEW_STATE_SCREENSPACE, true);
                continue;
            }
            case RENDER_TARGET_CONVERSION_PANEL: case RENDER_TARGET_OUTRO:
            case RENDER_TARGET_GUI_TOOLBAR: case RENDER_TARGET_CHISEL: case RENDER_TARGET_GUI_CONVERSIONS:
            case RENDER_TARGET_TOOLTIP: {
                gs->render.render_targets[i] = RenderMakeTarget(width, height, VIEW_STATE_UNDEFINED, false);
                continue;
            }
            case RENDER_TARGET_HAMMER: case RENDER_TARGET_HAMMER2: {
                gs->render.render_targets[i] = RenderMakeTarget(gs->gw, gs->gh, VIEW_STATE_PIXELS, true);
				continue;
            }
            case RENDER_TARGET_3D: {
                gs->render.render_targets[i] = RenderMakeTargetEx(SCALE_3D*gs->window_width,
                                                                  SCALE_3D*gs->window_width,
                                                                  VIEW_STATE_SCREENSPACE,
                                                                  false,
                                                                  true);
                continue;
            }
            case RENDER_TARGET_PREVIEW: {
                gs->render.render_targets[i] = RenderMakeTarget(gs->gw, gs->gh, VIEW_STATE_PIXELS, false);
                continue;
            }
        }
        gs->render.render_targets[i] = RenderMakeTarget(gs->gw, gs->gh, VIEW_STATE_PIXELS, true);
    }
}

static void textures_init(Textures *textures) {
    SDL_Surface *surf = null;

    memset(textures, 0, sizeof(Textures));

    // Converter Item Textures || previously item_init()
    for (int i = 0; i < CELL_TYPE_COUNT; i++) {
        if (i == CELL_NONE) continue;

        char file[64] = {0};
        get_filename_from_type(i, file);

        surf = IMG_Load(file);
        Assert(surf);

        GetTexture(TEXTURE_ITEMS+i) = RenderCreateTextureFromSurface(surf);

        SDL_FreeSurface(surf);
        surf = null;
    }

    GetTexture(TEXTURE_CONFIRM_BUTTON) = RenderLoadTexture("buttons/confirm.png");
    GetTexture(TEXTURE_CANCEL_BUTTON)  = RenderLoadTexture("buttons/cancel.png");

    GetTexture(TEXTURE_TAB)        = RenderLoadTexture("tab.png");
    GetTexture(TEXTURE_DELETER)    = RenderLoadTexture("deleter.png");
    GetTexture(TEXTURE_PLACER)     = RenderLoadTexture("placer.png");
    GetTexture(TEXTURE_KNIFE)      = RenderLoadTexture("knife.png");
    GetTexture(TEXTURE_POPUP)      = RenderLoadTexture("popup.png");
    GetTexture(TEXTURE_TEXT_ARROW) = RenderLoadTexture("text_arrow.png");

    for (enum Tool_Type i = 0; i < TOOL_COUNT; i++) {
        char filename[128] = {0};
        char path[128] = {0};

        get_file_from_tool(i, filename);
        sprintf(path, "buttons/%s", filename);

        GetTexture(TEXTURE_TOOL_BUTTONS+i) = RenderLoadTexture(path);
        Assert(GetTexture(TEXTURE_TOOL_BUTTONS+i).handle);
    }

    GetTexture(TEXTURE_BLOB_HAMMER)= RenderLoadTexture("hammer.png");
    GetTexture(TEXTURE_CONVERTER_ARROW) = RenderLoadTexture("arrow.png");
    GetTexture(TEXTURE_CONVERT_BUTTON) = RenderLoadTexture("buttons/convert.png");
    GetTexture(TEXTURE_ALTERNATE_BUTTON) = RenderLoadTexture("buttons/alternate.png");
    GetTexture(TEXTURE_OK_BUTTON) = RenderLoadTexture("buttons/tutorial_ok.png");

    const char *chisel_files[] = {
        "chisel_small",
        "chisel_medium",
        "chisel_large",
    };
    GetTexture(TEXTURE_CHISEL_HAMMER) = RenderLoadTexture("hammer.png");

    // Loop through all chisels
    for (int i = 0; i < 3; i++) {
        // Alternate through face mode
        for (int face = 1; face != -1; face--) {
            char file[512] = {0};
            strcpy(file, chisel_files[i]);

            if (face)
                strcat(file, "_face");

            strcat(file, ".png");

            GetTexture(TEXTURE_CHISEL+i) = RenderLoadTexture(file);
        }
    }
}

static void surfaces_init(Surfaces *surfaces) {
    surfaces->a = null;
    surfaces->renderer_3d = SDL_CreateRGBSurfaceWithFormat(0, gs->desktop_w, gs->desktop_h, 32, ALASKA_PIXELFORMAT);
    surfaces->bark_surface = RenderLoadSurface("bark.png");
    surfaces->glass_surface = RenderLoadSurface("glass.png");
    surfaces->wood_plank_surface = RenderLoadSurface("plank.png");
    surfaces->marble_surface = RenderLoadSurface("marble.png");
    surfaces->granite_surface = RenderLoadSurface("granite.png");
    surfaces->diamond_surface = RenderLoadSurface("diamond.png");
    surfaces->ice_surface = RenderLoadSurface("ice.png");
    surfaces->grass_surface = RenderLoadSurface("grass.png");

    // TODO: Hack. 64,64 should be gs->gw, gs->gh, but they're not
    //       initialized as yet here.
    //       This shouldn't belong here. It should be re-created
    //       in goto_level
    surfaces->background = SDL_CreateRGBSurfaceWithFormat(0,
                                                          64*2,
                                                          64*2,
                                                          32,
                                                          ALASKA_PIXELFORMAT);

    SDL_FreeSurface(gs->pixel_format_surf);
    gs->pixel_format_surf = null;
}

static void fonts_init(Fonts *fonts) {
    fonts->font          = RenderLoadFont("Courier Prime.ttf", Scale(font_sizes[0]));
    fonts->font_times    = RenderLoadFont("EBGaramond-Medium.ttf", Scale(font_sizes[1]));
    fonts->font_consolas = RenderLoadFont("consola.ttf", Scale(font_sizes[2]));
    fonts->font_courier  = RenderLoadFont("LiberationMono-Regular.ttf", Scale(font_sizes[3]));
    fonts->font_small    = RenderLoadFont("consola.ttf", Scale(font_sizes[4]));
    fonts->font_bold_small = RenderLoadFont("courbd.ttf", Scale(font_sizes[5]));
    fonts->font_title    = RenderLoadFont("EBGaramond-Medium.ttf", Scale(font_sizes[6]));
    fonts->font_title_2  = RenderLoadFont("EBGaramond-Medium.ttf", Scale(font_sizes[7]));
    fonts->font_titlescreen = RenderLoadFont("EBGaramond-Medium.ttf", Scale(font_sizes[8]));
    fonts->font_converter_gui = RenderLoadFont("consola.ttf", Scale(font_sizes[9]));

    for (size_t i = 0; i < FONT_COUNT; i++) {
        TTF_SetFontHinting(fonts->fonts[i]->handle, TTF_HINTING_LIGHT_SUBPIXEL);
    }
}

static void fonts_deinit(Fonts *fonts) {
    for (size_t i = 0; i < FONT_COUNT; i++) {
        TTF_CloseFont(fonts->fonts[i]->handle);
    }
}

static void audio_init(Audio *audio) {
    audio->music_titlescreen = Mix_LoadMUS(RES_DIR "audio/titlescreen.ogg");
    audio->music_creation = Mix_LoadMUS(RES_DIR "audio/music_creation.ogg");
    audio->ambience1  = Mix_LoadMUS(RES_DIR "audio/ambience1.ogg");
    audio->music_rain = Mix_LoadMUS(RES_DIR "audio/rain.ogg");

    audio->pip = Mix_LoadWAV(RES_DIR "audio/pip.ogg");

    for (int i = 0; i < 6; i++) {
        char name[64];
        sprintf(name, RES_DIR "audio/chisel_%d.wav", i+1);
        audio->medium_chisel[i] = Mix_LoadWAV(name);
        Assert(audio->medium_chisel[i]);
    }
    audio->small_chisel = Mix_LoadWAV(RES_DIR "audio/small_chisel.wav");
    audio->large_chisel = Mix_LoadWAV(RES_DIR "audio/large_chisel.wav");

    audio->sprinkle = Mix_LoadWAV(RES_DIR "audio/sprinkle.ogg");
    audio->macabre = Mix_LoadWAV(RES_DIR "audio/macabre.ogg");

    audio->accept = Mix_LoadWAV(RES_DIR "audio/accept.ogg");

    Mix_Volume(AUDIO_CHANNEL_CHISEL, AUDIO_CHISEL_VOLUME);
    Mix_Volume(AUDIO_CHANNEL_GUI, AUDIO_GUI_VOLUME);
}

static void audio_deinit(Audio *audio) {
    Mix_FreeMusic(audio->music_titlescreen);
    Mix_FreeMusic(audio->music_creation);
    Mix_FreeMusic(audio->ambience1);
    Mix_FreeMusic(audio->music_rain);

    for (int i = 0; i < 6; i++)
        Mix_FreeChunk(audio->medium_chisel[i]);
    Mix_FreeChunk(audio->small_chisel);
    Mix_FreeChunk(audio->pip);
    Mix_FreeChunk(audio->large_chisel);

    Mix_FreeChunk(audio->sprinkle);
    Mix_FreeChunk(audio->macabre);
}
