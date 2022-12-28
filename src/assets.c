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
                         int width, // In screen coords, not game coords.
                         struct Level *levels,
                         struct Textures *textures) {
    for (int lvl = 0; lvl < LEVEL_COUNT; lvl++) {
        struct Level *l = &levels[lvl];
        Assert(l->w != 0 && l->h != 0);
        
        for (int i = 0; i < RENDER_TARGET_COUNT; i++) {
            if (i == RENDER_TARGET_GUI_TOOLBAR) {
                textures->render_targets[lvl][i] = CreateRenderTarget(width, GUI_H);
                Assert(textures->render_targets[lvl][i]);
                continue;
            }
            if (i == RENDER_TARGET_3D) {
                textures->render_targets[lvl][i] = SDL_CreateTexture(renderer, ALASKA_PIXELFORMAT, SDL_TEXTUREACCESS_STREAMING, SCALE_3D*gs->window_width, SCALE_3D*(gs->window_height-GUI_H));
                SDL_SetTextureBlendMode(textures->render_targets[lvl][i], SDL_BLENDMODE_BLEND);
                Assert(textures->render_targets[lvl][i]);
                continue;
            }
            
            textures->render_targets[lvl][i] = CreateRenderTarget(l->w, l->h);
            Assert(textures->render_targets[lvl][i]);
        }
    }
}

void textures_init(SDL_Renderer *renderer, struct Textures *textures) {
    SDL_Surface *surf = NULL;
    
    memset(textures, 0, sizeof(struct Textures));
    
    // Converter Item Textures || previously item_init()
    for (int i = 0; i < CELL_TYPE_COUNT; i++) {
        if (i == CELL_NONE) continue;
        
        char file[64] = {0};
        get_filename_from_type(i, file);
        
        surf = IMG_Load(file);
        Assert(surf);
        
        textures->items[i] = SDL_CreateTextureFromSurface(renderer, surf);
        Assert(textures->items[i]);
        
        SDL_FreeSurface(surf);
        surf = NULL;
    }
    
    textures->tab = load_texture(renderer, RES_DIR "tab.png");
    textures->deleter = load_texture(renderer, RES_DIR "deleter.png");
    textures->placer = load_texture(renderer, RES_DIR "placer.png");
    textures->knife = load_texture(renderer, RES_DIR "knife.png");
    textures->popup = load_texture(renderer, RES_DIR "popup.png");
    
    for (enum Tool_Type i = 0; i < TOOL_COUNT; i++) {
        char filename[128] = {0};
        char path[128] = {0};
        
        get_file_from_tool(i, filename);
        sprintf(path, RES_DIR "buttons/%s", filename);
        
        textures->tool_buttons[i] = load_texture(renderer, path);
        Assert(textures->tool_buttons[i]);
    }
    
    textures->blob_hammer = load_texture(renderer, RES_DIR "hammer.png");
    textures->converter_arrow = load_texture(renderer, RES_DIR "arrow.png");
    textures->convert_button = load_texture(renderer, RES_DIR "buttons/convert.png");
    textures->tutorial_ok_button = load_texture(renderer, RES_DIR "buttons/tutorial_ok.png");
    
    const char *chisel_files[] = {
        RES_DIR "chisel_small",
        RES_DIR "chisel_medium",
        RES_DIR "chisel_large",
    };
    textures->chisel_hammer = load_texture(renderer, RES_DIR "hammer.png");
    
    // Loop through all chisels
    for (int i = 0; i < 3; i++) {
        // Alternate through face mode
        for (int face = 1; face != -1; face--) {
            char file[512] = {0};
            strcpy(file, chisel_files[i]);
            
            if (face)
                strcat(file, "_face");
            
            strcat(file, ".png");
            
            if (face) {
                textures->chisel_face[i] = load_texture(renderer, file);
                Assert(textures->chisel_face[i]);
            } else {
                textures->chisel_outside[i] = load_texture(renderer, file);
                Assert(textures->chisel_outside[i]);
            }
        }
    }
    
    SDL_FreeSurface(pixel_format_surf);
}

void textures_deinit(struct Textures *textures) {
    int *texs = (int*) textures;
    size_t tex_count = sizeof(struct Textures)/sizeof(SDL_Texture*);
    
    for (int i = 0; i < tex_count; i++) {
        if ((SDL_Texture*)(texs+i) != NULL) {
            SDL_DestroyTexture((SDL_Texture*) (texs+i));
        }
    }
}

void surfaces_init(struct Surfaces *surfaces) {
    surfaces->a = IMG_Load(RES_DIR "lvl/desired/level 10.png");
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

void surfaces_deinit(struct Surfaces *surfaces) {
    SDL_Surface **surfs = (SDL_Surface**) surfaces;
    size_t surf_count = sizeof(struct Surfaces)/sizeof(SDL_Surface*);
    
    for (size_t i = 0; i < surf_count; i++) {
        if (surfs[i])
            SDL_FreeSurface(surfs[i]);
    }
}

void fonts_init(struct Fonts *fonts) {
    fonts->font          = TTF_OpenFont(RES_DIR "LiberationMono-Regular.ttf", 19);
    fonts->font_consolas = TTF_OpenFont(RES_DIR "consola.ttf", 24);
    fonts->font_courier  = TTF_OpenFont(RES_DIR "LiberationMono-Regular.ttf", 20);
    fonts->font_small    = TTF_OpenFont(RES_DIR "consola.ttf", 16);
    fonts->font_title    = TTF_OpenFont(RES_DIR "LiberationMono-Regular.ttf", 45);
    fonts->font_bold_small = TTF_OpenFont(RES_DIR "courbd.ttf", 16);
    
    TTF_Font **ttf_fonts = (TTF_Font**) fonts;
    size_t font_count = sizeof(struct Fonts)/sizeof(TTF_Font*);
    
    for (size_t i = 0; i < font_count; i++) {
        TTF_SetFontHinting(ttf_fonts[i], TTF_HINTING_LIGHT_SUBPIXEL);
    }
}

void fonts_deinit(struct Fonts *fonts) {
    TTF_Font **ttf_fonts = (TTF_Font**) fonts;
    size_t font_count = sizeof(struct Fonts)/sizeof(TTF_Font*);
    
    for (size_t i = 0; i < font_count; i++) {
        TTF_CloseFont(ttf_fonts[i]);
    }
}

void audio_init(struct Audio *audio) {
    audio->music = Mix_LoadMUS(RES_DIR "audio/mus.mp3");
    for (int i = 0; i < 6; i++) {
        char name[64];
        sprintf(name, RES_DIR "audio/chisel_%d.wav", i+1);
        audio->medium_chisel[i] = Mix_LoadWAV(name);
        Assert(audio->medium_chisel[i]);
    }
    audio->small_chisel = Mix_LoadWAV(RES_DIR "audio/small_chisel.wav");
    audio->large_chisel = Mix_LoadWAV(RES_DIR "audio/large_chisel.wav");
    
    Assert(audio->music);
}

void audio_deinit(struct Audio *audio) {
    Mix_FreeMusic(audio->music);
    for (int i = 0; i < 6; i++)
        Mix_FreeChunk(audio->medium_chisel[i]);
    Mix_FreeChunk(audio->small_chisel);
    Mix_FreeChunk(audio->large_chisel);
}