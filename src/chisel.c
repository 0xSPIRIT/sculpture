static Chisel chisel_init(Chisel_Size size) {
    Chisel chisel = {0};
    
    chisel.size = size;
    chisel.texture = &GetTexture(TEXTURE_CHISEL+size);
    chisel.lookahead = 5;
    
    chisel.mask = PushSize(gs->persistent_memory,
                           gs->gw*gs->gh*1);
    
    return chisel;
}

static bool is_tool_chisel(void) {
    return gs->current_tool >= TOOL_CHISEL_SMALL && gs->current_tool <= TOOL_CHISEL_LARGE;
}

static void chisel_play_sound(Cell_Type material, int size) {
    int channel = AUDIO_CHANNEL_CHISEL;
    
    switch (material) {
        case CELL_ICE: {
            switch (size) {
                case CHISEL_SMALL:  {
                    int indices[] = {5, 6};
                    Mix_PlayChannel(channel, gs->audio.ice_chisel[indices[rand()%2]], 0); 
                } break;
                case CHISEL_MEDIUM: {
                    int indices[] = {1, 2, 3};
                    int idx = indices[rand()%3];
                    Mix_PlayChannel(channel, gs->audio.ice_chisel[idx], 0); 
                } break;
                case CHISEL_LARGE:  { Mix_PlayChannel(channel, gs->audio.ice_chisel[0], 0); } break;
            }
        } break;
        default: {
            switch (size) {
                case CHISEL_SMALL: { Mix_PlayChannel(channel, gs->audio.small_chisel, 0); } break;
                case CHISEL_MEDIUM: { Mix_PlayChannel(channel, gs->audio.medium_chisel[rand()%3], 0); } break;
                case CHISEL_LARGE: { Mix_PlayChannel(channel, gs->audio.medium_chisel[rand()%6], 0); } break;
            }
        } break;
    }
}

static int chisel_get_distance_from_facing_cell(int i, f64 angle) {
    angle = M_PI * angle / 180.0;
    
    f64 real_dir_x = cos(angle);
    f64 real_dir_y = sin(angle);
    
    int dir_x = round(real_dir_x);
    int dir_y = round(real_dir_y);
    
    f64 x = (int)i % gs->gw;
    f64 y = (int)i / gs->gw; // Integer division
    
    int distance = 0;
    
    while (true) {
        x += dir_x;
        y += dir_y;
        
        if (!is_in_bounds(x, y) ||
            gs->grid[(int)round(x)+(int)round(y)*gs->gw].type != CELL_NONE)
        {
            return distance;
        }
        
        distance++;
    }
}

static bool chisel_is_facing_cell(int i, f64 angle, int lookahead) {
    Assert(lookahead > 0);
    return chisel_get_distance_from_facing_cell(i, angle) <= lookahead;
}

// Finds a position closest to (x, y) that is on the grid
// Returns: The index of the position
static int chisel_clamp_to_grid(f64 angle, int xx, int yy) {
    f64 closest_distance = gs->gw*gs->gh;
    
    bool is_mouse_on_cell = (gs->grid[xx+yy*gs->gw].type != CELL_NONE);
    
    // Special case for when mouse is on a cell
    
    if (is_mouse_on_cell) {
        f64 rad_angle = M_PI * angle / 180.0;
        int dir_x = round(cos(rad_angle));
        int dir_y = round(sin(rad_angle));
        
        int nx = xx - dir_x;
        int ny = yy - dir_y;
        
        if (gs->grid[nx+ny*gs->gw].type == CELL_NONE)
            return nx+ny*gs->gw;
    }
    
    int closest_idx = -1;
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        int x = i%gs->gw;
        int y = i/gs->gw;
        int neighbours = number_neighbours(gs->grid, x, y, 1);
        
        if (neighbours == 0) continue;
        if (gs->grid[i].type != CELL_NONE) continue;
        
        f64 dx = x - xx;
        f64 dy = y - yy;
        f64 distance = sqrt(dx*dx + dy*dy);
        
        if (distance < closest_distance) {
            closest_distance = distance;
            closest_idx = i;
        }
    }
    
    return closest_idx;
}

static bool is_bad_corner(Chisel *chisel, int x, int y) {
    if (chisel->size != CHISEL_SMALL) return false;
    
    int nx, ny;
    
    f64 angle = M_PI * chisel->angle / 180.0;
    
    f64 dir_x = cos(angle);
    f64 dir_y = sin(angle);
    
    nx = round(x + dir_x);
    ny = round(y + dir_y);
    
    if (gs->grid[nx+ny*gs->gw].type == CELL_NONE) return false;
    
    if (chisel->angle == 135) {
        // x
        // x x
        if (gs->grid[nx+(ny-1)*gs->gw].type != CELL_NONE &&
            gs->grid[nx+1+ny*gs->gw].type != CELL_NONE)
        {
            return true;
        }
    }
    if (chisel->angle == 45) {
        //   x
        // x x
        if (gs->grid[nx+(ny-1)*gs->gw].type != CELL_NONE &&
            gs->grid[nx-1+ny*gs->gw].type != CELL_NONE)
        {
            return true;
        }
    }
    if (chisel->angle == -135) {
        // x x
        // x
        if (gs->grid[nx+1+ny*gs->gw].type != CELL_NONE &&
            gs->grid[nx+(ny+1)*gs->gw].type != CELL_NONE)
        {
            return true;
        }
    }
    if (chisel->angle == -45) {
        // x x
        //   x
        if (gs->grid[nx-1+ny*gs->gw].type != CELL_NONE &&
            gs->grid[nx+(ny+1)*gs->gw].type != CELL_NONE)
        {
            return true;
        }
    }
    
    return false;
}

static bool is_bad_medium_chisel_position(Chisel *chisel) {
    return ((int)chisel->angle % 90 == 0);
}

static void chisel_move_mouse_until_cell(Chisel *chisel, int x, int y, f64 angle, f64 max_length) {
    angle = M_PI * angle / 180.0;
    f64 dir_x = cos(angle);
    f64 dir_y = sin(angle);
    f64 xx = x;
    f64 yy = y;
    
    f64 length = sqrt((x-xx)*(x-xx) + (y-yy)*(y-yy));
    
    while (length < max_length &&
           is_in_bounds(xx,yy) &&
           gs->grid[(int)round(xx)+((int)round(yy))*gs->gw].type == CELL_NONE)
    {
        xx += dir_x;
        yy += dir_y;
        length = sqrt((x-xx)*(x-xx) + (y-yy)*(y-yy));
    }
    xx -= dir_x;
    yy -= dir_y;
    
    chisel->x = round(xx);
    chisel->y = round(yy);
    
    move_mouse_to_grid_position((int)round(xx), (int)round(yy));
}

static void flood_fill(Uint8 *grid, int x, int y, Uint8 value, int count) {
    if (count == 0) return;
    if (!is_in_bounds(x, y)) return;
    if (gs->grid[x+y*gs->gw].type == CELL_NONE) return;
    if (grid[x+y*gs->gw] == value) return;
    
    grid[x+y*gs->gw] = value;
    
    flood_fill(grid, x+1, y, value, count-1);
    flood_fill(grid, x-1, y, value, count-1);
    flood_fill(grid, x, y-1, value, count-1);
    flood_fill(grid, x, y+1, value, count-1);
}

// Returns false if there is no space in the inventory.
static bool chisel_attempt_add_to_inventory(int x, int y) {
    int type = gs->grid[x+y*gs->gw].type;
    int amount = 1;
    if (gs->level_current+1 != 11 && gs->grid[x+y*gs->gw].is_initial)
        amount = (my_rand(x+y*gs->gw)%2==0) ? 2 : 1;
    
    return add_item_to_inventory_slot(type, amount);
}

static bool should_display_pressure_tutorial() {
    return !gs->did_pressure_tutorial;
}

// Unused.
static bool special_case_for_diamond(int x, int y) {
    if (gs->level_current+1 == 5 && gs->grid[x+y*gs->gw].type == CELL_DIAMOND)
    {
        return false;
    }
    return true;
}

// Sets a mask of the cells in the grid which are in front of the chisel.
static void generate_chisel_mask(Uint8 *mask, f64 chisel_angle_deg, int chisel_x, int chisel_y) {
    f64 chisel_angle = Radians(chisel_angle_deg);
    // dx and dy represent the direction of the chisel
    // as a unit vector.
    f64 dx = round(cos(chisel_angle));
    f64 dy = round(sin(chisel_angle));
    
    memset(mask, 0, gs->gw*gs->gh*sizeof(Uint8));
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            f64 xx = x - chisel_x;
            f64 yy = y - chisel_y;
            f64 dot = dx*xx + dy*yy;
            
            // If the dot product between these two vectors
            // is positive, that means the cosine of the
            // angle between the vectors is positive,
            // meaning the angle is < 90 degrees in this case.
            if (dot > 0) mask[x+y*gs->gw] = 1;
        }
    }
}

// This returns if a given cell in the grid is in front of the chisel.
static bool is_cell_in_front_of_chisel(Chisel *chisel,
                                       int cell_x,
                                       int cell_y)
{
    return chisel->mask[cell_x + cell_y * gs->gw];
}

// Returns the index of the nearest successful hit.
static int find_nearest_successful_hit(int x, int y, int radius) {
    int result = -1;
    int max_dist_sq = radius*radius+1;
    
    for (int dy = -radius; dy < radius; dy++) {
        for (int dx = -radius; dx < radius; dx++) {
            int dist_sq = dx*dx+dy*dy;
            if (dist_sq > radius*radius) continue;
            if (!is_in_bounds(x+dx, y+dy)) continue;
            
            int idx = x+dx + (y+dy)*gs->gw;
            
            bool success = (gs->grid[idx].type != gs->overlay.grid[idx]);
            if (success && dist_sq < max_dist_sq) {
                max_dist_sq = dist_sq;
                result = idx;
            }
        }
    }
    
    return result;
}

// dx and dy represent any offset you want in the circle placement.
static void chisel_destroy_circle(Chisel *chisel, int x, int y, int dx, int dy, int size) {
#if USE_PRESSURE
    if (!is_pressure_low_enough(gs->grid[x+y*gs->gw])) {
        if (should_display_pressure_tutorial()) {
            gs->tutorial = *tutorial_rect(TUTORIAL_PRESSURE_STRING,
                                          -1,
                                          NormY(768.0/8.0 + 32),
                                          null);
            gs->did_pressure_tutorial = true;
        }
        return;
    }
#endif
    if (!chisel->is_calculating_highlight)
        save_state_to_next();

    // We want to only destroy the flood-filled object.

    Uint8 *grid = PushArray(gs->transient_memory,
                            gs->gw * gs->gh,
                            sizeof(Uint8));

    flood_fill(grid, x, y, 1, (int)(M_PI*size*size*2));

    if (!chisel->is_calculating_highlight) chisel_play_sound(gs->grid[x+y*gs->gw].type, chisel->size);

    if (size == 0) {
        if (special_case_for_diamond(x, y) && can_add_item_to_inventory(gs->grid[x+y*gs->gw].type)) {
            int type = gs->grid[x+y*gs->gw].type;
            if (!chisel->is_calculating_highlight) {
                chisel_attempt_add_to_inventory(x, y);
                move_mouse_to_grid_position(x, y);
                
                f64 magnitude = 1;
                f64 vx, vy;
                vx = magnitude * (randf(2)-1);
                vy = -magnitude;
                emit_dust(type, x, y, vx, vy);
            }
        }
        set(x, y, 0, -1);
    } else {
        // Destroy in a circle.
        int type = 0;

        x -= dx;
        y -= dy;

        generate_chisel_mask(chisel->mask, chisel->angle, x, y);

        for (int yy = -size; yy <= size; yy++) {
            for (int xx = -size; xx <= size; xx++) {
                if (xx*xx + yy*yy > size*size) continue;
                if (!is_in_bounds(x+xx, y+yy)) continue;
                if (grid[x+xx+(y+yy)*gs->gw] != 1) continue;
                if (!is_cell_hard(gs->grid[x+xx+(y+yy)*gs->gw].type)) continue;
                if (chisel->size == CHISEL_MEDIUM && !is_cell_in_front_of_chisel(chisel, x+xx, y+yy)) continue;
                if (!special_case_for_diamond(x+xx, y+yy)) continue;

                if (gs->grid[x+xx+(y+yy)*gs->gw].type != 0)
                    type = gs->grid[x+xx+(y+yy)*gs->gw].type;

                if (can_add_item_to_inventory(gs->grid[x+xx + (y+yy)*gs->gw].type)) {
                    if (!chisel->is_calculating_highlight) {
                        chisel_attempt_add_to_inventory(x+xx, y+yy);
                        f64 angle = randf(2*M_PI);
                        f64 magnitude = 0.5;
                        f64 vx = magnitude * cos(angle);
                        f64 vy = magnitude * sin(angle);
                        emit_dust(type, x+xx, y+yy, vx, vy);
                    }
                    set(x+xx, y+yy, 0, -1);
                }
            }
        }
    }

    chisel->x = x;
    chisel->y = y;
    if (!chisel->is_calculating_highlight && size > 0) {
        chisel_move_mouse_until_cell(chisel, x, y, chisel->angle, size+1);
    }

    objects_reevaluate();
}

// Returns if succesful
static bool chisel_handle_bad_corner(Chisel *chisel, int ix, int iy,  int dir_x, int size) {
    int y = 0;
    int x = dir_x;

    if (gs->grid[ix+x+(iy+y)*gs->gw].type != CELL_NONE) {
        chisel_destroy_circle(chisel, ix+x, iy+y, 0, 0, size);
        return true;
    }

    return false;
}

// Returns if the chisel was successful.
// Note that size != chisel->size. size just means the
// size of the circle to destroy.
static bool chisel_circle(Chisel *chisel, int size) {
    f64 angle = M_PI * chisel->angle / 180.0;

    f64 dir_x = round(cos(angle));
    f64 dir_y = round(sin(angle));

    f64 xx = chisel->x;
    f64 yy = chisel->y;

    // If we got a bad corner, snap to the closest cell.
    if (is_bad_corner(chisel, chisel->x, chisel->y)) {
        return chisel_handle_bad_corner(chisel, chisel->x, chisel->y, dir_x, size);
    }
    
    for (int lookahead = chisel->lookahead; lookahead > 0; lookahead--) {
        int ix = (int)round(xx);
        int iy = (int)round(yy);

        if (is_in_bounds(ix, iy) &&
            gs->grid[ix+iy*gs->gw].type != CELL_NONE)
        {
            xx = round(xx);
            yy = round(yy);
            
            int dx = dir_x;
            int dy = dir_y;
            
            if (chisel->size == CHISEL_MEDIUM && is_bad_medium_chisel_position(chisel)) {
                dx = 2*dir_x;
                dy = 2*dir_y;
            }
            
            gs->chisel->temp_idx = find_nearest_successful_hit(xx, yy, 3);
            chisel_destroy_circle(chisel, xx, yy, dx, dy, size);
            return true;
        }

        if (is_bad_corner(chisel, ix, iy)) {
            return chisel_handle_bad_corner(chisel, ix, iy, dir_x, size);
        }

        xx += dir_x;
        yy += dir_y;
    }
    return false;
}

static bool chisel_small(Chisel *chisel) {
    return chisel_circle(chisel, 0);
}

static bool chisel_medium(Chisel *chisel) {
    return chisel_circle(chisel, 2);
}

static bool chisel_large(Chisel *chisel) {
    return chisel_circle(chisel, 4);
}

// Performs a chisel on `chisel`.
static bool chisel_chisel(Chisel *chisel) {
    switch (chisel->size) {
        case CHISEL_SMALL:  return chisel_small(chisel);
        case CHISEL_MEDIUM: return chisel_medium(chisel);
        case CHISEL_LARGE:  return chisel_large(chisel);
    }
    Assert(false);
    return false;
}

static void chisel_calculate_highlights(Chisel *chisel) {
    // Store some copies so we could roll back afterwards.
    Cell *grid_copy = PushArray(gs->transient_memory, gs->gw*gs->gh, sizeof(Cell));
    memcpy(grid_copy, gs->grid, gs->gw*gs->gh*sizeof(Cell));

    Chisel chisel_copy = *chisel;

    chisel->is_calculating_highlight = true;

    int count = 0;
    int highlights[HIGHLIGHT_MAX] = {0};

    chisel_chisel(chisel);

    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (gs->grid[i].type != grid_copy[i].type) {
            highlights[count++] = i;
        }
    }

    int idx = chisel->temp_idx;
    memcpy(chisel, &chisel_copy, sizeof(Chisel));
    chisel->temp_idx = idx;
    memcpy(gs->grid, grid_copy, gs->gw*gs->gh*sizeof(Cell));

    memcpy(chisel->highlights, highlights, sizeof(int)*HIGHLIGHT_MAX);
    chisel->highlight_count = count;
}


static void chisel_draw_highlights(int target,
                                   int *highlights,
                                   int count,
                                   int xoff,
                                   int yoff)
{
    bool hit = false;

    for (int i = 0; i < count; i++) {
        int x = highlights[i]%gs->gw;
        int y = highlights[i]/gs->gw;

        if (gs->overlay.grid[x+y*gs->gw] == gs->grid[x+y*gs->gw].type) {
            hit = true;
            break;
        }
    }

    for (int i = 0; i < count; i++) {
        int x = highlights[i]%gs->gw;
        int y = highlights[i]/gs->gw;

        if (gs->overlay.show) {
            if (hit) {
                if (gs->overlay.grid[x+y*gs->gw]) {
                    RenderColor(255, 0, 0, 255);
                } else {
                    RenderColor(255, 0, 0, 60);
                }
            } else {
                RenderColor(0, 0, 0, 80);
            }
        } else {
            RenderColor(0, 0, 0, 60);
        }

        RenderPointRelative(target, xoff+x, yoff+y);
    }

    // Note: I was doing some testing with trying to make chiseling
    //       auto-correct itself, through the function find_nearest_successful_hit,
    //       and using this to output the cell position temp_idx.
#if 0
    int idx = gs->chisel->temp_idx;

    RenderColor(255, 255, 255, 255);
    RenderPointRelative(target, idx % gs->gw, idx / gs->gw);
#endif
}

static void update_other_chisel_draw_positions(int size, f64 draw_x, f64 draw_y) {
    Chisel *chisels[3] = { &gs->chisel_small, &gs->chisel_medium, &gs->chisel_large };
    
    for (int i = 0; i < 3; i++) {
        if (i != size) {
            chisels[i]->draw_x = draw_x;
            chisels[i]->draw_y = draw_y;
        }
    }
}

static void chisel_tick(Chisel *chisel) {
    if (gs->gui.restart_popup_confirm.active) return;
    if (gs->is_mouse_on_tab_icon) return; // Hacky!!!!! ew
    if (gs->conversions.active) return; // There should really be a focus variable

    chisel->did_chisel_this_frame = false;
    if (chisel->click_delay > 0) chisel->click_delay--;
    if (!(gs->input.mouse & SDL_BUTTON_LEFT))
        chisel->click_delay = -1;

    if (gs->hammer.state == HAMMER_STATE_WINDUP ||
        gs->hammer.state == HAMMER_STATE_ATTACK) return;

    switch (chisel->state) {
        case CHISEL_STATE_IDLE: {
            int idx = chisel_clamp_to_grid(chisel->angle, gs->input.mx, gs->input.my);
            if (idx == -1) {
                chisel->x = -1;
                chisel->y = -1;
                gs->current_tool = TOOL_GRABBER;
                for (int i = 0; i < TOOL_COUNT; i++) {
                    gs->gui.tool_buttons[i]->active = false;
                }
                gs->gui.tool_buttons[TOOL_GRABBER]->active = true;
                return;
            } else {
                chisel->x = idx % gs->gw;
                chisel->y = idx / gs->gw;
            }

            if (gs->input.keys_pressed[SDL_SCANCODE_LSHIFT]) {
                chisel->state = CHISEL_STATE_ROTATING;
                move_mouse_to_grid_position(chisel->x, chisel->y);
            }

            chisel_calculate_highlights(chisel);

            break;
        }
        case CHISEL_STATE_ROTATING: {
            int x = chisel->x;
            int y = chisel->y;
            
            // Hardcode
            if (gs->gw == 128) {
                x -= 32;
            }
            
            f64 rmx = (f64)(gs->input.real_mx + gs->render.view.x) / (f64)gs->S;
            f64 rmy = (f64)(gs->input.real_my-GUI_H + gs->render.view.y) / (f64)gs->S;

            chisel->angle = 180 + 360 * atan2f(rmy - y, rmx - x) / (f32)(2*M_PI);

            f32 step = 45.0;
            chisel->angle /= step;
            chisel->angle = ((int)chisel->angle) * step;
            chisel->angle -= 180;

            gs->chisel_small.angle = gs->chisel_medium.angle = gs->chisel_large.angle = chisel->angle;

            if (!gs->input.keys[SDL_SCANCODE_LSHIFT]) {
                chisel->state = CHISEL_STATE_IDLE;
                move_mouse_to_grid_position(chisel->x, chisel->y);
            }

            chisel->highlight_count = 0;
            break;
        }
        case CHISEL_STATE_CHISELING: {
            if (chisel->x != -1 && chisel->y != -1)
            {
                if (chisel_chisel(chisel)) {
                    chisel->num_times_chiseled++;
                    chisel->did_chisel_this_frame = true;
                }
            }

            chisel->highlight_count = 0;

            chisel->state = CHISEL_STATE_IDLE;
            break;
        }
    }
    
    if (!chisel->did_chisel_this_frame) {
        chisel->draw_x = lerp64(chisel->draw_x, chisel->x, 0.7);
        chisel->draw_y = lerp64(chisel->draw_y, chisel->y, 0.7);
    } else {
        chisel->draw_x = chisel->x;
        chisel->draw_y = chisel->y;
    }
    
    update_other_chisel_draw_positions(chisel->size, chisel->draw_x, chisel->draw_y);
}

// Don't worry, we'll remove this later when we
// make sprites for each rotation.
static void chisel_get_adjusted_positions(int angle, int size, int *x, int *y) {
    angle += 180;

    if (size == 0 || size == 1) {
        if (angle == 225) {
            (*y) += 2;
            (*x)++;
        } else if (angle == 180) {
            (*x)++;
            (*y)++;
        } else if (angle == 90) {
            (*x)++;
        } else if (angle == 45) {
            (*x)++;
        } else if (angle == 135) {
            (*x) += 2;
        } else if (angle == 270) {
            (*y)++;
        }
    } else if (size == 2) {
        if (angle == 0) {
            (*x)++;
        } else if (angle == 225) {
            (*y)++;
        } else if (angle == 45) {
            (*x)++;
            (*y)++;
        } else if (angle == 135) {
            (*x)++;
            (*y)++;
        } else if (angle == 315) {
            (*x)++;
        } else if (angle == 180) {
            (*y)++;
        } else if (angle == 90) {
            (*x)++;
        }
    }
}

static void chisel_draw(int target, Chisel *chisel) {
    int x, y;
    
    x = chisel->draw_x;
    y = chisel->draw_y;

    RenderTextureColorMod(chisel->texture, 255, 255, 255);

    SDL_Rect dst = {
        x, y - chisel->texture->height/2,
        chisel->texture->width, chisel->texture->height
    };
    chisel_get_adjusted_positions(chisel->angle, chisel->size, &dst.x, &dst.y);

    SDL_Point center = { 0, chisel->texture->height/2 };
    
#if CHISEL_FLASHING
    if (chisel->state == CHISEL_STATE_ROTATING) {
        const f64 speed = 0.01;
        chisel->rotating_flash = 0.5*(1+sin(speed * SDL_GetTicks()));
        
        Uint8 a = (Uint8) (255 * chisel->rotating_flash);
        
        a = 127+a/2;
        
        RenderTextureColorMod(chisel->texture, a, a, 0);
    }
#endif

    RenderTextureExRelative(target,
                            chisel->texture,
                            null,
                            &dst,
                            180+chisel->angle,
                            &center,
                            SDL_FLIP_NONE);

    RenderColor(127, 127, 127, 255);
    RenderPointRelative(target, (int)chisel->draw_x, (int)chisel->draw_y);

    if (chisel->state == CHISEL_STATE_IDLE)
        chisel_draw_highlights(target,
                               chisel->highlights,
                               chisel->highlight_count,
                               0,
                               0);
}
