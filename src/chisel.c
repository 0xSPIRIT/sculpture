void chisel_init(struct Chisel *type) {
    struct Chisel *chisel = NULL;
    
    struct Chisel *chisel_small = &gs->chisel_small;
    struct Chisel *chisel_medium = &gs->chisel_medium;
    struct Chisel *chisel_large = &gs->chisel_large;

    gs->chisel = type;
    chisel = gs->chisel;

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

    chisel->pixels = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(Uint32));

    chisel->texture = chisel->outside_texture;
    chisel->w = chisel->outside_w;
    chisel->h = chisel->outside_h;

    chisel->highlights = arena_alloc(gs->persistent_memory, gs->gw*gs->gh, sizeof(int));
    chisel->highlight_count = 0;

    chisel->spd = 3.;
}

void chisel_set_depth() {
    struct Chisel *chisel = gs->chisel;
    struct Cell *grid = gs->grid;

    switch (chisel->size) {
    case 0:
        grid[(int)chisel->x + ((int)chisel->y)*gs->gw].depth = 127;
        break;
    case 1:
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

void chisel_tick() {
    struct Chisel *chisel = gs->chisel;
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;
    struct Input *input = &gs->input;
    struct Object *objects = gs->objects;

    bool prev_changing_angle = chisel->is_changing_angle;

    chisel->is_changing_angle = input->keys[SDL_SCANCODE_LSHIFT];

    if (prev_changing_angle && !chisel->is_changing_angle) {
        /* SDL_WarpMouseInWindow(gs->window, (int)chisel->x*gs->S, GUI_H + (int)chisel->y*gs->S); */
        move_mouse_to_grid_position(chisel->x, chisel->y);
        input->mx = (int)chisel->x;
        input->my = (int)chisel->y;
    }

    if (hammer->state == HAMMER_STATE_IDLE && !chisel->is_changing_angle && !chisel->click_cooldown) {
        int index = clamp_to_grid(input->mx, input->my, !chisel->face_mode, false, true, true);
        if (index != -1) {
            chisel->x = (f32) (index%gs->gw);
            chisel->y = (f32) (index/gs->gw);

            // Highlight the current blob.
            // This is a fake chiseling- we're resetting position afterwards.
            f32 chisel_dx = (f32) cosf(2.f*(f32)M_PI * ((chisel->angle+180) / 360.f));
            f32 chisel_dy = (f32) sinf(2.f*(f32)M_PI * ((chisel->angle+180) / 360.f));
            f32 dx = chisel->spd * chisel_dx;
            f32 dy = chisel->spd * chisel_dy;
            f32 len = sqrtf(dx*dx + dy*dy);
            f32 ux = dx/len;
            f32 uy = dy/len;

            struct Chisel copy = *chisel;

            Uint32 blob_highlight = chisel_goto_blob(false, ux, uy, len);

            *chisel = copy;

            if (blob_highlight > 0) {
                memset(chisel->highlights, 0, chisel->highlight_count);
                chisel->highlight_count = 0;

                if (blob_can_destroy(gs->object_current, chisel->size, blob_highlight)) {
                    for (int i = 0; i < gs->gw*gs->gh; i++) {
                        Uint32 b = objects[gs->object_current].blob_data[chisel->size].blobs[i];
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
        /* SDL_ShowCursor(1); */
    }/*  else { */
    /*     f32 dx = chisel->x - input->mx; */
    /*     f32 dy = chisel->y - input->my; */
    /*     f32 dist = sqrt(dx*dx + dy*dy); */
    /*     SDL_ShowCursor(dist > 1); */
    /* } */

    if (input->keys_pressed[SDL_SCANCODE_S]) {
        chisel->face_mode = !chisel->face_mode;
        chisel->w = chisel->face_mode ? chisel->face_w : chisel->outside_w;
        chisel->h = chisel->face_mode ? chisel->face_h : chisel->outside_h;
        chisel->texture = chisel->face_mode ? chisel->face_texture : chisel->outside_texture;
    }    

    if (input->mouse & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        chisel->is_changing_angle = 0;
    }

    if (chisel->click_cooldown) {
        if (chisel->click_cooldown >= CHISEL_COOLDOWN-CHISEL_TIME) {
            // Cut out the stone.
            f32 px = chisel->x;
            f32 py = chisel->y;
            f32 ux = (f32) cosf(2.f * (f32)M_PI * ((chisel->angle+180) / 360.f));
            f32 uy = (f32) sinf(2.f * (f32)M_PI * ((chisel->angle+180) / 360.f));
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
                    chisel_set_depth(chisel);
                    chisel_update_texture();
                }
            } else if (!chisel->did_remove && gs->object_current != -1) {
                chisel_goto_blob(true, ux, uy, len);
            }
            /* SDL_WarpMouseInWindow(gs->window, (int)(chisel->x * gs->S), GUI_H + (int)(chisel->y * gs->S)); */
            move_mouse_to_grid_position(chisel->x, chisel->y);
            input->mx = (int)chisel->x;
            input->my = (int)chisel->y;
        }
        chisel->click_cooldown--;
        if (chisel->click_cooldown == 0) {
            chisel->line = NULL;
            int index = clamp_to_grid(input->mx, input->my, !chisel->face_mode, false, true, true);
            chisel->x = (f32) (index%gs->gw);
            chisel->y = (f32) (index/gs->gw);
        }
    }
    chisel_hammer_tick();
}

void chisel_update_texture() {
    struct Chisel *chisel = gs->chisel;

    SDL_Texture *prev_target = SDL_GetRenderTarget(gs->renderer);
    SDL_SetTextureBlendMode(RenderTarget(RENDER_TARGET_CHISEL), SDL_BLENDMODE_BLEND);
    
    Assert(RenderTarget(RENDER_TARGET_CHISEL));
    SDL_SetRenderTarget(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL));

    SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gs->renderer);
    
    int x = (int)chisel->x;
    int y = (int)chisel->y;

    // Disgusting hardcoding to adjust the weird rotation SDL does.
    // TODO: Clean this up.
    if (!chisel->face_mode) {
        if (chisel->size == 0 || chisel->size == 1) {
            if (chisel->angle == 225) {
                x++;
                y += 2;
            } else if (chisel->angle == 180) {
                x++;
                y++;
            } else if (chisel->angle == 90 || chisel->angle == 45) {
                x++;
            } else if (chisel->angle == 135) {
                x += 2;
            }
        } else {
            if (chisel->angle == 0) {
                y--;
            } else if (chisel->angle == 270) {
                y++;
                x--;
            } else if (chisel->angle == 225) {
                y += 2;
            } else if (chisel->angle == 180) {
                x++;
                y += 2;
            } else if (chisel->angle == 90) {
                x += 2;
            } else if (chisel->angle == 45) {
                y -= 2;
            } else if (chisel->angle == 135) {
                x += 2;
                y++;
            } else if (chisel->angle == 315) {
                x--;
            }
        }
    }

    const SDL_Rect dst = {
        x, y - chisel->h/2,
        chisel->w, chisel->h
    };
    const SDL_Point center = { 0, chisel->h/2 };

    SDL_RenderCopyEx(gs->renderer, chisel->texture, NULL, &dst, chisel->angle, &center, SDL_FLIP_NONE);

    chisel_hammer_draw();

    if (!chisel->face_mode) {
        SDL_SetRenderDrawColor(gs->renderer, 127, 127, 127, 255);
        SDL_RenderDrawPoint(gs->renderer, (int)chisel->x, (int)chisel->y);
    }

    // PROBLEM AREA!
    int w, h;

    SDL_QueryTexture(RenderTarget(RENDER_TARGET_CHISEL), NULL, NULL, &w, &h);
    Assert(w == gs->gw && h == gs->gh);
    
    SDL_RenderReadPixels(gs->renderer, NULL, 0, chisel->pixels, 4*gs->gw);

    SDL_SetRenderTarget(gs->renderer, prev_target);
}

void chisel_draw() {
    struct Chisel *chisel = gs->chisel;

    chisel_update_texture();
    SDL_RenderCopy(gs->renderer, RenderTarget(RENDER_TARGET_CHISEL), NULL, NULL);

    // Draw the highlights for blobs now.
    if (DRAW_CHISEL_HIGHLIGHTS) {
        for (int i = 0; i < chisel->highlight_count; i++) {
            SDL_SetRenderDrawColor(gs->renderer, 255, 103, 93, 200);
            SDL_RenderDrawPoint(gs->renderer, chisel->highlights[i]%gs->gw, chisel->highlights[i]/gs->gw);
        }
    }
}

// if remove, delete the blob
// else, highlight it and don't change anything about the chisel's state.
// ux, uy = unit vector for the direction of chisel.
// px, py = initial positions.
// Returns the blob it reaches only if remove == 0.
Uint32 chisel_goto_blob(bool remove, f32 ux, f32 uy, f32 len) {
    struct Chisel *chisel = gs->chisel;
    struct Object *objects = gs->objects;
    Uint32 *curr_blobs = objects[gs->object_current].blob_data[chisel->size].blobs;

    bool did_hit_blocker = false;
    f32 px = chisel->x, py = chisel->y;

    if (gs->object_current == -1) return 0;

    f32 current_length = (f32) sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y));

    while (current_length < len) {
        // If we hit the chisel blocker, keep that in mind for later.

        struct Chisel_Blocker *cb = &gs->chisel_blocker;
        if (gs->current_tool == TOOL_CHISEL_MEDIUM &&
            cb->state != CHISEL_BLOCKER_OFF &&
            cb->pixels[(int)chisel->x + ((int)chisel->y)*gs->gw] != cb->side)
            {
                did_hit_blocker = true;
            }

        // If we come into contact with a cell, locate its blob
        // then remove it. We only remove one blob per chisel,
        // so we stop our speed right here.
        Uint32 b = curr_blobs[(int)chisel->x + (int)chisel->y*gs->gw];

        if (b > 0 && !remove) {
            if (gs->grid[(int)chisel->x + (int)chisel->y*gs->gw].type == 0) b = -1;
            return b;
        } else if (remove && b > 0 && !chisel->did_remove) {
            // We want to attempt to destroy this blob now.
            // Firstly, we want to do a diagonal check.
            
            //      /       
            //   xx/    xxx/
            //   xxx    xx/x
            //   xxx    xxxxx

            //  ^ This is what we want to prevent.

            if (chisel->size != TOOL_CHISEL_SMALL || number_direct_neighbours(gs->grid, (int)chisel->x, (int)chisel->y) < 4) {
                // We continue at our current direction until we reach
                // another blob, then backtrack one.
                while (curr_blobs[(int)chisel->x + (int)chisel->y*gs->gw] == b) {
                    chisel->x += ux;
                    chisel->y += uy;
                }

                chisel->x -= ux;
                chisel->y -= uy;

                if (blob_can_destroy(gs->object_current, chisel->size, b)) {
                    object_remove_blob(gs->object_current, b, chisel->size, true);

                    move_mouse_to_grid_position(chisel->x, chisel->y);
                    chisel->did_remove = true;
                }

                chisel->click_cooldown = CHISEL_COOLDOWN-CHISEL_TIME-1;
            }

            break;
        }

        chisel->x += ux;
        chisel->y += uy;

        current_length = (f32) sqrt((px-chisel->x)*(px-chisel->x) + (py-chisel->y)*(py-chisel->y));
    }

    Uint32 b = curr_blobs[(int)chisel->x + (int)chisel->y*gs->gw];
    if (!remove) {
        if (gs->grid[(int)chisel->x + (int)chisel->y*gs->gw].type == 0) b = -1;
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
                    object_remove_blob(gs->object_current, blob, chisel->size, true);
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
        for (int i = 0; i < gs->object_count; i++) {
            if (objects[i].cell_count <= 4) {
                convert_object_to_dust(i);
                did_remove = true;
            }
        }
        if (did_remove) {
            objects_reevaluate();
        }
    }

    return 0;
}

void chisel_hammer_init() {
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

void chisel_hammer_tick() {
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

    const int stop = 24;
    
    switch (hammer->state) {
    case HAMMER_STATE_WINDUP:
        hammer->time++;
        if (hammer->time < 3) {
            hammer->dist += hammer->time * 4;
        } else {
            hammer->time = 0;
            hammer->state = HAMMER_STATE_SWING;
        }
        break;
    case HAMMER_STATE_SWING:
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
    case HAMMER_STATE_IDLE:
        hammer->dist = hammer->normal_dist;
        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            hammer->state = HAMMER_STATE_WINDUP;
            save_state_to_next();
        }
        break;
    }
}

void chisel_hammer_draw() {
    struct Chisel_Hammer *hammer = &gs->chisel_hammer;

    const SDL_Rect dst = {
        (int)hammer->x, (int)(hammer->y - hammer->h/2),
        hammer->w, hammer->h
    };
    const SDL_Point center = { 0, hammer->h/2 };

    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (hammer->angle > 90 && hammer->angle < 270) {
        flip = SDL_FLIP_VERTICAL;
    }

    SDL_RenderCopyEx(gs->renderer, hammer->texture, NULL, &dst, hammer->angle, &center, flip);
}
