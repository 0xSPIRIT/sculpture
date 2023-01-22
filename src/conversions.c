void conversions_gui_init(void) {
    struct Conversions *c = &gs->conversions;
    
    FILE *f = fopen(RES_DIR "layout_converter.txt", "r");
    
    char buf[128] = {0};
    
    while (fgets(buf, 128, f)) {
        const size_t len = strlen(buf);
        for (int i = 0; i < len; i++) {
            if (buf[i] == '\n' || buf[i] == '\r') buf[i] = 0;
        }
        strcpy(c->lines[c->line_count++], buf);
    }
    
    fclose(f);
    
    c->r = (SDL_Rect){
        340, 32,
        400, 0
    };
    
    int h;
    
    TTF_SizeText(gs->fonts.font_small, "A", NULL, &h);
    c->r.h = h*c->line_count + 32;
}

void conversions_gui_tick(void) {
    struct Conversions *c = &gs->conversions;
    
    if (gs->input.keys_pressed[SDL_SCANCODE_I]) {
        c->active = !c->active;
    }
}

void conversions_gui_draw(void) {
    struct Conversions *c = &gs->conversions;
    
    const SDL_Color bg = {0, 0, 32, 200};
    
    if (!c->active) return;
    
    const bool update = false;
    
    if (!c->calculated_render_target || update) {
        c->calculated_render_target = true;
        int cum = 0;
        
        SDL_Texture *prev = SDL_GetRenderTarget(gs->renderer);
        SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_CONVERSION_PANEL));
        
        SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
        SDL_RenderClear(gs->renderer);
        
        SDL_SetRenderDrawColor(gs->renderer, bg.r, bg.g, bg.b, bg.a);
        SDL_RenderFillRect(gs->renderer, &c->r);
        
        SDL_SetRenderDrawColor(gs->renderer, 128, 128, 128, 255);
        SDL_RenderDrawRect(gs->renderer, &c->r);
        
        int count = 0;
        
        for (int i = 0; i < c->line_count; i++) {
            count++;
            
            int h;
            
            draw_text_blended_indexed(i,
                                      gs->fonts.font_small,
                                      c->lines[i],
                                      (SDL_Color){255, 255, 255, 255},
                                      false,
                                      false,
                                      c->r.x + 16,
                                      c->r.y + 16 + cum,
                                      NULL,
                                      &h);
            cum += h;
        }
        
        {
            SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
            SDL_Rect a = {
                0, c->r.y + c->r.h,
                gs->window_width, gs->window_height - (c->r.y + c->r.h+GUI_H)
            };
            SDL_RenderFillRect(gs->renderer, &a);
            
            SDL_Rect b = {
                c->r.x+c->r.w, 0,
                gs->window_width - (c->r.x + c->r.w), gs->window_height-GUI_H
            };
            SDL_RenderFillRect(gs->renderer, &b);
        }
        
        SDL_SetRenderTarget(gs->renderer, prev);
    }
    
    SDL_Rect dst = {
        0, GUI_H,
        gs->window_width, gs->window_height-GUI_H
    };
    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_CONVERSION_PANEL), SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_CONVERSION_PANEL), NULL, &dst);
}