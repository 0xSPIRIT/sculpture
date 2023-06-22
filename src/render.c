// A wrapper for calls to SDL's draw functions.
// We use this for things like global offsets,
// and to manage render targets better.

//~ Initializations and Creations

RENDERAPI Render RenderInit(SDL_Renderer *sdl_renderer) {
    Render result = {0};
    result.sdl = sdl_renderer;
    result.view_type = VIEW_STATE_SCREENSPACE;
    result.render_targets = PushArray(gs->persistent_memory,
                                      RENDER_TARGET_COUNT,
                                      sizeof(Render_Target));
    return result;
}

RENDERAPI void RenderCleanup(Render *render) {
    // Cleanup render targets.
    for (int i = 0; i < RENDER_TARGET_COUNT; i++) {
        RenderDestroyTarget(&render->render_targets[i]);
    }

    // Cleanup textures.
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        if (gs->textures.texs[i].handle)
            RenderDestroyTexture(&gs->textures.texs[i]);
    }

    // Cleanup surfaces.
    for (size_t i = 0; i < SURFACE_COUNT; i++) {
        if (gs->surfaces.surfaces[i])
            SDL_FreeSurface(gs->surfaces.surfaces[i]);
    }

    // Cleanup text data cache. Non-cached temporary text rendering calls
    // are cleaned up at end-of-frame.
    RenderCleanupTextCache(&gs->render.text_cache);
}

RENDERAPI SDL_Surface *RenderLoadSurface(const char *fp) {
    if (!gs->pixel_format_surf) {
        gs->pixel_format_surf = SDL_CreateRGBSurfaceWithFormat(0, 16, 16, 32,
                                                               ALASKA_PIXELFORMAT);
    }

    char filepath[8192] = {0};
    strcat(filepath, RES_DIR);
    strcat(filepath, fp);
    SDL_Surface *surf = IMG_Load(filepath);
    if (!surf) puts(SDL_GetError());

    if (surf->format->format != ALASKA_PIXELFORMAT) {
        SDL_Surface *new_surf = SDL_ConvertSurface(surf,
                                                   gs->pixel_format_surf->format,
                                                   0);
        SDL_FreeSurface(surf);
        surf = new_surf;
    }

    Assert(surf);
    return surf;
}

RENDERAPI Texture RenderLoadTexture(const char *fp) {
    Texture texture = {0};

    SDL_Surface *surf = RenderLoadSurface(fp);
    Assert(surf);

    texture.width = surf->w;
    texture.height = surf->h;
    texture.handle = SDL_CreateTextureFromSurface(gs->render.sdl, surf);
    Assert(texture.handle);

    SDL_FreeSurface(surf);

    return texture;
}

RENDERAPI Texture RenderCreateTextureFromSurface(SDL_Surface *surf) {
    Texture texture = {0};

    texture.handle = SDL_CreateTextureFromSurface(gs->render.sdl, surf);
    Assert(texture.handle);

    texture.width = surf->w;
    texture.height = surf->h;

    return texture;
}

RENDERAPI void RenderDestroyTexture(Texture *tex) {
    SDL_DestroyTexture(tex->handle);
    memset(tex, 0, sizeof(Texture));
}

RENDERAPI Font *RenderLoadFont(const char *fp, int size) {
    Font *font = PushSize(gs->persistent_memory, sizeof(Font));
    char filename[MAX_PATH] = {0};
    strcat(filename, RES_DIR);
    strcat(filename, fp);
    font->handle = TTF_OpenFont(filename, size);
    Assert(font->handle);

    TTF_SizeText(font->handle, "A", &font->char_width, &font->char_height);
    return font;
}

//~ Render Targets

RENDERAPI SDL_Rect RenderGetUpdatedRect(Render_Target *target, SDL_Rect *rect) {
    if (!target) return *rect;

    SDL_Rect result;
    SDL_Rect actual_rect;

    if (!rect) {
        return (SDL_Rect){
            target->top_left.x,
            target->top_left.y,
            target->working_width,
            target->working_height
        };
    } else {
        actual_rect = *rect;
    }

    result.x = actual_rect.x + target->top_left.x;
    result.y = actual_rect.y + target->top_left.y;
    result.w = actual_rect.w;
    result.h = actual_rect.h;

    return result;
}

RENDERAPI Render_Target RenderMakeTargetEx(int width,
                                           int height,
                                           View_State view,
                                           bool use_negative_coords,
                                           bool streaming)
{
    Render_Target target = {0};

    int full_width, full_height;

    if (!use_negative_coords) {
        target.working_width = full_width = width;
        target.working_height = full_height = height;

        target.top_left = (SDL_Point){0, 0};
    } else {
        target.working_width = width;
        target.working_height = height;

        full_width = width*2;
        full_height = height*2;

        target.top_left = (SDL_Point){
            width  - target.working_width/2,
            height - target.working_height/2
        };
    }

    target.view = view;

    target.texture.handle = SDL_CreateTexture(gs->render.sdl,
                                              ALASKA_PIXELFORMAT,
                                              streaming ? SDL_TEXTUREACCESS_STREAMING : SDL_TEXTUREACCESS_TARGET,
                                              full_width,
                                              full_height);
    SDL_SetTextureBlendMode(target.texture.handle, SDL_BLENDMODE_BLEND);

    target.texture.width = full_width;
    target.texture.height = full_height;

    return target;
}

RENDERAPI Render_Target RenderMakeTarget(int width,
                                         int height,
                                         View_State view,
                                         bool use_negative_coords)
{
    return RenderMakeTargetEx(width, height, view, use_negative_coords, false);
}

RENDERAPI void RenderTargetToTarget(int target_dst,
                                    int target_src,
                                    SDL_Rect *src,
                                    SDL_Rect *dst)
{
    RenderMaybeSwitchToTarget(target_dst);
    SDL_RenderCopy(gs->render.sdl,
                   RenderTarget(target_src)->texture.handle,
                   src,
                   dst);
}

RENDERAPI void RenderTargetToTargetRelative(int target_dst,
                                            int target_src,
                                            SDL_Rect *src,
                                            SDL_Rect *dst)
{
    RenderMaybeSwitchToTarget(target_dst);

    SDL_Rect updated_src = RenderGetUpdatedRect(RenderTarget(target_src), src);
    SDL_Rect updated_dst = RenderGetUpdatedRect(RenderTarget(target_dst), dst);

    SDL_RenderCopy(gs->render.sdl,
                   RenderTarget(target_src)->texture.handle,
                   &updated_src,
                   &updated_dst);
}

RENDERAPI void RenderTargetToTargetRelativeEx(int target_dst,
                                              int target_src,
                                              SDL_Rect *src,
                                              SDL_Rect *dst,
                                              f64 angle,
                                              SDL_Point *center,
                                              SDL_RendererFlip flip)
{
    // Note: we dont modify src and dst to target_dst's top_left.
    //       If you want to do that, do it yourself before the call.

    RenderMaybeSwitchToTarget(target_dst);
    SDL_Point output_center = *center;

    output_center = (SDL_Point){
        center->x + RenderTarget(target_dst)->top_left.x,
        center->y + RenderTarget(target_dst)->top_left.y
    };

    SDL_RenderCopyEx(gs->render.sdl,
                     RenderTarget(target_src)->texture.handle,
                     src,
                     dst,
                     angle,
                     &output_center,
                     flip);
}

RENDERAPI void RenderDestroyTarget(Render_Target *target) {
    if (target && target->texture.handle) {
        SDL_DestroyTexture(target->texture.handle);
    }
    memset(target, 0, sizeof(Render_Target));
}

//~ Actual Rendering Code

RENDERAPI void RenderColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(gs->render.sdl, r, g, b, a);
}

RENDERAPI void RenderColorStruct(SDL_Color rgba) {
    SDL_SetRenderDrawColor(gs->render.sdl, rgba.r, rgba.g, rgba.b, rgba.a);
}

// When target is NULL, the render target is inferred to be the screen.
static Render_Target *RenderMaybeSwitchToTarget(int target_enum) {
    if (target_enum < 0) {
        SDL_SetRenderTarget(gs->render.sdl, NULL);
        gs->render.current_target = NULL;
        return NULL;
    }

    Render_Target *target = RenderTarget(target_enum);
    if (gs->render.current_target != target) {
        SDL_Texture *texture_target = target->texture.handle;
        SDL_SetRenderTarget(gs->render.sdl, texture_target);
        gs->render.current_target = target;
    }
    return gs->render.current_target;
}

RENDERAPI void RenderLineRelative(int target_enum, int x1, int y1, int x2, int y2) {
    Render_Target *target = RenderMaybeSwitchToTarget(target_enum);
    x1 += target->top_left.x;
    y1 += target->top_left.y;
    x2 += target->top_left.x;
    y2 += target->top_left.y;
    SDL_RenderDrawLine(gs->render.sdl, x1, y1, x2, y2);
}

RENDERAPI void RenderLine(int target_enum, int x1, int y1, int x2, int y2) {
    RenderMaybeSwitchToTarget(target_enum);
    SDL_RenderDrawLine(gs->render.sdl, x1, y1, x2, y2);
}

RENDERAPI void RenderPointRelative(int target_enum, int x, int y) {
    Render_Target *target = RenderMaybeSwitchToTarget(target_enum);
    x += target->top_left.x;
    y += target->top_left.y;
    SDL_RenderDrawPoint(gs->render.sdl, x, y);
}

RENDERAPI void RenderPoint(int target_enum, int x, int y) {
    RenderMaybeSwitchToTarget(target_enum);
    SDL_RenderDrawPoint(gs->render.sdl, x, y);
}

RENDERAPI void RenderDrawRectRelative(int target_enum, SDL_Rect rect) {
    Render_Target *target = RenderMaybeSwitchToTarget(target_enum);
    rect = RenderGetUpdatedRect(target, &rect);
    SDL_RenderDrawRect(gs->render.sdl, &rect);
}

RENDERAPI void RenderDrawRect(int target_enum, SDL_Rect rect) {
    RenderMaybeSwitchToTarget(target_enum);
    SDL_RenderDrawRect(gs->render.sdl, &rect);
}

RENDERAPI void RenderFillRectRelative(int target_enum, SDL_Rect rect) {
    Render_Target *target = RenderMaybeSwitchToTarget(target_enum);
    rect = RenderGetUpdatedRect(target, &rect);
    SDL_RenderFillRect(gs->render.sdl, &rect);
}

RENDERAPI void RenderFillRect(int target_enum, SDL_Rect rect) {
    RenderMaybeSwitchToTarget(target_enum);
    SDL_RenderFillRect(gs->render.sdl, &rect);
}

RENDERAPI void RenderClear(int target_enum) {
    RenderMaybeSwitchToTarget(target_enum);
    SDL_RenderClear(gs->render.sdl);
}

//~ Fonts

RENDERAPI void RenderSetFontSize(Font *font, int size) {
    Assert(size>0);
    Assert(font);
    TTF_SetFontSize(font->handle, size);
    TTF_SizeText(font->handle, "A", &font->char_width, &font->char_height);
}

// Returns the index, or -1 if the identifier is not found.
static int _find_render_text_data_in_cache(const char identifier[]) {
    Render_Text_Data_Cache *cache = &gs->render.text_cache;

    for (int i = 0; i < cache->size; i++) {
        if (strcmp(cache->data[i].identifier, identifier) == 0)
            return i;
    }

    return -1;
}

static bool _has_text_data_changed(Render_Text_Data text_old,
                                   Render_Text_Data text_new)
{
    // We should only be checking if the data has changed
    // if both structures we're comparing are the same identifier.
    Assert(0 == strcmp(text_old.identifier, text_new.identifier));
    Assert(!text_new.force_update); // We shouldn't be in here if this is true

    if (text_old.font != text_new.font)
        return true;
    if (strcmp(text_old.str, text_new.str) != 0)
        return true;
    if (text_old.alignment != text_new.alignment)
        return true;
    if (text_old.render_type != text_new.render_type)
        return true;
    if (text_old.game_scale != text_new.game_scale)
        return true;

    // Compare both foregroud and background.
    if (memcmp(&text_old.foreground,
               &text_new.foreground,
               sizeof(SDL_Color)*2) != 0)
        return true;

    return false;
}

RENDERAPI void RenderTexture(int target_enum,
                             Texture *texture,
                             SDL_Rect *src,
                             SDL_Rect *dst)
{
    RenderMaybeSwitchToTarget(target_enum);
    SDL_RenderCopy(gs->render.sdl,
                   texture->handle,
                   src,
                   dst);
}

RENDERAPI void RenderTextureRelative(int target_enum,
                                     Texture *texture,
                                     SDL_Rect *src,
                                     SDL_Rect *dst)
{
    Render_Target *target = RenderMaybeSwitchToTarget(target_enum);

    SDL_Rect output_dst = RenderGetUpdatedRect(target, dst);

    SDL_RenderCopy(gs->render.sdl,
                   texture->handle,
                   src,
                   &output_dst);
}

RENDERAPI void RenderTextureExRelative(int target_enum,
                                       Texture *texture,
                                       SDL_Rect *src,
                                       SDL_Rect *dst,
                                       f64 angle,
                                       SDL_Point *center,
                                       SDL_RendererFlip flip)
{
    Render_Target *target = RenderMaybeSwitchToTarget(target_enum);

    SDL_Rect output_dst = RenderGetUpdatedRect(target, dst);

    SDL_Point output_center = *center;
    if (target) {
        output_center = (SDL_Point){
            center->x + target->top_left.x,
            center->y + target->top_left.y
        };
    }

    SDL_RenderCopyEx(gs->render.sdl,
                     texture->handle,
                     src,
                     &output_dst,
                     angle,
                     center,
                     flip);
}

RENDERAPI void RenderTextureEx(int target_enum,
                               Texture *texture,
                               SDL_Rect *src,
                               SDL_Rect *dst,
                               f64 angle,
                               SDL_Point *center,
                               SDL_RendererFlip flip)
{
    RenderMaybeSwitchToTarget(target_enum);
    SDL_RenderCopyEx(gs->render.sdl,
                     texture->handle,
                     src,
                     dst,
                     angle,
                     center,
                     flip);
}

RENDERAPI void RenderDrawTextQuick(int target_enum,
                                   const char *identifier,
                                   Font *font,
                                   const char *str,
                                   SDL_Color color,
                                   Uint8 alpha,
                                   int x,
                                   int y,
                                   int *w,
                                   int *h,
                                   bool force_redraw)
{
    Render_Text_Data text_data = {0};

    strcpy(text_data.identifier, identifier);
    text_data.font = font;
    strcpy(text_data.str, str);
    text_data.foreground = color;
    text_data.x = x;
    text_data.y = y;
    text_data.alignment = ALIGNMENT_TOP_LEFT;
    text_data.render_type = TEXT_RENDER_BLENDED;
    text_data.alpha = alpha;
    text_data.force_update = force_redraw;

    RenderDrawText(target_enum, &text_data);

    if (w) *w = text_data.texture.width;
    if (h) *h = text_data.texture.height;
}

RENDERAPI void RenderCleanupTextCache(Render_Text_Data_Cache *cache) {
    for (int i = 0; i < cache->size; i++) {
        Render_Text_Data *text_data = &cache->data[i];
        RenderDestroyTexture(&text_data->texture);
        SDL_FreeSurface(text_data->surface);
        memset(text_data, 0, sizeof(Render_Text_Data));
    }
    cache->size = 0;
}

RENDERAPI void RenderApplyAlignmentToRect(SDL_Rect *dst, Alignment alignment)
{
    switch(alignment) {
        case ALIGNMENT_TOP_LEFT: {
            break;
        }
        case ALIGNMENT_TOP_RIGHT: {
            dst->x -= dst->w;
        } break;
        case ALIGNMENT_BOTTOM_LEFT: {
            dst->y -= dst->h;
        } break;
        case ALIGNMENT_BOTTOM_RIGHT: {
            dst->y -= dst->h;
            dst->x -= dst->w;
        } break;
        case ALIGNMENT_CENTER: {
            dst->x -= dst->w/2;
            dst->y -= dst->h/2;
        } break;
    }
}

RENDERAPI void RenderDrawText(int target_enum, Render_Text_Data *text_data)
{
    Assert(text_data->font);
    if (!text_data->str[0]) return;

    bool should_redraw = text_data->force_update;

    text_data->game_scale = gs->S;

    Render_Text_Data *cache_object = NULL;

    // Caching stuff, but only if we care. If they don't set any identifier,
    // then just draw it always.
    if (text_data->identifier[0]) {
        int cache_index = _find_render_text_data_in_cache(text_data->identifier);
        if (cache_index < 0) {
            cache_object = &gs->render.text_cache.data[gs->render.text_cache.size];
            gs->render.text_cache.size++;
            should_redraw = true;
        } else {
            // Compares the stored cache to the new one,
            // and see if anything has changed.
            cache_object = &gs->render.text_cache.data[cache_index];

            // If the game's size has changed, it also returns true.
            bool changed = _has_text_data_changed(*cache_object, *text_data);

            // If so, we should redraw.
            if (changed) {
                should_redraw = true;
            } else { // Otherwise, set up our new texture and GTFO.
                text_data->texture = cache_object->texture;
            }
        }
    } else {
        Render_Text_Data_Cache *temp_cache = &gs->render.temp_text_cache;
        cache_object = &temp_cache->data[temp_cache->size++];
        should_redraw = true;
    }

    Assert(cache_object);

    if (!should_redraw) {
        SDL_Rect dst = {
            text_data->x,
            text_data->y,
            text_data->texture.width,  // text_data and cache_object
            text_data->texture.height  // should be the same for w&h
        };

        RenderApplyAlignmentToRect(&dst, text_data->alignment);

        // NOTE: We use the text data's alpha here, not the cached alpha,
        //       since it may have changed. We obviously don't update
        //       the cache if the alpha of the draw call changed.
        //       Same concept behind using text_data's x and y for the
        //       dst rect.
        RenderTextureAlphaMod(&cache_object->texture, text_data->alpha);
        RenderTexture(target_enum, &cache_object->texture, NULL, &dst);

        // Retain the text_data's alpha.
        Uint8 text_data_alpha = text_data->alpha;
        *text_data = *cache_object;
        text_data->alpha = text_data_alpha;

        return;
    }

    RenderMaybeSwitchToTarget(target_enum);

    if (cache_object && cache_object->surface) {
        SDL_FreeSurface(cache_object->surface);
        cache_object->surface = NULL;
    }
    if (cache_object && cache_object->texture.handle) {
        RenderDestroyTexture(&cache_object->texture);
    }

    switch(text_data->render_type){
        case TEXT_RENDER_BLENDED:
        {
            text_data->surface = TTF_RenderText_Blended(text_data->font->handle,
                                                        text_data->str,
                                                        text_data->foreground);
        } break;
        case TEXT_RENDER_LCD:
        {
            text_data->surface = TTF_RenderText_LCD(text_data->font->handle,
                                                    text_data->str,
                                                    text_data->foreground,
                                                    text_data->background);
        } break;
        case TEXT_RENDER_SOLID:
        {
            text_data->surface = TTF_RenderText_Solid(text_data->font->handle,
                                                      text_data->str,
                                                      text_data->foreground);
        } break;
    }

    Assert(text_data->surface);

    text_data->texture = RenderCreateTextureFromSurface(text_data->surface);

    *cache_object = *text_data;

    SDL_Rect dst = {
        text_data->x,
        text_data->y,
        text_data->texture.width,
        text_data->texture.height
    };

    RenderApplyAlignmentToRect(&dst, text_data->alignment);

    RenderTextureAlphaMod(&text_data->texture, text_data->alpha);
    RenderTexture(target_enum, &text_data->texture, NULL, &dst);
}

RENDERAPI void RenderReadPixels(int target, Uint8 *pixels, int pitch) {
    RenderMaybeSwitchToTarget(target);
    SDL_RenderReadPixels(gs->render.sdl, NULL, ALASKA_PIXELFORMAT, pixels, pitch);
}

RENDERAPI void RenderTextureColorMod(Texture *texture,
                                     Uint8 r,
                                     Uint8 g,
                                     Uint8 b)
{
    SDL_SetTextureColorMod(texture->handle, r, g, b);
}

RENDERAPI void RenderTextureAlphaMod(Texture *texture, Uint8 a) {
    SDL_SetTextureAlphaMod(texture->handle, a);
}

RENDERAPI int RenderLockTexture(Texture *texture,
                                SDL_Rect *rect,
                                void **pixels,
                                int *pitch)
{
    return SDL_LockTexture(texture->handle, rect, pixels, pitch);
}

RENDERAPI void RenderUnlockTexture(Texture *texture) {
    SDL_UnlockTexture(texture->handle);
}

RENDERAPI void RenderFillCircle(int target, int x, int y, int size) {
    RenderMaybeSwitchToTarget(target);

    for (int yy = -size; yy <= size; yy++) {
        for (int xx = -size; xx <= size; xx++) {
            if (xx*xx + yy*yy > size*size) continue;

            SDL_RenderDrawPoint(gs->render.sdl, x+xx, y+yy);
        }
    }
}
