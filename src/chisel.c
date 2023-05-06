struct Chisel chisel_init(enum Chisel_Size size) {
    struct Chisel chisel = {0};
    
    chisel.size = size;
    chisel.texture = gs->textures.chisel_outside[size];
    chisel.lookahead = 5;
    
    SDL_QueryTexture(chisel.texture, NULL, NULL, &chisel.w, &chisel.h);
    
    return chisel;
}

int chisel_get_distance_from_facing_cell(int i, f64 angle) {
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

bool chisel_is_facing_cell(int i, f64 angle, int lookahead) {
    Assert(lookahead > 0);
    return chisel_get_distance_from_facing_cell(i, angle) <= lookahead;
}

// Finds a position closest to (x, y) that is on the grid
// Returns: The index of the position
int chisel_clamp_to_grid(int xx, int yy) {
    f64 closest_distance = gs->gw*gs->gh;
    
    int closest_idx = -1;
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        int x = i%gs->gw;
        int y = i/gs->gh;
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

bool is_bad_corner(struct Chisel *chisel, int x, int y) {
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

void chisel_move_mouse_until_cell(struct Chisel *chisel, int x, int y, f64 angle, f64 max_length) {
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

// dx and dy represent any offset you want in the circle placement.
void chisel_destroy_circle(struct Chisel *chisel, int x, int y, int dx, int dy, int size) {
    if (!chisel->is_calculating_highlight)
        save_state_to_next();
    
    if (size == 0) {
        set(x, y, 0, -1);
        if (!chisel->is_calculating_highlight)
            move_mouse_to_grid_position(x, y);
    } else {
        // Destroy in a circle.
        int type = 0;
        int count = 0;
        
        x -= dx;
        y -= dy;
        
        for (int yy = -size; yy <= size; yy++) {
            for (int xx = -size; xx <= size; xx++) {
                if (!is_in_bounds(x+xx, y+yy)) continue;
                if (xx*xx+yy*yy > size*size)   continue;
                
                if (gs->grid[x+xx+(y+yy)*gs->gw].type != 0)
                    type = gs->grid[x+xx+(y+yy)*gs->gw].type;
                set(x+xx, y+yy, 0, -1);
                count++;
            }
        }
        
        if (!chisel->is_calculating_highlight)
            emit_dust_explosion(type, x+dx, y+dy, count);
    }
    chisel->x = x;
    chisel->y = y;
    if (!chisel->is_calculating_highlight && size > 0) {
        chisel_move_mouse_until_cell(chisel, x, y, chisel->angle, size+1);
    }
    objects_reevaluate();
}

// Returns if succesful
bool chisel_handle_bad_corner(struct Chisel *chisel, int ix, int iy,  int dir_x, int size) {
    int y = 0;
    int x = dir_x;
    
    if (gs->grid[ix+x+(iy+y)*gs->gw].type != CELL_NONE) {
        chisel_destroy_circle(chisel, ix+x, iy+y, 0, 0, size);
        return true;
    }
    
    return false;
}

// Returns if the chisel was successful.
bool chisel_chisel_circle(struct Chisel *chisel, int size) {
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
            chisel_destroy_circle(chisel, xx, yy, 0, 0, size);
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

bool chisel_chisel_small(struct Chisel *chisel) {
    return chisel_chisel_circle(chisel, 0);
}
    
bool chisel_chisel_medium(struct Chisel *chisel) {
    return chisel_chisel_circle(chisel, 1);
}

bool chisel_chisel_large(struct Chisel *chisel) {
    return chisel_chisel_circle(chisel, 3);
}

bool chisel_chisel(struct Chisel *chisel) {
    switch (chisel->size) {
        case CHISEL_SMALL:  return chisel_chisel_small(chisel);
        case CHISEL_MEDIUM: return chisel_chisel_medium(chisel);
        case CHISEL_LARGE:  return chisel_chisel_large(chisel);
    }
    Assert(false);
    return false;
}

void chisel_calculate_highlights(struct Chisel *chisel) {
    // Store some copies so we could roll back afterwards.
    struct Cell *grid_copy = PushArray(gs->transient_memory, gs->gw*gs->gh, sizeof(struct Cell));
    memcpy(grid_copy, gs->grid, gs->gw*gs->gh*sizeof(struct Cell));
    
    struct Chisel chisel_copy = *chisel;
    
    chisel->is_calculating_highlight = true;
    
    int count = 0;
    int highlights[HIGHLIGHT_MAX] = {0};
    
    chisel_chisel(chisel);
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (gs->grid[i].type != grid_copy[i].type) {
            highlights[count++] = i;
        }
    }
    
    memcpy(chisel, &chisel_copy, sizeof(struct Chisel));
    memcpy(gs->grid, grid_copy, gs->gw*gs->gh*sizeof(struct Cell));
    
    memcpy(chisel->highlights, highlights, sizeof(int)*HIGHLIGHT_MAX);
    chisel->highlight_count = count;
}


void chisel_draw_highlights(int *highlights, int count) {
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
                    SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 255);
                } else {
                    SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 60);
                }
            } else {
                SDL_SetRenderDrawColor(gs->renderer, 0, 255, 0, 150);
            }
        } else {
            SDL_SetRenderDrawColor(gs->renderer, 0, 0, 0, 55);
        }
        
        SDL_RenderDrawPoint(gs->renderer, x, y);
    }
}

void chisel_tick(struct Chisel *chisel) {
    chisel->did_chisel_this_frame = false;
    
    switch (chisel->state) {
        case CHISEL_STATE_IDLE: {
            int idx = chisel_clamp_to_grid(gs->input.mx, gs->input.my);
            if (idx == -1) {
                chisel->x = -1;
                chisel->y = -1;
            } else {
                chisel->x = idx % gs->gw;
                chisel->y = idx / gs->gw;
            }
            
            if (gs->input.keys_pressed[SDL_SCANCODE_LSHIFT]) {
                chisel->state = CHISEL_STATE_ROTATING;
                move_mouse_to_grid_position(chisel->x, chisel->y);
            }
            
            chisel_calculate_highlights(chisel);
            
            if (gs->input.real_my > GUI_H &&
                !gs->tutorial.active &&
                !gs->gui.popup &&
                gs->input.mouse_pressed[SDL_BUTTON_LEFT])
            {
                chisel->state = CHISEL_STATE_CHISELING;
            }
            break;
        }
        case CHISEL_STATE_ROTATING: {
            f64 rmx = (f64)gs->input.real_mx / (f64)gs->S;
            f64 rmy = (f64)(gs->input.real_my-GUI_H) / (f64)gs->S;
            
            chisel->angle = 180 + 360 * atan2f(rmy - chisel->y, rmx - chisel->x) / (f32)(2*M_PI);
            
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
}

void chisel_draw(struct Chisel *chisel) {
    int x, y;
    
    x = chisel->x;
    y = chisel->y;
    
    if (chisel->x == -1 || chisel->y == -1) {
        SDL_SetTextureColorMod(chisel->texture, 127, 127, 127);
        x = gs->input.mx;
        y = gs->input.my;
    } else {
        SDL_SetTextureColorMod(chisel->texture, 255, 255, 255);
    }
    
    SDL_Rect dst = {
        x, y - chisel->h/2,
        chisel->w, chisel->h
    };
    chisel_get_adjusted_positions(chisel->angle, chisel->size, &dst.x, &dst.y);
    
    SDL_Point center = { 0, chisel->h/2 };
    
    SDL_RenderCopyEx(gs->renderer,
                     chisel->texture,
                     NULL,
                     &dst,
                     180+chisel->angle,
                     &center,
                     SDL_FLIP_NONE);
    
    SDL_SetRenderDrawColor(gs->renderer, 127, 127, 127, 255);
    SDL_RenderDrawPoint(gs->renderer, (int)chisel->x, (int)chisel->y);
    
    if (chisel->state == CHISEL_STATE_IDLE)
        chisel_draw_highlights(chisel->highlights, chisel->highlight_count);
}

void chisel_get_adjusted_positions(int angle, int size, int *x, int *y) {
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
