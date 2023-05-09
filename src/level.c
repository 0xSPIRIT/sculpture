// Gets cells based on pixels in the image.
//
// path                  - path to the image
// out                   - pointer to array of Cells (it's allocated in this function)
// source_cells          - pointer to a bunch of source cells (NULL if don't want). Must not be heap allocated.
// out_source_cell_count - Pointer to the cell count. Updates in this func.
// out_w & out_h         - width and height of the image.
void level_get_cells_from_image(const char *path,
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
            Uint8 r, g, b;
            
            Uint32 pixel = ((Uint32*)surface->pixels)[x+y*w];
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);
            
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

void level_setup_initial_grid() {
    // Reset everything except the IDs of the grid, since there's no reason to recalculate it.
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        int id = gs->grid[i].id;
        gs->grid[i] = (Cell){.type = gs->levels[gs->level_current].initial_grid[i].type, .rand = rand(), .id = id, .object = -1, .is_initial = true};
    }
    objects_reevaluate();
}

int level_add(const char *name, const char *desired_image, const char *initial_image, int effect_type) {
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

void levels_setup(void) {
    level_add("Marrow",
              RES_DIR "lvl/desired/level 1.png",
              RES_DIR "lvl/initial/level 1.png",
              EFFECT_SNOW);
    level_add("Alaska",
              RES_DIR "lvl/desired/level 2.png",
              RES_DIR "lvl/initial/level 2.png",
              EFFECT_SNOW);
    level_add("Fireplace",
              RES_DIR "lvl/desired/level 3.png",
              RES_DIR "lvl/initial/level 3.png",
              EFFECT_NONE);
    level_add("Bliss",
              RES_DIR "lvl/desired/level 4.png",
              RES_DIR "lvl/initial/level 4.png",
              EFFECT_NONE);
    level_add("The Process",
              RES_DIR "lvl/desired/level 5.png",
              RES_DIR "lvl/initial/level 5.png",
              EFFECT_NONE);
    level_add("Premonition",
              RES_DIR "lvl/desired/level 6.png",
              RES_DIR "lvl/initial/level 6.png",
              EFFECT_RAIN);
    level_add("Metamorphosis",
              RES_DIR "lvl/desired/level 7.png",
              RES_DIR "lvl/initial/level 7.png",
              EFFECT_NONE);
    level_add("Procedure Lullaby",
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
              EFFECT_NONE);
}

void goto_level(int lvl) {
    gs->level_current = lvl;
    
    gs->levels[lvl].first_frame_compare = false;
    
    grid_init(gs->levels[lvl].w, gs->levels[lvl].h);
    
    tooltip_reset(&gs->gui.tooltip);
    
    narrator_init(gs->level_current);
    
    gs->levels[lvl].popup_time_current = 0;
    
    memcpy(&gs->levels[lvl].source_cell,
           &gs->levels[lvl].default_source_cell,
           sizeof(Source_Cell)*SOURCE_CELL_MAX);
    gs->levels[lvl].source_cell_count = gs->levels[lvl].default_source_cell_count;
    
    gs->conversions.calculated_render_target = false;
    
    gs->current_tool = TOOL_GRABBER;
    
    //gs->S = (f64)gs->window_width/(f64)gs->gw;
    Assert(gs->gw==gs->gh);
    
    gs->item_holding = (Item){0};
    gs->current_placer = 0;
    inventory_init();
    
    dust_init();
    
    gs->chisel_small = chisel_init(CHISEL_SMALL);
    gs->chisel_medium = chisel_init(CHISEL_MEDIUM);
    gs->chisel_large = chisel_init(CHISEL_LARGE);
    gs->chisel = &gs->chisel_small;
    
    gs->hammer = hammer_init();
    
    //chisel_hammer_init();
    deleter_init();
    for (int i = 0; i < PLACER_COUNT; i++)
        placer_init(i);
    grabber_init();
    gui_init();
    all_converters_init();
    overlay_init();
    
    setup_item_indices();
    
#if SHOW_NARRATION
    if (gs->narrator.line_count) {
        level_set_state(lvl, LEVEL_STATE_NARRATION);
        effect_set(EFFECT_SNOW, gs->window_width, gs->window_height);
    } else {
        level_set_state(lvl, LEVEL_STATE_INTRO);
        effect_set(gs->levels[gs->level_current].effect_type, gs->gw, gs->gh);
    }
#else
    level_set_state(lvl, LEVEL_STATE_PLAY);
    level_setup_initial_grid();
#endif
    
    for (int i = 0; i < TOOL_COUNT; i++) {
        gs->gui.tool_buttons[i]->highlighted = false;
        gs->gui.tool_buttons[i]->disabled = false;
    }
    
    timelapse_init();
    
    check_for_tutorial();
    
}

void level_set_state(int level, enum Level_State state) {
    Level *l = &gs->levels[level];
    
    if (state == LEVEL_STATE_PLAY) {
        effect_set(l->effect_type, gs->gw, gs->gh);
        if (!gs->undo_initialized) {
            undo_system_init();
        } else {
            undo_system_reset();
            save_state_to_next();
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

void goto_level_string_hook(const char *string) {
    int lvl = atoi(string) - 1;
    
    if (lvl < 0) return;
    if (lvl >= LEVEL_COUNT) return;
    
    goto_level(lvl);
}

void level_tick(void) {
    Level *level = &gs->levels[gs->level_current];
    Input *input = &gs->input;
    
    if (gs->text_field.active) return;
    if (gs->gui.popup) return;
    
    switch (level->state) {
        case LEVEL_STATE_NARRATION: {
            if (gs->fade.active) break;
            narrator_tick();
            break;
        }
        case LEVEL_STATE_INTRO: {
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
            break;
        }
        case LEVEL_STATE_OUTRO: {
            level->outro_alpha = goto64(level->outro_alpha, level->desired_alpha, 25);
            if (fabs(level->outro_alpha - level->desired_alpha) < 15)
                level->outro_alpha = level->desired_alpha;
            
            if (wait_for_fade(FADE_LEVEL_FINISH)) {
                reset_fade();
                goto_level(++gs->level_current);
            }
            
            if (input->keys[SDL_SCANCODE_N]) {
                if (gs->level_current+1 < 11) {
                    int lvl = gs->level_current+1;
                    bool same = compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY);
                    
                    if ((lvl == 1 || lvl >= 8) && same) {
                        set_fade(FADE_LEVEL_FINISH, 0, 255);
                    } else if (lvl > 1 && lvl < 8) {
                        set_fade(FADE_LEVEL_FINISH, 0, 255);
                    } else {
                        level_set_state(gs->level_current, LEVEL_STATE_PLAY);
                        if (lvl != 1)
                            gs->tutorial = *tutorial_rect(TUTORIAL_COMPLETE_LEVEL_2,
                                                          NormX(32),
                                                      NormY((768.8/8.0)+32),
                                                          NULL);
                        else
                            gs->tutorial = *tutorial_rect(TUTORIAL_COMPLETE_LEVEL,
                                                          NormX(32),
                                                      NormY((768.8/8.0)+32),
                                                          NULL);
                    }
                } else {
                    level_set_state(gs->level_current, LEVEL_STATE_PLAY);
                    object_activate(&gs->obj);
                    effect_set(EFFECT_SNOW, gs->window_width, gs->window_height);
                }
            }
            
            if (input->keys_pressed[SDL_SCANCODE_F]) {
                level->off = true;
                level->desired_alpha = 0;
            }
            
            if (level->off && level->outro_alpha == 0) {
                level_set_state(gs->level_current, LEVEL_STATE_PLAY);
                level->off = false;
            }
            break;
        }
        case LEVEL_STATE_PLAY: {
            if (gs->fade.active) break;
            
            if (input->keys_pressed[SDL_SCANCODE_F]) {
                level_set_state(gs->level_current, LEVEL_STATE_OUTRO);
                input->keys[SDL_SCANCODE_F] = 0;
            }
            
            simulation_tick();
            
            if (!gs->paused || gs->step_one) {
                for (int i = 0; i < gs->object_count; i++) {
                    object_tick(i);
                }
            }
            
            if (gs->step_one) {
                gs->step_one = 0;
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
#if 0
                case TOOL_BLOCKER: {
                    blocker_tick();
                    break;
                }
#endif
                case TOOL_OVERLAY: {
                    //overlay_tick();
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
            
            hammer_tick(&gs->hammer);
            
            break;
        }
    }
}

void level_draw_intro(void) {
    Level *level = &gs->levels[gs->level_current];
    
    Assert(RenderTarget(RENDER_TARGET_GLOBAL));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL));
    
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
    SDL_RenderClear(gs->renderer);
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (level->desired_grid[x+y*gs->gw].type == 0) continue;
            SDL_Color col = pixel_from_index(level->desired_grid[x+y*gs->gw].type, x+y*gs->gw);
            SDL_SetRenderDrawColor(gs->renderer, col.r, col.g, col.b, 255);
            SDL_RenderDrawPoint(gs->renderer, x, y);
        }
    }
    
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_MASTER));
    
    const SDL_Rect global_dst = {
        0, GUI_H,
        gs->window_width, gs->window_height-GUI_H
    };
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL), NULL, &global_dst);
    
    char name[256] = {0};
    sprintf(name, "%d. %s", level->index+1, level->name);
    
    TTF_Font *font = gs->fonts.font_title;
    if (gs->level_current+1 == 8)
        font = gs->fonts.font_title_2;
    
    SDL_Surface *surf = TTF_RenderText_Blended(font,
                                               name,
                                               WHITE);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(gs->renderer, surf);
    
    SDL_Rect dst = {
        gs->window_width/2 - surf->w/2,
        surf->h * .5,
        surf->w, surf->h
    };
    SDL_RenderCopy(gs->renderer, texture, NULL, &dst);
    
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(texture);
    
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_MASTER));
}

void draw_outro(Level *level) {
    Uint8 alpha = 255;
    
    SDL_Texture *previous = SDL_GetRenderTarget(gs->renderer);
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_OUTRO));
    
    SDL_Rect rect = {gs->S*gs->gw/8, GUI_H + gs->S*gs->gh/2 - (gs->S*3*gs->gh/4)/2, gs->S*3*gs->gw/4, gs->S*3*gs->gh/4};
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, alpha);
    SDL_RenderFillRect(gs->renderer, &rect);
    SDL_SetRenderDrawColor(gs->renderer, 91, 91, 91, alpha);
    SDL_RenderDrawRect(gs->renderer, &rect);
    
    const int margin = Scale(36);
    
    { // Level name
        char string[256] = {0};
        sprintf(string, "Level %d - \"%s\"", gs->level_current+1, level->name);
        
        int x = rect.x + margin;
        int y = rect.y + margin;
        
        draw_text_indexed(TEXT_OUTRO_LEVEL_NAME,
                          gs->fonts.font,
                          string, WHITE, BLACK, alpha, 0, 0, x, y, NULL, NULL, false);
    }
    
    
    // Desired and Your grid.
    
    const int scale = Scale(3);
    
    int dx = rect.x + margin;
    int dy = rect.y + Scale(100);
    
    draw_text_indexed(TEXT_OUTRO_INTENDED,
                      gs->fonts.font,
                      "What you intended", WHITE, BLACK, alpha, 0, 0, dx, dy, NULL, NULL, false);
    draw_text_indexed(TEXT_OUTRO_RESULT,
                      gs->fonts.font,
                      "The result", WHITE, BLACK, alpha, 0, 0, dx+rect.w - margin - scale*level->w - margin, dy, NULL, NULL, false);
    
    // Desired
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            SDL_Rect r;
            
            if (!level->desired_grid[x+y*gs->gw].type) {
                SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, alpha);
            } else {
                SDL_Color col = pixel_from_index(level->desired_grid[x+y*gs->gw].type, x+y*gs->gw);
                SDL_SetRenderDrawColor(gs->renderer, col.r, col.g, col.b, alpha); // 255 on this because desired_grid doesn't have depth set.
            }
            
            r = (SDL_Rect){ scale*x + dx, scale*y + dy + 32, scale, scale };
            SDL_RenderFillRect(gs->renderer, &r);
        }
    }
    
    SDL_SetRenderDrawColor(gs->renderer, 91, 91, 91, alpha);
    SDL_Rect desired_rect = (SDL_Rect){
        dx, dy+32,
        scale*gs->gw, scale*gs->gh
    };
    SDL_RenderDrawRect(gs->renderer, &desired_rect);
    
    dx += rect.w - margin - scale*level->w - margin;
    
    // Your grid
    
    timelapse_tick_and_draw(dx, dy+32, scale, scale);
    
    SDL_SetRenderDrawColor(gs->renderer, 91, 91, 91, alpha);
    desired_rect = (SDL_Rect){
        dx, dy+32,
        scale*gs->gw, scale*gs->gh
    };
    SDL_RenderDrawRect(gs->renderer, &desired_rect);
    
    SDL_Color color_next_level = (SDL_Color){0, 200, 0, alpha};
    bool update = true;
    
    if ((gs->level_current+1 == 1 && !compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY)) ||
        gs->level_current+1 >= 8 && gs->level_current+1 <= 10 && !compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY)) {
        color_next_level = (SDL_Color){200, 0, 0, alpha};
    }
    
    draw_text_indexed(TEXT_OUTRO_NEXT_LEVEL,
                      gs->fonts.font,
                      "Next Level [n]",
                      color_next_level,
                      (SDL_Color){0, 0, 0, alpha},
                      alpha,
                      1, 1,
                      rect.x + rect.w - margin,
                      rect.y + rect.h - margin,
                      NULL,
                      NULL,
                      update);
    draw_text_indexed(TEXT_OUTRO_PREV_LEVEL,
                      gs->fonts.font,
                      "Close [f]",
                      (SDL_Color){128, 128, 128, alpha},
                      (SDL_Color){0, 0, 0, alpha}, 
                      alpha,
                      0, 1,
                      rect.x + margin,
                      rect.y + rect.h - margin,
                      NULL,
                      NULL,
                      false);
    
    if (gs->level_current+1 >= 8 && !compare_cells_to_int(gs->grid, gs->overlay.grid, COMPARE_LEEWAY)) {
        char comment[128]={0};
        
        strcpy(comment, "I cannot settle for this.");
        
        draw_text_indexed(TEXT_NOT_GOOD_ENOUGH,
                          gs->fonts.font,
                          comment,
                          (SDL_Color){180, 0, 0, alpha},
                          BLACK,
                          alpha,
                          0, 0,
                          rect.x + rect.w - 300,
                          rect.y + rect.h/2 + 64,
                          NULL, NULL,
                          false);
    }
    
    SDL_SetRenderTarget(gs->renderer, previous);
    SDL_SetTextureAlphaMod(RenderTarget(RENDER_TARGET_OUTRO), level->outro_alpha);
    SDL_Rect dst = {
        0, 0,
        gs->window_width, gs->window_height - GUI_H
    };
    SDL_Rect src = {
        0, 0,
        gs->window_width, gs->window_height - GUI_H
    };
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_OUTRO), &src, &dst);
}

void level_draw(void) {
    Level *level = &gs->levels[gs->level_current];
    
    switch (level->state) {
        case LEVEL_STATE_NARRATION: {
            SDL_SetRenderDrawColor(gs->renderer, 20, 20, 20, 255);
            SDL_RenderClear(gs->renderer);
            
            effect_draw(&gs->current_effect, false);
            
            narrator_run(WHITE);
            text_field_draw();
            break;
        }
        case LEVEL_STATE_INTRO: {
            SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
            SDL_RenderClear(gs->renderer);
    
            level_draw_intro();
            text_field_draw();
            break;
        }
        case LEVEL_STATE_OUTRO: case LEVEL_STATE_PLAY: {
            SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
            SDL_RenderClear(gs->renderer);
            
            SDL_Texture *prev = SDL_GetRenderTarget(gs->renderer);
            
            SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL));
            
            if (gs->current_preview.recording)
                SDL_SetRenderDrawColor(gs->renderer, 53, 20, 20, 255);
            else
                SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 255);
            SDL_RenderClear(gs->renderer);
            
            grid_draw();
            
            effect_draw(&gs->current_effect, true);
            
            switch (gs->current_tool) {
                case TOOL_CHISEL_SMALL: case TOOL_CHISEL_MEDIUM: case TOOL_CHISEL_LARGE: {
                    chisel_draw(gs->chisel);
                    hammer_draw(&gs->hammer);
                    break;
                }
                case TOOL_DELETER: {
                    deleter_draw();
                    break;
                }
                case TOOL_PLACER: {
                    if (!gs->gui.popup) // When gui.popup = true, we draw in converter
                        placer_draw(&gs->placers[gs->current_placer], false);
                    break;
                }
            }
            
            draw_objects();
            
            SDL_SetRenderTarget(gs->renderer, prev);
            
            if (level->state == LEVEL_STATE_OUTRO) {
                SDL_Rect dst = {
                    -gs->view.x,
                    -gs->view.y + GUI_H,
                    gs->view.w, gs->view.h
                };
                
                SDL_RenderCopy(gs->renderer,
                               RenderTarget(RENDER_TARGET_GLOBAL),
                               NULL,
                               &dst);
                gui_draw();
                draw_outro(level);
                
                text_field_draw();
            } else {
                SDL_Rect dst = {
                    -gs->view.x,
                    -gs->view.y + GUI_H,
                    gs->view.w, gs->view.h
                };
                
                SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_GLOBAL), NULL, &dst);
                
                gui_draw();
                gui_popup_draw();
                
                tutorial_rect_run();
                
                tooltip_draw(&gs->gui.tooltip);
                
                if (gs->gui.popup)
                    gui_draw_profile();
                
                gui_message_stack_tick_and_draw();
                text_field_draw();
            }
            break;
        }
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
