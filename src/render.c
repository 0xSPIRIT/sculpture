// A wrapper for calls to SDL's draw functions.
// We pretty much use this to manage render targets better.

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
        gs->pixel_format_surf = SDL_CreateRGBSurfaceWithFormat(0, 16, 16, 32, ALASKA_PIXELFORMAT);
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

RENDERAPI SDL_FRect RenderGetUpdatedRectF(Render_Target *target, SDL_FRect *rect) {
    if (!target) return *rect;

    SDL_FRect result;
    SDL_FRect actual_rect;

    if (!rect) {
        return (SDL_FRect){
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

// When target is null, the render target is inferred to be the screen.
RENDERAPI Render_Target *RenderMaybeSwitchToTarget(int target_enum) {
    if (target_enum < 0) {
        SDL_SetRenderTarget(gs->render.sdl, null);
        gs->render.current_target = null;
        return null;
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

RENDERAPI void RenderArrowRelative(int target_enum, SDL_Point from, SDL_Point to, int head_size) {
    Render_Target *target = RenderMaybeSwitchToTarget(target_enum);
    from.x += target->top_left.x;
    to.x += target->top_left.x;
    from.y += target->top_left.y;
    to.y += target->top_left.y;
    RenderArrow(target_enum, from, to, head_size);
}

RENDERAPI void RenderArrow(int target_enum, SDL_Point from, SDL_Point to, int head_size) {
    RenderLine(target_enum, from.x, from.y, to.x, to.y);

    f64 ux = to.x - from.x;
    f64 uy = to.y - from.y;
    f64 length = sqrt(ux*ux + uy*uy);
    if (length == 0) return;

    ux /= length;
    uy /= length;

    // Rotate by 135 degrees.
    const f64 angle = Radians(135);

    f64 arrow_x = ux * cos(angle) - uy * sin(angle);
    f64 arrow_y = ux * sin(angle) + uy * cos(angle);

    arrow_x *= head_size;
    arrow_y *= head_size;

    arrow_x = round(arrow_x);
    arrow_y = round(arrow_y);

    RenderLine(target_enum, to.x, to.y, to.x + arrow_x, to.y + arrow_y);
    RenderLine(target_enum, to.x, to.y, to.x - arrow_y, to.y + arrow_x); // Rotate by 90 degrees
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
    if (size == 0) return;
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

    if (text_old.game_scale != text_new.game_scale)
        return true;
    if (text_old.font != text_new.font)
        return true;
    if (text_old.alignment != text_new.alignment)
        return true;
    if (text_old.render_type != text_new.render_type)
        return true;
    if (strcmp(text_old.str, text_new.str) != 0)
        return true;

    // Redraws if the color is different (but alpha can be changed)
    // This can probably be changed with drawing the text as white,
    // and then using SDL_SetTextureColorMod() with the RGB,
    // but this isn't a problem for us so why bother changing it?

    if (text_old.foreground.r != text_new.foreground.r ||
        text_old.foreground.g != text_new.foreground.g ||
        text_old.foreground.b != text_new.foreground.b)
        return true;

    if (memcmp(&text_old.background, &text_new.background, sizeof(SDL_Color)) != 0)
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
    Assert(center);

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

RENDERAPI void RenderTextureExRelativeF(int target_enum,
                                        Texture *texture,
                                        SDL_Rect *src,
                                        SDL_FRect *dst,
                                        f64 angle,
                                        SDL_FPoint *center,
                                        SDL_RendererFlip flip)
{
    Assert(center);

    Render_Target *target = RenderMaybeSwitchToTarget(target_enum);

    SDL_FRect output_dst = RenderGetUpdatedRectF(target, dst);

    SDL_FPoint output_center = *center;
    if (target) {
        output_center = (SDL_FPoint){
            center->x + target->top_left.x,
            center->y + target->top_left.y
        };
    }

    SDL_RenderCopyExF(gs->render.sdl,
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

RENDERAPI void RenderTextureExF(int target_enum,
                                Texture *texture,
                                SDL_Rect *src,
                                SDL_FRect *dst,
                                f64 angle,
                                SDL_FPoint *center,
                                SDL_RendererFlip flip)
{
    RenderMaybeSwitchToTarget(target_enum);
    SDL_RenderCopyExF(gs->render.sdl,
                      texture->handle,
                      src,
                      dst,
                      angle,
                      center,
                      flip);
}

RENDERAPI void RenderTextQuick(int target_enum,
                               const char *identifier,
                               Font *font,
                               const char *str,
                               SDL_Color color,
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
    text_data.force_update = force_redraw;

    RenderText(target_enum, &text_data);

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

RENDERAPI void RenderText(int target_enum, Render_Text_Data *text_data) {
    Assert(text_data->font);

    if (!text_data->str[0]) return;

    bool should_redraw = text_data->force_update;

    text_data->game_scale = gs->S;

    Render_Text_Data *cache_object = null;

    // Caching stuff, but only if we care. If they don't set any identifier,
    // then just draw it always.
    if (text_data->identifier[0]) {

        // Search the cache for an existing cache index
        int cache_index = _find_render_text_data_in_cache(text_data->identifier);

        // If it's not found in the cache...
        if (cache_index < 0) {
            cache_object = &gs->render.text_cache.data[gs->render.text_cache.size];
            gs->render.text_cache.size++;
            should_redraw = true;
        } else { // It is found!

            cache_object = &gs->render.text_cache.data[cache_index];

            // Compare the stored cache object to our current text_draw
            // and see if anything has changed. If the game's size has
            // changed, this also returns true, because we want to redraw.
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
        RenderTextureAlphaMod(&cache_object->texture, text_data->foreground.a);
        RenderTexture(target_enum, &cache_object->texture, null, &dst);

        // Retain the text_data's color, as well as the draw_x and draw_y
        SDL_Color text_data_color = text_data->foreground;
        *text_data = *cache_object;
        text_data->foreground = text_data_color;

        text_data->draw_x = dst.x;
        text_data->draw_y = dst.y;

        return;
    }

    RenderMaybeSwitchToTarget(target_enum);

    if (cache_object && cache_object->surface) {
        SDL_FreeSurface(cache_object->surface);
        cache_object->surface = null;
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
    text_data->draw_x = dst.x;
    text_data->draw_y = dst.y;

    RenderTextureAlphaMod(&text_data->texture, text_data->foreground.a);
    RenderTexture(target_enum, &text_data->texture, null, &dst);
}

RENDERAPI void RenderReadPixels(int target, Uint8 *pixels, int pitch) {
    RenderMaybeSwitchToTarget(target);
    SDL_RenderReadPixels(gs->render.sdl, null, ALASKA_PIXELFORMAT, pixels, pitch);
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

// Method 1 --> ~2.648ms
// Method 2 --> ~3.119ms
// Method 3 --> ~2.057ms

// Method 3
RENDERAPI void RenderFillCircle(int target, int x, int y, int r) {
    RenderMaybeSwitchToTarget(target);

    for (int cy = -r; cy <= r; cy++) {
        int cx = (int)(sqrt(r * r - cy * cy) + 0.5);
        int cyy = cy + y;

        SDL_RenderDrawLine(gs->render.sdl, x - cx, cyy, x + cx, cyy);
    }
}

RENDERAPI void RenderFillCircleRelative(int target_enum, int x, int y, int r) {
    Render_Target *target = RenderMaybeSwitchToTarget(target_enum);
    x += target->top_left.x;
    y += target->top_left.y;
    RenderFillCircle(target_enum, x, y, r);
}

#if 0
// Method 1
RENDERAPI void RenderFillCircle(int target, int center_x, int center_y, int radius) {
    RenderMaybeSwitchToTarget(target);

    int radius_sqr = radius*radius;

    for (int x = -radius; x < radius ; x++) {
        int hh = (int)sqrt(radius_sqr - x * x);
        int rx = center_x + x;
        int ph = center_y + hh;

        for (int y = center_y-hh; y < ph; y++)
            SDL_RenderDrawPoint(gs->render.sdl, rx, y);
    }
}

// Method 2
RENDERAPI void RenderFillCircle(int target, int center_x, int center_y, int radius) {
    RenderMaybeSwitchToTarget(target);

    int r2 = radius * radius;
    int area = r2 << 2;
    int rr = radius << 1;

    for (int i = 0; i < area; i++) {
        int tx = (i % rr) - radius;
        int ty = (i / rr) - radius;

        if (tx * tx + ty * ty <= r2)
            SDL_RenderDrawPoint(gs->render.sdl, center_x + tx, center_y + ty);
    }
}
#endif
