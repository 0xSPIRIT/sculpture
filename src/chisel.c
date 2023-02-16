void chisel_play_sound(int size) {
    if (size == 0) {
        Mix_PlayChannel(AUDIO_CHANNEL_CHISEL, gs->audio.small_chisel, 0);
    } else if (size == 1) {
        Mix_PlayChannel(AUDIO_CHANNEL_CHISEL, gs->audio.medium_chisel[0], 0);
    } else if (size == 2) {
        Mix_PlayChannel(AUDIO_CHANNEL_CHISEL, gs->audio.medium_chisel[rand()%6], 0);
    }
}

void chisel_hammer_init(void) {
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;
    struct Chisel *chisel = gs->chisel;
    
    hammer->x = chisel->x;
    hammer->y = chisel->y;
    hammer->normal_dist = (f32) (chisel->w+4);
    hammer->dist = hammer->normal_dist;
    hammer->time = 0;
    hammer->angle = 0;
    
    hammer->texture = gs->textures.chisel_hammer;
    SDL_QueryTexture(hammer->texture, NULL, NULL, &hammer->w, &hammer->h);
}

void chisel_hammer_tick(void) {
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;
    struct Input *input = &gs->input;
    struct Chisel *chisel = gs->chisel;
    
    hammer->angle = chisel->angle;
    
    f32 rad = (hammer->angle) / 360.f;
    rad *= 2 * (f32)M_PI;
    
    const f32 off = 6;
    
    int dir = 1;
    
    if (hammer->angle > 90 && hammer->angle < 270) {
        dir = -1;
    }
    
    hammer->x = chisel->x + hammer->dist * cosf(rad) - dir * off * sinf(rad);
    hammer->y = chisel->y + hammer->dist * sinf(rad) + dir * off * cosf(rad);
    
    bool click = false;
    
    if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
        click = true;
        chisel->click_cd = 15;
    } else if (input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (chisel->click_cd > 0) {
            chisel->click_cd--;
        } else {
            chisel->click_cd = 3;
            click = true;
        }
    }
    
    const int stop = 24;
    
    switch (hammer->state) {
        case HAMMER_STATE_WINDUP: {
            hammer->time++;
            if (hammer->time < 3) {
                hammer->dist += hammer->time * 4;
            } else {
                hammer->time = 0;
                hammer->state = HAMMER_STATE_SWING;
            }
            break;
        }
        case HAMMER_STATE_SWING: {
            hammer->dist -= 8;
            if (hammer->dist < stop) {
                hammer->dist = (f32) stop;
                // Activate the chisel.
                if (chisel->click_cooldown == 0) {
                    chisel->click_cooldown = CHISEL_COOLDOWN;
                    chisel->spd = 3.;
                }
                chisel->did_remove = false;
                hammer->state = HAMMER_STATE_IDLE;
            }
            break;
        }
        case HAMMER_STATE_IDLE: {
            hammer->dist = hammer->normal_dist;
            if (!gs->is_mouse_over_any_button &&
                !gs->tutorial.active &&
                gs->chisel->highlight_count > 0 &&
                click)
            {
                hammer->state = HAMMER_STATE_WINDUP;
                save_state_to_next();
            }
            break;
        }
    }
}

//
// If remove == true, delete the blob
// Else, highlight it and don't change anything about the chisel's state.
//
// ux, uy = unit vector for the direction of chisel.
// px, py = initial positions.
//
// Returns the blob it reaches only if remove == false.
//
Uint32 chisel_goto_blob(int obj, bool remove, f32 ux, f32 uy, f32 len) {
    struct Chisel *chisel = gs->chisel;
    struct Object *objects = gs->objects;
    Uint32 *curr_blobs = objects[obj].blob_data[chisel->size].blobs;
    
    memset(chisel->highlights, 0, chisel->highlight_count);
    chisel->highlight_count = 0;
    
    bool did_hit_blocker = false;
    f32 px = chisel->x, py = chisel->y;
    
    if (obj == -1) return 0;
    
    chisel->num_times_chiseled++;
    
    f32 current_length = (f32) sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y));
    
    int iterations = 0;
    while (current_length < len && is_in_boundsf(chisel->x, chisel->y)) {
        // If we come into contact with a cell, locate its blob
        // then remove it. We only remove one blob per chisel,
        // so we stop our speed right here.
        Uint32 b = curr_blobs[(int)chisel->x + ((int)chisel->y)*gs->gw];
        
        if (b > 0 && ((remove && !chisel->did_remove) || !remove)) {
            // We want to attempt to destroy this blob now.
            // Firstly, we want to do a diagonal check.
            
            //
            //      /
            //  xxx/    xxx/
            //  xxxx    xx/x
            //  xxxxx   xxxxx
            //
            //  ^ This is what we want to prevent.
            //
            
            // If we encounter this however, we want to snap to the closest blob.
            if (chisel->size == TOOL_CHISEL_SMALL && number_direct_neighbours(gs->grid, (int)chisel->x, (int)chisel->y) == 4) {
                int ix = (int)chisel->x;
                int iy = (int)chisel->y;
                
                for (int y = -1; y <= 1; y++) {
                    for (int x = -1; x <= 1; x++) {
                        if (abs(x) == abs(y)) continue;
                        
                        if (number_direct_neighbours(gs->grid, ix+x, iy+y) == 4) continue;
                        
                        b = curr_blobs[ix+x+(iy+y)*gs->gw];
                        if (b) {
                            chisel->x = ix+x;
                            chisel->y = iy+y;
                            break;
                        }
                    }
                    if (b) break;
                }
                
            }
            
            if (chisel->size != TOOL_CHISEL_SMALL || number_direct_neighbours(gs->grid, (int)chisel->x, (int)chisel->y) < 4) {
                // We continue at our current direction until we reach
                // another blob, then backtrack one.
                while (curr_blobs[(int)chisel->x + (int)chisel->y*gs->gw] == b) {
                    chisel->x += ux;
                    chisel->y += uy;
                }
                
                chisel->x -= ux;
                chisel->y -= uy;
                
                bool can_destroy = blob_can_destroy(obj, chisel->size, b);
                
                if (!can_destroy && remove && !gs->did_pressure_tutorial) {
                    gs->tutorial = *tutorial_rect(TUTORIAL_PRESSURE_STRING,
                                                  32,
                                                  GUI_H+32,
                                                  NULL);
                    gs->did_pressure_tutorial = true;
                }
                
                if (can_destroy) {
                    if (!remove) {
                        chisel->x = px;
                        chisel->y = py;
                        return b;
                    }
                    
                    int count = cell_count_of_blob(obj, b, chisel->size);
                    enum Cell_Type type = gs->grid[(int)chisel->x + ((int)chisel->y)*gs->gw].type;
                    
                    chisel_play_sound(chisel->size);
                    object_remove_blob(obj, b, chisel->size, true);
                    chisel->did_chisel_this_frame = true;
                    
                    move_mouse_to_grid_position(chisel->x, chisel->y);
                    emit_dust_explosion(type, chisel->x, chisel->y, count);
                    
                    chisel->did_remove = true;
                } else if (!remove) {
                    b = 0;
                    chisel->x = px;
                    chisel->y = py;
                    return b;
                }
                
                chisel->click_cooldown = CHISEL_COOLDOWN-CHISEL_TIME-1;
            }
            
            break;
        }
        
        chisel->x += ux;
        chisel->y += uy;
        
        current_length = (f32) sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y));
        iterations++;
    }
    
    Uint32 b = curr_blobs[(int)chisel->x + (int)chisel->y*gs->gw];
    if (!remove) {
        b = 0;
        chisel->x = px;
        chisel->y = py;
        return b;
    }
    
    // remove=true from here on out.
    
    // Do a last-ditch effort if a blob is around a certain radius in order for the player
    // not to feel frustrated if it's a small blob or it's one pixel away.
    if (did_hit_blocker || (CHISEL_FORGIVING_AIM && !chisel->did_remove)) {
        const int r = 1;
        for (int y = -r; y <= r; y++) {
            for (int x = -r; x <= r; x++) {
                if (x*x + y*y > r*r) continue;
                int xx = (int)chisel->x + x;
                int yy = (int)chisel->y + y;
                Uint32 blob = curr_blobs[xx+yy*gs->gw];
                
                if (blob > 0) {
                    object_remove_blob(obj, blob, chisel->size, true);
                    chisel->did_chisel_this_frame = true;
                    
                    chisel_play_sound(chisel->size);
                    chisel->did_remove = true;
                    goto chisel_did_remove;
                }
            }
        }
    }
    
    chisel_did_remove:
    if (chisel->did_remove) {
        objects_reevaluate();
        // Here, we check for all the small objects that pop up from repeated
        // chiseling that make chiseling an annoyance. We convert that to
        // dust particles in order to get out of the player's way.
        
        bool did_remove = false;
        
        const bool CHISEL_REMOVE_SMALL_OBJECTS = false;
        if (CHISEL_REMOVE_SMALL_OBJECTS) {
            for (int i = 0; i < gs->object_count; i++) {
                if (objects[i].cell_count <= 4) {
                    convert_object_to_dust(i);
                    did_remove = true;
                }
            }
        } else {
            for (int i = 0; i < gs->object_count; i++) {
                if (objects[i].cell_count == 1) {
                    convert_object_to_dust(i);
                    did_remove = true;
                }
            }
        }
        
        if (did_remove) {
            objects_reevaluate();
        }
    }
    
    return 0;
}

void chisel_init(struct Chisel *type) {
    struct Chisel *chisel = NULL;
    
    struct Chisel *chisel_small = &gs->chisel_small;
    struct Chisel *chisel_medium = &gs->chisel_medium;
    struct Chisel *chisel_large = &gs->chisel_large;
    
    gs->chisel = type;
    chisel = gs->chisel;
    
    chisel->angle = 315;
    
    chisel->num_times_chiseled = 0;
    
    if (chisel == chisel_small) {
        chisel->size = 0;
    } else if (chisel == chisel_medium) {
        chisel->size = 1;
    } else if (chisel == chisel_large) {
        chisel->size = 2;
    }
    
    for (int face = 1; face != -1; face--) {
        if (face) {
            chisel->face_texture = gs->textures.chisel_face[chisel->size];
            SDL_QueryTexture(chisel->face_texture, NULL, NULL, &chisel->face_w, &chisel->face_h);
        } else {
            chisel->outside_texture = gs->textures.chisel_outside[chisel->size];
            SDL_QueryTexture(chisel->outside_texture, NULL, NULL, &chisel->outside_w, &chisel->outside_h);
        }
    }
    
    chisel->click_cooldown = 0;
    chisel->line = NULL;
    chisel->face_mode = false;
    
    if (chisel->pixels == NULL) {
        chisel->pixels = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(Uint32));
    }
    
    chisel->texture = chisel->outside_texture;
    chisel->w = chisel->outside_w;
    chisel->h = chisel->outside_h;
    
    if (chisel->highlights == NULL) {
        chisel->highlights = PushArray(gs->persistent_memory, gs->gw*gs->gh, sizeof(int));
    }
    
    chisel->highlight_count = 0;
    
    chisel->spd = 3.;
}

void chisel_hammer_draw(int dx, int dy) {
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;
    
    const SDL_Rect dst = {
        dx + (int)hammer->x, dy + (int)(hammer->y - hammer->h/2),
        hammer->w, hammer->h
    };
    const SDL_Point center = { 0, hammer->h/2 };
    
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    
    if (hammer->angle > 90 && hammer->angle < 270) {
        flip = SDL_FLIP_VERTICAL;
    }
    
    SDL_RenderCopyEx(gs->renderer, hammer->texture, NULL, &dst, hammer->angle, &center, flip);
}

void chisel_draw_target(struct Chisel *chisel, int dx, int dy, int render_target) {
    SDL_Texture *prev_target = SDL_GetRenderTarget(gs->renderer);
    
    SDL_SetRenderTarget(gs->renderer, RenderTarget(render_target));
    
    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);
    
    int x = (int)chisel->x;
    int y = (int)chisel->y;
    
    // Disgusting hardcoding to adjust the weird rotation SDL does.
    if (!chisel->face_mode) {
        if (gs->use_software_renderer) {
            if (chisel->size == 0 || chisel->size == 1) {
                if (chisel->angle == 225) {
                    y += 2;
                } else if (chisel->angle == 180) {
                    x++;
                    y++;
                } else if (chisel->angle == 90) {
                    x++;
                } else if (chisel->angle == 45) {
                    y--;
                    x++;
                } else if (chisel->angle == 135) {
                    x += 2;
                    y++;
                } else if (chisel->angle == 315) {
                }
            } else if (chisel->size == 2) {
                if (chisel->angle == 225) {
                    y++;
                } else if (chisel->angle == 45) {
                    x++;
                } else if (chisel->angle == 135) {
                    x++;
                    y++;
                }
            }
        } else {
            if (gs->input.keys[SDL_SCANCODE_U])
                Log("%.2f\n", chisel->angle);
            if (chisel->size == 0 || chisel->size == 1) {
                if (chisel->angle == 225) {
                    y += 2;
                    x++;
                } else if (chisel->angle == 180) {
                    x++;
                    y++;
                } else if (chisel->angle == 90) {
                    x++;
                } else if (chisel->angle == 45) {
                    x++;
                } else if (chisel->angle == 135) {
                    x += 2;
                } else if (chisel->angle == 315) {
                }
            } else if (chisel->size == 2) {
                if (chisel->angle == 0) {
                    x++;
                } else if (chisel->angle == 225) {
                    y++;
                } else if (chisel->angle == 45) {
                    x++;
                    y++;
                } else if (chisel->angle == 135) {
                    x++;
                    y++;
                } else if (chisel->angle == 315) {
                    x++;
                } else if (chisel->angle == 180) {
                    y++;
                } else if (chisel->angle == 90) {
                    x++;
                }
            }
        }
    }
    
    const SDL_Rect dst = {
        dx + x, dy + y - chisel->h/2,
        chisel->w, chisel->h
    };
    const SDL_Point center = { 0, chisel->h/2 };
    
    SDL_RenderCopyEx(gs->renderer, chisel->texture, NULL, &dst, chisel->angle, &center, SDL_FLIP_NONE);
    
    chisel_hammer_draw(dx, dy);
    
    if (!chisel->face_mode) {
        SDL_SetRenderDrawColor(gs->renderer, 127, 127, 127, 255);
        SDL_RenderDrawPoint(gs->renderer, dx + (int)chisel->x, dy + (int)chisel->y);
    }
    
    SDL_RenderReadPixels(gs->renderer, NULL, 0, chisel->pixels, 4*gs->gw);
    
    SDL_SetRenderTarget(gs->renderer, prev_target);
}

void chisel_update_texture(void) {
    struct Chisel *chisel = gs->chisel;
    
    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_CHISEL), SDL_BLENDMODE_BLEND);
    
    chisel_draw_target(chisel, 0, 0, RENDER_TARGET_CHISEL);
}

void chisel_set_depth(void) {
    struct Chisel *chisel = gs->chisel;
    struct Cell *grid = gs->grid;
    
    switch (chisel->size) {
        case 0: {
            grid[(int)chisel->x + ((int)chisel->y)*gs->gw].depth = 127;
            break;
        }
        case 1: {
            for (int y = 0; y < gs->gh; y++) {
                for (int x = 0; x < gs->gw; x++) {
                    if (chisel->pixels[x+y*gs->gw] == 0x9B9B9B) {
                        const int amt = 127;
                        if (grid[x+y*gs->gw].depth > amt)
                            grid[x+y*gs->gw].depth -= amt;
                    }
                }
            }
            break;
        }
    }
}

void set_all_chisel_positions(void) {
    struct Chisel *chisels = &gs->chisel_small;
    
    for (int i = 0; i < 3; i++) {
        struct Chisel *curr = &chisels[i];
        if (curr != gs->chisel) {
            curr->x = gs->chisel->x;
            curr->y = gs->chisel->y;
            curr->angle = gs->chisel->angle;
        }
    }
}

void click_gui_tool_button(void *type_ptr);
void chisel_tick(void) {
    struct Chisel *chisel = gs->chisel;
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;
    struct Input *input = &gs->input;
    struct Object *objects = gs->objects;
    
    bool prev_changing_angle = chisel->is_changing_angle;
    
    chisel->did_chisel_this_frame = false;
    
    chisel->is_changing_angle = input->keys[SDL_SCANCODE_LSHIFT];
    
    int grid_null = true;
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (gs->grid[i].type) {
            grid_null = false;
            break;
        }
    }
    
    if (grid_null) {
        int type = TOOL_GRABBER;
        
        click_gui_tool_button(&type);
        return;
    }
    
    if (chisel->size != 0) {
        for (int i = 0; i < gs->object_count; i++) {
            object_generate_blobs(i, chisel->size);
        }
    }
    
    if (!gs->did_chisel_tutorial) {
        struct Tutorial_Rect *t = tutorial_rect(TUTORIAL_CHISEL_ROTATE_STRING,
                                                32,
                                                GUI_H+32,
                                                NULL);
        gs->tutorial = *t;
        gs->did_chisel_tutorial = true;
    }
    
    if (prev_changing_angle && !chisel->is_changing_angle) {
        move_mouse_to_grid_position(chisel->x, chisel->y);
        input->mx = (int)chisel->x;
        input->my = (int)chisel->y;
    }
    
    if (hammer->state == HAMMER_STATE_IDLE && !chisel->is_changing_angle && !chisel->click_cooldown) {
        int index = clamp_to_grid(input->mx,
                                  input->my,
                                  !chisel->face_mode,
                                  false,
                                  true,
                                  true);
        gs->overlay.current_material = get_neighbour_type_in_direction(index%gs->gw, index/gs->gw, chisel->angle);
            
        if (index != -1) {
            chisel->x = (f32) (index%gs->gw);
            chisel->y = (f32) (index/gs->gw);
            
            // Highlight the current blob.
            // This is a fake chiseling- we're resetting position afterwards.
            f32 chisel_dx = cosf(2.f*(f32)M_PI * ((chisel->angle+180) / 360.f));
            f32 chisel_dy = sinf(2.f*(f32)M_PI * ((chisel->angle+180) / 360.f));
            f32 dx = chisel->spd * chisel_dx;
            f32 dy = chisel->spd * chisel_dy;
            f32 len = sqrtf(dx*dx + dy*dy);
            f32 ux = dx/len;
            f32 uy = dy/len;
            
            switch ((int)chisel->angle) {
                case 135:
                ux = 1;
                uy = -1;
                break;
                case 225:
                ux = 1;
                uy = 1;
                break;
                case 270:
                ux = 0;
                uy = 1;
                break;
                case 315:
                ux = -1;
                uy = 1;
                break;
            }
            
            struct Chisel copy = *chisel;
            
            int obj = gs->object_current;
            
            Uint32 blob_highlight = chisel_goto_blob(obj, false, ux, uy, len);
            
            *chisel = copy;
            
            memset(chisel->highlights, 0, chisel->highlight_count);
            chisel->highlight_count = 0;
            
            if (blob_highlight > 0) {
                if (blob_can_destroy(obj, chisel->size, blob_highlight)) {
                    for (int i = 0; i < gs->gw*gs->gh; i++) {
                        Uint32 b = objects[obj].blob_data[chisel->size].blobs[i];
                        if (b == blob_highlight) {
                            chisel->highlights[chisel->highlight_count++] = i;
                        }
                    }
                }
            }
        }
    }
    
    if (chisel->is_changing_angle) {
        f32 rmx = (f32)input->real_mx / (f32)gs->S;
        f32 rmy = (f32)(input->real_my-GUI_H) / (f32)gs->S;
        chisel->angle = 180 + 360 * atan2f(rmy - chisel->y, rmx - chisel->x) / (f32)(2*M_PI);
        
        f32 step = 45.0;
        if (chisel->face_mode) {
            step = 22.5;
        }
        chisel->angle /= step;
        chisel->angle = ((int)chisel->angle) * step;
        
        //SDL_ShowCursor(1);
    } else {
        // f32 dx = chisel->x - input->mx;
        // f32 dy = chisel->y - input->my;
        // f32 dist = sqrt(dx*dx + dy*dy);
        //SDL_ShowCursor(dist > 1);
    }
    
    if (input->mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        chisel->is_changing_angle = 0;
    }
    
    if (chisel->click_cooldown) {
        if (chisel->click_cooldown >= CHISEL_COOLDOWN-CHISEL_TIME) {
            // Cut out the stone.
            f32 px = chisel->x;
            f32 py = chisel->y;
            f32 ux = cosf(2.f * (f32)M_PI * ((chisel->angle+180) / 360.f));
            f32 uy = sinf(2.f * (f32)M_PI * ((chisel->angle+180) / 360.f));
            f32 len = chisel->spd;
            
            switch ((int)chisel->angle) {
                case 135:
                ux = 1;
                uy = -1;
                break;
                case 225:
                ux = 1;
                uy = 1;
                break;
                case 270:
                ux = 0;
                uy = 1;
                break;
                case 315:
                ux = -1;
                uy = 1;
                break;
            }
            
            if (chisel->face_mode) {
                while (sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y)) < len) {
                    chisel->x += ux;
                    chisel->y += uy;
                    chisel_set_depth();
                    chisel_update_texture();
                }
            } else if (!chisel->did_remove && gs->object_current != -1) {
                chisel_goto_blob(gs->object_current, true, ux, uy, len);
            }
            move_mouse_to_grid_position(chisel->x, chisel->y);
            input->mx = (int)chisel->x;
            input->my = (int)chisel->y;
        }
        chisel->click_cooldown--;
        if (chisel->click_cooldown == 0) {
            chisel->line = NULL;
            int index = clamp_to_grid(input->mx,
                                      input->my,
                                      !chisel->face_mode,
                                      false,
                                      true,
                                      true);
            gs->overlay.current_material = get_neighbour_type_in_direction(index%gs->gw, index/gs->gw, chisel->angle);
            
            chisel->x = (f32) (index%gs->gw);
            chisel->y = (f32) (index/gs->gw);
        }
    }
    
    set_all_chisel_positions();
    
    chisel_hammer_tick();
}

void chisel_draw(void) {
    struct Chisel *chisel = gs->chisel;
    
    chisel_update_texture();
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL), NULL, NULL);
    
    bool close = false;
    bool hit = false;
    
    for (int i = 0; i < chisel->highlight_count; i++) {
        int x = chisel->highlights[i]%gs->gw;
        int y = chisel->highlights[i]/gs->gh;
        
        if (gs->overlay.grid[x+y*gs->gw] == gs->grid[x+y*gs->gw].type) {
            hit = true;
            break;
        }
        
        // We don't need to loop if we already know at least
        // one of them is close.
        if (!close) {
            int r = 5;
            for (int yy = -r; yy <= r; yy++) {
                for (int xx = -r; xx <= r; xx++) {
                    if (xx == 0 && yy == 0) continue;
                    if (x+xx < 0 || x+xx >= gs->gw) continue;
                    if (y+yy < 0 || y+yy >= gs->gh) continue;
                    
                    if (gs->overlay.grid[x+xx+(y+yy)*gs->gw]) {
                        close = true;
                        goto next_highlight_loop;
                    }
                }
            }
        }
        
        next_highlight_loop:;
    }
    
    // Draw the highlights for blobs now.
    
    if (DRAW_CHISEL_HIGHLIGHTS) {
        for (int i = 0; i < chisel->highlight_count; i++) {
            int x = chisel->highlights[i]%gs->gw;
            int y = chisel->highlights[i]/gs->gw;
            
            if (gs->overlay.show) {
                if (hit) {
                    if (gs->overlay.grid[x+y*gs->gw]) {
                        SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 255);
                    } else {
                        SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 60);
                    }
                } else {
                    SDL_SetRenderDrawColor(gs->renderer, 0, 255, 0, 150);
                }
            } else {
                SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 35);
            }

            SDL_RenderDrawPoint(gs->renderer, x, y);
        }
    }
}

