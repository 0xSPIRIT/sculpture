struct Placer *get_current_placer(void) {
    struct Placer *placers = gs->placers;
    if (gs->current_tool != TOOL_PLACER) return NULL;
    return &placers[gs->current_placer];
}

void placer_init(int num) {
    struct Placer *placer = &gs->placers[num];
    
    placer->state = PLACER_PLACE_CIRCLE_MODE;
    placer->index = num;
    placer->x = gs->gw/2;
    placer->y = gs->gh/2;
    
    placer->texture = gs->textures.placer;
    SDL_QueryTexture(placer->texture, NULL, NULL, &placer->w, &placer->h);
    
    placer->object_index = -1;
    placer->did_click = 0;
    placer->contains_type = CELL_QUARTZ;
    placer->contains_amount = 100;
    placer->radius = 2;

    placer->placing_solid_time = 0;

    placer->rect.x = placer->rect.y = -1;
}

// Places a circle down.
void placer_place_circle(struct Placer *placer) {
    f32 dx = (f32) (placer->x - placer->px);
    f32 dy = (f32) (placer->y - placer->py);
    f32 len = sqrtf(dx*dx + dy*dy);
    f32 ux = dx/len;
    f32 uy = dy/len;

    f32 fx = (f32) placer->px;
    f32 fy = (f32) placer->py;

    int did_set_object = 1;

    placer->did_set_new = 0;

    if (is_cell_hard(placer->contains_type)) {
        placer->placing_solid_time++;
        if (placer->placing_solid_time >= MAX_PLACE_SOLID_TIME) {
            return;
        }
    }

    while ((len == 0 || sqrt((fx-placer->px)*(fx-placer->px) + (fy-placer->py)*(fy-placer->py)) < len) && placer->contains_amount > 0) {
        int radius = placer->radius;

        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y > radius*radius) continue;
                if (!is_in_boundsf(x+fx, y+fy)) continue;

                if (placer->contains_amount <= 0) {
                    placer->contains_amount = 0;
                    goto end1;
                }

                if (is_cell_hard(placer->contains_type) &&
                    (gs->grid[(int)(x+fx)+(int)(fy+y)*gs->gw].type == 0 ||
                     gs->grid[(int)(x+fx)+(int)(fy+y)*gs->gw].object == placer->object_index))
                {
                    placer->did_set_new = 1;
                }

                if (gs->grid[(int)(x+fx)+(int)(fy+y)*gs->gw].type) continue;
                int object_index = placer->object_index;

                if (!is_cell_hard(placer->contains_type)) {
                    object_index = -1;
                    did_set_object = 0;
                }

                set((int)(x+fx), (int)(y+fy), placer->contains_type, object_index);
                placer->contains_amount--;
            }
        }

    end1:
        if (len == 0) break;
        fx += ux;
        fy += uy;
    }

    placer->x = (int)fx;
    placer->y = (int)fy;

    // Stop drawing / reset everything if we stopped.
    if (!placer->did_set_new) {
        if (placer->object_index != -1) {
            object_generate_blobs(placer->object_index, 0);
            object_generate_blobs(placer->object_index, 1);
            object_generate_blobs(placer->object_index, 2);
        }
        placer->did_click = 0;
        return;
    }

    placer->did_click = did_set_object;
}

void placer_set_and_resize_rect(struct Placer *placer) {
    struct Input *input = &gs->input;

    if (placer->rect.x == -1) {
        placer->rect.x = input->mx;
        placer->rect.y = input->my;
    }

    placer->rect.w = input->mx - placer->rect.x;
    placer->rect.h = input->my - placer->rect.y;

    int area = abs((placer->rect.w) * (placer->rect.h));
    if (area > placer->contains_amount) {
        placer->rect.h = clamp(placer->rect.h, -placer->contains_amount, placer->contains_amount);

        int placer_rect_h = placer->rect.h;
        if (placer_rect_h == 0) placer_rect_h = 1;
        int w = abs(placer->contains_amount / placer_rect_h);
        placer->rect.w = clamp(placer->rect.w, -w, w);

        int placer_rect_w = placer->rect.w;
        if (placer_rect_w == 0) placer_rect_w = 1;
        int h = abs(placer->contains_amount / placer_rect_w);
        placer->rect.h = clamp(placer->rect.h, -h, h);
    }
}

void placer_place_rect(struct Placer *placer) {
    if (placer->rect.x == -1) return;

    placer->rect.w--;
    placer->rect.h--;
    
    if (placer->rect.w < 0) {
        placer->rect.x += placer->rect.w;
    }
    if (placer->rect.h < 0) {
        placer->rect.y += placer->rect.h;
    }
    placer->rect.w = abs(placer->rect.w);
    placer->rect.h = abs(placer->rect.h);
    
    int object_index = 0;
    
    // Check if we're intersecting with any other object.
    // If so, then we just join that object with this by
    // using its object index, not a new one.
    for (int y = placer->rect.y; y <= placer->rect.y+placer->rect.h; y++) {
        for (int x = placer->rect.x; x <= placer->rect.x+placer->rect.w; x++) {
            if (!is_in_bounds(x, y)) continue;
            if (gs->grid[x+y*gs->gw].object != 0) {
                object_index = gs->grid[x+y*gs->gw].object;
                break;
            }
        }
        
        if (object_index)
            break;
    }
    
    if (object_index) {
        placer->object_index = object_index;
    } else {
        placer->object_index = gs->object_count++;
    }

    for (int y = placer->rect.y; y <= placer->rect.y+placer->rect.h; y++) {
        for (int x = placer->rect.x; x <= placer->rect.x+placer->rect.w; x++) {
            if (!is_in_bounds(x, y)) continue;
            if (placer->contains_amount <= 0) {
                placer->contains_amount = 0;
                goto end2;
            }
            if (gs->grid[x+y*gs->gw].type == placer->contains_type) continue; // Don't overwrite anything.

            object_index = placer->object_index;
            if (!is_cell_hard(placer->contains_type)) {
                object_index = -1;
            }

            set(x, y, placer->contains_type, object_index);
            placer->contains_amount--;
        }
    }

 end2:             
    placer->rect.x = -1;
    placer->rect.y = -1;
    placer->rect.w = 0;
    placer->rect.h = 0;

    if (placer->object_index != -1) {
        object_generate_blobs(placer->object_index, 0);
        object_generate_blobs(placer->object_index, 1);
        object_generate_blobs(placer->object_index, 2);
    }
}

// Suck up the stuff if LMB is down.
void placer_suck(struct Placer *placer) {
    if (gs->gui.popup) return;

    struct Input *input = &gs->input;

    int can_continue = 0;
    if (input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        can_continue = 1;
    }

    if (!can_continue) return;

    f32 x = (f32) placer->px;
    f32 y = (f32) placer->py;

    f32 dx = (f32) (input->mx - x);
    f32 dy = (f32) (input->my - y);
    f32 len = sqrtf(dx*dx + dy*dy);
    f32 ux, uy;
    if (len == 0) {
        ux = 0;
        uy = 0;
        len = 1;
    } else {
        ux = dx/len;
        uy = dy/len;
    }

    while (distance(x, y, (f32)placer->px, (f32)placer->py) < len) {

        // Include the grid as well as pickup grid.
        for (int a = 0; a < 3; a++) {
            struct Cell *arr = NULL;

            switch (a) {
            case 0:
                arr = gs->grid;
                break;
            case 1:
                arr = gs->pickup_grid;
                break;
            case 2:
                arr = gs->gas_grid;
                break;
            }

            // Remove cells in a circle
            const int r = placer->radius;

            for (int ky = -r; ky <= r; ky++) {
                for (int kx = -r; kx <= r; kx++) {
                    if (kx*kx + ky*ky > r*r)  continue;

                    int xx = (int) (x+kx);
                    int yy = (int) (y+ky);
                    if (!is_in_bounds(xx, yy)) continue;

                    int type = arr[xx+yy*gs->gw].type;
                    if (type == 0) continue;

                    if (arr != gs->pickup_grid && is_cell_hard(type)) {
                        continue;
                    }
                    
                    if (placer->contains_type == type || placer->contains_type == 0 || placer->contains_amount == 0) {
                        placer->contains_type = type;
                        placer->contains_amount++;
                        set_array(arr, xx, yy, 0, -1);
                    }
                }
            }

        }

        x += ux;
        y += uy;
        if (ux == 0 && uy == 0) break;
    }

    objects_reevaluate();
}

void placer_tick(struct Placer *placer) {
    struct Input *input = &gs->input;

    placer->px = placer->x;
    placer->py = placer->y;

    placer->x = gs->input.mx;
    placer->y = gs->input.my;
    
    if (gs->creative_mode) {
        placer->contains_amount = gs->gw*gs->gh;
    }
    
    // Switch from ejecting to sucking.
    if (input->keys_pressed[SDL_SCANCODE_P]) {
        if (placer->state == PLACER_SUCK_MODE) {
            placer->state = PLACER_PLACE_CIRCLE_MODE;
        } else {
            placer->state = PLACER_SUCK_MODE;
        }
    }

    // If the cell type is hard, use rectangle placing, otherwise use brush/circle placing.
    if (placer->state != PLACER_SUCK_MODE) {
        if (is_cell_hard(placer->contains_type)) {
            placer->state = PLACER_PLACE_RECT_MODE;
        } else {
            placer->state = PLACER_PLACE_CIRCLE_MODE;
        }
    }

    if (gs->is_mouse_over_any_button) return;

    switch (placer->state) {
    case PLACER_PLACE_CIRCLE_MODE:
        if (input->mouse_pressed[SDL_BUTTON_LEFT] &&
            is_cell_hard(placer->contains_type) &&
            placer->contains_amount > 0 &&
            !gs->gui.popup && gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT))
        {
            save_state_to_next();
            placer->object_index = gs->object_count++;
        }

        if (placer->contains_amount > 0 && !gs->gui.popup && (input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT))) {
            placer_place_circle(placer);
        } else if (placer->did_click) {
            if (placer->object_index != -1) {
                object_generate_blobs(placer->object_index, 0);
                object_generate_blobs(placer->object_index, 1);
                object_generate_blobs(placer->object_index, 2);
            }
            placer->placing_solid_time = 0;
            placer->did_click = 0;
        }
        break;
    case PLACER_PLACE_RECT_MODE:
        if (gs->gui.popup) break;

        if (input->mouse_pressed[SDL_BUTTON_LEFT]) {
            save_state_to_next();
        }
        
        if (input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            placer_set_and_resize_rect(placer);
        } else {
            placer_place_rect(placer);
        }
        break;
    case PLACER_SUCK_MODE:
        placer_suck(placer);
        break;
    }

    // Set up the tooltip.
    tooltip_set_position(&gs->gui.tooltip, placer->x + placer->w/2 + 3, placer->y - placer->h, TOOLTIP_TYPE_PLACER);

    strcpy(gs->gui.tooltip.str[0], "Placer");

    // Get name from type.
    char string[256] = {0};
    tooltip_get_string(placer->contains_type, placer->contains_amount, string);

    if (!gs->gui.popup) {
        if (placer->state == PLACER_PLACE_CIRCLE_MODE || placer->state == PLACER_PLACE_RECT_MODE) {
            strcpy(gs->gui.tooltip.str[1], "Mode: [PLACE]");
        } if (placer->state == PLACER_SUCK_MODE) {
            strcpy(gs->gui.tooltip.str[1], "Mode: [TAKE]");
        } 
        strcpy(gs->gui.tooltip.str[2], string);
    } else {
        strcpy(gs->gui.tooltip.str[1], string);
    }
}

void placer_draw(struct Placer *placer, bool full_size) {
    const int scale = full_size ? gs->S : 1;
    int y_off = full_size ? GUI_H : 0;
    
    if (placer->state == PLACER_SUCK_MODE || placer->state == PLACER_PLACE_CIRCLE_MODE) {
        int radius = placer->radius;
        int fx = placer->x;
        int fy = placer->y;
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y > radius*radius) continue;
                if (x+fx < 0 || x+fx >= gs->gw || y+fy < 0 || y+fy >= gs->gh) continue;
                if (placer->state == PLACER_SUCK_MODE) {
                    SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 64);
                } else {
                    SDL_SetRenderDrawColor(gs->renderer, 0, 255, 0, 64);
                }

                int gx = x+fx;
                int gy = y+fy;
                SDL_RenderDrawPoint(gs->renderer, scale*gx, scale*gy + y_off);
            }
        }
    }

    SDL_Rect dst = {
        placer->x - placer->w/2, placer->y - placer->h,
        placer->w, placer->h
    };

    dst.x *= scale;
    dst.y *= scale;
    dst.w *= scale;
    dst.h *= scale;

    dst.y += y_off;

    switch (placer->index) {
    case 0:
        SDL_SetTextureColorMod(placer->texture, 255, 0, 0);
        break;
    case 1:
        SDL_SetTextureColorMod(placer->texture, 0, 255, 0);
        break;
    case 2:
        SDL_SetTextureColorMod(placer->texture, 0, 0, 255);
        break;
    }

    SDL_RenderCopy(gs->renderer, placer->texture, NULL, &dst);

    if (placer->rect.x != -1) {
        SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(gs->renderer, &placer->rect);
    }
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 64);
    
    SDL_RenderDrawLine(gs->renderer, 0, placer->y, gs->gw, placer->y);
    SDL_RenderDrawLine(gs->renderer, placer->x, 0, placer->x, gs->gh);
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 255, 255, 255);
}

bool is_mouse_in_placer(struct Placer *placer) {
    struct Input *input = &gs->input;

    SDL_Point mouse = {input->mx, input->my};
    SDL_Rect rectangle = {
        placer->x - placer->w/2,
        placer->y - placer->h,
        placer->w, placer->h
    };
    return is_point_in_rect(mouse, rectangle);
}

/* SDL_RendererFlip get_placer_flip(struct Converter *converter, int placer_socket) { */
/*     if (converter == furnace && placer_socket == PLACER_OUTPUT) { */
/*         return SDL_FLIP_VERTICAL; */
/*     } */
/*     return SDL_FLIP_NONE; */
/* } */
