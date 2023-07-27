static void calculate_tutorial_rect_size(Tutorial_Rect *tut) {
    tut->margin = Scale(8);

    int fw=0, fh=0;
    int largest = 0, idx=0;
    for (int i = 0; i < tut->line_count; i++) {
        if (strlen(tut->lines[i]) > largest) {
            largest = (int)strlen(tut->lines[i]);
            idx = i;
        }
    }
    TTF_SizeText(tut->font, tut->lines[idx], &fw, &fh);

    Assert(fw);
    Assert(fh);

    const int space_before_button = Scale(32);

    tut->rect.x = tut->x * gs->game_width;
    tut->rect.y = tut->y * gs->game_height;
    tut->rect.w = fw + tut->margin*2;
    tut->rect.h = space_before_button + tut->ok_button->h + (tut->line_count-1) * fh + tut->margin*2;

    if (tut->x == -1)
        tut->rect.x = gs->game_width/2 - tut->rect.w/2;
    if (tut->y == -1)
        tut->rect.y = (gs->game_height-GUI_H)/2 - tut->rect.h/2;

    // For debugging
    SDL_Rect r = tut->rect;
    (void)r;

    tut->ok_button->x = tut->rect.x + tut->rect.w / 2 - tut->ok_button->w/2;
    tut->ok_button->y = tut->rect.y + tut->rect.h - tut->ok_button->h - tut->margin;
}

static Tutorial_Rect* tutorial_rect(const char *str, Tutorial_Rect *next) {
    Tutorial_Rect *tut = PushSize(gs->persistent_memory, sizeof(Tutorial_Rect));

    tut->font = gs->fonts.font->handle;
    tut->active = gs->show_tutorials;
    tut->next = next;

    strcpy(tut->str, str);
    memset(tut->lines, 0, MAX_TUTORIAL_LINES*64);

    tut->ok_button = button_allocate(BUTTON_TUTORIAL, &GetTexture(TEXTURE_OK_BUTTON), "", null);
    tut->ok_button->w = Scale(tut->ok_button->w);
    tut->ok_button->h = Scale(tut->ok_button->h);

    int i = 0;

    tut->line_count = 0;

    while (*str) {
        if (*str == '\n') {
            tut->line_count++;
            ++str;
            i = 0;
            continue;
        }
        tut->lines[tut->line_count][i++] = *str;
        ++str;
    }

    tut->x = -1;
    tut->y = -1;

    calculate_tutorial_rect_size(tut);

    return tut;
}

static void tutorial_rect_close(void *ptr) {
    (void)ptr;

    Tutorial_Rect *tut = &gs->tutorial;
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

static void tutorial_rect_run(int target) {
    Assert(target == RENDER_TARGET_MASTER);

    Tutorial_Rect *tut = &gs->tutorial;

    if (!SHOW_TUTORIAL) {
        while (tut->active)
            tutorial_rect_close(null);
        return;
    }

    if (!tut->active) return;

    if (gs->input.keys_pressed[SDL_SCANCODE_RETURN] || gs->input.keys_pressed[SDL_SCANCODE_SPACE]) {
        tutorial_rect_close(null);
    }

    calculate_tutorial_rect_size(tut);

    const SDL_Color bg = (SDL_Color){0, 0, 0, 255};

    RenderColor(bg.r, bg.g, bg.b, 255);
    RenderFillRect(target, tut->rect);
    RenderColor(127, 127, 127, 255);
    RenderDrawRect(target, tut->rect);

    RenderColor(255, 255, 255, 255);

    int c = 0;
    SDL_Rect dst;
    dst.h = -1;

    for (int i = 0; i < tut->line_count; i++) {
        if (!*tut->lines[i]) {
            if (dst.h == -1)
                TTF_SizeText(tut->font, "L", null, &dst.h);
            c += dst.h;
            continue;
        }

        // TODO: Change to the new font renderer.
        if (!tut->textures[i]) {
            (void)bg;
            SDL_Surface *surf = TTF_RenderText_Blended(tut->font, tut->lines[i], WHITE);
            tut->textures[i] = SDL_CreateTextureFromSurface(gs->renderer, surf);
            tut->texture_rects[i].w = surf->w;
            tut->texture_rects[i].h = surf->h;
            SDL_FreeSurface(surf);
        }

        dst.w = tut->texture_rects[i].w;
        dst.h = tut->texture_rects[i].h;
        dst.x = tut->rect.x + tut->margin;
        dst.y = tut->rect.y + c + tut->margin;

        RenderColor(255, 255, 255, 255);
        SDL_RenderCopy(gs->renderer, tut->textures[i], null, &dst);

        c += dst.h + 2;
    }

    button_tick(tut->ok_button, null);
    button_draw(target, tut->ok_button);
}

static void check_for_tutorial() {
    int l = gs->level_current;

    switch (l+1) {
        case 1: {
            Tutorial_Rect *t4 = tutorial_rect(TUTORIAL_COMPLETE_LEVEL, null);
            Tutorial_Rect *t3 = tutorial_rect(TUTORIAL_UNDO_STRING,    t4);
            Tutorial_Rect *t2 = tutorial_rect(TUTORIAL_CHISEL_STRING,  t3);
            Tutorial_Rect *t1 = tutorial_rect(TUTORIAL_OVERLAY_STRING, t2);
            gs->tutorial = *t1;

            gs->gui.tool_buttons[TOOL_OVERLAY]->highlighted = true;
            break;
        }
        case 4: {
            Tutorial_Rect *t2 = tutorial_rect(TUTORIAL_PLACER_F_KEYS, null);
            Tutorial_Rect *t1 = tutorial_rect(TUTORIAL_PLACER_STRING, t2);
            gs->tutorial = *t1;
            break;
        }
    }
}
