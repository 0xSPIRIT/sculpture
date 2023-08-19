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

void get_file_from_tool(int type, char *out);
void get_filename_from_type(int type, char *out);

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
    GetTexture(TEXTURE_CONFIRM_X_BUTTON) = RenderLoadTexture("buttons/confirm_x.png");
    GetTexture(TEXTURE_CANCEL_BUTTON)  = RenderLoadTexture("buttons/cancel.png");
    
    GetTexture(TEXTURE_TAB)        = RenderLoadTexture("tab.png");
    GetTexture(TEXTURE_DELETER)    = RenderLoadTexture("deleter.png");
    GetTexture(TEXTURE_PLACER)     = RenderLoadTexture("placer.png");
    GetTexture(TEXTURE_KNIFE)      = RenderLoadTexture("knife.png");
    GetTexture(TEXTURE_POPUP)      = RenderLoadTexture("popup.png");
    GetTexture(TEXTURE_TEXT_ARROW) = RenderLoadTexture("text_arrow.png");
    
    GetTexture(TEXTURE_PLANK) = RenderLoadTexture("plank.png");
    
    GetTexture(TEXTURE_W_KEY) = RenderLoadTexture("buttons/W.png");
    GetTexture(TEXTURE_A_KEY) = RenderLoadTexture("buttons/A.png");
    GetTexture(TEXTURE_S_KEY) = RenderLoadTexture("buttons/S.png");
    GetTexture(TEXTURE_D_KEY) = RenderLoadTexture("buttons/D.png");
    
    for (Tool_Type i = 0; i < TOOL_COUNT; i++) {
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
    GetTexture(TEXTURE_RECIPE_BOOK_BUTTON) = RenderLoadTexture("buttons/recipebook.png");
    GetTexture(TEXTURE_OK_BUTTON) = RenderLoadTexture("buttons/tutorial_ok.png");
    GetTexture(TEXTURE_CHISEL_HAMMER) = RenderLoadTexture("hammer.png");
    
    GetTexture(TEXTURE_TEST) = RenderLoadTexture("test.png");
    
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
    surfaces->renderer_3d = SDL_CreateRGBSurfaceWithFormat(0, gs->desktop_w, gs->desktop_h, 32, ALASKA_PIXELFORMAT);
    surfaces->bark_surface = RenderLoadSurface("bark.png");
    surfaces->glass_surface = RenderLoadSurface("glass.png");
    surfaces->wood_plank_surface = RenderLoadSurface("plank.png");
    surfaces->marble_surface = RenderLoadSurface("marble.png");
    surfaces->granite_surface = RenderLoadSurface("granite.png");
    surfaces->diamond_surface = RenderLoadSurface("diamond.png");
    surfaces->ice_surface = RenderLoadSurface("ice.png");
    surfaces->grass_surface = RenderLoadSurface("grass.png");
    
    // Hack
    surfaces->background = SDL_CreateRGBSurfaceWithFormat(0,
                                                          64*2,
                                                          64*2,
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
    audio->music_titlescreen = Mix_LoadMUS(RES_DIR "audio/titlescreen.ogg");

    audio->ambience1     = load_sound(RES_DIR "audio/ambience1.ogg", 0.32);
    audio->ambience_rain = load_sound(RES_DIR "audio/rain.ogg",      0.16);
    audio->music0        = load_sound(RES_DIR "audio/music0.ogg",    0.55);
    //audio->music1        = load_sound(RES_DIR "audio/music1.ogg",    0.50);
    audio->music2        = load_sound(RES_DIR "audio/music2.ogg",    0.50);

    audio->pip = load_sound(RES_DIR "audio/pip.ogg", 1);

    for (int i = 0; i < 6; i++) {
        char name[64];
        sprintf(name, RES_DIR "audio/chisel_%d.wav", i+1);
        audio->medium_chisel[i] = load_sound(name, 1);
    }
    audio->small_chisel = load_sound(RES_DIR "audio/small_chisel.wav", 1);

    for (int i = 0; i < ArrayCount(audio->ice_chisel); i++) {
        char name[64];
        sprintf(name, RES_DIR "audio/ice_chisel_%d.wav", i+1);
        audio->ice_chisel[i] = load_sound(name, 1);
    }

    audio->sprinkle = load_sound(RES_DIR "audio/sprinkle 3.wav", 1);
    audio->macabre = load_sound(RES_DIR "audio/macabre.ogg", 1);

    audio->accept = load_sound(RES_DIR "audio/accept.ogg", 1);

    audio_setup_initial_channel_volumes();
}

static void audio_deinit(Audio *audio) {
    Mix_FreeMusic(audio->music_titlescreen);

    free_sound(&audio->ambience1);
    free_sound(&audio->ambience_rain);
    free_sound(&audio->music0);
    //free_sound(&audio->music1);
    free_sound(&audio->music2);

    for (int i = 0; i < 6; i++)
        free_sound(&audio->medium_chisel[i]);
    for (int i = 0; i < ArrayCount(audio->ice_chisel); i++)
        free_sound(&audio->ice_chisel[i]);
    free_sound(&audio->small_chisel);
    free_sound(&audio->pip);

    free_sound(&audio->sprinkle);
    free_sound(&audio->macabre);
}
