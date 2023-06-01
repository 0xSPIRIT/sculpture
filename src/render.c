// A wrapper for calls to SDL's draw functions.
// We use this for things like global offsets,
// and to manage render targets better.

//~ Initializations and Creations

static Render RenderInit(SDL_Renderer *sdl_renderer) {
    Render result = {0};
    result.sdl = sdl_renderer;
    result.view = VIEW_STATE_SCREENSPACE;
    return result;
}

static SDL_Surface *RenderLoadSurface(const char *fp) {
    char filepath[8192] = {0};
    strcat(filepath, RES_DIR);
    strcat(filepath, fp);
    SDL_Surface *surf = IMG_Load(filepath);
    Assert(surf);
    return surf;
}

static Texture *RenderLoadTexture(const char *fp) {
    Texture *texture = PushSize(gs->persistent_memory, sizeof(Texture));
    SDL_Surface *surf = RenderLoadSurface(fp);
    Assert(surf);
    
    texture->width = surf->w;
    texture->height = surf->h;
    texture->handle = SDL_CreateTextureFromSurface(gs->render.sdl, surf);
    Assert(texture->handle);
    
    return texture;
}

static Texture *RenderCreateTextureFromSurface(SDL_Surface *surf) {
    Texture *texture = PushSize(gs->persistent_memory, sizeof(Texture));
    texture->handle = SDL_CreateTextureFromSurface(gs->render.sdl, surf);
    Assert(texture->handle);
    return texture;
}

static Font *RenderLoadFont(const char *fp, int size) {
    Font *font = PushSize(gs->persistent_memory, sizeof(Font));
    font->handle = TTF_OpenFont(fp, size);
    Assert(font->handle);
    
    TTF_SizeText(font->handle, "A", &font->char_width, &font->char_height);
    return font;
}

//~ Render Targets

static Render_Target *RenderMakeRenderTarget(View_State view,
                                      bool use_negative_coords,
                                      int width,
                                      int height)
{
    Render_Target *target = PushSize(gs->persistent_memory,
                                     sizeof(Render_Target));
    
    if (!use_negative_coords) {
        target->full_width = target->working_width = width;
        target->full_height = target->working_height = height;
        
        target->top_left = (Point){0, 0};
    } else {
        target->full_width = width*2;
        target->full_height = height*2;
        target->working_width = width;
        target->working_height = height;
        
        target->top_left = (Point){
            target->full_width/2 - target->working_width/2,
            target->full_height/2 - target->working_height/2
        };
    }
    
    target->view = view;
    
    target->handle = SDL_CreateTexture(gs->render.sdl,
                                       ALASKA_PIXELFORMAT,
                                       SDL_TEXTUREACCESS_TARGET,
                                       target->full_width,
                                       target->full_height);
    
    return target;
}

    
//~ Actual Rendering Code

static void RenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(gs->render.sdl, r, g, b, a);
}

static void RenderColorStruct(SDL_Color rgba) {
    SDL_SetRenderDrawColor(gs->render.sdl, rgba.r, rgba.g, rgba.b, rgba.a);
}

// When target is NULL, the render target is inferred to be the screen.
static void _maybe_switch_to_target(Render_Target *target) {
    if (gs->render.current_target != target) {
        SDL_Texture *texture_target = NULL;
        if (target) texture_target = target->handle;
        
        SDL_SetRenderTarget(gs->render.sdl, target->handle);
        gs->render.current_target = target;
    }
}

static void RenderLine(Render_Target *target, int x1, int y1, int x2, int y2) {
    _maybe_switch_to_target(target);
    SDL_RenderDrawLine(gs->render.sdl, x1, y1, x2, y2);
}
    
static void RenderPoint(Render_Target *target, int x1, int y1) {
    _maybe_switch_to_target(target);
    SDL_RenderDrawPoint(gs->render.sdl, x1, y1);
}

static void RenderDrawRect(Render_Target *target, Rect rect) {
    _maybe_switch_to_target(target);
    SDL_Rect sdl_rect = {rect.x, rect.y, rect.width, rect.height};
    SDL_RenderDrawRect(gs->render.sdl, &sdl_rect);
}

static void RenderFillRect(Render_Target *target, Rect rect) {
    _maybe_switch_to_target(target);
    SDL_Rect sdl_rect = {rect.x, rect.y, rect.width, rect.height};
    SDL_RenderFillRect(gs->render.sdl, &sdl_rect);
}

static void RenderClear(Render_Target *target) {
    _maybe_switch_to_target(target);
    SDL_RenderClear(gs->render.sdl);
}

static void RenderPresent(Render_Target *target) {
    _maybe_switch_to_target(target);
    SDL_RenderPresent(gs->render.sdl);
}

//~ Fonts

static void RenderSetFontSize(Font *font, int size) {
    Assert(size>0);
    Assert(font);
    TTF_SetFontSize(font->handle, size);
}