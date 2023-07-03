static void preview_start_recording(Preview *p, const char *name) {
    strcpy(p->name, name);
    p->length = 0;
    memset(p->states, 0, sizeof(Preview_State)*MAX_PREVIEW_STATES);
    p->recording = true;
}

static void preview_finish_recording(Preview *p) {
    char filename[64] = {0};
    sprintf(filename, RES_DIR "previews/%s", p->name);

    FILE *fp = fopen(filename, "wb");

    p->recording = false;
    p->play = true;

    fprintf(fp, "%d\n", p->length);
    fwrite((const void*)gs->overlay.grid,
           sizeof(int),
           PREVIEW_GRID_SIZE,
           fp);
    fwrite((const void*)p->states,
           sizeof(Preview_State),
           p->length,
           fp);

    fclose(fp);
}

static void preview_load(Preview *p, const char *file) {
    FILE *fp = fopen(file, "rb");

    memset(p, 0, sizeof(Preview));
    p->recording = false;

    fscanf(fp, "%d\n", &p->length);
    fread(p->overlay,
          sizeof(int),
          PREVIEW_GRID_SIZE,
          fp);
    fread(p->states,
          sizeof(Preview_State),
          p->length,
          fp);

    fclose(fp);
}

static void previews_load(void) {
    preview_load(&gs->tool_previews[TOOL_CHISEL_SMALL],
                 RES_DIR "previews/small_chisel.bin");
    preview_load(&gs->tool_previews[TOOL_CHISEL_MEDIUM],
                 RES_DIR "previews/medium_chisel.bin");
    preview_load(&gs->tool_previews[TOOL_CHISEL_LARGE],
                 RES_DIR "previews/large_chisel.bin");
    preview_load(&gs->tool_previews[TOOL_PLACER],
                 RES_DIR "previews/placer.bin");
}

static void preview_record(Preview *p) {
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
            p->states[p->length].angle = gs->placers[gs->current_placer].rect.x != -1;
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

static void preview_draw(int target, Preview *p, int dx, int dy, int scale) {
    const int preview_w = 64;
    const int preview_h = 64;
    
    Assert(preview_w * preview_h == PREVIEW_GRID_SIZE);

    {
        for (int y = 0; y < preview_w; y++) {
            for (int x = 0; x < preview_h; x++) {
                if (!p->states[p->index].grid[x+y*preview_w]) {
                    RenderColor(0, 0, 0, 255);
                } else {
                    SDL_Color col = pixel_from_index(p->states[p->index].grid[x+y*preview_w], x+y*preview_w);
                    RenderColor(col.r, col.g, col.b, 255); // 255 on this because desired_grid doesn't have depth set.
                }

                RenderPoint(RENDER_TARGET_PREVIEW, x, y);
            }
        }

        for (int y = 0; y < preview_h; y++) {
            for (int x = 0; x < preview_w; x++) {
                int t = p->overlay[x+y*preview_w];
                if (!t) continue;

                RenderColor(255, 255, 255, 127);
                RenderPoint(RENDER_TARGET_PREVIEW, x, y);
            }
        }

        RenderColor(255, 255, 0, 255);

        int tool = p->states[p->index].tool;

        switch (tool) {
            case TOOL_PLACER: {
                int x = p->states[p->index].x;
                int y = p->states[p->index].y;
                bool is_rect = p->states[p->index].angle == 1;

                RenderColor(255, 255, 255, 64);

                SDL_RenderDrawLine(gs->renderer, 0, y, preview_w, y);
                SDL_RenderDrawLine(gs->renderer, x, 0, x, preview_h);

                if (is_rect) {
                    if (p->placer_rect.x == -1) {
                        p->placer_rect.x = x;
                        p->placer_rect.y = y;
                    }
                    p->placer_rect.w = 1+x - p->placer_rect.x;
                    p->placer_rect.h = 1+y - p->placer_rect.y;

                    RenderColor(255, 0, 0, 255);
                    RenderDrawRect(RENDER_TARGET_PREVIEW, p->placer_rect);
                } else {
                    p->placer_rect.x = p->placer_rect.y = -1;
                }
                break;
            }
            case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE: {
                int x = p->states[p->index].x;
                int y = p->states[p->index].y;
                int angle = p->states[p->index].angle;

                Chisel *chisel = NULL;
                if (tool == TOOL_CHISEL_SMALL)  chisel = &gs->chisel_small;
                if (tool == TOOL_CHISEL_MEDIUM) chisel = &gs->chisel_medium;
                if (tool == TOOL_CHISEL_LARGE)  chisel = &gs->chisel_large;
                Assert(chisel);

                // Disgusting hardcoding to adjust the weird rotation SDL does.

                //chisel_get_adjusted_positions(angle, tool, &x, &y);
                if (angle == 270 && tool == TOOL_CHISEL_SMALL) {
                    y++;
                }

                SDL_Rect dst = {
                    x, y - chisel->texture->height/2,
                    chisel->texture->width, chisel->texture->height
                };
                SDL_Point center = { 0, chisel->texture->height/2 };

                RenderTextureEx(RENDER_TARGET_PREVIEW,
                                chisel->texture,
                                NULL,
                                &dst,
                                angle,
                                &center,
                                SDL_FLIP_NONE);
                break;
            }
        }
    }

    SDL_Rect target_dst = {
        dx, dy+GUI_H,
        scale*gs->gw, scale*gs->gh
    };

    RenderTargetToTarget(target,
                         RENDER_TARGET_PREVIEW,
                         NULL,
                         &target_dst);

    p->index++;
    if (p->index >= p->length) p->index = 0;
}

static void preview_start_current(const char *name) {
    gs->current_preview.recording = true;
    preview_start_recording(&gs->current_preview, name);
}

static void preview_load_current(const char *name) {
    char file[64];
    sprintf(file, RES_DIR "previews/%s", name);
    preview_load(&gs->current_preview, file);
}

static void preview_tick() {
#ifndef ALASKA_RELEASE_MODE
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
#endif
}
