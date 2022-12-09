void set_tutorial_rect(const char *str, int x, int y) {
    struct Tutorial_Rect *tut = &gs->tutorial;
    tut->font = gs->fonts.font;
    tut->active = true;
    
    memset(tut->lines, 0, 64*64);
    int i = 0;
    
    int fw, fh;
    TTF_SizeText(tut->font, "=", &fw, &fh);
    
    tut->line_count = 0;
    
    tut->margin = 8;
    
    int largest = 0;
    while (*str) {
        if (*str == '\n') {
            tut->line_count++;
            ++str;
            if (i > largest) largest = i;
            i = 0;
            continue;
        }
        tut->lines[tut->line_count][i++] = *str;
        ++str;
    }
    if (i > largest) largest = i;
    tut->line_count++;
    
    tut->ok_button = button_allocate(BUTTON_TYPE_TUTORIAL, gs->textures.tutorial_ok_button, "OK", NULL);
    
    int bw, bh;
    SDL_QueryTexture(gs->textures.tutorial_ok_button, NULL, NULL, &bw, &bh);
    
    const int space_before_button = 32;
    
    tut->rect.x = x;
    tut->rect.y = y;
    tut->rect.w = fw * largest + tut->margin*2;
    tut->rect.h = space_before_button + bh + tut->line_count * fh + tut->margin*2;
    
    tut->ok_button->x = x + tut->rect.w / 2 - bw/2;
    tut->ok_button->y = y + tut->rect.h - bh - tut->margin;
}

void tutorial_rect_close(void *ptr) {
    (void)ptr;
    
    struct Tutorial_Rect *tut = &gs->tutorial;
    tut->active = false;
}

void tutorial_rect_run() {
    struct Tutorial_Rect *tut = &gs->tutorial;
    
    if (!tut->active) return;
    
    const SDL_Color bg = (SDL_Color){32, 32, 32, 255};
    
    SDL_SetRenderDrawColor(gs->renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderFillRect(gs->renderer, &tut->rect);
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    
    int c = 0;
    int h = -1;
    
    for (int i = 0; i < tut->line_count; i++) {
        if (!*tut->lines[i]) {
            if (h == -1)
                TTF_SizeText(tut->font, "L", NULL, &h);
            c += h;
            
            continue;
        }
        
        draw_text(tut->font,
                  tut->lines[i],
                  WHITE,
                  bg,
                  false,
                  false,
                  tut->rect.x + tut->margin,
                  tut->rect.y + c + tut->margin,
                  NULL,
                  &h);
        c += h + 2;
    }
    
    
    button_tick(tut->ok_button, NULL);
    button_draw(tut->ok_button);
}