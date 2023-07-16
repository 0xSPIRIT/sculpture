static void level_setup_initial_grid(void) {
    // Reset everything except the IDs of the grid, since there's no reason to recalculate it.
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        int id = gs->grid[i].id;
        gs->grid[i] = (Cell){
            .type = gs->levels[gs->level_current].initial_grid[i].type,
            .rand = rand(),
            .id = id,
            .object = -1,
            .is_initial = true
        };
    }
    objects_reevaluate();
}

static void play_level_end_sound(int level) {
    Mix_Chunk *sound = null;
    
    if (level+1 != 7) {
        sound = gs->audio.sprinkle;
    } else {
        sound = gs->audio.macabre;
    }
    
    Assert(sound);
    Mix_PlayChannel(AUDIO_CHANNEL_GUI, sound, 0);
}

static int level_add(const char *name, const char *desired_image, const char *initial_image, int effect_type) {
    Level *level = &gs->levels[gs->level_count++];

    level->index = gs->level_count-1;
    strcpy(level->name, name);
    level->popup_time_current = 0;
    level->popup_time_max = POPUP_TIME;
    level->state = LEVEL_STATE_NARRATION;
    level->effect_type = effect_type;

    int w, h;
    level_get_cells_from_image(desired_image,
                               &level->desired_grid,
                               null,
                               null,
                               &level->w,
                               &level->h);
    level_get_cells_from_image(initial_image,
                               &level->initial_grid,
                               level->source_cell,
                               &level->source_cell_count,
                               &w,
                               &h);
    level->default_source_cell_count = level->source_cell_count;

    memcpy(&level->default_source_cell,
           &level->source_cell,
           sizeof(Source_Cell)*SOURCE_CELL_MAX);

    Assert(w > 0);
    Assert(h > 0);

    if (w != level->w || h != level->h) {
        Error("%s and %s aren't the same size. Initial W: %d, Desired W: %d.\n", initial_image, desired_image, w, level->w);
        Assert(0);
    }

    return level->index;
}

static void levels_setup(void) {
    level_add("Marrow",
              RES_DIR "lvl/desired/level 1.png",
              RES_DIR "lvl/initial/level 1.png",
              EFFECT_SNOW);
    level_add("Alaska",
              RES_DIR "lvl/desired/level 2.png",
              RES_DIR "lvl/initial/level 2.png",
              EFFECT_SNOW);
    level_add("Form",
              RES_DIR "lvl/desired/level 3.png",
              RES_DIR "lvl/initial/level 3.png",
              EFFECT_SNOW);
    level_add("Transformation",
              RES_DIR "lvl/desired/level 4.png",
              RES_DIR "lvl/initial/level 4.png",
              EFFECT_NONE);
    level_add("Reformation",
              RES_DIR "lvl/desired/level 5.png",
              RES_DIR "lvl/initial/level 5.png",
              EFFECT_NONE);
    level_add("Monster",
              RES_DIR "lvl/desired/level 6.png",
              RES_DIR "lvl/initial/level 6.png",
              EFFECT_RAIN);
    level_add("Monster (II)",
              RES_DIR "lvl/desired/level 7.png",
              RES_DIR "lvl/initial/level 7.png",
              EFFECT_RAIN);
    level_add("Metamorphosis",
              RES_DIR "lvl/desired/level 8.png",
              RES_DIR "lvl/initial/level 8.png",
              EFFECT_NONE);
    level_add("Yearning",
              RES_DIR "lvl/desired/level 9.png",
              RES_DIR "lvl/initial/level 9.png",
              EFFECT_NONE);
    level_add("Showpiece",
              RES_DIR "lvl/desired/level 10.png",
              RES_DIR "lvl/initial/level 10.png",
              EFFECT_NONE);
    level_add("Glass Body",
              RES_DIR "lvl/desired/level 11.png",
              RES_DIR "lvl/initial/level 11.png",
              EFFECT_RAIN);
}

static void goto_level(int lvl) {
    gs->level_current = lvl;

    gs->render.view.x = gs->render.to.x = 0;
    gs->render.view.y = gs->render.to.y = 0;

    gs->levels[lvl].first_frame_compare = false;

    grid_init(gs->levels[lvl].w, gs->levels[lvl].h);

    tooltip_reset(&gs->gui.tooltip);
    narrator_init(gs->level_current);

    gs->background = background_init();

    gs->levels[lvl].popup_time_current = 0;

    memcpy(&gs->levels[lvl].source_cell,
           &gs->levels[lvl].default_source_cell,
           sizeof(Source_Cell)*SOURCE_CELL_MAX);
    gs->levels[lvl].source_cell_count = gs->levels[lvl].default_source_cell_count;

    gs->converter.calculated_render_target = false;

    gs->current_tool = TOOL_GRABBER;

    //gs->S = (f64)gs->window_width/(f64)gs->gw;
    Assert(gs->gw==gs->gh);

    gs->item_holding = (Item){0};
    gs->current_placer = 0;

    dust_init();

    gs->chisel_small  = chisel_init(CHISEL_SMALL);
    gs->chisel_medium = chisel_init(CHISEL_MEDIUM);
    gs->chisel_large  = chisel_init(CHISEL_LARGE);
    
    gs->chisel = &gs->chisel_small;
    
    gs->hammer = hammer_init();

    for (int i = 0; i < PLACER_COUNT; i++)
        placer_init(i);
    gs->has_any_placed = false;

    inventory_init();
    grabber_init();
    gui_init();
    all_converters_init();
    overlay_init();

    setup_item_indices();

#if SHOW_NARRATION
    if (gs->narrator.line_count) {
        level_set_state(lvl, LEVEL_STATE_NARRATION);
        effect_set(&gs->current_effect,
                   EFFECT_SNOW,
                   true,
                   0,
                   0,
                   gs->window_width,
                   gs->window_height);
    } else {
        level_set_state(lvl, LEVEL_STATE_INTRO);
        effect_set(&gs->current_effect,
                   gs->levels[gs->level_current].effect_type,
                   false,
                   0,
                   0,
                   gs->gw*2,
                   gs->gh*2);
    }
#else
    level_set_state(lvl, LEVEL_STATE_PLAY);
#endif

    level_setup_initial_grid();

    if (!gs->undo_initialized) {
        undo_system_init();
    } else {
        undo_system_reset();
        save_state_to_next();
    }


    for (int i = 0; i < TOOL_COUNT; i++) {
        gs->gui.tool_buttons[i]->highlighted = false;
        gs->gui.tool_buttons[i]->disabled = false;
    }

    timelapse_init();

    check_for_tutorial();
}

static void level_set_state(int level, enum Level_State state) {
    Level *l = &gs->levels[level];
    
#if AUDIO_PLAY_AMBIANCE
    if (state == LEVEL_STATE_PLAY || state == LEVEL_STATE_NARRATION) {
        if (gs->levels[level].effect_type == EFFECT_RAIN) {
            //play_music(AMBIENCE_RAIN);
        } else {
            //play_music(AMBIENCE_NORMAL);
        }
    }
#endif
    
    if (state == LEVEL_STATE_PLAY) {
        if (gs->current_effect.type != l->effect_type) {
            effect_set(&gs->current_effect,
                       l->effect_type,
                       false,
                       0,
                       0,
                       2*gs->gw,
                       2*gs->gh);
        }
    } else if (state == LEVEL_STATE_OUTRO) {
        l->outro_alpha = 0;
        l->desired_alpha = 255;
        gs->timelapse.frame = 0;
        gs->timelapse.sticky = 0;
    } else {
        for (int i = 0; i < gs->gw*gs->gh; i++) {
            gs->grid[i].type = l->desired_grid[i].type;
        }
    }

    if (state == LEVEL_STATE_INTRO)
        set_fade(FADE_LEVEL_INTRO, 255, 0);
    if (state == LEVEL_STATE_NARRATION)
        set_fade(FADE_LEVEL_NARRATION, 255, 0);

    l->state = state;
}

static void goto_level_string_hook(const char *string) {
    int lvl = atoi(string) - 1;

    if (lvl < 0) return;
    if (lvl >= LEVEL_COUNT) return;

    goto_level(lvl);
}

static void level_tick(Level *level) {
    if (gs->text_field.active) return;
    if (gs->gui.popup) return;

    switch (level->state) {
        case LEVEL_STATE_NARRATION: {
            if (gs->fade.active) break;
            narrator_tick();
            break;
        }
        case LEVEL_STATE_INTRO: { level_tick_intro(level); break; }
        case LEVEL_STATE_OUTRO: { level_tick_outro(level); break; }
        case LEVEL_STATE_PLAY:  { level_tick_play(level); break;  }
        case LEVEL_STATE_CONFIRMATION: { break; } // Check draw code.
    }
}

static void level_tick_intro(Level *level) {
    level->popup_time_current++;

    if (wait_for_fade(FADE_LEVEL_PLAY_IN)) {
        reset_fade();
        set_fade(FADE_LEVEL_PLAY_OUT, 255, 0);

        level_set_state(gs->level_current, LEVEL_STATE_PLAY);
        level_setup_initial_grid();
    }

    if (gs->input.keys_pressed[SDL_SCANCODE_TAB] || level->popup_time_current >= level->popup_time_max) {
        level->popup_time_current = 0;

        reset_fade();
        set_fade(FADE_LEVEL_PLAY_IN, 0, 255);
    }
}

static void level_tick_outro(Level *level) {
    level->outro_alpha = goto64(level->outro_alpha, level->desired_alpha, 25);
    if (fabs(level->outro_alpha - level->desired_alpha) < 15)
        level->outro_alpha = level->desired_alpha;

    if (!gs->gui.eol_popup_confirm.active && gs->input.keys[SDL_SCANCODE_N]) {
        if (gs->level_current+1 < 11) {
#if 0
            int lvl = gs->level_current+1;
            bool same = compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY);
#endif

#if 0
            if ((lvl == 1 || lvl >= 8) && same) {
#endif
                popup_confirm_activate(&gs->gui.eol_popup_confirm);
#if 0
            } else if (lvl > 1 && lvl < 8) {
                popup_confirm_activate(&gs->gui.eol_popup_confirm);
            } else {
                level_set_state(gs->level_current, LEVEL_STATE_PLAY);
                if (lvl != 1)
                    gs->tutorial = *tutorial_rect(TUTORIAL_COMPLETE_LEVEL_2,
                                                  NormX(32),
                                                  NormY((768.8/8.0)+32),
                                                  null);
                else
                    gs->tutorial = *tutorial_rect(TUTORIAL_COMPLETE_LEVEL,
                                                  NormX(32),
                                                  NormY((768.8/8.0)+32),
                                                  null);
            }
#endif
        } else {
            level_set_state(gs->level_current, LEVEL_STATE_PLAY);
            object_activate(&gs->obj);
            effect_set(&gs->current_effect,
                       EFFECT_SNOW,
                       true,
                       0,
                       0,
                       gs->window_width,
                       gs->window_height);
        }
    }

    if (!gs->gui.eol_popup_confirm.active && gs->input.keys_pressed[SDL_SCANCODE_F]) {
        level->off = true;
        level->desired_alpha = 0;
    }

    if (level->off && level->outro_alpha == 0) {
        level_set_state(gs->level_current, LEVEL_STATE_PLAY);
        level->off = false;
    }
}

static void level_tick_play(Level *level) {
    if (gs->fade.active) return;

    if (gs->input.keys_pressed[SDL_SCANCODE_F]) {
        level_set_state(gs->level_current, LEVEL_STATE_OUTRO);
        gs->input.keys[SDL_SCANCODE_F] = 0;
        return;
    }
    
    simulation_tick();

    if (!gs->paused || gs->step_one) {
        for (int i = 0; i < gs->object_count; i++) {
            object_tick(i);
        }
    }

    overlay_swap_tick();

    if (gs->current_tool > TOOL_CHISEL_LARGE) {
        gs->overlay.current_material = -1;
    }

    if (!level->first_frame_compare && compare_cells_to_int(gs->grid, gs->overlay.grid, 0)) {
        level->first_frame_compare = true;
        gs->overlay.show = false;
    }

    switch (gs->current_tool) {
        case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE: {
            chisel_tick(gs->chisel);
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

    hammer_tick(&gs->hammer);
}

static void level_draw_popup_confirms(int target) {
    end_of_level_popup_confirm_run(target);
    restart_popup_confirm_run(target);
}

static void level_draw(Level *level) {
    switch (level->state) {
        case LEVEL_STATE_NARRATION:    { level_draw_narration(RENDER_TARGET_MASTER); break; }
        case LEVEL_STATE_INTRO:        { level_draw_intro(level);         break; }
        case LEVEL_STATE_OUTRO:        { level_draw_outro_or_play(level); break; }
        case LEVEL_STATE_PLAY:         { level_draw_outro_or_play(level); break; }
    }

    level_draw_popup_confirms(RENDER_TARGET_MASTER);
}

static void level_draw_intro(Level *level) {
    RenderColor(0, 0, 0, 255);
    RenderClear(RENDER_TARGET_PIXELGRID);

    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (level->desired_grid[x+y*gs->gw].type == 0) continue;
            SDL_Color col = pixel_from_index(level->desired_grid[x+y*gs->gw].type, x+y*gs->gw);
            RenderColor(col.r, col.g, col.b, 255);
            RenderPointRelative(RENDER_TARGET_PIXELGRID, x, y);
        }
    }
    
    SDL_Rect src = {
        gs->gw/2, gs->gh/2,
        gs->gw, gs->gh
    };
    SDL_Rect global_dst = {
        0, GUI_H,
        gs->window_width, gs->window_height-GUI_H
    };
    RenderTargetToTarget(RENDER_TARGET_MASTER,
                         RENDER_TARGET_PIXELGRID,
                         &src,
                         &global_dst);
    
    char name[256] = {0};
    sprintf(name, "%d. %s", level->index+1, level->name);
    
    Font *font = gs->fonts.font_title;
    
    int width, height;
    
    TTF_SizeText(font->handle, name, &width, &height);
    
    RenderTextQuick(RENDER_TARGET_MASTER,
                    "bg",
                    font,
                    name,
                    BLACK,
                    255,
                    gs->window_width/2 - width/2+5,
                    64+5,
                    null,
                    null,
                    false);
    RenderTextQuick(RENDER_TARGET_MASTER,
                    "level",
                    font,
                    name,
                    WHITE,
                    255,
                    gs->window_width/2 - width/2,
                    64,
                    null,
                    null,
                    false);
    
#if 0
    TTF_Font *font = gs->fonts.font_title->handle;
    if (gs->level_current+1 == 8)
        font = gs->fonts.font_title_2->handle;

    SDL_Surface *surf = TTF_RenderText_Blended(font,
                                               name,
                                               WHITE);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);

    SDL_Rect dst = {
        gs->window_width/2 - surf->w/2,
        surf->h * .5,
        surf->w, surf->h
    };
    SDL_RenderCopy(gs->renderer, texture, null, &dst);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(texture);
#endif
}

#define LEVEL_MARGIN Scale(36)

static void level_draw_name_intro(int target, Level *level, SDL_Rect rect) {
    // Level name
    char string[256] = {0};
    sprintf(string, "Level %d - \"%s\"", gs->level_current+1, level->name);

    int x = rect.x + LEVEL_MARGIN;
    int y = rect.y + LEVEL_MARGIN;

    char identifier[64] = {0};
    sprintf(identifier, "erhejrh %d",TEXT_OUTRO_LEVEL_NAME);

    RenderTextQuick(target,
                    identifier,
                    gs->fonts.font,
                    string,
                    WHITE,
                    255,
                    x,
                    y,
                    null,
                    null,
                    false);
}

#define LEVEL_DESIRED_GRID_SCALE Scale(4) // This is dumb. Just draw it to a texture at 64x64 scale, then draw the texture at the appropriate scale for the screen size.

static void level_draw_desired_grid(Level *level, int dx, int dy) {
    const int scale = round(LEVEL_DESIRED_GRID_SCALE);

    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            SDL_Rect r;

            if (!level->desired_grid[x+y*gs->gw].type) {
                RenderColor(0, 0, 0, 255);
            } else {
                SDL_Color col = pixel_from_index(level->desired_grid[x+y*gs->gw].type, x+y*gs->gw);
                RenderColor(col.r, col.g, col.b, 255);
            }

            r = (SDL_Rect){ scale*x + dx, scale*y + dy + 32, scale, scale };
            SDL_RenderFillRect(gs->renderer, &r);
        }
    }
}

static void level_draw_outro(int target, Level *level) {
    Assert(target == RENDER_TARGET_MASTER);

    int outro = RENDER_TARGET_OUTRO;

    RenderColor(0, 0, 0, 0);
    RenderClear(outro);

    Uint8 alpha = 255;

    SDL_Rect rect = {gs->S*gs->gw/8, GUI_H + gs->S*gs->gh/2 - (gs->S*3*gs->gh/4)/2, gs->S*3*gs->gw/4, gs->S*3*gs->gh/4};
    RenderColor(0, 0, 0, alpha);
    RenderFillRect(outro, rect);
    RenderColor(91, 91, 91, alpha);
    RenderDrawRect(outro, rect);

    level_draw_name_intro(outro, level, rect);

    //~ Desired and Your grid.

    const int scale = (int)round(LEVEL_DESIRED_GRID_SCALE);
    const int margin = LEVEL_MARGIN;

    int dx = rect.x + LEVEL_MARGIN;
    int dy = rect.y + Scale(100);


    RenderTextQuick(outro,
                    "AAAAAj",
                    gs->fonts.font,
                    "What Max intended",
                    WHITE,
                    alpha,
                    dx,
                    dy,
                    null,
                    null,
                    false);
    RenderTextQuick(outro,
                    "Result",
                    gs->fonts.font,
                    "The result",
                    WHITE,
                    alpha,
                    dx+rect.w - LEVEL_MARGIN - scale*level->w - margin,
                    dy,
                    null,
                    null,
                    false);
    
    //~
    level_draw_desired_grid(level, dx, dy);

    RenderColor(91, 91, 91, alpha);
    SDL_Rect desired_rect = (SDL_Rect){
        dx, dy+32,
        scale*gs->gw, scale*gs->gh
    };
    RenderDrawRect(outro, desired_rect);

    dx += rect.w - margin - scale*level->w - margin;

    //~ Your grid

    timelapse_tick_and_draw(dx, dy+32, scale, scale);

    RenderColor(91, 91, 91, alpha);
    desired_rect = (SDL_Rect){
        dx, dy+32,
        scale*gs->gw, scale*gs->gh
    };
    RenderDrawRect(outro, desired_rect);

    SDL_Color color_next_level = (SDL_Color){255,255,255,255};

    RenderTextQuick(outro,
                    "next level",
                    gs->fonts.font,
                    "Next Level [n]",
                    color_next_level,
                    255,
                    rect.x + rect.w - Scale(200),
                    rect.y + rect.h - margin - Scale(20),
                    null,
                    null,
                    false);
    RenderTextQuick(outro,
                    "close",
                    gs->fonts.font,
                    "Close [f]",
                    (SDL_Color){128, 128, 128, alpha},
                    255,
                    rect.x + margin,
                    rect.y + rect.h - margin - Scale(20),
                    null,
                    null,
                    false);
    
    SDL_Rect src = {
        0, 0,
        gs->window_width,
        gs->window_height
    };

    SDL_Rect dst = {
        0, 0,
        gs->window_width,
        gs->window_height
    };

    RenderTextureAlphaMod(&RenderTarget(RENDER_TARGET_OUTRO)->texture, level->outro_alpha);
    RenderTargetToTarget(target,
                         RENDER_TARGET_OUTRO,
                         &src,
                         &dst);
}

static void level_get_cells_from_image(const char *path,
                                       Cell **out,
                                       Source_Cell *source_cells,
                                       int *out_source_cell_count,
                                       int *out_w,
                                       int *out_h)
{
    SDL_Surface *surface = IMG_Load(path);
    Assert(surface);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surface);
    Assert(texture);

    int w = surface->w;
    int h = surface->h;

    *out_w = w;
    *out_h = h;

    *out = PushArray(gs->persistent_memory, w*h, sizeof(Cell));

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            Uint8 r=0, g=0, b=0;

            if (surface->format->BytesPerPixel == 1) {
                Uint8 pixel = ((Uint8*)surface->pixels)[x + y * w];
                SDL_GetRGB(pixel, surface->format, &r, &g, &b);
            } else if (surface->format->BytesPerPixel == 4) {
                Uint32 pixel = ((Uint32*)surface->pixels)[x + y * w];
                SDL_GetRGB(pixel, surface->format, &r, &g, &b);
            } else {
                Assert(0);
            }

            int cell = 0;

            if (r == 255 && g == 0 && b == 0) {
                Source_Cell *s = &source_cells[(*out_source_cell_count)++];
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

static void level_draw_narration(int target) {
    RenderColor(20, 20, 20, 255);
    RenderClear(target);
    
    effect_draw(target, &gs->current_effect, false, ONLY_SLOW_ALL);
    narrator_run(target, WHITE);
    //effect_draw(target, &gs->current_effect, false, ONLY_SLOW_FAST);
}

static void level_draw_outro_or_play(Level *level) {
    if (gs->current_preview.recording)
        RenderColor(53, 20, 20, 255);
    else
        RenderColor(0, 0, 0, 255);
    RenderClear(RENDER_TARGET_PIXELGRID);

    background_draw(RENDER_TARGET_PIXELGRID, &gs->background);

    effect_draw(RENDER_TARGET_PIXELGRID, &gs->current_effect, true, ONLY_SLOW_ALL);

    grid_draw(RENDER_TARGET_PIXELGRID);

    switch (gs->current_tool) {
        case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE: {
            chisel_draw(RENDER_TARGET_PIXELGRID, gs->chisel);
            hammer_draw(RENDER_TARGET_PIXELGRID, &gs->hammer);
            break;
        }
        case TOOL_PLACER: {
            if (!gs->gui.popup) // When gui.popup = true, we draw in converter
                placer_draw(RENDER_TARGET_PIXELGRID, &gs->placers[gs->current_placer], false);
            break;
        }
    }
    dust_grid_run(RENDER_TARGET_PIXELGRID);

    draw_objects(RENDER_TARGET_PIXELGRID);

    RenderColor(0,0,0,255);
    RenderClear(RENDER_TARGET_MASTER);

    SDL_Rect src = {
        0,
        0,
        2*gs->gw,
        2*gs->gh
    };

    SDL_Rect dst = {
        -gs->window_width/2-gs->render.view.x,
        -gs->window_height/2-gs->render.view.y + GUI_H*1.5,
        gs->window_width*2,
        2*(gs->window_height-GUI_H),
    };

    RenderTargetToTarget(RENDER_TARGET_MASTER,
                         RENDER_TARGET_PIXELGRID,
                         &src,
                         &dst);

    draw_grid_outline(RENDER_TARGET_MASTER);
    if (level->state == LEVEL_STATE_OUTRO) {
        level_draw_outro(RENDER_TARGET_MASTER, level);
        gui_draw(RENDER_TARGET_MASTER);
    } else {
        gui_draw(RENDER_TARGET_MASTER);

        gui_popup_draw(RENDER_TARGET_MASTER);
        tutorial_rect_run(RENDER_TARGET_MASTER);
        tooltip_draw(RENDER_TARGET_MASTER, &gs->gui.tooltip);

        if (gs->gui.popup)
            gui_draw_profile(RENDER_TARGET_MASTER);
    }
}

static SDL_Color type_to_rgb(int type) {
    Assert(type < CELL_TYPE_COUNT);
    return (SDL_Color){type_to_rgb_table[4*type+1], type_to_rgb_table[4*type+2], type_to_rgb_table[4*type+3], 255};
}

static int rgb_to_type(Uint8 r, Uint8 g, Uint8 b) {
    for (int i = 0; i < CELL_TYPE_COUNT; i++) {
        SDL_Color col = type_to_rgb(i);
        if (col.r == r && col.g == g && col.b == b) {
            return i;
        }
    }
    return CELL_NONE;
}

static void level_output_to_png(const char *output_file) {
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
