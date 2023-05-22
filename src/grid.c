int is_cell_hard(int type) {
    return
        type == CELL_ICE         ||
        type == CELL_WOOD_LOG    ||
        type == CELL_WOOD_PLANK  ||
        type == CELL_COBBLESTONE ||
        type == CELL_MARBLE      ||
        type == CELL_SANDSTONE   ||
        
        type == CELL_CONCRETE    ||
        type == CELL_QUARTZ      ||
        type == CELL_GLASS       ||
        
        type == CELL_GRANITE     ||
        type == CELL_BASALT      ||
        type == CELL_DIAMOND     ||
        
        type == CELL_UNREFINED_COAL ||
        type == CELL_REFINED_COAL;
}

int is_cell_liquid(int type) {
    return type == CELL_WATER || type == CELL_LAVA;
}

int is_cell_gas(int type) {
    return
        type == CELL_SMOKE ||
        type == CELL_STEAM;
}


bool any_neighbours_free(Cell *array, int x, int y) {
    for (int xx = -1; xx <= 1; xx++) {
        for (int yy = -1; yy <= 1; yy++) {
            if (xx == 0 && yy == 0) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (array[x+xx+(y+yy)*gs->gw].type == CELL_NONE)
                return true;
        }
    }
    return false;
}

int number_neighbours_inclusive(Cell *array, int x, int y, int r) {
    int c = 0;
    for (int xx = -r; xx <= r; xx++) {
        for (int yy = -r; yy <= r; yy++) {
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (array[x+xx+(y+yy)*gs->gw].type) c++;
        }
    }
    return c;
}

int number_neighbours(Cell *array, int x, int y, int r) {
    int c = 0;
    for (int xx = -r; xx <= r; xx++) {
        for (int yy = -r; yy <= r; yy++) {
            if (xx == 0 && yy == 0) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (array[x+xx+(y+yy)*gs->gw].type) c++;
        }
    }
    return c;
}

int is_any_neighbour_hard(int x, int y) {
    int r = 1;
    for (int xx = -r; xx <= r; xx++) {
        for (int yy = -r; yy <= r; yy++) {
            if (xx == 0 && yy == 0) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (is_cell_hard(gs->grid[x+xx+(y+yy)*gs->gw].type)) return 1;
        }
    }
    return 0;
}

int number_neighbours_of_object(int x, int y, int r, int obj) {
    int c = 0;
    
    for (int xx = -r; xx <= r; xx++) {
        for (int yy = -r; yy <= r; yy++) {
            if (xx == 0 && yy == 0) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (gs->grid[x+xx+(y+yy)*gs->gw].type && gs->grid[x+xx+(y+yy)*gs->gw].object == obj) c++;
        }
    }
    
    return c;
}

int get_neighbour_type_in_direction(int x, int y, f32 angle) {
    angle += 180;
    
    f64 len = 2;
    
    while (len <= 3) {
        f32 dx = len * cos(DEGTORAD*angle);
        f32 dy = len * sin(DEGTORAD*angle);
        
        int xx = x+dx;
        int yy = y+dy;
        
        if (!is_in_bounds(xx, yy)) {
            return -1;
        }
        
        int type = gs->grid[xx+yy*gs->gw].type;
        
        if (type) return type;
        
        len++;
    }
    
    return -1;
}

int get_any_neighbour_object(int x, int y) {
    for (int xx = -1; xx <= 1; xx++) {
        for (int yy = -1; yy <= 1; yy++) {
            if (xx == 0 && yy == 0)        continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (gs->grid[x+xx+(y+yy)*gs->gw].object != -1) {
                return gs->grid[x+xx+(y+yy)*gs->gw].object;
            }
        }
    }
    return -1;
}

// Compares the types of a cell array to an integer array.
// leeway = number of fails allowed to have before returning false.
bool compare_cells_to_int(Cell *a, int *b, int leeway) {
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (a[i].type != b[i]) {
            leeway--;
            if (leeway < 0)
                return false;
        }
    }
    return true;
}

int compare_cells_to_int_count(Cell *a, int *b) {
    int hits = 0;
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (a[i].type != b[i]) hits++;
    }
    return hits;
}

bool compare_cells(Cell *a, Cell *b) {
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (a[i].type != b[i].type) return false;
    }
    return true;
}

//
// Sets an index in a grid array.
//
// object can be left to -1 to automatically find an
// object to assign to the cell, or it can be any positive
// integer.
//
// Alternately, if you don't want any object data and want
// the object field set to -1, confusingly, use -2 as the
// object parameter.
//
void set_array(Cell *arr, int x, int y, int val, int object) {
    if (x < 0 || x >= gs->gw || y < 0 || y >= gs->gh) return;
    
    arr[x+y*gs->gw].type = val;
    arr[x+y*gs->gw].time = 0;
    arr[x+y*gs->gw].is_initial = false;
    
    if (object == -2) {
        arr[x+y*gs->gw].object = -1;
    } else if (object != -1) {
        arr[x+y*gs->gw].object = object;
    } else if (is_cell_hard(val)) { // Automatically set if the cell should be an obj
        // by finding any neighbouring object, and setting it to that
        // or just creating a new one if it can't find it.
        int any = get_any_neighbour_object(x, y);
        if (any != -1) {
            arr[x+y*gs->gw].object = any;
        } else {
            arr[x+y*gs->gw].object = gs->object_count++;
        }
    }
    
    int obj = arr[x+y*gs->gw].object;
    
    if (obj == -1 || obj >= MAX_OBJECTS) {
        return;
    }
    
    if (val == 0) {
        arr[x+y*gs->gw].object = -1;
    }
}

// Sets an index in the grid array. Obj can be left to -1 to automatically
// find an object to assign to the cell, or it can be any value.
void set(int x, int y, int val, int object) {
    set_array(gs->grid, x, y, val, object);
}

void swap_array(Cell *arr, int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x1 >= gs->gw || y1 < 0 || y1 >= gs->gh) return;
    if (x2 < 0 || x2 >= gs->gw || y2 < 0 || y2 >= gs->gh) return;
    
    arr[x1+y1*gs->gw].px = x1;
    arr[x1+y1*gs->gw].py = y1;
    
    arr[x2+y2*gs->gw].px = x2;
    arr[x2+y2*gs->gw].py = y2;
    
    Cell temp = arr[x2+y2*gs->gw];
    arr[x2+y2*gs->gw] = arr[x1+y1*gs->gw];
    arr[x1+y1*gs->gw] = temp;
}

void swap(int x1, int y1, int x2, int y2) {
    swap_array(gs->grid, x1, y1, x2, y2);
}

f32 damp(int i) {
    srand(i);
    return 0.5f + ((f32)rand())/RAND_MAX * 0.4f;
}

f32 water_spread(void) {
    return rand()%3 + ((f32)rand())/RAND_MAX;
}

void grid_init(int w, int h) {
    gs->gw = w;
    gs->gh = h;
    
    if (gs->grid_layers[0] == NULL) {
        for (int i = 0; i < NUM_GRID_LAYERS; i++) {
            gs->grid_layers[i] = PushArray(gs->persistent_memory, w*h, sizeof(Cell));
        }
    }
    
    for (int i = 0; i < w*h; i++) {
        for (int j = 0; j < NUM_GRID_LAYERS; j++) {
            gs->grid_layers[j][i] = (Cell){.type = 0, .object = -1 };
            gs->grid_layers[j][i].rand = rand();
            gs->grid_layers[j][i].id = i;
        }
    }
    
    gs->grid = gs->grid_layers[0];
    gs->gas_grid = gs->grid_layers[1];
}

SDL_Color get_pressure_color(Cell *cell) {
    SDL_Color c = {
        cell->pressure,
        0,
        0,
        255
    };
    return c;
}

SDL_Color pixel_from_index_grid(Cell *grid, enum Cell_Type type, int i) {
    SDL_Color color = {0};
    int r, amt;
    
    switch (type) {
        default:
        break;
        
        case CELL_DIRT: {
            const int variance = 10;
            color = (SDL_Color){
                100+my_rand(grid[i].rand)%variance,
                80+my_rand(grid[i].rand)%variance,
                60+my_rand(grid[i].rand)%variance,
                255
            };
            break;
        }
        case CELL_SAND: {
            int v = my_rand(i) % 20;
            color = (SDL_Color){
                216 + v + randR(i)%15,
                190 + v + randG(i)%15,
                125 + v + randB(i)%15,
                255
            };
            break;
        }
        case CELL_STEAM: {
            color = (SDL_Color){50, 50, 50, 255};
            break;
        }
        
        case CELL_WATER: {
            color = (SDL_Color){131, 160, 226, 255};
            break;
        }
        case CELL_ICE: {
            color = get_pixel(gs->surfaces.ice_surface, i%gs->gw, i/gs->gw);
            break;
        }
        
        case CELL_WOOD_LOG: {
            color = get_pixel(gs->surfaces.bark_surface, i%gs->gw, i/gs->gh);
            break;
        }
        
        case CELL_WOOD_PLANK: {
            color = get_pixel(gs->surfaces.wood_plank_surface, i%gs->gw, i/gs->gw);
            break;
        }
        
        case CELL_COBBLESTONE: {
            r = randR(i) % 100 < 10;
            amt = 25;
            color = (SDL_Color){140 + r*amt + (randR(i)%20 - 10), 140 + r*amt + (randR(i)%20 - 10), 135 + r*amt + (randR(i)%20 - 10), 255};
            // color = (SDL_Color){127, 255, 0, 255};
            break;
        }
        
        case CELL_MARBLE: {
            //color = (SDL_Color){245+randR(i)%10, 245+randG(i)%10, 245+randB(i)%10, 255};
            int id = grid[i].id*2;
            color = get_pixel(gs->surfaces.marble_surface, id%gs->gw, id/gs->gw);
            break;
        }
        
        case CELL_SANDSTONE: {
            r = randR(i) % 100 < 10;
            amt = 25;
            color = (SDL_Color){200 + r*amt + (randR(i)%20 - 10), 140 + r*amt + (randG(i)%20 - 10), 100 + r*amt + (randB(i)%20 - 10), 255};
            /* color = (SDL_Color){127, 255, 0, 255}; */
            break;
        }
        
        case CELL_CEMENT: {
            color = (SDL_Color){100, 100, 100, 255};
            break;
        }
        
        case CELL_CONCRETE: {
            color = (SDL_Color){125, 125, 125, 255};
            break;
        }
        case CELL_QUARTZ: {
            int base = 255;
            amt = abs(my_rand(i))%15;
            color = (SDL_Color){
                base - amt,
                base - amt,
                base - amt,
                255
            };
            break;
        }
        
        case CELL_GLASS: {
            color = get_pixel(gs->surfaces.glass_surface, i%gs->gw, i/gs->gw);
            const f32 coeff = 0.2f;
            #ifndef __EMSCRIPTEN__
            color.a = 180 + sin(coeff * (i + sin(gs->frames/200.0)*6))*20;
            #endif
            break;
        }
        
        case CELL_GRANITE: {
            color = get_pixel(gs->surfaces.granite_surface, i%gs->gw, i/gs->gw);
            color.r *= 0.79;
            color.g *= 0.79;
            color.b *= 0.79;
            break;
        }
        
        case CELL_BASALT: {
            color = (SDL_Color){64, 64, 64, 255};
            color.r += randR(i)%20;
            color.g += randR(i)%20;
            color.b += randR(i)%20;
            break;
        }
        
        case CELL_DIAMOND: {
            int id = grid[i].id*2;
            color = get_pixel(gs->surfaces.diamond_surface, id%gs->gw, id/gs->gw);
            break;
        }
        
        case CELL_UNREFINED_COAL: case CELL_REFINED_COAL: {
            r = randR(i) % 100 < 10;
            amt = 25;
            color = (SDL_Color){70-10 + r*amt + (randR(i)%20 - 10), 70-10 + r*amt + (randR(i)%20 - 10), 70-10 + r*amt + (randR(i)%20 - 10), 255};
            break;
        }
        
        case CELL_LAVA: {
            color = (SDL_Color){255, 255, 255, 255};
            break;
        }
        
        case CELL_SMOKE: {
            color = (SDL_Color){120, 120, 120, 255};
            break;
        }
        
        case CELL_DUST: {
            color = (SDL_Color){100, 100, 100, 255};
            break;
        }
    }
    //if (cells[i].type != CELL_GLASS && is_cell_hard(cells[i].type))
    //color.a = cells[i].depth;
    return color;
}

SDL_Color pixel_from_index(enum Cell_Type type, int i) {
    return pixel_from_index_grid(gs->grid, type, i);
}

// In this function, we use vx_acc and vy_acc as a higher precision position value.
bool move_by_velocity_gas(Cell *arr, int x, int y) {
    Cell *p = &arr[x+y*gs->gw];
    
    if (p->vx_acc == 0 && p->vy_acc == 0) {
        p->vx_acc = (f32) x;
        p->vy_acc = (f32) y;
        return false;
    }
    
    p->vx_acc += p->vx;
    p->vy_acc += p->vy;
    
    int tx = (int)p->vx_acc;
    int ty = (int)p->vy_acc;
    
    if (!is_in_bounds(tx, ty) || (is_in_bounds(tx, ty) && arr[tx+ty*gs->gw].type && arr[tx+ty*gs->gw].type != p->type)) {
        p->vx_acc = (f32) x;
        p->vy_acc = (f32) y;
        p->vx = 0;
        p->vy = 0;
        return true;
    }
    
    swap_array(arr, x, y, (int)p->vx_acc, (int)p->vy_acc);
    return false;
}

void move_by_velocity(Cell *arr, int x, int y) {
    Cell *p = &arr[x+y*gs->gw];
    
    if (p->vx == 0 && p->vy == 0) {
        return;
    }
    
    // If vel < 1, that means we should wait for it to accumulate before moving.
    if (fabsf(p->vx) < 1) {
        p->vx_acc += p->vx;
    }
    if (fabsf(p->vy) < 1) {
        p->vy_acc += p->vy;
    }
    
    if (fabsf(p->vx_acc) >= 1) {
        p->vx = p->vx_acc;
        p->vx_acc = 0;
    }
    if (fabsf(p->vy_acc) >= 1) {
        p->vy = p->vy_acc;
        p->vy_acc = 0;
    }
    
    f32 xx = (f32) x;
    f32 yy = (f32) y;
    
    f32 len = sqrtf(p->vx*p->vx + p->vy*p->vy);
    f32 ux = p->vx/len;
    f32 uy = p->vy/len;
    
    while (sqrt((xx-x)*(xx-x) + (yy-y)*(yy-y)) <= len) {
        xx += ux;
        yy += uy;
        if (xx < 0 || (int)xx >= gs->gw) {
            p->vx *= -damp(x+y*gs->gw);
            xx -= ux;
            yy -= uy;
            break;
        }
        if (yy < 0 || (int)yy >= gs->gh) {
            p->vy = 0;
            xx -= ux;
            yy -= uy;
            break;
        }
        if (((int)xx != x || (int)yy != y) && arr[(int)xx+((int)yy)*gs->gw].type) {
            if (ux) {
                if (!is_cell_liquid(p->type)) {
                    p->vx *= -0.2f;
                } else {
                    p->vx = 0;
                }
            }
            if (uy) {
                if (!is_cell_liquid(p->type)) {
                    p->vy *= -0.6f;
                } else {
                    p->vx = 0;
                }
            }
            
            xx -= ux;
            yy -= uy;
            break;
        }
    }
    
    swap_array(arr, x, y, (int)xx, (int)yy);
}


// Checks if an object still exists, or if all its cells were removed.
int object_does_exist(int obj) {
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (gs->grid[x+y*gs->gw].object == obj) return 1;
        }
    }
    return 0;
}

bool can_gas_cell_swap(int x, int y) {
    return !gs->gas_grid[x+y*gs->gw].type || is_cell_gas(gs->gas_grid[x+y*gs->gw].type);
}

// For x_direction and y_direction, a value of -1 or 1 should be used.
// Returns amount of cells updated.
int grid_array_tick(Cell *array, int x_direction, int y_direction) {
    int start_y = (y_direction == 1) ? 0 : gs->gh-1;
    int start_x = (x_direction == 1) ? 0 : gs->gw-1;
    
#define y_condition(y) ((start_y == 0) ? (y < gs->gh) : (y >= 0))
#define x_condition(x) ((start_x == 0) ? (x < gs->gw) : (x >= 0))
    
    int cells_updated = 0;
    
    for (int y = start_y; y_condition(y); y += y_direction) {
        for (int q = start_x; x_condition(q); q += x_direction) {
            int x = q;
            if (gs->frames%2 == 0) {
                x = gs->gw - 1 - x;
            }
            
            Cell *c = &array[x+y*gs->gw];
            
            // Make sure we're only dealing with non-objects.
            if (!c->type || c->object != -1 || c->updated) continue;
            
            c->updated = true;
            c->time++;
            
            cells_updated++;
            
            switch (c->type) {
                case CELL_WATER: {
                    f32 sp = 0.5;
                    
                    if (is_in_bounds(x, y+1) && !array[x+(y+1)*gs->gw].type) {
                        c->vy += GRAV;
                        if (c->vy > MAX_GRAV) c->vy = MAX_GRAV;
                    } else if (is_in_bounds(x+1, y+1) && !array[x+1+(y+1)*gs->gw].type) {
                        c->vx = sp;
                        c->vy = sp;
                    } else if (is_in_bounds(x-1, y+1) && !array[x-1+(y+1)*gs->gw].type) {
                        c->vx = -sp;
                        c->vy = sp;
                    } else if (is_in_bounds(x+1, y) && is_in_bounds(x-1, y) && !array[x+1+y*gs->gw].type && x-1 >= 0 && !array[x-1+y*gs->gw].type) {
                        f32 dx = water_spread() * ((rand()%2==0)?1:-1);
                        c->vx = dx/4.f;
                        c->vy = 0;
                    } else if (is_in_bounds(x+1, y) && !array[x+1+y*gs->gw].type) {
                        c->vx = sp;
                        c->vy = 0;
                    } else if (is_in_bounds(x-1, y) && !array[x-1+y*gs->gw].type) {
                        c->vx = -sp;
                        c->vy = 0;
                    } else {
                        c->vx = c->vy = 0;
                    }
                    break;
                }
                case CELL_SAND: {
                    if (is_in_bounds(x, y+1) && !array[x+(y+1)*gs->gw].type) {
                        c->vy += GRAV;
                        if (c->vy > MAX_GRAV) c->vy = MAX_GRAV;
                        c->vx *= damp(x+y*gs->gw);
                    } else if (is_in_bounds(x+1, y+1) && !array[x+1+(y+1)*gs->gw].type) {
                        c->vx = 1;
                        c->vy = 1;
                    } else if (is_in_bounds(x-1, y+1) && !array[x-1+(y+1)*gs->gw].type) {
                        c->vx = -1;
                        c->vy = 1;
                    } else {
                        c->vx = c->vy = 0;
                    }
                    break;
                }
                case CELL_DUST: {
                    if (is_in_bounds(x, y+1) && !array[x+(y+1)*gs->gw].type) {
                        swap_array(array, x, y, x, y+1);
                    } else if (is_in_bounds(x+1, y+1) && !array[x+1+(y+1)*gs->gw].type) {
                        swap_array(array, x, y, x+1, y+1);
                    } else if (is_in_bounds(x-1, y+1) && !array[x-1+(y+1)*gs->gw].type) {
                        swap_array(array, x, y, x-1, y+1);
                    }
                    return cells_updated;
                };
                case CELL_STEAM: case CELL_SMOKE: {
                    Assert(array == gs->gas_grid);
                    
                    f32 fac = 0.4f*randf(1.f);
                    f32 amplitude = 1.0;
                    
                    // If we hit something last frame...
                    if (is_in_bounds(x, y-1) && can_gas_cell_swap(x, y-1)) {
                        c->vy = -1;
                        c->vx = amplitude * sinf(c->rand + c->time * fac);
                    } else if (is_in_bounds(x+1, y-1) && can_gas_cell_swap(x+1, y-1)) {
                        c->vx = 1;
                        c->vy = -1;
                    } else if (is_in_bounds(x-1, y-1) && can_gas_cell_swap(x-1, y-1)) {
                        c->vx = -1;
                        c->vy = -1;
                    } else if (is_in_bounds(x-1, y) && can_gas_cell_swap(x-1, y)) {
                        c->vx = -1;
                    } else if (is_in_bounds(x+1, y) && can_gas_cell_swap(x-1, y)) {
                        c->vx = 1;
                    }
                    
                    /* for (int i = 1; i >= -1; i -= 2) { */
                    /*     int tx = x+(int)(i*c->vx); */
                    /*     if (!is_in_bounds(tx, y) || (is_in_bounds(tx, y) && gs->grid[tx+y*gs->gw].type && gs->grid[tx+y*gs->gw].type != c->type)) { */
                    /*         c->vx = 0; */
                    /*     } */
                    /* } */
                    
                    if (y == 0 ||
                        (x == 0 && c->vx < 0) ||
                        (x == gs->gw-1 && c->vx > 0) ||
                        (y == gs->gw-1 && c->vy > 0)) {
                        set_array(array, x, y, 0, -1);
                    }
                    
                    if (c->type) {
                        move_by_velocity_gas(array, x, y);
                    }
                    continue;
                }
                default: {
                    if (is_in_bounds(x, y+1) && !array[x+(y+1)*gs->gw].type) {
                        c->vx = 0;
                        c->vy = 1;
                    } else if (is_in_bounds(x+1, y+1) && !array[x+1+(y+1)*gs->gw].type) {
                        c->vx = 1;
                        c->vy = 1;
                    } else if (is_in_bounds(x-1, y+1) && !array[x-1+(y+1)*gs->gw].type) {
                        c->vx = -1;
                        c->vy = 1;
                    } else {
                        c->vx = 0;
                        c->vy = 0;
                    }
                    break;
                }
            }
            
            // Make sure the cell still exists, and wasn't destroyed
            // during this function (CELL_STEAM, CELL_SMOKE, and CELL_DUST)
            if (c->type) {
                move_by_velocity(array, x, y);
            }
        }
    }
    
    return cells_updated;
}

void simulation_tick(void) {
    if (!gs->step_one)
        if (gs->paused) return;
    
    gs->frames++;
    
    for (int i = 0; i < gs->levels[gs->level_current].source_cell_count; i++) {
        Source_Cell *sc = &gs->levels[gs->level_current].source_cell[i];
        
        int x = sc->x;
        int y = sc->y;
        if (!gs->gas_grid[x+y*gs->gw].type && sc->type != 0) {
            char str[256] = {0};
            get_name_from_type(sc->type, str);
            
            set_array(gs->gas_grid, x, y, sc->type, -1);
            gs->gas_grid[x+y*gs->gw].vy = -1;
            
            if (!gs->gas_grid[x+y*gs->gw].type) {
                gs->gas_grid[x+y*gs->gw].type = sc->type;
            }
        }
    }
    
    grid_array_tick(gs->grid, 1, -1);
    dust_grid_tick();
    grid_array_tick(gs->gas_grid, 1, 1);
}

void grid_array_draw(Cell *array, Uint8 alpha) {
    for (int i = 0; i < gs->gw*gs->gh; i++)
        array[i].updated = 0;
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (!array[x+y*gs->gw].type) continue;
            
            SDL_Color col = pixel_from_index(array[x+y*gs->gw].type, x+y*gs->gw);
            
            const int DRAW_PRESSURE = 0;
            if (DRAW_PRESSURE && array[x+y*gs->gw].type) {
                col = get_pressure_color(&array[x+y*gs->gw]);
            }
            
            f64 a = alpha/255.0;
            SDL_SetRenderDrawColor(gs->renderer, col.r, col.g, col.b, col.a * a);
            
            const bool draw_lines = false;
            if (draw_lines) {
                Line l = {x, y, array[x+y*gs->gw].px, array[x+y*gs->gw].py};
                if (array[x+y*gs->gw].px != 0 && array[x+y*gs->gw].py != 0 && array[x+y*gs->gw].type == CELL_WATER) {
                    SDL_RenderDrawLine(gs->renderer, l.x1, l.y1, l.x2, l.y2);
                } else {
                    SDL_RenderDrawPoint(gs->renderer, l.x1, l.y1);
                }
            } else {
                SDL_RenderDrawPoint(gs->renderer, x, y);
            }
        }
    }
}

void grid_draw(void) {
    // Draw all the grids in a layered order.
    grid_array_draw(gs->gas_grid, 255);
    grid_array_draw(gs->grid, 255);
    dust_grid_draw();
    
    overlay_draw();
    
    // Draw inspiration ghost
    if (gs->grid_show_ghost) {
        for (int y = 0; y < gs->gh; y++) {
            for (int x = 0; x < gs->gw; x++) {
                if (!gs->levels[gs->level_current].desired_grid[x+y*gs->gw].type) continue;
                
                const f32 strobe_speed = 3.5f;
                
                f32 alpha;
                alpha = 1 + sinf(strobe_speed * SDL_GetTicks()/1000.0);
                alpha /= 2;
                alpha *= 16;
                alpha += 180;
                
                SDL_Color col = pixel_from_index(gs->levels[gs->level_current].desired_grid[x+y*gs->gw].type, x+y*gs->gw);
                f32 b = (f32) (col.r + col.g + col.b);
                b /= 3.;
                b = (f32) clamp((int)b, 0, 255);
                SDL_SetRenderDrawColor(gs->renderer,
                                       (Uint8) (b/2),
                                       (Uint8) (b/4),
                                       (Uint8) (b),
                                       (Uint8) (alpha));
                SDL_RenderDrawPoint(gs->renderer, x, y);
            }
        }
    } 
}

bool is_pressure_low_enough(Cell cell) {
    if (cell.pressure < 220)
        return true;
    return false;
}

void calculate_pressure(Cell *grid) {
    const int radius = 3;
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        int neighbours = number_neighbours(grid, i%gs->gw, i/gs->gw, radius);
        int total = (2*radius+1)*(2*radius+1)-1; // +1 including middle row/col
        grid[i].pressure = (Uint8) (255 * (f64)neighbours/total);
    }
}

// Try to move us down 1 cell.
// If any cell is unable to move, undo the work.
void object_tick(int obj) {
    if (gs->current_tool == TOOL_GRABBER && gs->grabber.object_holding == obj) return;
    
    // Copy of grid to fall back to if we abort.
    Cell *grid_temp = PushArray(gs->transient_memory, gs->gw*gs->gh, sizeof(Cell));
    memcpy(grid_temp, gs->grid, sizeof(Cell)*gs->gw*gs->gh);
    
    int dy = 1;
    
    for (int y = gs->gh-1; y >= 0; y--) {
        for (int x = 0; x < gs->gw; x++) {
            if (gs->grid[x+y*gs->gw].object != obj) continue;
            
            if (y+1 >= gs->gh || gs->grid[x+(y+1)*gs->gw].type) {
                // Abort!
                memcpy(gs->grid, grid_temp, sizeof(Cell)*gs->gw*gs->gh);
                return;
            } else {
                swap(x, y, x, y+dy);
            }
        }
    }
    
    if (!object_does_exist(obj)) {
        gs->object_count--;
    }
}

void mark_neighbours(int x, int y, int obj, int pobj, int *cell_count) {
    if (x < 0 || y < 0 || x >= gs->gw || y >= gs->gh ||
        !gs->grid[x+y*gs->gw].type ||
        !is_cell_hard(gs->grid[x+y*gs->gw].type) ||
        gs->grid[x+y*gs->gw].temp != -1 ||
        gs->grid[x+y*gs->gw].object != pobj) {
        return;
    }
    
    gs->grid[x+y*gs->gw].temp = obj;
    (*cell_count)++;
    
    mark_neighbours(x+1, y, obj, gs->grid[x+y*gs->gw].object, cell_count);
    mark_neighbours(x-1, y, obj, gs->grid[x+y*gs->gw].object, cell_count);
    mark_neighbours(x, y+1, obj, gs->grid[x+y*gs->gw].object, cell_count);
    mark_neighbours(x, y-1, obj, gs->grid[x+y*gs->gw].object, cell_count);
}

// Destroys old objects array, and creates new objects for all cells.
// This is useful when an existing object is split into two, and we want
// to split that into two separate objects.
// We don't want to bind together two objects which happen to be neighbouring,
// though, only break apart things.
void objects_reevaluate(void) {
    gs->object_count = 0;
    gs->object_current = 0;
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        gs->grid[i].temp = -1;
    }
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (!gs->grid[x+y*gs->gw].type || !is_cell_hard(gs->grid[x+y*gs->gw].type) || gs->grid[x+y*gs->gw].temp != -1) continue;
            int cell_count = 0;
            mark_neighbours(x, y, gs->object_count, gs->grid[x+y*gs->gw].object, &cell_count);
            gs->object_count++;
        }
    }
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        gs->grid[i].object = gs->grid[i].temp;
        gs->grid[i].temp = 0;
    }
    
    calculate_pressure(gs->grid);
}

int condition(int a, int end, int dir) {
    if (dir == 1) {
        return a <= end;
    }
    return a >= end;
}

int object_attempt_move(int object, int dx, int dy) {
    f32 len = sqrtf((f32) (dx*dx + dy*dy));
    if (len == 0) return 0;
    
    f32 ux = dx/len;
    f32 uy = dy/len;
    
    int start_y = 0;
    int end_y = 0;
    int start_x = 0;
    int end_x = 0;
    int dir_x = 0;
    int dir_y = 0;
    
    if (uy > 0) {
        start_y = gs->gh - 1;
        end_y = 0;
        dir_y = -1;
    } else {
        start_y = 0;
        end_y = gs->gh - 1;
        dir_y = 1;
    }
    
    if (ux > 0) {
        start_x = gs->gw - 1;
        end_x = 0;
        dir_x = -1;
    } else {
        start_x = 0;
        end_x = gs->gw - 1;
        dir_x = 1;
    }
    
    f32 vx = ux; // = 0;
    f32 vy = uy; // = 0;
    
    Cell *grid_temp = PushArray(gs->transient_memory, gs->gw*gs->gh, sizeof(Cell));
    memcpy(grid_temp, gs->grid, sizeof(Cell)*gs->gw*gs->gh);
    
    /* while (sqrt(vx*vx + vy*vy) < len) { */
    /*     vx += ux; */
    /*     vy += uy; */
    for (int y = start_y; condition(y, end_y, dir_y); y += dir_y) {
        for (int x = start_x; condition(x, end_x, dir_x); x += dir_x) {
            if (gs->grid[x+y*gs->gw].object == object) {
                int rx = x + (int)vx;
                int ry = y + (int)vy;
                if (rx < 0 || ry < 0 || rx >= gs->gw || ry >= gs->gh || gs->grid[rx+ry*gs->gw].type) {
                    // Abort
                    memcpy(gs->grid, grid_temp, sizeof(Cell) * gs->gw * gs->gh);
                    /* temp_dealloc(result_grid); */
                    return 0;
                }
                // Otherwise, go through with the set.
                swap(x, y, rx, ry);
            }
        }
    }
    /* } */
    
    return (int) (ux+uy*gs->gw);
}

int get_cell_index_by_id(Cell *array, int id) {
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (array[i].id == id) {
            return i;
        }
    }
    return -1;
}

void convert_object_to_dust(int object) {
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (gs->grid[x+y*gs->gw].object != object) continue;
            set(x, y, 0, -1);
        }
    }
}

void draw_objects(void) {
    if (!gs->do_draw_objects) return;
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (gs->grid[x+y*gs->gw].object == -1) continue;
            int b = gs->grid[x+y*gs->gw].object + 10;
            b *= b;
            SDL_SetRenderDrawColor(gs->renderer, randR(b), randG(b), randB(b), 255);
            SDL_RenderDrawPoint(gs->renderer, x, y);
        }
    }
}

// Returns the index of the closest cell to the point (px, py)
int clamp_to_grid(int px, 
                  int py, 
                  bool outside, 
                  bool on_edge, 
                  bool set_current_object, 
                  bool must_be_hard) 
{
    int closest_index = -1;
    f32 closest_distance = (f32) (gs->gw*gs->gh); // Arbitrarily high number
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            
            if (on_edge) {
                if (!gs->grid[x+y*gs->gw].type) continue;
                if (must_be_hard && !is_cell_hard(gs->grid[x+y*gs->gw].type)) continue;
            } else if (outside) {
                if (gs->grid[x+y*gs->gw].type) continue;
            } else {
                if (!gs->grid[x+y*gs->gw].type) continue;
            }
            
            int condition;
            if (outside) {
                if (on_edge) {
                    if (must_be_hard) {
                        condition = number_neighbours(gs->grid, x, y, 1) < 8 && is_any_neighbour_hard(x, y);
                    } else {
                        condition = number_neighbours(gs->grid, x, y, 1) < 8;
                    }
                } else {
                    if (must_be_hard) {
                        condition = (number_neighbours(gs->grid, x, y, 1) >= 1) && is_any_neighbour_hard(x, y);
                    } else {
                        condition = number_neighbours(gs->grid, x, y, 1) >= 1;
                    }
                }
            } else {
                condition = 1;
            }
            
            if (condition) {
                f32 dx = (f32) (px - x);
                f32 dy = (f32) (py - y);
                f32 dist = sqrtf(dx*dx + dy*dy);
                
                if (dist < closest_distance) {
                    closest_distance = dist;
                    closest_index = x+y*gs->gw;
                }
            }
        }
    }
    
    if (set_current_object) {
        if (on_edge) {
            gs->object_current = gs->grid[closest_index].object;
        } else {
            // NOTE: Can (possibly?) cause fuckup if the point is directly in between two objects.
            gs->object_current = get_any_neighbour_object(closest_index%gs->gw, closest_index/gs->gw);
        }
        if (gs->object_current < -1) {
            Error("Object Current was < -1. You are permitted to panic! D:\n");
            gs->object_current = -1;
        }
    }
    return closest_index;
}