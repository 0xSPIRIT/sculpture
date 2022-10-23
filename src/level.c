Uint8 type_to_rgb_table[CELL_TYPE_COUNT*4] = {
    // Type              R    G    B
    CELL_NONE,            0,   0,   0,
    CELL_DIRT,          200,   0,   0,
    CELL_SAND,          255, 255,   0,

    CELL_WATER,           0,   0, 255,
    CELL_ICE,           188, 255, 255,
    CELL_STEAM,         225, 225, 225,

    CELL_WOOD_LOG,      128,  80,   0,
    CELL_WOOD_PLANK,    200,  80,   0,

    CELL_COBBLESTONE,   128, 128, 128,
    CELL_MARBLE,        255, 255, 255,
    CELL_SANDSTONE,     255, 128,   0,

    CELL_CEMENT,        130, 130, 130,
    CELL_CONCRETE,      140, 140, 140,

    CELL_QUARTZ,        200, 200, 200,
    CELL_GLASS,         180, 180, 180,

    CELL_GRANITE,       132, 158, 183,
    CELL_BASALT,         32,  32,  32,
    CELL_DIAMOND,       150, 200, 200,

    CELL_UNREFINED_COAL, 50,  50,  50,
    CELL_REFINED_COAL,   70,  70,  70,
    CELL_LAVA,          255,   0,   0,

    CELL_SMOKE,         170, 170, 170,
    CELL_DUST,          150, 150, 150
};

// Gets cells based on pixels in the image.
//
// path                  - path to the image
// out                   - pointer to array of Cells (it's allocated in this function)
// source_cells          - pointer to a bunch of source cells (NULL if don't want). Must not be heap allocated.
// out_source_cell_count - Pointer to the cell count. Updates in this func.
// out_w & out_h         - width and height of the image.
void level_get_cells_from_image(const char *path, struct Cell **out, struct Source_Cell *source_cells, int *out_source_cell_count, int *out_w, int *out_h) {
    SDL_Surface *surface = IMG_Load(path);
    Assert(surface);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surface);
    Assert(texture);

    int w = surface->w;
    int h = surface->h;

    *out_w = w;
    *out_h = h;

    *out = arena_alloc(gs->persistent_memory, w*h, sizeof(struct Cell));

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Uint8 r, g, b;

            Uint32 pixel = ((Uint32*)surface->pixels)[x+y*w];
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            int cell = 0;

            if (r == 255 && g == 0 && b == 0) {
                struct Source_Cell *s = &source_cells[(*out_source_cell_count)++];
                s->x = x;
                s->y = y;
                s->type = CELL_STEAM;
            } else {
                for (int i = 0; i < CELL_TYPE_COUNT; i++) {
                    SDL_Color c = {
                        type_to_rgb_table[i*4 + 1],
                        type_to_rgb_table[i*4 + 2],
                        type_to_rgb_table[i*4 + 3],
                        255
                    };
                        
                    if (r == c.r && g == c.g && b == c.b) {
                        cell = i;
                        break;
                    }
                }
            }

            (*out)[x+y*w].type = cell;
        }
    }

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int level_add(const char *name, const char *desired_image, const char *initial_image, int effect_type) {
    struct Level *level = &gs->levels[gs->level_count++];
    level->index = gs->level_count-1;
    strcpy(level->name, name);
    level->popup_time_current = 0;
    level->popup_time_max = POPUP_TIME;
    level->state = LEVEL_STATE_INTRO;
    level->effect_type = effect_type;

    int w, h;
    level_get_cells_from_image(desired_image,
                               &level->desired_grid,
                               NULL,
                               NULL,
                               &level->w,
                               &level->h);
    level_get_cells_from_image(initial_image,
                               &level->initial_grid,
                               level->source_cell,
                               &level->source_cell_count,
                               &w,
                               &h);

    Assert(w > 0);
    Assert(h > 0);

    if (w != level->w || h != level->h) {
        fprintf(stderr, "%s and %s aren't the same size. Initial: %d, Desired: %d.\n", initial_image, desired_image, w, level->w);
        exit(1);
    }

    return level->index;
}

void levels_setup() {
    level_add("Alaska",
              RES_DIR "lvl/desired/level 1.png",
              RES_DIR "lvl/initial/level 1.png",
              EFFECT_SNOW);
    level_add("Fireplace",
              RES_DIR "lvl/desired/level 2.png",
              RES_DIR "lvl/initial/level 2.png",
              EFFECT_NONE);
    level_add("Clock's Ticking",
              RES_DIR "lvl/desired/level 3.png",
              RES_DIR "lvl/initial/level 3.png",
              EFFECT_NONE);
    level_add("The Process",
              RES_DIR "lvl/desired/level 4.png",
              RES_DIR "lvl/initial/level 4.png",
              EFFECT_NONE);
    level_add("Carbon Copy",
              RES_DIR "lvl/desired/level 5.png",
              RES_DIR "lvl/initial/level 5.png",
              EFFECT_RAIN);
    level_add("Metamorphosis",
              RES_DIR "lvl/desired/level 1.png",
              RES_DIR "lvl/initial/level 1.png",
              EFFECT_NONE);
    level_add("Procedure Lullaby",
              RES_DIR "lvl/desired/level 1.png",
              RES_DIR "lvl/initial/level 1.png",
              EFFECT_NONE);
    level_add("Polished Turd",
              RES_DIR "lvl/desired/level 1.png",
              RES_DIR "lvl/initial/level 1.png",
              EFFECT_NONE);
    level_add("Showpiece",
              RES_DIR "lvl/desired/level 1.png",
              RES_DIR "lvl/initial/level 1.png",
              EFFECT_NONE);
    level_add("Glass Body",
              RES_DIR "lvl/desired/level 1.png",
              RES_DIR "lvl/initial/level 1.png",
              EFFECT_NONE);
}

void goto_level(int lvl) {
    gs->level_current = lvl;
    gs->levels[lvl].state = LEVEL_STATE_INTRO;
    
    gs->current_tool = TOOL_GRABBER;
    
    grid_init(gs->levels[lvl].w, gs->levels[lvl].h);

    gs->S = gs->window_width/gs->gw;
    Assert(gs->gw==gs->gh);

    chisel_init(&gs->chisel_small);
    chisel_init(&gs->chisel_medium);
    chisel_init(&gs->chisel_large);
    gs->chisel = &gs->chisel_small;
    
    chisel_blocker_init();
    blocker_init();
    chisel_hammer_init();
    deleter_init();
    for (int i = 0; i < PLACER_COUNT; i++)
        placer_init(i);
    grabber_init();
    gui_init();
    all_converters_init();
    overlay_init();

    effect_set(gs->levels[lvl].effect_type);

    for (int i = 0; i < gs->gw*gs->gh; i++) {
        gs->grid[i].type = gs->levels[lvl].desired_grid[i].type;
    }
}

void goto_level_string_hook(const char *string) {
    int lvl = atoi(string) - 1;

    if (lvl < 0) return;
    if (lvl >= LEVEL_COUNT) return;

    goto_level(lvl);
}

void level_tick() {
    struct Level *level = &gs->levels[gs->level_current];
    struct Input *input = &gs->input;

    if (gs->text_field.active) return;
    if (gs->gui.popup) return;
    
    switch (level->state) {
    case LEVEL_STATE_INTRO:
        level->popup_time_current++;
        if (level->popup_time_current >= level->popup_time_max) {
            level->popup_time_current = 0;
            level->state = LEVEL_STATE_PLAY;
            srand((unsigned int) time(0));

            // Reset everything except the IDs of the grid, since there's no reason to recalculate it.
            for (int i = 0; i < gs->gw*gs->gh; i++) {
                int id = gs->grid[i].id;
                gs->grid[i] = (struct Cell){.type = level->initial_grid[i].type, .rand = rand(), .id = id, .object = -1, .depth = 255};
            }
            objects_reevaluate();

            if (!gs->undo_initialized) {
                undo_system_init();
            } else {
                undo_system_reset();
            }
        }
        break;
    case LEVEL_STATE_OUTRO:
        if (input->keys[SDL_SCANCODE_N]) {
            if (gs->level_current+1 < 10) {
                goto_level(++gs->level_current);
            }
        }

        if (input->keys_pressed[SDL_SCANCODE_F]) {
            gs->levels[gs->level_current].state = LEVEL_STATE_PLAY;
            input->keys[SDL_SCANCODE_F] = 0;
        }
        break;
    case LEVEL_STATE_PLAY:
        if (input->keys_pressed[SDL_SCANCODE_F]) {
            gs->levels[gs->level_current].state = LEVEL_STATE_OUTRO;
            input->keys[SDL_SCANCODE_F] = 0;
        }

        effect_tick(&gs->current_effect);

        simulation_tick();
    
        if (!gs->paused || gs->step_one) {
            for (int i = 0; i < gs->object_count; i++) {
                object_tick(i);
            }
        }

        if (gs->step_one) {
            gs->step_one = 0;
        }

        /* if (input->my < 0) { // If the mouse is in the GUI gs->window...
         *     SDL_ShowCursor(1);
         *     if (SDL_GetCursor() != gs->normal_cursor) {
         *         SDL_SetCursor(gs->normal_cursor);
         *     }
         *     break;
         * } else if (gs->current_tool == TOOL_GRABBER) {
         *     if (SDL_GetCursor() != gs->grabber_cursor) {
         *         SDL_ShowCursor(1);
         *         SDL_SetCursor(gs->grabber_cursor);
         *     }
         * } else if (SDL_GetCursor() != gs->normal_cursor) {
         *     SDL_SetCursor(gs->normal_cursor);
         * } */
    
        //chisel_blocker_tick();
        
        if (gs->chisel_blocker_mode) break;
        
        switch (gs->current_tool) {
            case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE: {
                chisel_tick();
                break;
            }
            case TOOL_BLOCKER: {
                blocker_tick();
                break;
            }
            case TOOL_OVERLAY: {
                overlay_tick();
                break;
            }
            case TOOL_DELETER: {
                deleter_tick();
                break;
            }
            case TOOL_PLACER: {
                if (!gs->gui.popup) // We'll handle updating it in the converter
                    placer_tick(&gs->placers[gs->current_placer]);
                break;
            }
            case TOOL_GRABBER: {
                grabber_tick();
                break;
            }
        }

        break;
    }
}

void level_draw_intro() {
    struct Level *level = &gs->levels[gs->level_current];

    Assert(RenderTarget(RENDER_TARGET_GLOBAL));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL));
        
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
    SDL_RenderClear(gs->renderer);

    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (level->desired_grid[x+y*gs->gw].type == 0) continue;
            SDL_Color col = pixel_from_index(level->desired_grid, x+y*gs->gw);
            SDL_SetRenderDrawColor(gs->renderer, col.r, col.g, col.b, 255);
            SDL_RenderDrawPoint(gs->renderer, x, y);
        }
    }

    SDL_SetRenderTarget(gs->renderer, NULL);
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL), NULL, NULL);

    char name[256] = {0};
    sprintf(name, "Level %d: %s", level->index+1, level->name);

    SDL_Surface *surf = TTF_RenderText_Blended(gs->fonts.font_title, name, (SDL_Color){255,255,255,255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);

    SDL_Rect dst = {
        gs->S*gs->gw/2 - surf->w/2, gs->S*gs->gh/2 - surf->h/2,
        surf->w, surf->h
    };
    SDL_RenderCopy(gs->renderer, texture, NULL, &dst);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(texture);
}

void level_draw() {
    struct Level *level = &gs->levels[gs->level_current];

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
    SDL_RenderClear(gs->renderer);

    switch (level->state) {
    case LEVEL_STATE_INTRO:
        level_draw_intro();
        break;
    case LEVEL_STATE_OUTRO: case LEVEL_STATE_PLAY:
        SDL_Texture *prev = SDL_GetRenderTarget(gs->renderer);

        SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL));
        
        SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
        SDL_RenderClear(gs->renderer);

        grid_draw();

        effect_draw(&gs->current_effect);

        switch (gs->current_tool) {
        case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE:
            chisel_draw();
            break;
        case TOOL_DELETER:
            deleter_draw();
            break;
        case TOOL_PLACER:
            if (!gs->gui.popup) // When gui.popup = true, we draw in converter
                placer_draw(&gs->placers[gs->current_placer], false);
            break;
        }

        chisel_blocker_draw();
        blocker_draw();

        draw_blobs();
        draw_objects();

        SDL_SetRenderTarget(gs->renderer, prev);
        break;
    }
}

SDL_Color type_to_rgb(int type) {
    Assert(type < CELL_TYPE_COUNT);
    return (SDL_Color){type_to_rgb_table[4*type+1], type_to_rgb_table[4*type+2], type_to_rgb_table[4*type+3], 255};
}

int rgb_to_type(Uint8 r, Uint8 g, Uint8 b) {
    for (int i = 0; i < CELL_TYPE_COUNT; i++) {
        SDL_Color col = type_to_rgb(i);
        if (col.r == r && col.g == g && col.b == b) {
            return i;
        }
    }
    return CELL_NONE;
}

void level_output_to_png(const char *output_file) {
    SDL_Surface *surf = SDL_CreateRGBSurface(0, gs->gw, gs->gh, 32, 0, 0, 0, 0);

    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            int type = gs->grid[x+y*gs->gw].type;

            SDL_Color color = type_to_rgb(type);
            Uint32 pixel = SDL_MapRGB(surf->format, color.r, color.g, color.b);
            set_pixel(surf, x, y, pixel);
        }
    }

    Assert(IMG_SavePNG(surf, output_file) == 0);
}
