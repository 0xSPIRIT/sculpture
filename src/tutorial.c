struct Tutorial_Rect* tutorial_rect(const char *str,
                                    int x,
                                    int y,
                                    struct Tutorial_Rect *next)
{
    struct Tutorial_Rect *tut = PushSize(gs->persistent_memory, sizeof(struct Tutorial_Rect));
    
    tut->font = gs->fonts.font;
    tut->active = gs->show_tutorials;
    tut->next = next;
    
    strcpy(tut->str, str);
    memset(tut->lines, 0, MAX_TUTORIAL_LINES*64);
    
    int i = 0;
    
    tut->line_count = 0;
    
    tut->margin = Scale(8);
    
    int l = 0;
    int largest = 0;
    while (*str) {
        if (*str == '\n') {
            tut->line_count++;
            ++str;
            if (i > l) {
                l=i;
                largest = tut->line_count-1;
            }
            i = 0;
            continue;
        }
        tut->lines[tut->line_count][i++] = *str;
        ++str;
    }
    
    int fw=0, fh=0;
    TTF_SizeText(tut->font, tut->lines[largest], &fw, &fh);
    
    Assert(fw);
    Assert(fh);
    
    tut->line_count++;
    
    tut->ok_button = button_allocate(BUTTON_TYPE_TUTORIAL, gs->textures.tutorial_ok_button, "", NULL);
    tut->ok_button->w = Scale(tut->ok_button->w);
    tut->ok_button->h = Scale(tut->ok_button->h);
    
    int bw, bh;
    SDL_QueryTexture(gs->textures.tutorial_ok_button, NULL, NULL, &bw, &bh);
    
    const int space_before_button = Scale(32);
    
    tut->rect.x = x;
    tut->rect.y = y;
    tut->rect.w = fw + tut->margin*2;
    tut->rect.h = space_before_button + bh + (tut->line_count-1) * fh + tut->margin*2;
    
    tut->ok_button->x = x + tut->rect.w / 2 - bw/2;
    tut->ok_button->y = y + tut->rect.h - bh - tut->margin;
    
    return tut;
}

void tutorial_rect_close(void *ptr) {
    (void)ptr;
    
    struct Tutorial_Rect *tut = &gs->tutorial;
    tut->active = false;
    
    for (int i = 0; i < MAX_TUTORIAL_LINES; i++) {
        SDL_DestroyTexture(tut->textures[i]);
        tut->textures[i] = 0;
    }
    
    if (tut->next) {
        gs->tutorial = *tut->next;
        gs->tutorial.active = true;
    }
}

void tutorial_rect_run() {
    struct Tutorial_Rect *tut = &gs->tutorial;
    
    if (!tut->active) return;
    
    if (gs->input.keys_pressed[SDL_SCANCODE_RETURN]) {
        tutorial_rect_close(NULL);
    }
    
    const SDL_Color bg = (SDL_Color){0, 0, 0, 255};
    
    SDL_SetRenderDrawColor(gs->renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderFillRect(gs->renderer, &tut->rect);
    SDL_SetRenderDrawColor(gs->renderer, 127, 127, 127, 255);
    SDL_RenderDrawRect(gs->renderer, &tut->rect);
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
    
    int c = 0;
    SDL_Rect dst;
    dst.h = -1;
    
    for (int i = 0; i < tut->line_count; i++) {
        if (!*tut->lines[i]) {
            if (dst.h == -1)
                TTF_SizeText(tut->font, "L", NULL, &dst.h);
            c += dst.h;
            continue;
        }
        
        if (!tut->textures[i]) {
            (void)bg;
            SDL_Surface *surf = TTF_RenderText_Blended(tut->font, tut->lines[i], WHITE);
            tut->textures[i] = SDL_CreateTextureFromSurface(gs->renderer, surf);
            SDL_FreeSurface(surf);
        }
        
        SDL_QueryTexture(tut->textures[i], NULL, NULL, &dst.w, &dst.h);
        
        dst.x = tut->rect.x + tut->margin;
        dst.y = tut->rect.y + c + tut->margin;
        
        SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
        SDL_RenderCopy(gs->renderer, tut->textures[i], NULL, &dst);
        
        c += dst.h + 2;
    }
    
    
    button_tick(tut->ok_button, NULL);
    button_draw(tut->ok_button);
}

void check_for_tutorial() {
    int l = gs->level_current;
    
    switch (l+1) {
        case 1: {
            struct Tutorial_Rect *t3 = tutorial_rect(TUTORIAL_COMPLETE_LEVEL,
                                                     32,
                                                     GUI_H+32,
                                                     NULL);
            
            struct Tutorial_Rect *t2 = tutorial_rect(TUTORIAL_CHISEL_STRING,
                                                     32,
                                                     GUI_H+32,
                                                     t3);
            
            struct Tutorial_Rect *t1 = tutorial_rect(TUTORIAL_OVERLAY_STRING,
                                                     32,
                                                     GUI_H+32,
                                                     t2);
            gs->tutorial = *t1;
            
            gs->gui.tool_buttons[TOOL_OVERLAY]->highlighted = true;
            break;
        }
        case 4: {
            gs->tutorial = *tutorial_rect(TUTORIAL_PLACER_STRING,
                                          32,
                                          GUI_H+32,
                                          NULL);
            break;
        }
        case 6: {
            gs->tutorial = *tutorial_rect(TUTORIAL_CHISEL_INVENTORY_STRING,
                                          32,
                                          GUI_H+32,
                                          NULL);
            break;
        }
        case 8: {
            gs->tutorial = *tutorial_rect(TUTORIAL_CAREFUL_STRING,
                                          32,
                                          GUI_H+32,
                                          NULL);
        }
    }
}