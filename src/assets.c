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

#define CreateRenderTarget(width, height) (SDL_CreateTexture(renderer, ALASKA_PIXELFORMAT, SDL_TEXTUREACCESS_TARGET, width, height))

SDL_Surface *pixel_format_surf = NULL;

SDL_Texture *load_texture(SDL_Renderer *renderer, const char *fp) {
    // A temp surface that exists solely to get its format
    // if surfaces are loaded with a format != ALASKA_PIXELFORMAT.
    if (!pixel_format_surf) {
        pixel_format_surf = SDL_CreateRGBSurfaceWithFormat(0, 16, 16, 32,
                                                           ALASKA_PIXELFORMAT);
    }
    
    SDL_Surface *surf = IMG_Load(fp);
    Assert(surf);
    
    if (surf->format->format != ALASKA_PIXELFORMAT) {
        SDL_Surface *new_surf = SDL_ConvertSurface(surf,
                                                   pixel_format_surf->format,
                                                   0);
        SDL_FreeSurface(surf);
        surf = new_surf;
    }
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    Assert(texture);
    SDL_FreeSurface(surf);
    return texture;
}

// Creates all render targets for all the levels.
void render_targets_init(SDL_Renderer *renderer,
                         Level *levels) {
    int width = gs->desktop_w;
    int height = gs->desktop_h;
    
    for (int lvl = 0; lvl < LEVEL_COUNT; lvl++) {
        Level *l = &levels[lvl];
        Assert(l->w != 0 && l->h != 0);
        
        for (int i = 0; i < RENDER_TARGET_COUNT; i++) {
            switch (i) {
                case RENDER_TARGET_MASTER: {
                    RenderTargetLvl(lvl, i) = CreateRenderTarget(width, height);
                    continue;
                }
                case RENDER_TARGET_CONVERSION_PANEL: case RENDER_TARGET_OUTRO: {
                    RenderTargetLvl(lvl, i) = CreateRenderTarget(width, height);
                    SDL_SetTextureBlendMode(RenderTargetLvl(lvl, i), SDL_BLENDMODE_BLEND);
                    continue;
                }
                case RENDER_TARGET_GUI_TOOLBAR: {
                    RenderTargetLvl(lvl, i) = CreateRenderTarget(width, height);
                    Assert(RenderTargetLvl(lvl, i));
                    continue;
                }
                case RENDER_TARGET_3D: {
                    RenderTargetLvl(lvl, i) = SDL_CreateTexture(renderer,
                                                                ALASKA_PIXELFORMAT,
                                                                SDL_TEXTUREACCESS_STREAMING,
                                                                SCALE_3D*gs->window_width,
                                                                SCALE_3D*gs->window_height);
                    SDL_SetTextureBlendMode(RenderTargetLvl(lvl, i), SDL_BLENDMODE_BLEND);
                    Assert(RenderTargetLvl(lvl, i));
                    continue;
                }
                case RENDER_TARGET_HAMMER: {
                    RenderTargetLvl(lvl, i) = CreateRenderTarget(width, height);
                    SDL_SetTextureBlendMode(RenderTargetLvl(lvl, i), SDL_BLENDMODE_BLEND);
                    continue;
                }
                case RENDER_TARGET_HAMMER2: {
                    RenderTargetLvl(lvl, i) = CreateRenderTarget(width, height);
                    SDL_SetTextureBlendMode(RenderTargetLvl(lvl, i), SDL_BLENDMODE_BLEND);
                    continue;
                }
            }
            
            RenderTargetLvl(lvl, i) = CreateRenderTarget(l->w, l->h);
            Assert(RenderTargetLvl(lvl, i));
        }
    }
}

void textures_init(SDL_Renderer *renderer, Textures *textures) {
    SDL_Surface *surf = NULL;
    
    memset(textures, 0, sizeof(Textures));
    
    // Converter Item Textures || previously item_init()
    for (int i = 0; i < CELL_TYPE_COUNT; i++) {
        if (i == CELL_NONE) continue;
        
        char file[64] = {0};
        get_filename_from_type(i, file);
        
        surf = IMG_Load(file);
        Assert(surf);
        
        Texture(TEXTURE_ITEMS+i) = SDL_CreateTextureFromSurface(renderer, surf);
        Assert(Texture(TEXTURE_ITEMS+i));
        
        SDL_FreeSurface(surf);
        surf = NULL;
    }
    
    Texture(TEXTURE_TAB)        = load_texture(renderer, RES_DIR "tab.png");
    Texture(TEXTURE_DELETER)    = load_texture(renderer, RES_DIR "deleter.png");
    Texture(TEXTURE_PLACER)     = load_texture(renderer, RES_DIR "placer.png");
    Texture(TEXTURE_KNIFE)      = load_texture(renderer, RES_DIR "knife.png");
    Texture(TEXTURE_POPUP)      = load_texture(renderer, RES_DIR "popup.png");
    Texture(TEXTURE_TEXT_ARROW) = load_texture(renderer, RES_DIR "text_arrow.png");
    
    //textures->level_backgrounds[0] = load_texture(renderer, RES_DIR "bg0.png");
    
    for (enum Tool_Type i = 0; i < TOOL_COUNT; i++) {
        char filename[128] = {0};
        char path[128] = {0};
        
        get_file_from_tool(i, filename);
        sprintf(path, RES_DIR "buttons/%s", filename);
        
        Texture(TEXTURE_TOOL_BUTTONS+i) = load_texture(renderer, path);
        Assert(Texture(TEXTURE_TOOL_BUTTONS+i));
    }
    
    Texture(TEXTURE_BLOB_HAMMER)= load_texture(renderer, RES_DIR "hammer.png");
    Texture(TEXTURE_CONVERTER_ARROW) = load_texture(renderer, RES_DIR "arrow.png");
    Texture(TEXTURE_CONVERT_BUTTON) = load_texture(renderer, RES_DIR "buttons/convert.png");
    Texture(TEXTURE_OK_BUTTON) = load_texture(renderer, RES_DIR "buttons/tutorial_ok.png");
    
    const char *chisel_files[] = {
        RES_DIR "chisel_small",
        RES_DIR "chisel_medium",
        RES_DIR "chisel_large",
    };
    Texture(TEXTURE_CHISEL_HAMMER) = load_texture(renderer, RES_DIR "hammer.png");
    
    // Loop through all chisels
    for (int i = 0; i < 3; i++) {
        // Alternate through face mode
        for (int face = 1; face != -1; face--) {
            char file[512] = {0};
            strcpy(file, chisel_files[i]);
            
            if (face)
                strcat(file, "_face");
            
            strcat(file, ".png");
            
            Texture(TEXTURE_CHISEL+i) = load_texture(renderer, file);
            Assert(Texture(TEXTURE_CHISEL+i));
        }
    }
    
    SDL_FreeSurface(pixel_format_surf);
}

void textures_deinit(Textures *textures) {
    for (int i = 0; i < TEXTURE_COUNT; i++)
        if (textures->texs[i]) SDL_DestroyTexture(textures->texs[i]);
}

void surfaces_init(Surfaces *surfaces) {
    surfaces->a = NULL;
    surfaces->renderer_3d = SDL_CreateRGBSurfaceWithFormat(0, gs->desktop_w, gs->desktop_h, 32, ALASKA_PIXELFORMAT);
    surfaces->bark_surface = IMG_Load(RES_DIR "bark.png");
    surfaces->glass_surface = IMG_Load(RES_DIR "glass.png");
    surfaces->wood_plank_surface = IMG_Load(RES_DIR "plank.png");
    surfaces->marble_surface = IMG_Load(RES_DIR "marble.png");
    surfaces->granite_surface = IMG_Load(RES_DIR "granite.png");
    surfaces->diamond_surface = IMG_Load(RES_DIR "diamond.png");
    surfaces->ice_surface = IMG_Load(RES_DIR "ice.png");
    surfaces->grass_surface = IMG_Load(RES_DIR "grass.png");
    surfaces->triangle_blob_surface = IMG_Load(RES_DIR "triangle_blob.png");
}

void surfaces_deinit(Surfaces *surfaces) {
    SDL_Surface **surfs = (SDL_Surface**) surfaces;
    size_t surf_count = sizeof(Surfaces)/sizeof(SDL_Surface*);
    
    for (size_t i = 0; i < surf_count; i++) {
        if (surfs[i])
            SDL_FreeSurface(surfs[i]);
    }
}

void fonts_init(Fonts *fonts) {
    // THESE MUST BE IN ORDER IN THE FONT STRUCT
    fonts->font          = TTF_OpenFont(RES_DIR "Courier Prime.ttf", Scale(font_sizes[0]));
    fonts->font_times    = TTF_OpenFont(RES_DIR "EBGaramond-Medium.ttf", Scale(font_sizes[1]));
    fonts->font_consolas = TTF_OpenFont(RES_DIR "consola.ttf", Scale(font_sizes[2]));
    fonts->font_courier  = TTF_OpenFont(RES_DIR "LiberationMono-Regular.ttf", Scale(font_sizes[3]));
    fonts->font_small    = TTF_OpenFont(RES_DIR "consola.ttf", Scale(font_sizes[4]));
    fonts->font_bold_small = TTF_OpenFont(RES_DIR "courbd.ttf", Scale(font_sizes[5]));
    fonts->font_title    = TTF_OpenFont(RES_DIR "EBGaramond-Medium.ttf", Scale(font_sizes[6]));
    fonts->font_title_2  = TTF_OpenFont(RES_DIR "EBGaramond-Medium.ttf", Scale(font_sizes[7]));
    fonts->font_titlescreen = TTF_OpenFont(RES_DIR "EBGaramond-Medium.ttf", Scale(font_sizes[8]));
    
    TTF_Font **ttf_fonts = (TTF_Font**) fonts;
    size_t font_count = sizeof(Fonts)/sizeof(TTF_Font*);
    
    for (size_t i = 0; i < font_count; i++) {
        TTF_SetFontHinting(ttf_fonts[i], TTF_HINTING_LIGHT_SUBPIXEL);
    }
}

void fonts_deinit(Fonts *fonts) {
    TTF_Font **ttf_fonts = (TTF_Font**) fonts;
    size_t font_count = sizeof(Fonts)/sizeof(TTF_Font*);
    
    for (size_t i = 0; i < font_count; i++) {
        TTF_CloseFont(ttf_fonts[i]);
    }
}

void audio_init(Audio *audio) {
    audio->music_titlescreen = Mix_LoadMUS(RES_DIR "audio/titlescreen.ogg");
    audio->music_creation = Mix_LoadMUS(RES_DIR "audio/music_creation.ogg");
    
    Mix_VolumeMusic(3 * MIX_MAX_VOLUME / 4);
    
    audio->pip = Mix_LoadWAV(RES_DIR "audio/pip.ogg");
    
    for (int i = 0; i < 6; i++) {
        char name[64];
        sprintf(name, RES_DIR "audio/chisel_%d.wav", i+1);
        audio->medium_chisel[i] = Mix_LoadWAV(name);
        Assert(audio->medium_chisel[i]);
    }
    audio->small_chisel = Mix_LoadWAV(RES_DIR "audio/small_chisel.wav");
    audio->large_chisel = Mix_LoadWAV(RES_DIR "audio/large_chisel.wav");
    
    audio->stinger_a = Mix_LoadWAV(RES_DIR "audio/sprinkle.ogg");
    audio->stinger_b = Mix_LoadWAV(RES_DIR "audio/macabre.ogg");
    
    audio->accept = Mix_LoadWAV(RES_DIR "audio/accept.ogg");
    
    Mix_Volume(AUDIO_CHANNEL_CHISEL, Volume(0.25));
    Mix_Volume(AUDIO_CHANNEL_GUI, Volume(0.20));
}

void audio_deinit(Audio *audio) {
    Mix_FreeMusic(audio->music_titlescreen);
    Mix_FreeMusic(audio->music_creation);
    for (int i = 0; i < 6; i++)
        Mix_FreeChunk(audio->medium_chisel[i]);
    Mix_FreeChunk(audio->small_chisel);
    Mix_FreeChunk(audio->pip);
    Mix_FreeChunk(audio->large_chisel);
    
    Mix_FreeChunk(audio->stinger_a);
    Mix_FreeChunk(audio->stinger_b);
}