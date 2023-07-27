static Placer *get_current_placer(void) {
    Placer *placers = gs->placers;
    if (gs->current_tool != TOOL_PLACER) return null;
    return &placers[gs->current_placer];
}

static void placer_init(int num) {
    Placer *placer = &gs->placers[num];

    placer->state = PLACER_PLACE_CIRCLE_MODE;
    placer->index = num;
    placer->x = gs->gw/2;
    placer->y = gs->gh/2;

    placer->texture = &GetTexture(TEXTURE_PLACER);
    placer->w = placer->texture->width;
    placer->h = placer->texture->height;

    placer->object_index = -1;
    placer->did_click = 0;
    placer->radius = 4;

    placer->placing_solid_time = 0;

    placer->rect.x = placer->rect.y = -1;

    placer->contains = &gs->inventory.slots[num].item;
    placer->contains->type = 0;
    placer->contains->amount = 0;

    placer->place_aspect = 24.0/16.0;
    placer->place_width = 16;
    placer->place_height = placer->place_width * placer->place_aspect;
}

static void placer_try_hard_tutorial() {
    if (!gs->did_placer_hard_tutorial) {
        gs->tutorial = *tutorial_rect(TUTORIAL_PLACER_HARD, null);
        gs->did_placer_hard_tutorial = true;
    }
}

static void placer_suck_circle(Placer *placer) {
    Input *input = &gs->input;

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

    Cell *arr = gs->grid;

    Cell_Type effect_pickup = effect_picked_up(&gs->current_effect);
        
    bool took_anything = false;
    while (arr == gs->grid || arr == gs->gas_grid) {
        while (distance(x, y, (f32)placer->px, (f32)placer->py) < len) {
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

                    if (is_cell_hard(type)) { placer_try_hard_tutorial(); continue; }

                    if (placer->contains->type == type || placer->contains->type == 0 || placer->contains->amount == 0) {
                        placer->contains->type = type;

                        bool is_initial = gs->grid[xx+yy*gs->gw].is_initial;
                        int amt = 1;

                        if (is_initial) {
                            amt = my_rand(xx+yy*gs->gw)%2 == 0 ? 1 : 2;
                        }

                        placer->contains->amount += amt;
                        took_anything = true;

                        set_array(arr, xx, yy, 0, -1);
                        placer->did_take_anything = true;
                    }
                }
            }
            
            if (!took_anything && effect_pickup) {
                effect_handle_placer(&gs->current_effect, x, y, r);
            }

            x += ux;
            y += uy;
            if (ux == 0 && uy == 0) break;

        }

        if (arr == gs->gas_grid) break;
        arr = gs->gas_grid;
    }

    objects_reevaluate();
}

// Places a circle down.
static void placer_place_circle(Placer *placer) {
    f32 dx = (f32) (placer->x - placer->px);
    f32 dy = (f32) (placer->y - placer->py);
    f32 len = sqrtf(dx*dx + dy*dy);
    f32 ux = dx/len;
    f32 uy = dy/len;

    f32 fx = (f32) placer->px;
    f32 fy = (f32) placer->py;

    int did_set_object = 1;

    placer->did_set_new = 0;

    if (!placer->was_placing) {
        save_state_to_next();
    }
    placer->was_placing = true;

    if (is_cell_hard(placer->contains->type)) {
        placer->placing_solid_time++;
        if (placer->placing_solid_time >= MAX_PLACE_SOLID_TIME) {
            return;
        }
    }

    while ((len == 0 || sqrt((fx-placer->px)*(fx-placer->px) + (fy-placer->py)*(fy-placer->py)) < len) && placer->contains->amount > 0) {
        int radius = placer->radius;

        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y > radius*radius)  continue;
                if (!is_in_boundsf(x+fx, y+fy)) continue;

                if (placer->contains->amount <= 0) {
                    placer->contains->amount = 0;
                    goto end1;
                }

                if (is_cell_hard(placer->contains->type) &&
                    (gs->grid[(int)(x+fx)+(int)(fy+y)*gs->gw].type == 0 ||
                     gs->grid[(int)(x+fx)+(int)(fy+y)*gs->gw].object == placer->object_index))
                {
                    placer->did_set_new = 1;
                }

                if (gs->grid[(int)(x+fx)+(int)(fy+y)*gs->gw].type) continue;
                int object_index = placer->object_index;

                if (!is_cell_hard(placer->contains->type)) {
                    object_index = -1;
                    did_set_object = 0;
                }

                set((int)(x+fx), (int)(y+fy), placer->contains->type, object_index);
                placer->contains->amount--;
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
        placer->did_click = 0;
        return;
    }

    placer->did_click = did_set_object;
}

static void fix_rectangle(SDL_Rect *c) {
    c->w--;
    c->h--;

    if (c->w < 0) {
        c->x += c->w;
    }
    if (c->h < 0) {
        c->y += c->h;
    }
    if (c->x+c->w >= gs->gw) {
        c->w = gs->gw - 1 - c->x;
    }
    if (c->y+c->h >= gs->gh) {
        c->h = gs->gh - 1 - c->y;
    }
    c->w = abs(c->w);
    c->h = abs(c->h);
}

static bool placer_is_able_to_place(Placer *placer, SDL_Rect *c, int *area_out) {
    Assert(c);

    if (placer->contains->amount == 0)
        return false;

    fix_rectangle(c);

    int area = abs((c->w+1) * (c->h+1));

    if (area_out)
        *area_out = area;

    // Test if we have enough by placing test cells into position
    int free_cells = 0;
    for (int y = c->y; y <= c->y+c->h; y++) {
        for (int x = c->x; x <= c->x+c->w; x++) {
            if (gs->grid[x+y*gs->gw].type == CELL_NONE) {
                free_cells++;
            }
        }
    }

    if (placer->contains->amount < free_cells)
        return false;

    else if (placer->contains->amount <= PLACER_MINIMUM_AREA)
        return true;

    return area >= PLACER_MINIMUM_AREA;
}

static void placer_set_and_resize_rect(Placer *placer, int mx, int my) {
    if (placer->rect.x == -1) {
        placer->rect.x = mx;
        placer->rect.y = my;
    }

    mx++;
    my++;

    placer->rect.w = mx - placer->rect.x;
    placer->rect.h = my - placer->rect.y;

    SDL_Rect copy = placer->rect; // We don't want to actually change the rectangle now.

    int area = -1;
    if (!placer_is_able_to_place(placer, &copy, &area))
        return;

    Assert(area != -1);

    if (area > placer->contains->amount) {
        placer->rect.h = clamp(placer->rect.h,
                               -placer->contains->amount,
                               placer->contains->amount);

        int placer_rect_h = placer->rect.h;
        if (placer_rect_h == 0) placer_rect_h = 1;
        int w = abs(placer->contains->amount / placer_rect_h);
        placer->rect.w = clamp(placer->rect.w, -w, w);

        int placer_rect_w = placer->rect.w;
        if (placer_rect_w == 0) placer_rect_w = 1;
        int h = abs(placer->contains->amount / placer_rect_w);
        placer->rect.h = clamp(placer->rect.h, -h, h);
    }
}

static void placer_place_rect(Placer *placer) {
    if (placer->rect.x == -1) return;

    int area;
    bool able_to_place = placer_is_able_to_place(placer, &placer->rect, &area);

    if (!able_to_place) {
        placer->rect.x = -1;
        placer->rect.y = -1;
        placer->rect.w = 0;
        placer->rect.h = 0;
        return;
    }

    save_state_to_next();

    int object_index = -1;

    // Check if we're intersecting with any other object and
    // if so, then we just join that object with this by
    // using its object index, not a new one.
    //
    // We check with 1 extra row/column to the rectangle
    // to make things connect when the player wants things to.
    for (int y = placer->rect.y-1; y <= placer->rect.y+placer->rect.h+1; y++) {
        for (int x = placer->rect.x-1; x <= placer->rect.x+placer->rect.w+1; x++) {
            if (!is_in_bounds(x, y)) continue;
            if (!is_in_view(x, y)) continue;
            if (gs->grid[x+y*gs->gw].object != -1) {
                object_index = gs->grid[x+y*gs->gw].object;
                break;
            }
        }

        if (object_index != -1)
            break;
    }

    if (object_index != -1) {
        placer->object_index = object_index;
    } else {
        placer->object_index = gs->object_count++;
    }

    for (int y = placer->rect.y; y <= placer->rect.y+placer->rect.h; y++) {
        for (int x = placer->rect.x; x <= placer->rect.x+placer->rect.w; x++) {
            if (!is_in_bounds(x, y)) continue;
            if (!is_in_view(x, y)) continue;
            if (placer->contains->amount <= 0) {
                placer->contains->amount = 0;
                goto end2;
            }
            if (gs->grid[x+y*gs->gw].type) continue; // Don't overwrite anything.

            object_index = placer->object_index;
            if (!is_cell_hard(placer->contains->type)) {
                object_index = -1;
            }

            gs->has_any_placed = true;
            set(x, y, placer->contains->type, object_index);
            placer->contains->amount--;
            placer->did_place_this_frame = true;
        }
    }

    end2:
    placer->rect.x = -1;
    placer->rect.y = -1;
    placer->rect.w = 0;
    placer->rect.h = 0;

    objects_reevaluate();
}

static void placer_tick(Placer *placer) {
    Input *input = &gs->input;

    placer->did_place_this_frame = false;

    placer->px = placer->x;
    placer->py = placer->y;

    placer->x = gs->input.mx;
    placer->y = gs->input.my;

    if (gs->is_mouse_on_tab_icon) return; // Hacky!!!!! ew
    if (gs->conversions.active) return; // There should really be a focus variable

    if (gs->creative_mode) {
        placer->contains->amount = gs->gw*gs->gh;
    }

    // If the cell type is hard, use rectangle placing, otherwise use brush/circle placing.
    if (placer->contains->type == 0) {
        placer->state = PLACER_SUCK_MODE;
    }
    else if (placer->state == PLACER_PLACE_RECT_MODE ||
             placer->state == PLACER_PLACE_CIRCLE_MODE)
    {
        if (is_cell_hard(placer->contains->type)) {
            placer->state = PLACER_PLACE_RECT_MODE;
        } else {
            placer->state = PLACER_PLACE_CIRCLE_MODE;
        }

        if (placer->contains->type) { // If statement is redundant?
            gs->overlay.current_material = placer->contains->type;
        }
    }

    if (placer->contains->type && gs->input.keys_pressed[SDL_SCANCODE_P]) {
        if (placer->state == PLACER_SUCK_MODE) {
            placer->state = PLACER_PLACE_CIRCLE_MODE;
        } else if (placer->state == PLACER_PLACE_CIRCLE_MODE) {
            placer->state = PLACER_SUCK_MODE;
        }
    }

    if (is_cell_hard(placer->contains->type) && placer->state == PLACER_SUCK_MODE) {
        placer->state = PLACER_PLACE_RECT_MODE;
    }

    if (gs->is_mouse_over_any_button) return;

    bool skip=false;
    if (placer->escape_rect && input->mouse_released[SDL_BUTTON_LEFT]) {
        placer->escape_rect = false;
        skip=true;
    }

    bool can_click = !gs->tutorial.active && !gs->gui.eol_popup_confirm.active && !gs->gui.restart_popup_confirm.active;

    switch (placer->state) {
        case PLACER_SUCK_MODE: {
            if (!can_click) break;
            if (input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                placer_suck_circle(placer);
            } else if (input->mouse_released[SDL_BUTTON_LEFT]) {
                if (placer->did_take_anything) {
                    save_state_to_next();
                }
            } else {
                placer->did_take_anything = false;
            }
            break;
        }
        case PLACER_PLACE_CIRCLE_MODE: {
            if (!can_click) break;

            if (input->mouse_pressed[SDL_BUTTON_LEFT] &&
                is_cell_hard(placer->contains->type) &&
                placer->contains->amount > 0 &&
                !gs->gui.popup && gs->input.mouse & SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                save_state_to_next();
                placer->object_index = gs->object_count++;
            }

            if (placer->contains->amount > 0 && !gs->gui.popup && (input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT))) {
                placer_place_circle(placer);
            } else if (placer->did_click) {
                placer->placing_solid_time = 0;
                placer->did_click = 0;
            }

            if (placer->was_placing && !(input->mouse & SDL_BUTTON(SDL_BUTTON_LEFT))) {
                placer->was_placing = false;
            }
            break;
        }
        case PLACER_PLACE_RECT_MODE: {
            if (gs->gui.popup) break;
            if (gs->input.real_my < GUI_H) break;
            if (placer->escape_rect) break;
            if (skip) break;

            if (!gs->did_placer_rectangle_tutorial) {
                Tutorial_Rect *t = tutorial_rect(TUTORIAL_RECTANGLE_PLACE, null);
                gs->tutorial = *t;
                gs->did_placer_rectangle_tutorial = true;
            }

            //placer_set_and_resize_rect(placer, gs->input.mx, gs->input.my);
            placer->rect.w = placer->place_width;
            placer->rect.h = placer->place_height;
            placer->rect.x = gs->input.mx - placer->rect.w/2;
            placer->rect.y = gs->input.my - placer->rect.h/2;

            if (can_click && input->mouse_pressed[SDL_BUTTON_LEFT]) {
                placer_place_rect(placer);
            }
            break;
        }
    }

    // Set up the tooltip.
    int x = placer->x + 3 - (gs->render.view.x / gs->S);
    // Hardcode
    if (gs->gw == 128) x -= 32;
    tooltip_set_position(&gs->gui.tooltip,
                         x,
                         placer->y + 3 - (gs->render.view.y / gs->S),
                         TOOLTIP_TYPE_PLACER);

    char name[64] = {0};
    sprintf(name, "Placer %d", placer->index+1);
    strcpy(gs->gui.tooltip.str[0], name);

    // Get name from type.
    char string[256] = {0};
    tooltip_get_string(placer->contains->type, placer->contains->amount, string);

    if (!gs->gui.popup) {
        if (placer->contains->type && !is_cell_hard(placer->contains->type)) {
            if (placer->state == PLACER_SUCK_MODE) {
                strcpy(gs->gui.tooltip.str[1], "Mode: [TAKE] (P for Toggle)");
            } else {
                strcpy(gs->gui.tooltip.str[1], "Mode: [PLACE] (P for Toggle)");
            }
        } else {
            if (placer->state == PLACER_SUCK_MODE) {
                strcpy(gs->gui.tooltip.str[1], "Mode: [TAKE]");
            } else {
                strcpy(gs->gui.tooltip.str[1], "Mode: [PLACE]");
            }
        }
        strcpy(gs->gui.tooltip.str[2], string);
    } else {
        strcpy(gs->gui.tooltip.str[1], string);
    }
}

static void placer_draw(int target, Placer *placer, bool full_size) {
    const int scale = full_size ? gs->S : 1;
    int y_off = full_size ? GUI_H : 0;

    if (placer->state == PLACER_PLACE_CIRCLE_MODE || placer->state == PLACER_SUCK_MODE) {
        int radius = placer->radius;
        int fx = placer->x;
        int fy = placer->y;

        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y > radius*radius) continue;
                if (x+fx < 0 || x+fx >= gs->gw || y+fy < 0 || y+fy >= gs->gh) continue;

                if (placer->state == PLACER_SUCK_MODE) {
                    RenderColor(255, 0, 0, 64);
                } else {
                    RenderColor(0, 255, 0, 64);
                }

                int gx = x+fx;
                int gy = y+fy;
                RenderPointRelative(target, scale*gx, scale*gy + y_off);
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
        case 0: {
            RenderTextureColorMod(placer->texture, 255, 0, 0);
            break;
        }
        case 1: {
            RenderTextureColorMod(placer->texture, 0, 255, 0);
            break;
        }
        case 2: {
            RenderTextureColorMod(placer->texture, 0, 0, 255);
            break;
        }
    }

    RenderTexture(target,
                  placer->texture,
                  null,
                  &dst);

    if (placer->state == PLACER_PLACE_RECT_MODE && placer->rect.x != -1) {
        // Make sure the area is enough before you start drawing the rectangle.
        SDL_Rect c = placer->rect;

        bool able_to_place = placer_is_able_to_place(placer, &c, null);

        if (able_to_place) {
            RenderColor(255, 255, 0, 255);
        } else {
            RenderColor(255, 0, 0, 255);
        }

        RenderDrawRectRelative(target, placer->rect);
    }

    RenderColor(255, 255, 255, 64);

    RenderColor(255, 255, 255, 255);
}

static bool is_mouse_in_placer(Placer *placer) {
    Input *input = &gs->input;

    SDL_Point mouse = {input->mx, input->my};
    SDL_Rect rectangle = {
        placer->x - placer->w/2,
        placer->y - placer->h,
        placer->w, placer->h
    };
    return is_point_in_rect(mouse, rectangle);
}
