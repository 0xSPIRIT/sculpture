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


bool any_neighbours_free(struct Cell *array, int x, int y) {
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

int number_neighbours(struct Cell *array, int x, int y, int r) {
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

int number_direct_neighbours(struct Cell *array, int x, int y) {
    int count = 0;
    
    if (array[x-1 + (y  ) * gs->gw].type) count++;
    if (array[x+1 + (y  ) * gs->gw].type) count++;
    if (array[x   + (y-1) * gs->gw].type) count++;
    if (array[x   + (y+1) * gs->gw].type) count++;
    
    return count;
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

int blob_find_pressure(int obj, Uint32 blob, int x, int y, int r) {
    int max_blob_size = 8;
    f32 pressure = 0.f;
    int count = 0;
    struct Object *object = &gs->objects[obj];
    
    for (int dy = -max_blob_size/2; dy < max_blob_size/2; dy++) {
        for (int dx = -max_blob_size/2; dx < max_blob_size/2; dx++) {
            int rx = x+dx;
            int ry = y+dy;
            if (object->blob_data[gs->chisel->size].blobs[rx+ry*gs->gw] == blob) {
                pressure += number_neighbours_of_object(rx, ry, r, obj);
                count++;
            }
        }
    }
    
    if (count == 0) return 0;
    
    pressure /= count;
    
    return (int)pressure;
}

void print_blob_data(struct Object *object, int chisel_size) {
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            Log("%u ", object->blob_data[chisel_size].blobs[x+y*gs->gw]);
        }
        Log("\n");
    }
    Log("\n\n");
}

// 249

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

// Find the amount of cells a blob has in a given object.
int blob_find_count_from_position(int object, int chisel_size, int x, int y) {
    int count = 0;
    const int max_size = 5;
    Uint32 *blobs = gs->objects[object].blob_data[chisel_size].blobs;
    
    Uint32 mine = blobs[x+y*gs->gw];
    
    for (int yy = -max_size; yy <= max_size; yy++) {
        for (int xx = -max_size; xx <= max_size; xx++) {
            int rx = x+xx;
            int ry = y+yy;
            
            if (blobs[rx+ry*gs->gw] == mine) {
                count++;
            }
        }
    }
    
    return count;
}

// Find the neighbouring blobs that are the same as 'blob'
int get_neighbouring_same_blobs(int x, int y, int blob, int obj, int size) {
    int result = 0;
    
    Uint32 *blobs = gs->objects[obj].blob_data[size].blobs;
    
    result += blobs[x-1+y*gs->gw] == blob;
    result += blobs[x+1+y*gs->gw] == blob;
    result += blobs[x+(y+1)*gs->gw] == blob;
    result += blobs[x+(y-1)*gs->gw] == blob;
    
    return result;
}

void object_blobs_set_pressure(int obj, int chisel_size) {
    int *temp_blobs = PushArray(gs->transient_memory, gs->gw*gs->gh, sizeof(int));
    struct Object *object = &gs->objects[obj];
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        temp_blobs[i] = object->blob_data[chisel_size].blobs[i];
    }
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (!object->blob_data[chisel_size].blobs[x+y*gs->gw] || temp_blobs[x+y*gs->gw] == -1) continue;
            
            Uint32 blob = object->blob_data[chisel_size].blobs[x+y*gs->gw];
            
            // Debugging
            Assert(blob < 2147483648-1);
            
            object->blob_data[chisel_size].blob_pressures[blob] = blob_find_pressure(obj, blob, x, y, (chisel_size == 2 ? 3 : 5));
            
            // Mark this blob as complete.
            int max_blob_size = 8;
            for (int dy = -max_blob_size/2; dy < max_blob_size/2; dy++) {
                for (int dx = -max_blob_size/2; dx < max_blob_size/2; dx++) {
                    int rx = x+dx;
                    int ry = y+dy;
                    if (rx >= 0 && rx < gs->gw && ry >= 0 && ry < gs->gh && temp_blobs[rx+ry*gs->gw] == blob)
                        temp_blobs[rx + ry*gs->gw] = -1;
                }
            }
        }
    }
}

void blob_generate_dumb(int obj, int chisel_size, Uint32 *blob_count) {
    Assert(obj >= 0);
    Assert(blob_count);
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (gs->grid[x+y*gs->gw].object != obj) continue;
            struct Object *o = &gs->objects[obj];
            
            o->blob_data[chisel_size].blobs[x+y*gs->gw] = *blob_count;
            (*blob_count)++;
        }
    }
}

void blob_generate_large(f64 s, int obj, int chisel_size, Uint32 *blob_count) {
    Uint32 *blobs = gs->objects[obj].blob_data[chisel_size].blobs;
    
    f32 rad = (gs->chisel->angle) / 360.f;
    rad *= 2 * (f32)M_PI;
    
    int x, y;
    
    if (chisel_size == 1) {
        x = gs->chisel->x - cos(rad);
        y = gs->chisel->y - sin(rad);
    } else {
        x = gs->chisel->x;
        y = gs->chisel->y;
    }
    
    for (int yy = -s; yy <= s; yy++) {
        for (int xx = -s; xx <= s; xx++) {
            if (xx*xx + yy*yy > s*s) continue;
            
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (gs->grid[x+xx+(y+yy)*gs->gw].object != obj) continue;
            if (blobs[x+xx+(y+yy)*gs->gw]) continue;
            
            blobs[x+xx+(y+yy)*gs->gw] = *blob_count;
        }
    }
    (*blob_count)++;
}

int flood_fill_outlines(Uint32 *blobs, Uint32 *blob_count, int obj, int x, int y, int counter) {
    if (counter <= 0) {
        (*blob_count)++;
        return 0;
    }
    
    for (int yy = -1; yy <= 1; yy++) {
        for (int xx = -1; xx <= 1; xx++) {
            int rx = x+xx;
            int ry = y+yy;
            
            if (gs->grid[rx+ry*gs->gw].object != obj) continue;
            if (blobs[rx+ry*gs->gw]) continue;
            
            if (any_neighbours_free(gs->grid, rx, ry)) {
                blobs[rx+ry*gs->gw] = *blob_count;
                counter = flood_fill_outlines(blobs, blob_count, obj, rx, ry, counter-1);
            }
        }
    }
    
    return counter;
}

bool compare_cells(struct Cell *a, struct Cell *b) {
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (a[i].type != b[i].type) return false;
    }
    return true;
}

// Start at an edge blob, and work through neighbours until you get
// to the start, or if you can't find another neighbour, start from
// somewhere else that hasn't been set yet.
void blob_generate_outlines(int obj, int length, int chisel_size, Uint32 *blob_count) {
    Uint32 *blobs = gs->objects[obj].blob_data[chisel_size].blobs;
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (gs->grid[x+y*gs->gw].object != obj) continue;
            if (blobs[x+y*gs->gw]) continue;
            
            // We are on an outline.
            if (any_neighbours_free(gs->grid, x, y)) {
                flood_fill_outlines(blobs, blob_count, obj, x, y, length);
            }
        }
    }
}

void grid_fill_circle(Uint32 *blobs, Uint32 *blob_count, int obj, int x, int y, int size) {
    for (int yy = -size; yy <= size; yy++) {
        for (int xx = -size; xx <= size; xx++) {
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (xx*xx+yy*yy > size*size)   continue;
            if (gs->grid[(x+xx)+(y+yy)*gs->gw].object != obj) continue;
            if (blobs[(x+xx)+(y+yy)*gs->gw]) continue;
            
            blobs[xx+x+(yy+y)*gs->gw] = *blob_count;
        }
    }
    (*blob_count)++;
}

void blob_generate_pizza(int obj, int size, Uint32 *blob_count) {
    Uint32 *blobs = gs->objects[obj].blob_data[size].blobs;
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (gs->grid[x+y*gs->gw].object != obj) continue;
            if (blobs[x+y*gs->gw]) continue;
            
            if (any_neighbours_free(gs->grid, x, y)) {
                int radius = size == 1 ? 2 : 4;
                grid_fill_circle(blobs, blob_count, obj, x, y, radius);
            }
        }
    }
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (!blobs[x+y*gs->gw]) continue;
            
            int b = blobs[x+y*gs->gw];
            
            if (blobs[(x+1)+y*gs->gw] != b &&
                blobs[x+(y+1)*gs->gw] != b &&
                blobs[(x-1)+y*gs->gw] != b &&
                blobs[x+(y-1)*gs->gw] != b) 
            {
                blobs[x+y*gs->gw] = 0;
            }
        }
    }
}

// Sets up all blobs whose cells belongs to our obj.
void object_generate_blobs(int object_index, int chisel_size) {
    if (object_index >= MAX_OBJECTS) return;
    
    Uint32 count = 1;
    
    int percentage = 0;
    
    if (chisel_size == 1) {
        percentage = 0;
    }
    
    struct Object *obj = &gs->objects[object_index];
    
    memset(obj->blob_data[chisel_size].blobs, 0, gs->gw*gs->gh*sizeof(Uint32));
    
    if (chisel_size == 0) {
        blob_generate_dumb(object_index, chisel_size, &count);
    } else if (obj->cell_count <= 6) {
        Uint32 *blobs = obj->blob_data[chisel_size].blobs;
        for (int y = 0; y < gs->gh; y++) {
            for (int x = 0; x < gs->gw; x++) {
                if (gs->grid[x+y*gs->gw].object != object_index) continue;
                
                blobs[x+y*gs->gw] = 1;
            }
        }
        count = 1;
    } else if (chisel_size == 1) {
        gs->blob_type = BLOB_CIRCLE_B;
        //blob_generate_pizza(object_index, chisel_size, &count);
        blob_generate_large(2, object_index, chisel_size, &count);
    } else {
        blob_generate_large(4, object_index, chisel_size, &count);
    }
    
    obj->blob_data[chisel_size].blob_count = count;
    
    object_blobs_set_pressure(object_index, chisel_size);
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
void set_array(struct Cell *arr, int x, int y, int val, int object) {
    if (x < 0 || x >= gs->gw || y < 0 || y >= gs->gh) return;
    
    arr[x+y*gs->gw].type = val;
    arr[x+y*gs->gw].time = 0;
    
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
        gs->objects[obj].blob_data[gs->chisel->size].blobs[x+y*gs->gw] = 0;
        arr[x+y*gs->gw].object = -1;
    }
}

// Sets an index in the grid array. Obj can be left to -1 to automatically
// find an object to assign to the cell, or it can be any value.
void set(int x, int y, int val, int object) {
    set_array(gs->grid, x, y, val, object);
}

void swap_array(struct Cell *arr, int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x1 >= gs->gw || y1 < 0 || y1 >= gs->gh) return;
    if (x2 < 0 || x2 >= gs->gw || y2 < 0 || y2 >= gs->gh) return;
    
    // The blob data isn't swapped here so after calling this,
    // you should update the blob data.
    
    arr[x1+y1*gs->gw].px = x1;
    arr[x1+y1*gs->gw].py = y1;
    
    arr[x2+y2*gs->gw].px = x2;
    arr[x2+y2*gs->gw].py = y2;
    
    struct Cell temp = arr[x2+y2*gs->gw];
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

f32 get_pressure_threshold(int chisel_size) {
    switch (chisel_size) {
        case 0: return 0.8f;
        case 1: return 0.8f;
        case 2: return 0.8f;
    }
    return 0;
}

void grid_init(int w, int h) {
    gs->gw = w;
    gs->gh = h;
    
    if (gs->grid_layers[0] == NULL) {
        for (int i = 0; i < NUM_GRID_LAYERS; i++) {
            gs->grid_layers[i] = PushArray(gs->persistent_memory, w*h, sizeof(struct Cell));
        }
    }
    
    if (gs->objects[0].blob_data[0].blobs == NULL) {
        for (int i = 0; i < MAX_OBJECTS; i++) {
            for (int j = 0; j < 3; j++) {
                gs->objects[i].blob_data[j].blobs = PushArray(gs->persistent_memory, w*h, sizeof(Uint32));
                gs->objects[i].blob_data[j].blob_pressures = PushArray(gs->persistent_memory, w*h, sizeof(int));
            }
        }
    }
    
    
    for (int i = 0; i < w*h; i++) {
        for (int j = 0; j < NUM_GRID_LAYERS; j++) {
            gs->grid_layers[j][i] = (struct Cell){.type = 0, .object = -1, .depth = 255 };
            gs->grid_layers[j][i].rand = rand();
            gs->grid_layers[j][i].id = i;
        }
    }
    
    gs->grid = gs->grid_layers[0];
    gs->gas_grid = gs->grid_layers[1];
}

SDL_Color pixel_from_index_grid(struct Cell *grid, enum Cell_Type type, int i) {
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
            color.a = 180 + sin(coeff * (i + sin(gs->frames/200.0)*6))*20;
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
            color = BLACK;
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
bool move_by_velocity_gas(struct Cell *arr, int x, int y) {
    struct Cell *p = &arr[x+y*gs->gw];
    
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

void move_by_velocity(struct Cell *arr, int x, int y) {
    struct Cell *p = &arr[x+y*gs->gw];
    
    if (p->vx == 0 && p->vy == 0) {
        return;
    }
    
    // If vel < 1, that means we should wait for it to accumulate before moving.
    if (abs(p->vx) < 1) {
        p->vx_acc += p->vx;
    }
    if (abs(p->vy) < 1) {
        p->vy_acc += p->vy;
    }
    
    if (abs(p->vx_acc) >= 1) {
        p->vx = p->vx_acc;
        p->vx_acc = 0;
    }
    if (abs(p->vy_acc) >= 1) {
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

// This removes all object data.
void switch_blob_to_array(struct Cell *from, struct Cell *to, int obj, int blob, int chisel_size) {
    struct Blob_Data *data = &gs->objects[obj].blob_data[chisel_size];
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (data->blobs[x+y*gs->gw] != blob) continue;
            
            struct Cell c = from[x+y*gs->gw];
            set(x, y, 0, -1);
            to[x+y*gs->gw] = c;
            to[x+y*gs->gw].object = -1;
            
            data->blobs[x+y*gs->gw] = 0;
        }
    }
}

// Can we destroy this blob or is it too far inside?
bool blob_can_destroy(int obj, int chisel_size, int blob) {
    int blob_pressure = gs->objects[obj].blob_data[chisel_size].blob_pressures[blob];
    f32 normalized_pressure = (f32) (blob_pressure / MAX_PRESSURE);
    
    return normalized_pressure < get_pressure_threshold(chisel_size);
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
int grid_array_tick(struct Cell *array, int x_direction, int y_direction) {
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
            
            struct Cell *c = &array[x+y*gs->gw];
            
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
        struct Source_Cell *sc = &gs->levels[gs->level_current].source_cell[i];
        
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

void grid_array_draw(struct Cell *array, Uint8 alpha) {
    for (int i = 0; i < gs->gw*gs->gh; i++)
        array[i].updated = 0;
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (!array[x+y*gs->gw].type) continue;
            
            SDL_Color col = pixel_from_index(array[x+y*gs->gw].type, x+y*gs->gw);
            
            const bool draw_pressure = false;
            // TODO: This only draws pressures for the last object.
            int blob_pressure = 0;
            f32 normalized_pressure = 0;
            
            if (gs->object_count > 0) {
                blob_pressure = (int)gs->objects[gs->object_count - 1].blob_data[gs->chisel->size].blob_pressures[gs->objects[gs->object_count - 1].blob_data[gs->chisel->size].blobs[x + y * gs->gw]];
                normalized_pressure = (f32)(blob_pressure / MAX_PRESSURE);
            }
            
            if (draw_pressure && gs->objects[gs->object_count-1].blob_data[gs->chisel->size].blobs[x+y*gs->gw] && normalized_pressure >= get_pressure_threshold(gs->chisel->size)) {
                SDL_SetRenderDrawColor(gs->renderer, min((int)col.r * 2.0, 255), col.g, col.b, col.a);
            } else {
                f64 a = alpha/255.0;
                //a += -0.075f + 0.075f * sin(x/4.0+y*gs->gw/2.0 + SDL_GetTicks()/1000.0);
                SDL_SetRenderDrawColor(gs->renderer, col.r, col.g, col.b, col.a * a);
            }
            
            const bool draw_lines = false;
            if (draw_lines) {
                struct Line l = {x, y, array[x+y*gs->gw].px, array[x+y*gs->gw].py};
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
    
    //    if (gs->overlay.show)
    //        grid_array_draw(gs->levels[gs->level_current].desired_grid, 255);
    
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

// Try to move us down 1 cell.
// If any cell is unable to move, undo the work.
void object_tick(int obj) {
    if (gs->current_tool == TOOL_GRABBER && gs->grabber.object_holding == obj) return;
    
    // Copy of grid to fall back to if we abort.
    struct Cell *grid_temp = PushArray(gs->transient_memory, gs->gw*gs->gh, sizeof(struct Cell));
    memcpy(grid_temp, gs->grid, sizeof(struct Cell)*gs->gw*gs->gh);
    
    int dy = 1;
    
    for (int y = gs->gh-1; y >= 0; y--) {
        for (int x = 0; x < gs->gw; x++) {
            if (gs->grid[x+y*gs->gw].object != obj) continue;
            
            if (y+1 >= gs->gh || gs->grid[x+(y+1)*gs->gw].type) {
                // Abort!
                memcpy(gs->grid, grid_temp, sizeof(struct Cell)*gs->gw*gs->gh);
                return;
            } else {
                swap(x, y, x, y+dy);
            }
        }
    }
    
    if (!object_does_exist(obj)) {
        for (int i = obj; i < gs->object_count-1; i++) {
            gs->objects[i] = gs->objects[i+1];
        }
        gs->object_count--;
    }
    
    object_generate_blobs(obj, 0);
    object_generate_blobs(obj, 1);
    object_generate_blobs(obj, 2);
}

int cell_count_of_blob(int object, Uint32 blob, int chisel_size) {
    struct Object *obj = &gs->objects[object];
    int count = 0;
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (gs->grid[x+y*gs->gw].object != object) continue;
            if (obj->blob_data[chisel_size].blobs[x+y*gs->gw] != blob) continue;
            count++;
        }
    }
    return count;
}

bool object_remove_blob(int object, Uint32 blob, int chisel_size, bool replace_dust) {
    struct Object *obj = &gs->objects[object];
    
    const bool easy_chiseling = false;
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            
            if (obj->blob_data[chisel_size].blobs[x+y*gs->gw] != blob) continue;
            //if (gs->blocker.active && gs->blocker.pixels[x+y*gs->gw] != blocker_side) continue;
            if (easy_chiseling && gs->overlay.grid[x+y*gs->gw]) continue;
            
            if (gs->level_current+1 == 1 && gs->overlay.grid[x+y*gs->gw] && !gs->did_undo_tutorial) {
                gs->tutorial = *tutorial_rect(TUTORIAL_UNDO_STRING,
                                              32,
                                              GUI_H+32,
                                              NULL);
                gs->did_undo_tutorial = true;
            }
            
            //if (gs->grid[x+y*gs->gw].type == CELL_DIAMOND && chisel_size != 0) continue;
            
            bool available = true;
            
            if (replace_dust) {
                //set_array(gs->pickup_grid, x, y, gs->grid[x+y*gs->gw].type, -2);
                available = add_item_to_inventory_slot(gs->grid[x+y*gs->gw].type, 1);
            }
            
            if (available) {
                set(x, y, CELL_NONE, -1);
                struct Level *lvl = &gs->levels[gs->level_current];
                for (int i = 0; i < lvl->source_cell_count; i++) {
                    if (abs(lvl->source_cell[i].x - x) <= 2 && abs(lvl->source_cell[i].y - y) <= 2){
                        lvl->source_cell_count--;
                        for (int j = i; j < lvl->source_cell_count; j++) {
                            lvl->source_cell[j] = lvl->source_cell[j+1];
                        }
                    }
                }
            }
        }
    }
    
    object_blobs_set_pressure(object, chisel_size);
    
    return false;
}

void object_darken_blob(struct Object *obj, Uint32 blob, int amt, int chisel_size) {
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (obj->blob_data[chisel_size].blobs[x+y*gs->gw] == blob) {
                if (gs->grid[x+y*gs->gw].depth <= amt) {
                    gs->grid[x+y*gs->gw].depth = 0;
                } else {
                    gs->grid[x+y*gs->gw].depth -= amt;
                }
            }
        }
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

// We don't want to use a memset because there are pointers
// that would need to be freed. So, we just reset all the memory
// the pointers point to, instead of having to reallocate everything.
void objects_zero_memory(void) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        gs->objects[i].cell_count = 0;
        for (int chisel_size = 0; chisel_size < 3; chisel_size++) {
            memset(gs->objects[i].blob_data[chisel_size].blobs, 0, sizeof(Uint32)*gs->gw*gs->gh);
            memset(gs->objects[i].blob_data[chisel_size].blob_pressures, 0, sizeof(Uint32)*gs->gw*gs->gh);
            gs->objects[i].blob_data[chisel_size].blob_count = 0;
            gs->objects[i].cell_count = 0;
        }
    }
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
    
    objects_zero_memory();
    
    for (int y = 0; y < gs->gh; y++) {
        for (int x = 0; x < gs->gw; x++) {
            if (!gs->grid[x+y*gs->gw].type || !is_cell_hard(gs->grid[x+y*gs->gw].type) || gs->grid[x+y*gs->gw].temp != -1) continue;
            int cell_count = 0;
            mark_neighbours(x, y, gs->object_count, gs->grid[x+y*gs->gw].object, &cell_count);
            gs->objects[gs->object_count].cell_count = cell_count;
            gs->object_count++;
        }
    }
    
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        gs->grid[i].object = gs->grid[i].temp;
        gs->grid[i].temp = 0;
    }
    
    // Always reevaluate blobs...
    // We used to have it only update if a new object is created
    // or it is forced to (update_blobs as a parameter) but
    // it seemed to cause a bug where chisel_goto_blob skips
    // over the blob. Maybe fix if that becomes a problem.
    
    /* if (update_blobs || object_count != previous_count) { */
    for (int i = 0; i < gs->object_count; i++) {
        object_generate_blobs(i, 0);
        object_generate_blobs(i, 1);
        object_generate_blobs(i, 2);
    }
    /* } */
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
    
    struct Cell *grid_temp = PushArray(gs->transient_memory, gs->gw*gs->gh, sizeof(struct Cell));
    memcpy(grid_temp, gs->grid, sizeof(struct Cell)*gs->gw*gs->gh);
    
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
                    memcpy(gs->grid, grid_temp, sizeof(struct Cell) * gs->gw * gs->gh);
                    /* temp_dealloc(result_grid); */
                    return 0;
                }
                // Otherwise, go through with the set.
                swap(x, y, rx, ry);
            }
        }
    }
    /* } */
    
    object_generate_blobs(object, 0);
    object_generate_blobs(object, 1);
    object_generate_blobs(object, 2);
    
    return (int) (ux+uy*gs->gw);
}

int get_cell_index_by_id(struct Cell *array, int id) {
    for (int i = 0; i < gs->gw*gs->gh; i++) {
        if (array[i].id == id) {
            return i;
        }
    }
    return -1;
}

void draw_blobs(void) {
    if (!gs->do_draw_blobs) return;
    
    for (int i = 0; i < gs->object_count; i++) {
        for (int y = 0; y < gs->gh; y++) {
            for (int x = 0; x < gs->gw; x++) {
                Uint32 b = gs->objects[i].blob_data[gs->chisel->size].blobs[x+y*gs->gw];
                b *= b;
                if (b == 0) continue;
                SDL_SetRenderDrawColor(gs->renderer, my_rand(b), my_rand(b*b), my_rand(b*b*b), 255);
                SDL_RenderDrawPoint(gs->renderer, x, y);
            }
        }
    }
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