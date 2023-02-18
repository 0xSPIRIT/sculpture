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
    n->char_curr = 0;
    n->curr_len = strlen(n->lines[n->line_curr]);
    
    n->time = 0;
}

void narrator_tick() {
    struct Narrator *n = &gs->narrator;
    
    if (n->off) return;
    
    if (gs->input.keys_pressed[SDL_SCANCODE_TAB]) {
        n->black = true;
        n->time = 0;
    }
    
    if (gs->input.keys_pressed[SDL_SCANCODE_RETURN] || gs->input.keys_pressed[SDL_SCANCODE_SPACE] || gs->input.mouse_pressed[SDL_BUTTON_LEFT])  {
        if (n->char_curr >= n->curr_len) {
            n->line_curr++;
            n->char_curr = 0;
            n->curr_len = strlen(n->lines[n->line_curr]);
            
            if (n->line_curr >= n->line_count) {
                n->black = true;
                n->time = 0;
            }
        } else {
            n->char_curr = (int)n->curr_len;
        }
    }
    
    if (n->black) {
        n->time++;
        
        if (gs->level_current+1 == 11 && gs->obj.active) {
            n->off = true;
            gs->credits.state = CREDITS_DELAY;
        } else if (n->time > 60) {
            level_set_state(gs->level_current, LEVEL_STATE_INTRO);
            effect_set(gs->levels[gs->level_current].effect_type, gs->gw, gs->gh);
            memset(&gs->narrator, 0, sizeof(struct Narrator));
        }
    }
}

void narrator_run(SDL_Color col) {
    struct Narrator *n = &gs->narrator;
    
    if (n->off) return;
    
    TTF_Font *font = gs->fonts.font_times;
    
    const int text_speed = 1; // more = slower, 0 = scroll every frame.
    
    int delay;
    if (n->char_curr < 1 || n->char_curr >= n->curr_len-2) {
        delay = text_speed;
    } else {
        delay = ispunctuation(n->lines[n->line_curr][n->char_curr-1]) ? 15 : text_speed;
    }
    
    if (n->black) return;
    
    if (n->time > delay && n->char_curr < n->curr_len) {
        n->char_curr++;
        n->time = 0;
    }
    n->time++;
    
    char *s = PushArray(gs->transient_memory, n->char_curr+1, sizeof(char));
    strcpy(s, n->lines[n->line_curr]);
    s[n->char_curr] = 0;
    
    int w;
    TTF_SizeText(font, n->lines[n->line_curr], &w, NULL);
    
    narrator_draw_text_blended(font,
                               s,
                               col,
                               false,
                               false,
                               gs->window_width/2 - w/2, 16,
                               NULL, NULL);
    
    if (n->char_curr == n->curr_len) {
        SDL_Rect dst = {
            9*gs->window_width/10 - 12,
            9*gs->window_height/10 - 12,
            Scale(25), Scale(24)
        };
        
        f64 start_value = 0.2;
        f64 alpha = 1+sin(SDL_GetTicks()/400.0);
        alpha /= 2.f;
        alpha = start_value + (1-start_value)*alpha;
        alpha *= 255;
        
        SDL_SetTextureAlphaMod(gs->textures.text_arrow, (Uint8)alpha);
        if (col.r == 0) {
            SDL_SetTextureColorMod(gs->textures.text_arrow, 0, 0, 0);
        } else {
            SDL_SetTextureColorMod(gs->textures.text_arrow, 255, 255, 255);
        }
        SDL_RenderCopy(gs->renderer, gs->textures.text_arrow, NULL, &dst);
    }
}
