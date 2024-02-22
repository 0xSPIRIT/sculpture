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
    TTF_SizeText(tut->font->handle, tut->lines[idx], &fw, &fh);

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

static Tutorial_Rect* tutorial_rect(const char *str, Tutorial_Rect *next, bool button_active) {
    Tutorial_Rect *tut = PushSize(gs->persistent_memory, sizeof(Tutorial_Rect));

    tut->x = -1;
    tut->y = -1;

    tut->font = gs->fonts.font;
    tut->active = gs->show_tutorials;
    tut->next = next;

    gs->finished_tutorial_for_now=false;

    strncpy(tut->str, str, 64*8-1);
    memset(tut->lines, 0, MAX_TUTORIAL_LINES*64);

    tut->ok_button = button_allocate(BUTTON_TUTORIAL, &GetTexture(TEXTURE_OK_BUTTON), "", "", null);
    tut->ok_button->disabled = !button_active;
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

    calculate_tutorial_rect_size(tut);

    return tut;
}

static void tutorial_rect_close(void *ptr) {
    (void)ptr;

    Tutorial_Rect *tut = &gs->tutorial;
    tut->active = false;

    if (tut->next) {
        gs->tutorial = *tut->next;
        gs->tutorial.active = true;
        gs->tutorial_flag_1 = false; // for level 1.
    } else {
        gs->finished_tutorial_for_now = true;

        if (gs->level_current+1 == 1) {
            for (int i = 0; i < TOOL_COUNT; i++) {
                gs->gui.tool_buttons[i]->disabled = false;
            }
            gs->gui.tool_buttons[TOOL_PLACER]->disabled = true;

            gui_wasd_popup_init(gs->level_current);
        }
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

    if (gs->should_update) {
        // special case for the first level
        if (tut->ok_button->disabled && gs->level_current+1 == 1) {
            if (strcmp(TUTORIAL_OVERLAY_STRING, tut->str)==0 && gs->clicked_overlay_button && gs->hovered_over_overlay) {
                tut->ok_button->disabled = false;
                tut->ok_button->green_timer = 0.5;
                play_sound(AUDIO_CHANNEL_PING, gs->audio.sprinkle, 0);
            } else if (strcmp(TUTORIAL_CHISEL_STRING, tut->str)==0 && gs->did_rotate_chisel) {
                tut->ok_button->disabled = false;
                tut->ok_button->green_timer = 0.5;
                play_sound(AUDIO_CHANNEL_PING, gs->audio.sprinkle, 0);
            }
        }

        if (!tut->ok_button->disabled) {
            if (gs->input.keys_pressed[SDL_SCANCODE_RETURN] || gs->input.keys_pressed[SDL_SCANCODE_SPACE]) {
                tutorial_rect_close(null);
            }
        }
    }

    calculate_tutorial_rect_size(tut);

    const SDL_Color bg = (SDL_Color){0, 0, 0, 255};

    RenderColor(bg.r, bg.g, bg.b, 255);
    RenderFillRect(target, tut->rect);
    RenderColor(127, 127, 127, 255);
    RenderDrawRect(target, tut->rect);

    RenderColor(255, 255, 255, 255);

    int cum = 0;
    SDL_Rect dst;

    for (int i = 0; i < tut->line_count; i++) {
        if (!*tut->lines[i]) {
            cum += tut->font->char_height;
            continue;
        }

        dst.x = tut->rect.x + tut->margin;
        dst.y = tut->rect.y + cum + tut->margin;

        char identifier[64];
        sprintf(identifier, "tut%d", i);

        RenderColor(255, 255, 255, 255);
        RenderTextQuick(target,
                        identifier,
                        tut->font,
                        tut->lines[i],
                        WHITE,
                        dst.x,
                        dst.y,
                        &dst.w,
                        &dst.h,
                        false);

        cum += dst.h + 2;
    }

    button_tick(tut->ok_button, null);
    button_draw(target, tut->ok_button);
}

static void check_for_tutorial(bool info_button) {
    int l = gs->level_current;

    switch (l+1) {
        case 1: {
            Tutorial_Rect *t4 = tutorial_rect(TUTORIAL_UNDO_STRING, null, true);
            Tutorial_Rect *t3 = tutorial_rect(TUTORIAL_COMPLETE_LEVEL, t4, true);
            Tutorial_Rect *t2 = tutorial_rect(TUTORIAL_CHISEL_STRING,  t3, false);
            Tutorial_Rect *t1 = tutorial_rect(TUTORIAL_OVERLAY_STRING, t2, false);
            gs->tutorial = *t1;

            gs->tutorial_flag_1 = true; // disable all tools except overlay

            for (int i = 0; i < TOOL_COUNT; i++) {
                gs->gui.tool_buttons[i]->disabled = true;
            }

            gs->gui.tool_buttons[TOOL_OVERLAY]->highlighted = true;
            gs->gui.tool_buttons[TOOL_OVERLAY]->disabled = false;
            gs->gui.tool_buttons[TOOL_POINTER]->highlighted = true;
            gs->gui.tool_buttons[TOOL_POINTER]->disabled = false;
            break;
        }
        case 4: {
            Tutorial_Rect *t2;
            if (info_button) {
                Tutorial_Rect *t3 = tutorial_rect(TUTORIAL_RECTANGLE_PLACE, null, true);
                t2 = tutorial_rect(TUTORIAL_PLACER_F_KEYS, t3, true);
            } else {
                t2 = tutorial_rect(TUTORIAL_PLACER_F_KEYS, null, true);
            }
            Tutorial_Rect *t1 = tutorial_rect(TUTORIAL_PLACER_STRING, t2, true);
            gs->tutorial = *t1;
            break;
        }
    }
}
