static void preview_start_recording(Preview *p, const char *name) {
    strcpy(p->name, name);
    p->length = 0;
    memset(p->states, 0, sizeof(Preview_State)*MAX_PREVIEW_STATES);
    p->recording = true;
}

static void preview_finish_recording(Preview *p) {
    char filename[64] = {0};
    sprintf(filename, DATA_DIR "previews/%s", p->name);

    FILE *fp = fopen(filename, "wb");
    Assert(fp);

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

    p->recording = false;
    p->play = true;
}

static void preview_load(Preview *p, const char *file) {
    FILE *fp = fopen(file, "rb");
    Assert(fp);

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
                 DATA_DIR "previews/small_chisel.bin");
    preview_load(&gs->tool_previews[TOOL_CHISEL_MEDIUM],
                 DATA_DIR "previews/medium_chisel.bin");
    preview_load(&gs->tool_previews[TOOL_CHISEL_LARGE],
                 DATA_DIR "previews/large_chisel.bin");
    preview_load(&gs->tool_previews[TOOL_PLACER],
                 DATA_DIR "previews/placer.bin");
}

static void preview_record(Preview *p) {
    if (p->length >= MAX_PREVIEW_STATES) {
        preview_finish_recording(p);
        return;
    }

    Preview_State *state = &p->states[p->length];

    state->tool = gs->current_tool;

    const int w = 64;
    const int h = 64;

    for (int i = 0; i < w*h; i++) {
        int index = 32 + (i%w) + (i/w)*gs->gw;
        state->grid[i] = (u8)gs->grid[index].type;
    }

    switch (gs->current_tool) {
        case TOOL_PLACER: {
            Placer *placer = &gs->placers[gs->current_placer];
            state->x = placer->x - 32;
            state->y = placer->y;
            if (placer->rect.x == -1) {
                state->data = 0xDEAD; // 0xDEAD represents an invalid rect.
            } else {
                // placer->rect.x == placer->place_width

                state->data = placer->place_width; // Value represents the rect size.
                // If you forgot, the placer's height can be calculated as:
                //   height = width * placer->place_aspect;
            }
            break;
        }
        case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE: {
            state->x = round(gs->chisel->draw_x) - 32;
            state->y = round(gs->chisel->draw_y);
            state->data = gs->chisel->angle+180;
            break;
        }
    }

    p->length++;
}

static SDL_Rect preview_draw(int final_target, Preview *p, int dx, int dy, int scale, bool alpha_background, bool dont_draw) {
    const int preview_w = 64;
    const int preview_h = 64;

    const int target = RENDER_TARGET_PREVIEW;

    Assert(preview_w * preview_h == PREVIEW_GRID_SIZE);

    RenderColor(0,0,0,0);
    RenderClear(target);

    for (int y = 0; y < preview_w; y++) {
        for (int x = 0; x < preview_h; x++) {
            if (alpha_background && !p->states[p->index].grid[x+y*preview_w]) {
                continue;
            } else if (!p->states[p->index].grid[x+y*preview_w]) {
                RenderColor(0, 0, 0, 255);
            } else {
                SDL_Color col = pixel_from_index(p->states[p->index].grid[x+y*preview_w], x+y*preview_w);
                RenderColor(col.r, col.g, col.b, 255); // 255 on this because desired_grid doesn't have depth set.
            }

            RenderPoint(target, x, y);
        }
    }

    for (int y = 0; y < preview_h; y++) {
        for (int x = 0; x < preview_w; x++) {
            int t = p->overlay[x+y*preview_w];

            if (!t) continue;

            RenderColor(255, 255, 255, 127);
            RenderPoint(target, x, y);
        }
    }

    RenderColor(255, 255, 0, 255);

    int tool = p->states[p->index].tool;

    switch (tool) {
        case TOOL_PLACER: {
            int x = p->states[p->index].x;
            int y = p->states[p->index].y;
            bool is_rect = (p->states[p->index].data != 0xDEAD);

            RenderColor(255, 255, 255, 64);

            if (is_rect) {
                int width = p->states[p->index].data;
                int height = width * gs->placers[gs->current_placer].place_aspect;

                SDL_Rect rect = {
                    x-width/2,
                    y-height/2,
                    width,
                    height
                };
                RenderDrawRect(target, rect);
            }

            break;
        }
        case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE: {
            int x = p->states[p->index].x;
            int y = p->states[p->index].y;
            int angle = p->states[p->index].data;

            Chisel *chisel = &gs->chisels[tool - TOOL_CHISEL_SMALL];

            bool is_diagonal = false;
            if (angle % 90 == 0) {
                chisel->texture = chisel->textures.straight;
            } else {
                chisel->texture = chisel->textures.diagonal;
                is_diagonal = true;
                angle += 45;
            }

            SDL_FRect dst = {
                x, y - chisel->texture->height/2, // integer divide
                chisel->texture->width, chisel->texture->height
            };

            f32 cx, cy;
            chisel_get_adjusted_positions(chisel->texture->height, is_diagonal, &cx, &cy);
            dst.x += cx;
            dst.y += cy;

            SDL_FPoint center = chisel_get_center_of_rotation(is_diagonal, chisel->texture->height);

            RenderTextureExRelativeF(target,
                                     chisel->texture,
                                     null,
                                     &dst,
                                     angle,
                                     &center,
                                     SDL_FLIP_NONE);

            RenderColor(127, 127, 127, 255);
            RenderPoint(target, x, y);

            break;
        }
    }

    p->index++;
    if (p->index >= p->length) p->index = 0;

    SDL_Rect target_dst = {
        dx,
        dy+GUI_H,
        scale*PREVIEW_GRID_W,
        scale*PREVIEW_GRID_W
    };

    if (dont_draw) return target_dst;

    RenderTargetToTarget(final_target,
                         target,
                         null,
                         &target_dst);

    return (SDL_Rect){0};
}

static void preview_start_current(const char *name) {
    gs->current_preview.recording = true;
    preview_start_recording(&gs->current_preview, name);
}

static void preview_load_current(const char *name) {
    char file[64];
    sprintf(file, DATA_DIR "previews/%s", name);
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
