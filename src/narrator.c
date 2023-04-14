void narrator_draw_text_blended(TTF_Font *font,
                                const char *str,
                                SDL_Color col,
                                bool align_right,
                                bool align_bottom,
                                int x,
                                int y,
                                int *out_w,
                                int *out_h) 
{
    if (!*str) {
        TTF_SizeText(font, "+", out_w, out_h);
        return;
    }
    
    SDL_Surface *surf = TTF_RenderText_Blended(font, str, col);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);
    
    SDL_Rect dst = { x, y + gs->window_height/2 - surf->h/2, surf->w, surf->h };
    
    if (align_right) dst.x -= surf->w;
    if (align_bottom) dst.y -= surf->h;
    
    if (out_w)
        *out_w = surf->w;
    if (out_h)
        *out_h = surf->h;
    
    SDL_SetTextureAlphaMod(texture, gs->narrator.alpha);
    SDL_RenderCopy(gs->renderer, texture, NULL, &dst);
    
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(texture);
}

char* get_narration(int level) {
    switch (level+1) {
        case 1:  return NARRATION_LEVEL_1;
        case 3:  return NARRATION_LEVEL_3;
        case 4:  return NARRATION_LEVEL_4;
        case 7:  return NARRATION_LEVEL_7;
        case 8:  return NARRATION_LEVEL_8;
        case 10: return NARRATION_LEVEL_10;
        case 11: return NARRATION_LEVEL_11;
        
        case 12: return NARRATION_END;
    }
    return NULL;
}

void narrator_init(int level) {
    struct Narrator *n = &gs->narrator;
    
    memset(&gs->narrator, 0, sizeof(struct Narrator));
    
    char *narration = NULL;
    
    narration = get_narration(level);
    
    if (!narration) return;
    
    char *c = narration;
    int i = 0;
    
    while (*c) {
        if (*c == '\n') {
            n->line_count++;
            ++c;
            i = 0;
            continue;
        }
        
        n->lines[n->line_count][i] = *c;
        
        ++i;
        ++c;
    }
    
    n->line_curr = 0;
    n->alpha = 0;
    n->fadeout = false;
    n->first_frame = true;
    n->delay = 30;
}

void narrator_tick() {
    struct Narrator *n = &gs->narrator;
    
    n->delay--;
    if (n->delay > 0) return;
    
    if (n->off) return;
    if (gs->fade.active) return;
    
    if (n->first_frame) {
        n->first_frame = false;
        return;
    }
    
    if (gs->input.keys_pressed[SDL_SCANCODE_TAB]) {
        set_fade(FADE_NARRATOR, 0, 255);
    }
    
    if (gs->input.keys_pressed[SDL_SCANCODE_RETURN] || gs->input.keys_pressed[SDL_SCANCODE_SPACE] || gs->input.mouse_pressed[SDL_BUTTON_LEFT])  {
        n->fadeout = true;
    }
    
    if (n->fadeout) {
        n->alpha -= 30;
        if (n->alpha < 0) {
            n->alpha = 0;
            n->line_curr++;
            if (n->line_curr >= n->line_count) {
                set_fade(FADE_NARRATOR, 0, 255);
            }
            
            n->fadeout = false;
        }
    }
}

void narrator_run(SDL_Color col) {
    struct Narrator *n = &gs->narrator;
    
    if (n->off) return;
    if (n->delay > 0) return;
    
    if (wait_for_fade(FADE_NARRATOR)) {
        reset_fade();
        
        if (gs->level_current+1 == 11 && gs->obj.active) {
            n->off = true;
            gs->credits.state = CREDITS_DELAY;
        } else {
            level_set_state(gs->level_current, LEVEL_STATE_INTRO);
            effect_set(gs->levels[gs->level_current].effect_type, gs->gw, gs->gh);
            memset(&gs->narrator, 0, sizeof(struct Narrator));
        }
        return;
    }
    
    TTF_Font *font = gs->fonts.font_times;
    
    char *s = PushArray(gs->transient_memory, n->curr_len, sizeof(char));
    strcpy(s, n->lines[n->line_curr]);
    
    int w, h;
    TTF_SizeText(font, n->lines[n->line_curr], &w, &h);
    
    gs->narrator.alpha = lerp64(gs->narrator.alpha, 255, NARRATOR_ALPHA);
        
    int surf_h;
    narrator_draw_text_blended(font,
                               s,
                               col,
                               false,
                               false,
                               gs->window_width/2 - w/2, 16,
                               NULL, &surf_h);
}