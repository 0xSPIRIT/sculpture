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
        case 1: return NARRATION_LEVEL_1;
        case 3: return NARRATION_LEVEL_3;
        case 6: return NARRATION_LEVEL_6;
        case 7: return NARRATION_LEVEL_7;
        case 9: return NARRATION_LEVEL_9;
        case 10: return NARRATION_LEVEL_10;
        
        case 11: return NARRATION_END;
    }
    return NULL;
}

void narrator_init(int level) {
    memset(&gs->narrator, 0, sizeof(struct Narrator));
    
    char *narration = NULL;
    
    narration = get_narration(level);
    
    if (!narration) return;
    
    char *c = narration;
    int i = 0;
    
    while (*c) {
        if (*c == '\n') {
            gs->narrator.line_count++;
            ++c;
            i = 0;
            continue;
        }
        
        gs->narrator.lines[gs->narrator.line_count][i] = *c;
        
        ++i;
        ++c;
    }
    
    gs->narrator.line_curr = 0;
    gs->narrator.char_curr = 0;
    gs->narrator.curr_len = strlen(gs->narrator.lines[gs->narrator.line_curr]);
    
    gs->narrator.time = 0;
}

void narrator_tick() {
    if (gs->input.keys_pressed[SDL_SCANCODE_RETURN])  {
        if (gs->input.keys[SDL_SCANCODE_LCTRL]) { // Skip to the end
            gs->narrator.black = true;
            gs->narrator.time = 0;
        } else if (gs->narrator.char_curr >= gs->narrator.curr_len) {
            gs->narrator.line_curr++;
            gs->narrator.char_curr = 0;
            gs->narrator.curr_len = strlen(gs->narrator.lines[gs->narrator.line_curr]);
            
            if (gs->narrator.line_curr >= gs->narrator.line_count) {
                gs->narrator.black = true;
                gs->narrator.time = 0;
            }
        } else {
            gs->narrator.char_curr = (int)gs->narrator.curr_len;
        }
    }
    
    if (gs->narrator.black) {
        gs->narrator.time++;
        if (gs->narrator.time > 30) {
            gs->levels[gs->level_current].state = LEVEL_STATE_PLAY;
            memset(&gs->narrator, 0, sizeof(struct Narrator));
        }
    }
}

void narrator_run(SDL_Color col) {
    TTF_Font *font = gs->fonts.font_times;
    struct Narrator *n = &gs->narrator;
    
    const int text_speed = 1; // more = slower, 0 = scroll every frame.
    
    int delay;
    if (n->char_curr < 1) {
        delay = text_speed;
    } else {
        delay = ispunctuation(n->lines[n->line_curr][n->char_curr-1]) ? 15 : text_speed;
    }
    
    if (gs->narrator.black) return;
    
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
            25, 24
        };
        SDL_SetTextureAlphaMod(gs->textures.text_arrow, 192+64*sin(SDL_GetTicks()/250.0));
        if (col.r == 0) {
            SDL_SetTextureColorMod(gs->textures.text_arrow, 0, 0, 0);
        } else {
            SDL_SetTextureColorMod(gs->textures.text_arrow, 255, 255, 255);
        }
        SDL_RenderCopy(gs->renderer, gs->textures.text_arrow, NULL, &dst);
    }
}
