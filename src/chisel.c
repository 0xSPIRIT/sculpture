struct Chisel chisel_init(enum Chisel_Size size) {
    struct Chisel chisel = {0};
    
    chisel.size = size;
    chisel.texture = gs->textures.chisel_outside[size];
    
    SDL_QueryTexture(chisel.texture, NULL, NULL, &chisel.w, &chisel.h);
    
    return chisel;
}

bool chisel_is_facing_cell(int i, f64 angle) {
    angle = M_PI * angle / 180.0;
    
    int x = i%gs->gw + round(cos(angle));
    int y = i/gs->gw + round(sin(angle));
    
    return gs->grid[x+y*gs->gw].type != CELL_NONE;
}

// Finds a position closest to (x, y) that is:
//   1. On the grid
//   2. In the direction of the chisel
// Returns: The index of the position
int chisel_clamp_to_grid(int xx, int yy, f64 angle) {
    f64 closest_distance = gs->gw*gs->gh;
    (void) angle;
    
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
        if (distance < closest_distance && chisel_is_facing_cell(i, angle)) {
            closest_distance = distance;
            closest_idx = i;
        }
    }
    
    return closest_idx;
}

void chisel_tick(struct Chisel *chisel) {
    chisel->did_chisel_this_frame = false;
    chisel->num_times_chiseled = 100;
    
    if (chisel->state != CHISEL_STATE_CHISELING &&
        gs->input.keys[SDL_SCANCODE_LSHIFT])
    {
        chisel->state = CHISEL_STATE_ROTATING;
    }
    
    switch (chisel->state) {
        case CHISEL_STATE_IDLE: {
            break;
        }
        case CHISEL_STATE_ROTATING: {
            f64 rmx = (f64)gs->input.real_mx / (f64)gs->S;
            f64 rmy = (f64)(gs->input.real_my-GUI_H) / (f64)gs->S;
            
            chisel->angle = 360 * atan2f(rmy - chisel->y, rmx - chisel->x) / (f32)(2*M_PI);
            
            f32 step = 45.0;
            chisel->angle /= step;
            chisel->angle = ((int)chisel->angle) * step;
            break;
        }
        case CHISEL_STATE_CHISELING: {
            break;
        }
    }
    
    if (gs->input.keys[SDL_SCANCODE_LSHIFT]) {
    } else {
        int idx = chisel_clamp_to_grid(gs->input.mx, gs->input.my, chisel->angle);
        if (idx != -1) {
            chisel->x = idx % gs->gw;
            chisel->y = idx / gs->gw;
        } else {
            chisel->x = -1;
            chisel->y = -1;
        }
    }
}

void chisel_draw(struct Chisel *chisel) {
    if (chisel->x == -1 || chisel->y == -1) return;
    
    SDL_Rect dst = {
        chisel->x, chisel->y - chisel->h/2,
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
    
    SDL_SetRenderDrawColor(gs->renderer, 255, 0, 0, 255);
    SDL_RenderDrawPoint(gs->renderer, chisel->x, chisel->y);
}

void chisel_get_adjusted_positions(int angle, int size, int *x, int *y) {
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
        } else if (angle == 315) {
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
