static void narrator_draw_text_blended(int i, // 0 to 10
                                       Font *font,
                                       const char *str,
                                       SDL_Color col,
                                       int x,
                                       int y,
                                       int *out_w,
                                       int *out_h)
{
    char identifier[64] = {0};
    sprintf(identifier, "narrator %d", i);
    
    RenderDrawTextQuick(RENDER_TARGET_MASTER,
                        identifier,
                        font,
                        str,
                        col,
                        max(min(gs->narrator.alpha, 255), 0),
                        x,
                        y,
                        out_w,
                        out_h,
                        false);
}

static char* get_narration(int level) {
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

static void narrator_init(int level) {
    Narrator *n = &gs->narrator;
    
    memset(&gs->narrator, 0, sizeof(Narrator));
    
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
    
    narrator_next_line(true);
}

static void narrator_next_line(bool init) {
    Narrator *n = &gs->narrator;
    
    n->update = true; // Gets reset in narrator_run
    
    if (!init)
        n->line_curr++;
    
    int a = 0;
    int i = 0;
    
    memset(n->current_lines, 0, 10*256);
    
    char *s = n->lines[n->line_curr];
    while (true) {
        if (*s == '\r') {
            a++;
            ++s;
            i=0;
            continue;
        }
        if (*s == 0) {
            a++;
            ++s;
            i=0;
            break;
        }
        n->current_lines[a][i++] = *s;
        ++s;
    }
    n->current_line_count = a;
}

static void narrator_tick() {
    Narrator *n = &gs->narrator;
    
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
    
    if (gs->input.keys_pressed[SDL_SCANCODE_RETURN] || gs->input.keys_pressed[SDL_SCANCODE_SPACE])  {
        n->fadeout = true;
    }
    
    if (n->fadeout) {
        n->alpha -= NARRATOR_ALPHA;
        if (n->alpha < -NARRATOR_ALPHA*NARRATOR_HANG_TIME) {
            n->alpha = 0;
            narrator_next_line(false);
            if (n->line_curr >= n->line_count) {
                set_fade(FADE_NARRATOR, 0, 255);
            }
            
            n->fadeout = false;
        }
    } else {
        if (gs->narrator.alpha < 255) {
            gs->narrator.alpha += NARRATOR_ALPHA;
        } else {
            gs->narrator.alpha = 255;
        }
    }
}

static int get_glitched_offset(void) {
    int xoff = 0;
    xoff = fmod(rand(),Scale(4))-Scale(2);
    return xoff;
}

static void narrator_run(int target, SDL_Color col) {
    Narrator *n = &gs->narrator;
    
    RenderMaybeSwitchToTarget(target);
    
    if (n->off) return;
    if (n->delay > 0) return;
    
    if (wait_for_fade(FADE_NARRATOR)) {
        reset_fade();
        
        if (gs->level_current+1 == 11 && gs->obj.active) {
            n->off = true;
            gs->credits.state = CREDITS_DELAY;
        } else {
            level_set_state(gs->level_current, LEVEL_STATE_INTRO);
            effect_set(gs->levels[gs->level_current].effect_type,
                       false,
                       -gs->gw/2,
                       -gs->gh/2,
                       2*gs->gw,
                       2*gs->gh);
            memset(&gs->narrator, 0, sizeof(Narrator));
        }
        return;
    }
    
    Font *font = gs->fonts.font_times;
    
    for (int i = 0; i < n->current_line_count; i++) {
        char *s = PushArray(gs->transient_memory, n->curr_len, sizeof(char));
        strcpy(s, n->current_lines[i]);
        
        int w, h;
        TTF_SizeText(font->handle, s, &w, &h);
        
        const int pad = 8;
        
        SDL_Color c = col;
        
        int xoff = 0;
#if 0
        if (gs->level_current+1 == 8 || gs->level_current+1 == 10) {
            xoff = get_glitched_offset();
            if (rand() < RAND_MAX/100) {xoff *= 25;}
        }
#endif
        
        int surf_h;
        narrator_draw_text_blended(i,
                                   font,
                                   s,
                                   c,
                                   xoff + gs->window_width/2 - w/2,
                                   gs->window_height/2 + 16 + i*(h+pad) - (h+pad)*n->current_line_count/2,
                                   NULL,
                                   &surf_h);
    }
    
    n->update = false;
}