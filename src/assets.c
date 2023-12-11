///
// In this file, we load / unload every single asset from file,
// as well as every single global texture or surface created
// which persists across frames.
//
// This is not necessary for textures created inside functions
// which are freed in the same function.
//
// This is included only in the platform/SDL layer.
//


//~ Textures and Surfaces

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
            case RENDER_TARGET_GRID: {
                gs->render.render_targets[i] = RenderMakeTarget(64, 64, VIEW_STATE_PIXELS, false);
                continue;
            }
            case RENDER_TARGET_CONVERSION_PANEL: case RENDER_TARGET_OUTRO:
            case RENDER_TARGET_GUI_TOOLBAR: case RENDER_TARGET_GUI_CONVERSIONS:
            case RENDER_TARGET_TOOLTIP: {
                gs->render.render_targets[i] = RenderMakeTarget(width, height, VIEW_STATE_UNDEFINED, false);
                continue;
            }
            case RENDER_TARGET_HAMMER: case RENDER_TARGET_HAMMER2: {
                gs->render.render_targets[i] = RenderMakeTarget(gs->gw, gs->gh, VIEW_STATE_PIXELS, true);
                continue;
            }
            case RENDER_TARGET_GLOW: case RENDER_TARGET_SHADOWS: {
                gs->render.render_targets[i] = RenderMakeTargetEx(gs->gw, gs->gh, VIEW_STATE_PIXELS, false, true);
                continue;
            }
            case RENDER_TARGET_PREVIEW: {
                gs->render.render_targets[i] = RenderMakeTarget(PREVIEW_GRID_W, PREVIEW_GRID_W, VIEW_STATE_PIXELS, false);
                continue;
            }
        }
        gs->render.render_targets[i] = RenderMakeTarget(gs->gw, gs->gh, VIEW_STATE_PIXELS, true);
    }
}

static void textures_init(Textures *textures) {
    memset(textures, 0, sizeof(Textures));

    // Converter Item Textures || previously item_init()
    for (int i = 0; i < CELL_TYPE_COUNT; i++) {
        if (i == CELL_NONE) continue;

        char file[64] = {0};
        get_filename_from_type(i, file);

        SDL_Surface *surf = IMG_Load(file);
        Assert(surf);

        GetTexture(TEXTURE_ITEMS+i) = RenderCreateTextureFromSurface(surf);

        Texture *t = &GetTexture(TEXTURE_ITEMS+i);
        char name[64];
        get_name_from_type(i, name);
        Log("Texture \"%s\": Handle: %p, Width: %d, Height: %d\n",
            name,
            t->handle,
            t->width,
            t->height);

        SDL_FreeSurface(surf);
    }

    GetTexture(TEXTURE_CONFIRM_BUTTON) = RenderLoadTexture("buttons/confirm.png");
    GetTexture(TEXTURE_CONFIRM_X_BUTTON) = RenderLoadTexture("buttons/confirm_x.png");
    GetTexture(TEXTURE_CANCEL_BUTTON)  = RenderLoadTexture("buttons/cancel.png");

    GetTexture(TEXTURE_TAB)   = RenderLoadTexture("tab.png");
    GetTexture(TEXTURE_SPEAKER)   = RenderLoadTexture("speaker.png");

    GetTexture(TEXTURE_PLANK) = RenderLoadTexture("plank.png");

    GetTexture(TEXTURE_W_KEY) = RenderLoadTexture("buttons/W.png");
    GetTexture(TEXTURE_A_KEY) = RenderLoadTexture("buttons/A.png");
    GetTexture(TEXTURE_S_KEY) = RenderLoadTexture("buttons/S.png");
    GetTexture(TEXTURE_D_KEY) = RenderLoadTexture("buttons/D.png");

    GetTexture(TEXTURE_CURSOR) = RenderLoadTexture("cursor.png");

    GetTexture(TEXTURE_CONVERTER_BG) = RenderLoadTexture("cbg.png");

    for (Tool_Type i = 0; i < TOOL_COUNT; i++) {
        char filename[128] = {0};
        char path[128] = {0};

        get_file_from_tool(i, filename);
        sprintf(path, "buttons/%s", filename);

        GetTexture(TEXTURE_TOOL_BUTTONS+i) = RenderLoadTexture(path);
        Assert(GetTexture(TEXTURE_TOOL_BUTTONS+i).handle);
    }

    GetTexture(TEXTURE_CONVERT_BUTTON) = RenderLoadTexture("buttons/convert.png");
    GetTexture(TEXTURE_ALTERNATE_BUTTON) = RenderLoadTexture("buttons/alternate.png");
    GetTexture(TEXTURE_RECIPE_BOOK_BUTTON) = RenderLoadTexture("buttons/recipebook.png");
    GetTexture(TEXTURE_OK_BUTTON) = RenderLoadTexture("buttons/tutorial_ok.png");
    GetTexture(TEXTURE_CHISEL_HAMMER) = RenderLoadTexture("hammer.png");

    textures_load_backgrounds(textures, true);

    struct File_To_Index {
        const char *filename;
        int index;
    };

    struct File_To_Index chisel_files[] = {
        { "chisel_small.png",           TEXTURE_CHISEL_SMALL },
        { "chisel_small_diagonal.png",  TEXTURE_CHISEL_SMALL_DIAGONAL },

        { "chisel_medium.png",          TEXTURE_CHISEL_MEDIUM },
        { "chisel_medium_diagonal.png", TEXTURE_CHISEL_MEDIUM_DIAGONAL },

        { "chisel_large.png",           TEXTURE_CHISEL_LARGE },
        { "chisel_large_diagonal.png",  TEXTURE_CHISEL_LARGE_DIAGONAL },
    };

    // Loop through all chisels
    for (int i = 0; i < ArrayCount(chisel_files); i++) {
        GetTexture(chisel_files[i].index) = RenderLoadTexture(chisel_files[i].filename);
    }
}

static void surfaces_init(Surfaces *surfaces) {
    surfaces->a = null;
    surfaces->renderer_3d        = SDL_CreateRGBSurfaceWithFormat(0, gs->desktop_w, gs->desktop_h, 32, ALASKA_PIXELFORMAT);
    surfaces->glass_surface      = RenderLoadSurface("glass.png");
    surfaces->wood_plank_surface = RenderLoadSurface("plank.png");
    surfaces->marble_surface     = RenderLoadSurface("marble.png");
    surfaces->granite_surface    = RenderLoadSurface("granite.png");
    surfaces->diamond_surface    = RenderLoadSurface("diamond.png");
    surfaces->ice_surface        = RenderLoadSurface("ice.png");

    // Hardcoded
    surfaces->background = SDL_CreateRGBSurfaceWithFormat(0,
                                                          128,
                                                          96,
                                                          32,
                                                          ALASKA_PIXELFORMAT);

    SDL_FreeSurface(gs->pixel_format_surf);
    gs->pixel_format_surf = null;
}

static void surfaces_deinit(Surfaces *surfaces) {
    for (int i = 0; i < SURFACE_COUNT; i++) {
        SDL_FreeSurface(surfaces->surfaces[i]);
    }
}

//~ Fonts

static void fonts_init(Fonts *fonts) {
    fonts->font               = RenderLoadFont("Courier Prime.ttf", Scale(font_sizes[0]));
    fonts->font_times         = RenderLoadFont("EBGaramond-Medium.ttf", Scale(font_sizes[1]));
    fonts->font_courier       = RenderLoadFont("LiberationMono-Regular.ttf", Scale(font_sizes[2]));
    fonts->font_small         = RenderLoadFont("consola.ttf", Scale(font_sizes[3]));
    fonts->font_bold_small    = RenderLoadFont("courbd.ttf", Scale(font_sizes[4]));
    fonts->font_title         = RenderLoadFont("EBGaramond-Medium.ttf", Scale(font_sizes[5]));
    fonts->font_title_2       = RenderLoadFont("EBGaramond-Medium.ttf", Scale(font_sizes[6]));
    fonts->font_titlescreen   = RenderLoadFont("EBGaramond-Medium.ttf", Scale(font_sizes[7]));
    fonts->font_converter_gui = RenderLoadFont("consola.ttf", Scale(font_sizes[8]));
    fonts->font_times_small   = RenderLoadFont("EBGaramond-Medium.ttf", Scale(font_sizes[9]));

    for (size_t i = 0; i < FONT_COUNT; i++) {
        TTF_SetFontHinting(fonts->fonts[i]->handle, TTF_HINTING_LIGHT_SUBPIXEL);
    }
}

static void fonts_deinit(Fonts *fonts) {
    for (size_t i = 0; i < FONT_COUNT; i++) {
        TTF_CloseFont(fonts->fonts[i]->handle);
    }
}

//~ Audio

static Sound load_sound(const char *relative_filepath, f32 volume) {
    Sound result = {0};

    char file[1024];
    sprintf(file, "%s%s", DATA_DIR, relative_filepath);

    result.sound = Mix_LoadWAV(file);
    Assert(result.sound);

    volume = clamp64(volume, 0, 1);
    result.volume = Volume(volume);

    Mix_VolumeChunk(result.sound, result.volume);

    return result;
}

static void free_sound(Sound *sound) {
    Mix_FreeChunk(sound->sound);
    sound->volume = -1;
}

static void audio_init(Audio *audio) {
    audio->music_titlescreen = Mix_LoadMUS(DATA_DIR "audio/music_titlescreen.ogg");

    f64 scale = 1.5;

    audio->ambience1     = load_sound("audio/ambience_snow.ogg", 9.0/20.0);
    audio->ambience_wind = load_sound("audio/ambience_wind.ogg", 0.8);
    audio->ambience_rain = load_sound("audio/rain.ogg", 1);
    audio->ambience_rain_reversed = load_sound("audio/rain_reversed.ogg", 1);
    audio->music0        = load_sound("audio/music0.ogg",    1);
    audio->music1        = load_sound("audio/music_photograph.ogg", 0.45);
    audio->music2        = load_sound("audio/music_frontier.ogg", 1);
    audio->music3        = load_sound("audio/music_weird.ogg", 0.8);
    audio->music4        = load_sound("audio/music_max's_lullaby.ogg", 0.70);
    audio->undo          = load_sound("audio/undo.wav", scale*0.25);

    audio->place = load_sound("audio/place.ogg", scale*0.75);
    audio->suck  = load_sound("audio/tap.ogg", 1);
    audio->ping  = load_sound("audio/ping.ogg", 0.7);
    audio->destroy = load_sound("audio/destroy.ogg", 1);
    audio->converter_material = load_sound("audio/converter_material.ogg", 0.20);

    audio->pip = load_sound("audio/pip.ogg", scale*1);

    f64 chisel_volume = 0.75;

    for (int i = 0; i < 6; i++) {
        char name[64];
        sprintf(name, "audio/chisel_%d.wav", i+1);
        audio->medium_chisel[i] = load_sound(name, chisel_volume);
    }
    audio->small_chisel = load_sound("audio/small_chisel.wav", 1);

    for (int i = 0; i < ArrayCount(audio->ice_chisel); i++) {
        char name[64];
        sprintf(name, "audio/ice_chisel_%d.wav", i+1);
        audio->ice_chisel[i] = load_sound(name, chisel_volume);
    }

    for (int i = 0; i < ArrayCount(audio->glass_chisel); i++) {
        char name[64];
        sprintf(name, "audio/glass_chisel_%d.ogg", i+1);

        if (i == 1) strcpy(name, "audio/glass_chisel_2.wav");

        audio->glass_chisel[i] = load_sound(name, chisel_volume);
    }

    for (int i = 0; i < ArrayCount(audio->small_glass_chisel); i++) {
        char name[64];
        sprintf(name, "audio/glass_small_%d.ogg", i+1);
        audio->small_glass_chisel[i] = load_sound(name, scale*1);
    }

    audio->sprinkle = load_sound("audio/win.ogg", 1);
    audio->macabre = load_sound("audio/macabre.ogg", 1);

    audio->accept = load_sound("audio/accept.ogg", scale*0.75);

    assign_channel_volumes(&gs->pause_menu, &gs->audio_handler);
}

// Do we really have to do this? Aren't all resources freed on exit?
static void audio_deinit(Audio *audio) {
    Mix_FreeMusic(audio->music_titlescreen);

    u8 *pointer = (u8*)audio;
    pointer += sizeof(Mix_Music*); // Skip the first field, which is a pointer, and get to the Sounds.

    Sound *sounds = (Sound*)pointer;

    int sound_count = (sizeof(Audio) - sizeof(Mix_Music*)) / sizeof(Sound);

    for (int i = 0; i < sound_count; i++) {
        free_sound(&sounds[i]);
    }
}
