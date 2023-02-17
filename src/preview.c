void preview_start_recording(struct Preview *p, const char *name) {
    strcpy(p->name, name);
    p->length = 0;
    memset(p->states, 0, sizeof(struct Preview_State)*MAX_PREVIEW_STATES);
    p->recording = true;
}

void preview_finish_recording(struct Preview *p) {
    char filename[64] = {0};
    sprintf(filename, RES_DIR "previews/%s", p->name);
    
    FILE *fp = fopen(filename, "wb");
    
    p->recording = false;
    p->play = true;
    
    fprintf(fp, "%d\n", p->length);
    fwrite((const void*)p->states,
           sizeof(struct Preview_State),
           p->length,
           fp);
    
    fclose(fp);
}

void preview_load(struct Preview *p, const char *file) {
    FILE *fp = fopen(file, "rb");
    
    memset(p, 0, sizeof(struct Preview));
    p->recording = false;
    
    fscanf(fp, "%d\n", &p->length);
    fread(p->states,
          sizeof(struct Preview_State),
          p->length,
          fp);
    
    fclose(fp);
}

void previews_load(void) {
    preview_load(&gs->tool_previews[TOOL_CHISEL_SMALL],
                 RES_DIR "previews/small_chisel.bin");
#if 0
    preview_load(&gs->tool_previews[TOOL_CHISEL_MEDIUM],
                 RES_DIR "previews/medium_chisel.bin");
    preview_load(&gs->tool_previews[TOOL_CHISEL_LARGE],
                 RES_DIR "previews/large_chisel.bin");
    preview_load(&gs->tool_previews[TOOL_PLACER],
                 RES_DIR "previews/placer.bin");
#endif
}

void preview_record(struct Preview *p) {
    if (p->length > MAX_PREVIEW_STATES) {
        preview_finish_recording(p);
        return;
    }
    
    p->states[p->length].tool = gs->current_tool;
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        p->states[p->length].grid[i] = (Uint8)gs->grid[i].type;
    }
    
    switch (gs->current_tool) {
        case TOOL_PLACER: {
            p->states[p->length].x = gs->placers[gs->current_placer].x;
            p->states[p->length].y = gs->placers[gs->current_placer].y;
            break;
        }
        case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE: {
            p->states[p->length].x = gs->chisel->x;
            p->states[p->length].y = gs->chisel->y;
            p->states[p->length].angle = gs->chisel->angle;
            break;
        }
    }
    
    p->length++;
}

void preview_draw(struct Preview *p, int dx, int dy, int scale) {
    Assert(gs->gw == 64);
    Assert(gs->gh == 64);
    
    SDL_Texture *prev = SDL_GetRenderTarget(gs->renderer);
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_PREVIEW));
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (!p->states[p->index].grid[x+y*gs->gw]) {
                SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
            } else {
                SDL_Color col = pixel_from_index(p->states[p->index].grid[x+y*gs->gw], x+y*gs->gw);
                SDL_SetRenderDrawColor(gs->renderer, col.r, col.g, col.b, 255); // 255 on this because desired_grid doesn't have depth set.
            }
            
            SDL_RenderDrawPoint(gs->renderer, x, y);
        }
    }
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 0, 255);
    
    int tool = p->states[p->index].tool;
    
    switch (tool) {
        case TOOL_PLACER: {
            int x = p->states[p->index].x;
            int y = p->states[p->index].y;
            
            SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 64);
            
            SDL_RenderDrawLine(gs->renderer, 0, y, gs->gw, y);
            SDL_RenderDrawLine(gs->renderer, x, 0, x, gs->gh);
            break;
        }
        case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE: {
            int x = p->states[p->index].x;
            int y = p->states[p->index].y;
            int angle = p->states[p->index].angle;
            
            struct Chisel *chisel = NULL;
            if (tool == TOOL_CHISEL_SMALL)  chisel = &gs->chisel_small;
            if (tool == TOOL_CHISEL_MEDIUM) chisel = &gs->chisel_medium;
            if (tool == TOOL_CHISEL_LARGE)  chisel = &gs->chisel_large;
            Assert(chisel);
            
            // Disgusting hardcoding to adjust the weird rotation SDL does.
            chisel_get_adjusted_positions(angle, tool, &x, &y);
            
            const SDL_Rect dst = {
                x, y - chisel->h/2,
                chisel->w, chisel->h
            };
            const SDL_Point center = { 0, chisel->h/2 };
            
            SDL_RenderCopyEx(gs->renderer,
                             chisel->texture,
                             NULL,
                             &dst,
                             angle,
                             &center,
                             SDL_FLIP_NONE);
            break;
        }
    }
    
    SDL_SetRenderTarget(gs->renderer, prev);
    
    SDL_Rect target_dst = {
        dx, dy+GUI_H,
        scale*gs->gw, scale*gs->gh
    };
    
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_PREVIEW), NULL, &target_dst);
    
    p->index++;
    if (p->index >= p->length) p->index = 0;
}

void preview_start_current(const char *name) {
    gs->current_preview.recording = true;
    preview_start_recording(&gs->current_preview, name);
}

void preview_load_current(const char *name) {
    char file[64];
    sprintf(file, RES_DIR "previews/%s", name);
    preview_load(&gs->current_preview, file);
}

void preview_tick() {
    if (gs->input.keys_pressed[SDL_SCANCODE_END]) {
        set_text_field("Load Preview File:", "", preview_load_current);
    } else if (gs->input.keys_pressed[SDL_SCANCODE_HOME]) {
        if (gs->input.keys[SDL_SCANCODE_LCTRL]) {
            gs->current_preview.play = false;
        } else if (gs->current_preview.recording) {
            preview_finish_recording(&gs->current_preview);
            SDL_SetWindowTitle(gs->window, "Alaska");
        } else {
            set_text_field("Save Preview File As:", "", preview_start_current);
        }
    }
    
    if (gs->current_preview.recording) {
        preview_record(&gs->current_preview);
    }
}
