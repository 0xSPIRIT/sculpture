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
                gs->render.render_targets[i] = RenderMakeTarget(gs->gw, gs->gh, VIEW_STATE_PIXELS, false);
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
            case RENDER_TARGET_3D: {
                gs->render.render_targets[i] = RenderMakeTargetEx(SCALE_3D*gs->game_width,
                                                                  SCALE_3D*gs->game_width,
                                                                  VIEW_STATE_SCREENSPACE,
                                                                  false,
                                                                  true);
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

        SDL_FreeSurface(surf);
    }

    GetTexture(TEXTURE_CONFIRM_BUTTON) = RenderLoadTexture("buttons/confirm.png");
    GetTexture(TEXTURE_CONFIRM_X_BUTTON) = RenderLoadTexture("buttons/confirm_x.png");
    GetTexture(TEXTURE_CANCEL_BUTTON)  = RenderLoadTexture("buttons/cancel.png");

    GetTexture(TEXTURE_TAB)   = RenderLoadTexture("tab.png");

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

    GetTexture(TEXTURE_BG_0) = RenderLoadTexture("bg_0.png");
    GetTexture(TEXTURE_BG_1) = RenderLoadTexture("bg_1.png");
    GetTexture(TEXTURE_BG_2) = RenderLoadTexture("bg_2.png");
    GetTexture(TEXTURE_BG_3) = RenderLoadTexture("bg_3.png");

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

static void audio_setup_initial_channel_volumes(void) {
    Mix_Volume(AUDIO_CHANNEL_CHISEL, AUDIO_CHISEL_VOLUME);
    Mix_Volume(AUDIO_CHANNEL_GUI,    AUDIO_GUI_VOLUME);
    Mix_Volume(AUDIO_CHANNEL_MUSIC,  AUDIO_MUSIC_VOLUME);
}

static Sound load_sound(const char *file, f32 volume) {
    Sound result = {0};

    result.sound = Mix_LoadWAV(file);
    Assert(result.sound);

    result.volume = Volume(volume);

    Mix_VolumeChunk(result.sound, result.volume);

    return result;
}

static void free_sound(Sound *sound) {
    Mix_FreeChunk(sound->sound);
    sound->volume = -1;
}

static void audio_init(Audio *audio) {
    audio->music_titlescreen = Mix_LoadMUS(DATA_DIR "audio/titlescreen.ogg");

    audio->ambience1     = load_sound(DATA_DIR "audio/ambience1.ogg", 0.32);
    audio->ambience_rain = load_sound(DATA_DIR "audio/rain.ogg",      0.16);
    audio->ambience_rain_reversed = load_sound(DATA_DIR "audio/rain_reversed.ogg", 0.16);
    audio->music0        = load_sound(DATA_DIR "audio/music0.ogg",    0.55);
    audio->music2        = load_sound(DATA_DIR "audio/music_companionless.ogg",    0.50);
    audio->undo          = load_sound(DATA_DIR "audio/undo.wav", 0.25);

    audio->place = load_sound(DATA_DIR "audio/place.ogg", 0.75);

    audio->pip = load_sound(DATA_DIR "audio/pip.ogg", 1);

    for (int i = 0; i < 6; i++) {
        char name[64];
        sprintf(name, DATA_DIR "audio/chisel_%d.wav", i+1);
        audio->medium_chisel[i] = load_sound(name, 1);
    }
    audio->small_chisel = load_sound(DATA_DIR "audio/small_chisel.wav", 1);

    for (int i = 0; i < ArrayCount(audio->ice_chisel); i++) {
        char name[64];
        sprintf(name, DATA_DIR "audio/ice_chisel_%d.wav", i+1);
        audio->ice_chisel[i] = load_sound(name, 1);
    }

    for (int i = 0; i < ArrayCount(audio->glass_chisel); i++) {
        char name[64];
        sprintf(name, DATA_DIR "audio/glass_chisel_%d.ogg", i+1);

        if (i == 1) strcpy(name, DATA_DIR "audio/glass_chisel_2.wav");

        audio->glass_chisel[i] = load_sound(name, 1);
    }

    for (int i = 0; i < ArrayCount(audio->small_glass_chisel); i++) {
        char name[64];
        sprintf(name, DATA_DIR "audio/glass_small_%d.ogg", i+1);
        audio->small_glass_chisel[i] = load_sound(name, 1);
    }

#if 0
    for (int i = 0; i < ArrayCount(audio->woosh); i++) {
        char name[64];
        sprintf(name, DATA_DIR "audio/woosh%d.ogg", i+1);
        audio->woosh[i] = load_sound(name, 0.125);
    }
#endif

    audio->sprinkle = load_sound(DATA_DIR "audio/sprinkle.wav", 1);
    audio->macabre = load_sound(DATA_DIR "audio/macabre.ogg", 1);

    audio->accept = load_sound(DATA_DIR "audio/accept.ogg", 1);

    audio_setup_initial_channel_volumes();
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
