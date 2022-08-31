#include "grid.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <time.h>
#include <signal.h>

#include "globals.h"
#include "util.h"
#include "chisel.h"
#include "grabber.h"
#include "level.h"
#include "chisel_blocker.h"
#include "placer.h"

struct Cell *grid_layers[NUM_GRID_LAYERS] = {0};

struct Cell *grid,        // Main grid of cells
            *fg_grid,     // Foreground grid, cannot be interacted with.
            *pickup_grid;
int gw, gh;
int grid_show_ghost = 0;

struct Object objects[MAX_OBJECTS];
int object_count = 0, object_current = 0;

int do_draw_blobs = 0, do_draw_objects = 0;
bool paused = false;
int frames = 0, step_one = 0;

static int draw_lines = 0;

static
SDL_Surface *bark_surface,
    *glass_surface,
    *wood_plank_surface,
    *diamond_surface,
    *ice_surface,
    *grass_surface,
    *triangle_blob_surface;

static float damp(int i) {
    srand(i);
    return 0.5 + ((float)rand())/RAND_MAX * 0.4;
}

static float water_spread() {
    return rand()%3 + ((float)rand())/RAND_MAX;
}

float get_pressure_threshold(int chisel_size) {
    switch (chisel_size) {
    case 0:
        return 0.75;
    case 1:
        return 0.85;
    case 2:
        return 0.8;
    }
    return 0;
}

void print_blob_data(struct Object *object, int chisel_size) {
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            printf("%u ", object->blob_data[chisel_size].blobs[x+y*gw]);
        }
        printf("\n");
    }
    printf("\n\n");
    fflush(stdout);
}

void grid_init(int w, int h) {
    gw = w;
    gh = h;

    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        grid_layers[i] = calloc(w*h, sizeof(struct Cell));
    }

    grid = grid_layers[0];
    fg_grid = grid_layers[1];
    pickup_grid = grid_layers[2];

    memset(objects, 0, sizeof(struct Object)*MAX_OBJECTS);
    for (int i = 0; i < MAX_OBJECTS; i++) {
        for (int j = 0; j < 3; j++) {
            objects[i].blob_data[j].blobs = calloc(w*h, sizeof(Uint32));
            objects[i].blob_data[j].blob_pressures = calloc(w*h, sizeof(int));
        }
    }

    srand(time(0));

    for (int i = 0; i < w*h; i++) {
        for (int j = 0; j < NUM_GRID_LAYERS; j++) {
            grid_layers[j][i] = (struct Cell){.type = 0, .object = -1, .depth = 255 };
            grid_layers[j][i].rand = rand();
        }
    }

    bark_surface = IMG_Load("../res/bark.png");
    glass_surface = IMG_Load("../res/glass.png");
    wood_plank_surface = IMG_Load("../res/plank.png");
    diamond_surface = IMG_Load("../res/diamond.png");
    ice_surface = IMG_Load("../res/ice.png");
    grass_surface = IMG_Load("../res/grass.png");
    triangle_blob_surface = IMG_Load("../res/triangle_blob.png");
    SDL_assert(triangle_blob_surface);
}

void grid_deinit() {
    for (int i = 0; i < NUM_GRID_LAYERS; i++) {
        free(grid_layers[i]);
    }
    SDL_FreeSurface(glass_surface);
    SDL_FreeSurface(bark_surface);
    SDL_FreeSurface(wood_plank_surface);
    SDL_FreeSurface(diamond_surface);
    SDL_FreeSurface(ice_surface);
    SDL_FreeSurface(grass_surface);
    SDL_FreeSurface(triangle_blob_surface);
}

SDL_Color pixel_from_index(struct Cell *cells, int i) {
    SDL_Color color;
    int r, amt;

    switch (cells[i].type) {
    case CELL_MARBLE:
        color = (SDL_Color){230+(i*i*i)%25, 230+(i*i*i*i)%25, 230+(i*i*i*i*i*i)%25, 255};
        break;
    case CELL_COBBLESTONE:
        r = my_rand(i*i) % 100 < 10;
        amt = 25;
        color = (SDL_Color){140 + r*amt + ((i*i*i*i)%20 - 10), 140 + r*amt + ((i*i*i*i)%20 - 10), 135 + r*amt + ((i*i*i*i)%20 - 10), 255};
        break;
    case CELL_QUARTZ:
        r = my_rand(i*i) % 100 < 10;
        amt = 25;
        color = (SDL_Color){213-10 + r*amt + ((i*i*i*i)%20 - 10), 215-10 + r*amt + ((i*i*i*i)%20 - 10), 226-10 + r*amt + ((i*i*i*i)%20 - 10), 255};
        break;
    case CELL_WOOD_LOG:
        color = get_pixel(bark_surface, i%gw, i/gh);
        break;
    case CELL_WOOD_PLANK:
        color = get_pixel(wood_plank_surface, i%gw, i/gw);
        break;
    case CELL_DIRT:;
        const int variance = 10;
        color = (SDL_Color){70+(i*i*i)%variance, 50+(i*i*i*i)%variance, 33+(i*i*i*i*i)%variance, 255};
        break;
    case CELL_SAND:
        color = (SDL_Color){216-30+(i*i*i)%30, 190-30+(i*i*i*i)%30, 125-30+(i*i*i*i*i)%30, 255};
        break;
    case CELL_GLASS:; 
        /* r = my_rand(i*i) % 100 < 5; */
        color = get_pixel(glass_surface, i%gw, i/gw);
        color.a = 255;
        /* if (r || get_neighbour_count_of_object(i%gw, i/gw, 1, cells[i].object) < 8) { */
        /*     color.a = 240; */
        /* } */
        break;
    case CELL_WATER:
        color = (SDL_Color){131, 160, 226, 255};
        break;
    case CELL_COAL:
        r = my_rand(i*i) % 100 < 10;
        amt = 25;
        color = (SDL_Color){70-10 + r*amt + ((i*i*i*i)%20 - 10), 70-10 + r*amt + ((i*i*i*i)%20 - 10), 70-10 + r*amt + ((i*i*i*i)%20 - 10), 255};
        break;
    case CELL_STEAM:
        color = (SDL_Color){50, 50, 50, 255};
        break;
    case CELL_DIAMOND:
        color = get_pixel(diamond_surface, i%gw, i/gw);
        break;
    case CELL_ICE:
        color = get_pixel(ice_surface, i%gw, i/gw);
        break;
    case CELL_LEAF:
        color = get_pixel(grass_surface, i%gw, i/gw);
        break;
    case CELL_SMOKE:
        color = (SDL_Color){120, 120, 120, 255};
        break;
    case CELL_DUST:
        color = (SDL_Color){100, 100, 100, 255};
        break;
    }
    if (cells[i].type != CELL_GLASS && is_cell_hard(cells[i].type))
        color.a = cells[i].depth;
    return color;
}

// In this function, we use vx_acc and vy_acc as a higher precision position value.
// Ensure it's set to that before calling this function.
bool move_by_velocity_gas(struct Cell *arr, int x, int y) {
    struct Cell *p = &arr[x+y*gw];

    if (p->vx_acc == 0 && p->vy_acc == 0) {
        p->vx_acc = x;
        p->vy_acc = y;
        return false;
    }

    p->vx_acc += p->vx;
    p->vy_acc += p->vy;

    int tx = (int)p->vx_acc;
    int ty = (int)p->vy_acc;
    
    if (!is_in_bounds(tx, ty) || (is_in_bounds(tx, ty) && grid[tx+ty*gw].type && grid[tx+ty*gw].type != p->type)) {
        p->vx_acc = x;
        p->vy_acc = y;
        p->vx = 0;
        p->vy = 0;
        return true;
    }

    swap_array(arr, x, y, (int)p->vx_acc, (int)p->vy_acc);
    return false;
}

void move_by_velocity(struct Cell *arr, int x, int y) {
    struct Cell *p = &arr[x+y*gw];

    if (p->vx == 0 && p->vy == 0) {
        return;
    }

    // If vel < 1, that means we should wait for it to accumulate before moving.
    if (p->vx < 1) {
        p->vx_acc += p->vx;
    }
    if (p->vy < 1) {
        p->vy_acc += p->vy;
    }

    if (p->vx_acc >= 1) {
        p->vx = p->vx_acc;
        p->vx_acc = 0;
    }
    if (p->vy_acc >= 1) {
        p->vy = p->vy_acc;
        p->vy_acc = 0;
    }

    float xx = x;
    float yy = y;

    float len = sqrt(p->vx*p->vx + p->vy*p->vy);
    float ux = p->vx/len;
    float uy = p->vy/len;

    while (sqrt((xx-x)*(xx-x) + (yy-y)*(yy-y)) <= len) {
        xx += ux;
        yy += uy;
        if (xx < 0 || (int)xx >= gw) {
            p->vx *= -damp(x+y*gw);
            xx -= ux;
            yy -= uy;
            break;
        }
        if (yy < 0 || (int)yy >= gh) {
            p->vy = 0;
            xx -= ux;
            yy -= uy;
            break;
        }
        if (((int)xx != x || (int)yy != y) && arr[(int)xx+((int)yy)*gw].type) {
            if (ux) {
                if (!is_cell_liquid(p->type)) {
                    p->vx *= -0.2;
                } else {
                    p->vx = 0;
                }
            }
            if (uy) {
                if (!is_cell_liquid(p->type)) {
                    p->vy *= -0.6;
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

int is_in_bounds(int x, int y) {
    return x >= 0 && y >= 0 && x < gw && y < gh;
}

int is_in_boundsf(float x, float y) {
    return is_in_bounds((int)x, (int)y);
}

void swap_array(struct Cell *arr, int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x1 >= gw || y1 < 0 || y1 >= gh) return;
    if (x2 < 0 || x2 >= gw || y2 < 0 || y2 >= gh) return;

    // The blob data isn't swapped here so after calling this,
    // you should update the blob dkta.

    arr[x1+y1*gw].px = x1;
    arr[x1+y1*gw].py = y1;
    
    arr[x2+y2*gw].px = x2;
    arr[x2+y2*gw].py = y2;

    struct Cell temp = arr[x2+y2*gw];
    arr[x2+y2*gw] = arr[x1+y1*gw];
    arr[x1+y1*gw] = temp;
}

void swap(int x1, int y1, int x2, int y2) {
    swap_array(grid, x1, y1, x2, y2);
}

// This removes all object data.
void switch_blob_to_array(struct Cell *from, struct Cell *to, int obj, int blob, int chisel_size) {
    struct Blob_Data *data = &objects[obj].blob_data[chisel_size];
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (data->blobs[x+y*gw] != blob) continue;

            struct Cell c = from[x+y*gw];
            set(x, y, 0, -1);
            to[x+y*gw] = c;
            to[x+y*gw].object = -1;

            data->blobs[x+y*gw] = 0;
        }
    }
}

// Sets an index in a grid array. object can be left to -1 to automatically
// find an object to assign to the cell, or it can be any value.
void set_array(struct Cell *arr, int x, int y, int val, int object) {
    if (x < 0 || x >= gw || y < 0 || y >= gh) return;

    arr[x+y*gw].type = val;
    arr[x+y*gw].time = 0;

    if (object != -1) {
        arr[x+y*gw].object = object;
    } else if (is_cell_hard(val)) { // Automatically set if the cell should be an obj
        // by finding any neighbouring object, and setting it to that
        // or just creating a new one if it can't find it.
        int any = get_any_neighbour_object(x, y);
        if (any != -1) {
            arr[x+y*gw].object = any;
        } else {
            arr[x+y*gw].object = object_count++;
        }
    }

    int obj = arr[x+y*gw].object;

    if (obj == -1 || obj >= MAX_OBJECTS) return;

    if (val == 0) {
        objects[obj].blob_data[chisel->size].blobs[x+y*gw] = 0;
        arr[x+y*gw].object = -1;
    }
}

// Sets an index in the grid array. Obj can be left to -1 to automatically
// find an object to assign to the cell, or it can be any value.
void set(int x, int y, int val, int object) {
    set_array(grid, x, y, val, object);
}

// Can we destroy this blob or is it too far inside?
bool blob_can_destroy(int obj, int chisel_size, int blob) {
    int blob_pressure = objects[obj].blob_data[chisel_size].blob_pressures[blob];
    float normalized_pressure = (float)blob_pressure / MAX_PRESSURE;

    if (normalized_pressure >= get_pressure_threshold(chisel_size)) {
        printf("Blocked due to too much pressure (%f > %f).\n", normalized_pressure, get_pressure_threshold(chisel_size));
        fflush(stdout);
    }

    return normalized_pressure < get_pressure_threshold(chisel_size);
}

// Checks if an object still exists, or if all its cells were removed.
int object_does_exist(int obj) {
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (grid[x+y*gw].object == obj) return 1;
        }
    }
    return 0;
}

bool can_gas_cell_swap(int x, int y) {
    return !grid[x+y*gw].type || is_cell_gas(grid[x+y*gw].type);
}

// For x_direction and y_direction, a value of -1 or 1 should be used.
void grid_array_tick(struct Cell *array, int x_direction, int y_direction) {
    int start_y = (y_direction == 1) ? 0 : gh-1;
    int start_x = (x_direction == 1) ? 0 : gw-1;

#define y_condition(y) ((start_y == 0) ? (y < gh) : (y >= 0))
#define x_condition(x) ((start_x == 0) ? (x < gw) : (x >= 0))

    for (int y = start_y; y_condition(y); y += y_direction) {
        for (int q = start_x; x_condition(q); q += x_direction) {
            int x = q;
            if (frames%2 == 0) {
                x = gw - 1 - x;
            }

            struct Cell *c = &array[x+y*gw];

            // Make sure we're only dealing with non-objects.
            if (!c->type || c->object != -1 || c->updated) continue;

            c->updated = 1;
            c->time++;

            float sp = 0.5;

            switch (c->type) {
            case CELL_WATER: {
                if (is_in_bounds(x, y+1) && !array[x+(y+1)*gw].type) {
                    c->vy += GRAV;
                    if (c->vy > MAX_GRAV) c->vy = MAX_GRAV;
                } else if (is_in_bounds(x+1, y+1) && !array[x+1+(y+1)*gw].type) {
                    c->vx = sp;
                    c->vy = sp;
                } else if (is_in_bounds(x-1, y+1) && !array[x-1+(y+1)*gw].type) {
                    c->vx = -sp;
                    c->vy = sp;
                } else if (is_in_bounds(x+1, y) && is_in_bounds(x-1, y) && !array[x+1+y*gw].type && x-1 >= 0 && !array[x-1+y*gw].type) {
                    float dx = water_spread() * ((rand()%2==0)?1:-1);
                    c->vx = dx/4.;
                    c->vy = 0;
                } else if (is_in_bounds(x+1, y) && !array[x+1+y*gw].type) {
                    c->vx = sp;
                    c->vy = 0;
                } else if (is_in_bounds(x-1, y) && !array[x-1+y*gw].type) {
                    c->vx = -sp;
                    c->vy = 0;
                } else {
                    c->vx = c->vy = 0;
                }
                break;
            }
            case CELL_SAND: {
                if (is_in_bounds(x, y+1) && !array[x+(y+1)*gw].type) {
                    c->vy += GRAV;
                    if (c->vy > MAX_GRAV) c->vy = MAX_GRAV;
                    c->vx *= damp(x+y*gw);
                } else if (is_in_bounds(x+1, y+1) && !array[x+1+(y+1)*gw].type) {
                    c->vx = 1;
                    c->vy = 1;
                } else if (is_in_bounds(x-1, y+1) && !array[x-1+(y+1)*gw].type) {
                    c->vx = -1;
                    c->vy = 1;
                } else {
                    c->vx = c->vy = 0;
                }
                break;
            }
            case CELL_DUST: {
                if (c->time >= 60 + rand()%80) {
                    set_array(array, x, y, 0, -1);
                    continue;
                }

                c->vy += GRAV;
                c->vx *= 0.75;

                const float small = 0.5f;
                if (c->vx < small) {
                    c->vx = 0;
                    c->vx_acc = 0;
                }
                break;
            };
            case CELL_STEAM: case CELL_SMOKE: {
                float fac = 0.4*randf(1.0);
                float amplitude = 1.0;

                // If we hit something last frame...
                if (is_in_bounds(x, y-1) && can_gas_cell_swap(x, y-1)) {
                    c->vy = -1;
                    c->vx = amplitude * sin(c->rand + c->time * fac);
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
                /*     if (!is_in_bounds(tx, y) || (is_in_bounds(tx, y) && grid[tx+y*gw].type && grid[tx+y*gw].type != c->type)) { */
                /*         c->vx = 0; */
                /*     } */
                /* } */

                if (y == 0 ||
                    (x == 0 && c->vx < 0) ||
                    (x == gw-1 && c->vx > 0) ||
                    (y == gw-1 && c->vy > 0)) {
                    set_array(array, x, y, 0, -1);
                }

                if (c->type) {
                    move_by_velocity_gas(grid, x, y);
                }
                continue;
            }
            default: {
                if (is_in_bounds(x, y+1) && !array[x+(y+1)*gw].type) {
                    c->vx = 0;
                    c->vy = 1;
                } else if (is_in_bounds(x+1, y+1) && !array[x+1+(y+1)*gw].type) {
                    c->vx = 1;
                    c->vy = 1;
                } else if (is_in_bounds(x-1, y+1) && !array[x-1+(y+1)*gw].type) {
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
}

void simulation_tick() {
    if (!step_one)
        if (paused) return;

    frames++;

    if (step_one) {
        step_one = 0;
    }

    for (int i = 0; i < levels[level_current].source_cell_count; i++) {
        struct Source_Cell *sc = &levels[level_current].source_cell[i];

        int x = sc->x;
        int y = sc->y;
        if (!grid[x+y*gw].type && sc->type != 0) {
            char str[256] = {0};
            get_name_from_type(sc->type, str);
                
            set(x, y, sc->type, -1);
            grid[x+y*gw].vy = -1;

            if (!grid[x+y*gw].type) {
                grid[x+y*gw].type = sc->type;
            }
        }
    }

    grid_array_tick(grid, 1, -1);
    grid_array_tick(fg_grid, 1, 1);
    if (frames % 2 == 0)
        grid_array_tick(pickup_grid, 1, -1);
}

static int outlines_total[256*256] = {0}; // 256x256 seems to be more than the max level size.
static int outlines_total_count = 0;

void grid_array_draw(struct Cell *array) {
    for (int i = 0; i < gw*gh; i++)
        array[i].updated = 0;

    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (!array[x+y*gw].type) continue;

            SDL_Color col = pixel_from_index(array, x+y*gw);
            if (DRAW_PRESSURE && objects[object_count-1].blob_data[chisel->size].blobs[x+y*gw]) {
                Uint8 c = (Uint8)(255 * ((float)objects[object_count-1].blob_data[chisel->size].blob_pressures[objects[object_count-1].blob_data[chisel->size].blobs[x+y*gw]] / MAX_PRESSURE));
                SDL_SetRenderDrawColor(renderer, c, c, c, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
            }
            if (draw_lines) {
                struct Line l = {x, y, array[x+y*gw].px, array[x+y*gw].py};
                if (array[x+y*gw].px != 0 && array[x+y*gw].py != 0 && array[x+y*gw].type == CELL_WATER) {
                    SDL_RenderDrawLine(renderer, l.x1, l.y1, l.x2, l.y2);
                } else {
                    SDL_RenderDrawPoint(renderer, l.x1, l.y1);
                }
            } else {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

void grid_draw(void) {
    // Draw main grid
    grid_array_draw(grid);
    grid_array_draw(fg_grid);
    grid_array_draw(pickup_grid);
    
    for (int i = 0; i < outlines_total_count; i++) {
        int o = outlines_total[i];
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawPoint(renderer, o%gw, o/gw);
    }

    // Draw inspiration ghost
    if (grid_show_ghost) {
        for (int y = 0; y < gh; y++) {
            for (int x = 0; x < gw; x++) {
                if (!levels[level_current].desired_grid[x+y*gw].type) continue;

                SDL_Color col = pixel_from_index(levels[level_current].desired_grid, x+y*gw);
                float b = col.r + col.g + col.b;
                b /= 3.;
                b = (int)clamp((int)b, 0, 255);
                SDL_SetRenderDrawColor(renderer, b/2, b/4, b, 200 + sin((2*x+2*y+SDL_GetTicks())/700.0)*10);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

int get_any_neighbour_object(int x, int y) {
    for (int xx = -1; xx <= 1; xx++) {
        for (int yy = -1; yy <= 1; yy++) {
            if (xx == 0 && yy == 0)        continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (grid[x+xx+(y+yy)*gw].object != -1) {
                return grid[x+xx+(y+yy)*gw].object;
            }
        }
    }
    return -1;
}

int get_neighbour_count(int x, int y, int r) {
    int c = 0;
    for (int xx = -r; xx <= r; xx++) {
        for (int yy = -r; yy <= r; yy++) {
            if (xx == 0 && yy == 0) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (grid[x+xx+(y+yy)*gw].type) c++;
        }
    }
    return c;
}

static int is_any_neighbour_hard(int x, int y) {
    int r = 1;
    for (int xx = -r; xx <= r; xx++) {
        for (int yy = -r; yy <= r; yy++) {
            if (xx == 0 && yy == 0) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (is_cell_hard(grid[x+xx+(y+yy)*gw].type)) return 1;
        }
    }
    return 0;
}

int get_neighbour_count_of_object(int x, int y, int r, int obj) {
    int c = 0;

    for (int xx = -r; xx <= r; xx++) {
        for (int yy = -r; yy <= r; yy++) {
            if (xx == 0 && yy == 0) continue;
            if (!is_in_bounds(x+xx, y+yy)) continue;
            if (grid[x+xx+(y+yy)*gw].type && grid[x+xx+(y+yy)*gw].object == obj) c++;
        }
    }
    return c;
}

int blob_find_pressure(int obj, Uint32 blob, int x, int y, int r) {
    int max_blob_size = 8;
    float pressure = 0.f;
    int count = 0;
    struct Object *object = &objects[obj];

    for (int dy = -max_blob_size/2; dy < max_blob_size/2; dy++) {
        for (int dx = -max_blob_size/2; dx < max_blob_size/2; dx++) {
            int rx = x+dx;
            int ry = y+dy;
            if (object->blob_data[chisel->size].blobs[rx+ry*gw] == blob) {
                pressure += get_neighbour_count_of_object(rx, ry, r, obj);
                count++;
            }
        }
    }

    if (count == 0) return 0;

    pressure /= count;

    return pressure;
}

// Try to move us down 1 cell.
// If any cell is unable to move, undo the work.
void object_tick(int obj) {
    if (current_tool == TOOL_GRABBER && grabber.object_holding == obj) return;

    // Copy of grid to fall back to if we abort.
    struct Cell grid_temp[gw*gh];
    memcpy(grid_temp, grid, sizeof(struct Cell)*gw*gh);

    int dy = 1;
    
    for (int y = gh-1; y >= 0; y--) {
        for (int x = 0; x < gw; x++) {
            if (grid[x+y*gw].object != obj) continue;
            
            if (y+1 >= gh || grid[x+(y+1)*gw].type) {
                // Abort!
                memcpy(grid, grid_temp, sizeof(struct Cell)*gw*gh);
                return;
            } else {
                swap(x, y, x, y+dy);
            }
        }
    }
    
    if (!object_does_exist(obj)) {
        for (int i = obj; i < object_count-1; i++) {
            objects[i] = objects[i+1];
        }
        object_count--;
    }
    
    object_generate_blobs(obj, 0);
    object_generate_blobs(obj, 1);
    object_generate_blobs(obj, 2);
}

void object_blobs_set_pressure(int obj, int chisel_size) {
    int temp_blobs[gw*gh];
    struct Object *object = &objects[obj];
    
    for (int i = 0; i < gw*gh; i++) {
        temp_blobs[i] = object->blob_data[chisel_size].blobs[i];
    }

    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (!object->blob_data[chisel_size].blobs[x+y*gw] || temp_blobs[x+y*gw] == -1) continue;

            Uint32 blob = object->blob_data[chisel_size].blobs[x+y*gw];

            if (blob >= 2147483648-1) {
                printf("Blob in question: %u\n", blob); fflush(stdout);
                print_blob_data(object, chisel_size);
            }

            object->blob_data[chisel_size].blob_pressures[blob] = blob_find_pressure(obj, blob, x, y, (chisel_size == 2 ? 3 : 5));

            // Mark this blob as complete.
            int max_blob_size = 8;
            for (int dy = -max_blob_size/2; dy < max_blob_size/2; dy++) {
                for (int dx = -max_blob_size/2; dx < max_blob_size/2; dx++) {
                    int rx = x+dx;
                    int ry = y+dy;
                    if (rx >= 0 && rx < gw && ry >= 0 && ry < gh && temp_blobs[rx+ry*gw] == blob)
                        temp_blobs[rx + ry*gw] = -1;
                }
            }
        }
    }
}

static void random_set_blob(struct Object *obj, int x, int y, Uint32 blob, int chisel_size, int perc) {
    if (!is_cell_hard(grid[x+y*gw].type) ||
        (x+1 < gw && !is_cell_hard(grid[x+1 + y*gw].type)) ||
        (x-1 >= 0 && !is_cell_hard(grid[x-1 + y*gw].type)) ||
        (y+1 < gh && !is_cell_hard(grid[x + (y+1)*gw].type)) ||
        (y-1 >= 0 && !is_cell_hard(grid[x + (y-1)*gw].type))) {
        return;
    }
    if (!is_in_bounds(x, y)) return;
    if (perc == 0) return;

    if (rand() % 100 < perc) {
        obj->blob_data[chisel_size].blobs[x+y*gw] = blob;
    }
}

// Find the amount of cells a blob has in a given object.
static int blob_find_count_from_position(int object, int chisel_size, int x, int y) {
    int count = 0;
    const int max_size = 5;
    Uint32 *blobs = objects[object].blob_data[chisel_size].blobs;

    Uint32 mine = blobs[x+y*gw];

    for (int yy = -max_size; yy <= max_size; yy++) {
        for (int xx = -max_size; xx <= max_size; xx++) {
            int rx = x+xx;
            int ry = y+yy;

            if (blobs[rx+ry*gw] == mine) {
                count++;
            }
        }
    }

    return count;
}

void blob_generate_old_smart(int obj, int chisel_size, int percentage, Uint32 *blob_count) {
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (grid[x+y*gw].object != obj) continue;
            if (objects[obj].blob_data[chisel_size].blobs[x+y*gw] > 0) continue;

            struct Object *o = &objects[obj];
            Uint32 *blobs = o->blob_data[chisel_size].blobs;

            int size = (rand()%2==0) ? 2 : 3;
            if (chisel_size == 2) size++;

            int did_fill = 0;

            for (int yy = 0; yy < size; yy++) {
                for (int xx = 0; xx < size; xx++) {
                    if (x+xx >= gw || y+yy >= gh) continue;
                    if (is_cell_hard(grid[x+xx + (y+yy)*gw].type)) {
                        blobs[x+xx + (y+yy)*gw] = *blob_count;
                        did_fill = 1;
                    }
                }
            }

            int perc = percentage;
            if (size == 2) {
                perc = 0; // 2x2 squares will still have swapped neighbours because of other blobs next to it swapping theirs.
            } else if (size == 4) {
                perc *= 3;
            }

            if (blob_find_count_from_position(obj, chisel_size, x, y)) {
                for (int yy = -1; yy < -1+(size+1)*2; yy += size+1) {
                    for (int xx = -1; xx <= size; xx++) {
                        // Not diagonals
                        if (xx == -1 && yy == -1) continue;
                        if (xx == size && yy == -1)  continue;
                        if (xx == size && yy == -2+(size+1)*2) continue;
                        if (xx == -1 && yy == -2+(size+1)*2)  continue;
                        if (!is_in_bounds(x+xx, y+yy)) continue;
                        if (blob_find_count_from_position(obj, chisel_size, x+xx, y+yy) <= 4) continue;
                        random_set_blob(o, x+xx, y+yy, *blob_count, chisel_size, perc);
                    }
                }
                for (int yy = -1; yy <= size; yy++) {
                    for (int xx = -1; xx < -1+(size+1)*2; xx += size+1) {
                        // Not diagonals
                        if (xx == -1 && yy == -1) continue;
                        if (xx == size && yy == -1)  continue;
                        if (xx == size && yy == -2+(size+1)*2) continue;
                        if (xx == -1 && yy == -2+(size+1)*2)  continue;
                        if (!is_in_bounds(x+xx, y+yy)) continue;
                        if (blob_find_count_from_position(obj, chisel_size, x+xx, y+yy) <= 4) continue;
                        random_set_blob(o, x+xx, y+yy, *blob_count, chisel_size, perc);
                    }
                }
            }

            if (did_fill) {
                (*blob_count)++;
            }
        }
    }
}

/* static void blob_generate_smart(int obj, int chisel_size, int percentage, Uint32 *blob_count) { */
/*     struct Object *o = &objects[obj]; */

/*     for (int y = 0; y < gh; y++) { */
/*         for (int x = 0; x < gw; x++) { */
/*             if (grid[x+y*gw].object != obj) continue; */
/*             if (objects[obj].blob_data[chisel_size].blobs[x+y*gw] > 0) continue; */

/*             int did_fill = 0; */
/*             int size = 3; */

/*             if (chisel_size == 2) { */
/*                 size += 2; */
/*             } */

/*             // Loop through and set a square as the current blob. */
/*             for (int yy = 0; yy < size; yy++) { */
/*                 for (int xx = 0; xx < size; xx++) { */
/*                     if (x+xx >= gw || y+yy >= gh) continue; */
/*                     if (is_cell_hard(grid[x+xx + (y+yy)*gw].type)) { */
/*                         o->blob_data[chisel_size].blobs[x+xx + (y+yy)*gw] = *blob_count; */
/*                         did_fill = 1; */
/*                     } */
/*                 } */
/*             } */

/*             int perc = percentage; */
/*             if (size == 2) { */
/*                 perc = 0; */
/*             } */

/*             if (did_fill) { */
/*                 (*blob_count)++; */
/*             } */
/*         } */
/*     } */

/*     for (int y = 0; y < gh; y++) { */
/*         for (int x = 0; x < gw; x++) { */
/*             if (grid[x+y*gw].object != obj) continue; */
/*             if (rand()%100 > percentage) continue; */

/*             Uint32 blob = o->blob_data[chisel_size].blobs[x+y*gw]; */

/*             const int radius = 6; */
/*             for (int yy = -radius; yy <= radius; yy++) { */
/*                 for (int xx = -radius; xx <= radius; xx++) { */
/*                     // Ensure we're on our blob. */
/*                     if (!is_in_bounds(x+xx, y+yy)) continue; */
/*                     if (o->blob_data[chisel_size].blobs[x+xx+(y+yy)*gw] != blob) continue; */

/*                     // Check if we're edging another blob. */
/*                     bool is_edging = false; */
/*                     int blob_edging_indices[8] = {-1}; // Blob that we're edging's index. Stored in an array to randomly choose from. */
/*                     int blob_edging_count = 0; */
/*                     for (int dy = -1; dy <= 1; dy++) { */
/*                         for (int dx = -1; dx <= 1; dx++) { */
/*                             if (dx == 0 && dy == 0) continue; */

/*                             Uint32 this = o->blob_data[chisel_size].blobs[xx+dx+(yy+dy)*gw]; */

/*                             if (this > 0 && this != blob) { */
/*                                 is_edging = true; */
/*                                 blob_edging_indices[blob_edging_count++] = xx+dx+(yy+dy)*gw; */
/*                             } */
/*                         } */
/*                     } */

/*                     if (is_edging) { */
/*                         int r = rand() % blob_edging_count; */
/*                         Uint32 blob_edging = o->blob_data[chisel_size].blobs[blob_edging_indices[r]]; */
/*                         // If it's big enough so it won't cause any odd artifacts... */
/*                         if (blob_find_count(obj, chisel_size, blob_edging)) { */
/*                             // Set that neighbouring blob to ours. */
/*                             o->blob_data[chisel_size].blobs[blob_edging_indices[r]] = blob; */
/*                         } */
/*                     } */
/*                 } */
/*             } */
/*         } */
/*     } */
/* } */

static void blob_generate_new(int obj, int chisel_size, Uint32 *blob_count) {
    struct Object *o = &objects[obj];

    Uint32 *temp_blobs = calloc(gw*gh, sizeof(Uint32));

    bool did_fill_for_loop = false;

    int loops = 0;

    do {
        did_fill_for_loop = false;
        for (int y = 0; y < gh; y++) {
            for (int x = 0; x < gw; x++) {
                if (grid[x+y*gw].object != obj) continue;
                if (temp_blobs[x+y*gw] > 0 || o->blob_data[chisel_size].blobs[x+y*gw] > 0) continue;

                // Find amount of empty cells or blobbed cells neighbouring our cell.
                int empty_or_blobbed = 0;
                for (int xx = -1; xx <= 1; xx++) {
                    for (int yy = -1; yy <= 1; yy++) {
                        if (xx == 0 && yy == 0) continue;
                        if (!grid[x+xx + (y+yy)*gw].type || o->blob_data[chisel_size].blobs[x+xx + (y+yy)*gw] > 0) {
                            empty_or_blobbed++;
                        }
                    }
                }

                bool did_fill = false;

                if (empty_or_blobbed) {
                    // Set in a circle.
                    int r = chisel_size == 1 ? 2 : 5;

                    for (int xx = -r+1; xx <= r+1; xx++) {
                        for (int yy = -r+1; yy <= r+1; yy++) {
                            if (!is_in_bounds(x+xx, y+yy)) continue;
                            if (xx*xx + yy*yy > r*r) continue;
                            if (grid[x+xx+(y+yy)*gw].object != obj) continue;
                            if (o->blob_data[chisel_size].blobs[x+xx+(y+yy)*gw] > 0) continue;

                            temp_blobs[x+xx + (y+yy)*gw] = *blob_count;
                            did_fill = true;
                            did_fill_for_loop = true;
                        }
                    }
                }

                if (did_fill)
                    (*blob_count)++;
            }
        }

        for (int i = 0; i < gw*gh; i++) {
            if (temp_blobs[i])
                o->blob_data[chisel_size].blobs[i] = temp_blobs[i];
        }
        memset(temp_blobs, 0, sizeof(Uint32)*gw*gh);
        loops++;
    } while (did_fill_for_loop);

    free(temp_blobs);
}

static void blob_generate_dumb(int obj, int chisel_size, Uint32 *blob_count) {
    SDL_assert(obj >= 0);
    SDL_assert(blob_count);
    
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (grid[x+y*gw].object != obj) continue;
            struct Object *o = &objects[obj];

            o->blob_data[chisel_size].blobs[x+y*gw] = *blob_count;
            (*blob_count)++;
        }
    }
}

// Sets up all blobs whose cells belongs to our obj.
void object_generate_blobs(int object_index, int chisel_size) {
    if (object_index >= MAX_OBJECTS) return;
        
    Uint32 count = 1;

    int percentage = 0;

    if (chisel_size == 1) {
        percentage = 10;
    }

    struct Object *obj = &objects[object_index];

    memset(obj->blob_data[chisel_size].blobs, 0, gw*gh*sizeof(Uint32));

    if (chisel_size == 0) {
        blob_generate_dumb(object_index, chisel_size, &count);
    } else {
        blob_generate_old_smart(object_index, chisel_size, percentage, &count);
    }

    // Remove single-celled blobs; join them with any neighbouring blob.
    if (chisel_size != 0) {
        for (int y = 0; y < gh; y++) {
            for (int x = 0; x < gw; x++) {
                if (!grid[x+y*gw].type) continue;
                if (grid[x+y*gw].object != object_index) continue;
                Uint32 blob = obj->blob_data[chisel_size].blobs[x+y*gw];
                if (!blob) continue;
            
                Uint32 blob_neighbour;
                // Check if any neighbours have the same blob as this.
                for (int yy = -1; yy <= 1; yy++) {
                    for (int xx = -1; xx <= 1; xx++) {
                        if (xx == 0 && yy == 0) continue;
                        if (x+xx >= gw || x+xx < 0 || y+yy >= gh || y+yy < 0) continue;
                        if (grid[x+xx+(y+yy)*gw].object != object_index) continue;
                            
                        Uint32 b = obj->blob_data[chisel_size].blobs[x+xx+(y+yy)*gw];
                        if (b == 0) continue;
                        blob_neighbour = b;
                        if (b == blob) goto next_one; // This isn't a single-celled blob. Go to next.
                    }
                }
                // If not, we know this is a single-celled blob.
                // So we join it with any neighbouring blob.
                obj->blob_data[chisel_size].blobs[x+y*gw] = blob_neighbour;
                count--;
            next_one:;
            }
        }
    }

    obj->blob_data[chisel_size].blob_count = count;

    object_blobs_set_pressure(object_index, chisel_size);
}

static void dust_set_random_velocity(int x, int y) {
    struct Cell *c = &fg_grid[x+y*gw];
    c->vx = 3.0  * (float)(rand()%100) / 100.0;
    c->vy = -2.0 * (float)(rand()%100) / 100.0;
}

bool object_remove_blob(int object, Uint32 blob, int chisel_size, int replace_dust) {
    struct Object *obj = &objects[object];
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (obj->blob_data[chisel_size].blobs[x+y*gw] != blob) continue;
            
            if (current_tool == TOOL_CHISEL_MEDIUM && chisel_blocker.state != CHISEL_BLOCKER_OFF && chisel_blocker.pixels[x+y*gw] != chisel_blocker.side) {
                return true;
            }

            set(x, y, CELL_NONE, -1);
            if (replace_dust) { // If we're replacing with dust
                set_array(fg_grid, x, y, CELL_DUST, -1);
                dust_set_random_velocity(x, y);
            }
        }
    }

    object_blobs_set_pressure(object, chisel_size);

    return false;
}

void object_darken_blob(struct Object *obj, Uint32 blob, int amt, int chisel_size) {
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (obj->blob_data[chisel_size].blobs[x+y*gw] == blob) {
                if (grid[x+y*gw].depth <= amt) {
                    grid[x+y*gw].depth = 0;
                } else {
                    grid[x+y*gw].depth -= amt;
                }
            }
        }
    }
}

static void mark_neighbours(int x, int y, int obj, int pobj, int *cell_count) {
    if (x < 0 || y < 0 || x >= gw || y >= gh ||
        !grid[x+y*gw].type ||
        !is_cell_hard(grid[x+y*gw].type) ||
        grid[x+y*gw].temp != -1 ||
        grid[x+y*gw].object != pobj) {
        return;
    }

    grid[x+y*gw].temp = obj;
    (*cell_count)++;

    mark_neighbours(x+1, y, obj, grid[x+y*gw].object, cell_count);
    mark_neighbours(x-1, y, obj, grid[x+y*gw].object, cell_count);
    mark_neighbours(x, y+1, obj, grid[x+y*gw].object, cell_count);
    mark_neighbours(x, y-1, obj, grid[x+y*gw].object, cell_count);
}

// We don't want to use a memset because there are pointers
// that would need to be freed. So, we just reset all the memory
// the pointers point to, instead of having to reallocate everything.
void objects_zero_memory() {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i].cell_count = 0;
        for (int chisel_size = 0; chisel_size < 3; chisel_size++) {
            memset(objects[i].blob_data[chisel_size].blobs, 0, sizeof(Uint32)*gw*gh);
            memset(objects[i].blob_data[chisel_size].blob_pressures, 0, sizeof(Uint32)*gw*gh);
            objects[i].blob_data[chisel_size].blob_count = 0;
            objects[i].cell_count = 0;
        }
    }
}

// Destroys old objects array, and creates new objects for all cells.
// This is useful when an existing object is split into two, and we want
// to split that into two separate objects.
// We don't want to bind together two objects which happen to be neighbouring,
// though, only break apart things.
void objects_reevaluate() {
    object_count = 0;
    object_current = 0;
    for (int i = 0; i < gw*gh; i++) {
        grid[i].temp = -1;
    }

    objects_zero_memory();

    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (!grid[x+y*gw].type || !is_cell_hard(grid[x+y*gw].type) || grid[x+y*gw].temp != -1) continue;
            int cell_count = 0;
            mark_neighbours(x, y, object_count, grid[x+y*gw].object, &cell_count);
            objects[object_count].cell_count = cell_count;
            object_count++;
        }
    }

    for (int i = 0; i < gw*gh; i++) {
        grid[i].object = grid[i].temp;
        grid[i].temp = 0;
    }

    // Always reevaluate blobs...
    // We used to have it only update if a new object is created
    // or it is forced to (update_blobs as a parameter) but
    // it seemed to cause a bug where chisel_goto_blob skips
    // over the blob. Maybe fix if that becomes a problem.
    
    /* if (update_blobs || object_count != previous_count) { */
    for (int i = 0; i < object_count; i++) {
        object_generate_blobs(i, 0);
        object_generate_blobs(i, 1);
        object_generate_blobs(i, 2);
    }
    /* } */
}

static int condition(int a, int end, int dir) {
    if (dir == 1) {
        return a <= end;
    }
    return a >= end;
}

int object_attempt_move(int object, int dx, int dy) {
    float len = sqrt(dx*dx + dy*dy);
    if (len == 0) return 0;

    float ux = dx/len;
    float uy = dy/len;

    int start_y = 0;
    int end_y = 0;
    int start_x = 0;
    int end_x = 0;
    int dir_x = 0;
    int dir_y = 0;
    
    if (uy > 0) {
        start_y = gh - 1;
        end_y = 0;
        dir_y = -1;
    } else {
		start_y = 0;
		end_y = gh - 1;
		dir_y = 1;
    }

    if (ux > 0) {
		start_x = gw - 1;
		end_x = 0;
		dir_x = -1;
	} else {
		start_x = 0;
		end_x = gw - 1;
		dir_x = 1;
	}

    float vx = ux; // = 0;
    float vy = uy; // = 0;

    struct Cell result_grid[gw*gh];
    memcpy(result_grid, grid, sizeof(struct Cell) * gw * gh);

    /* while (sqrt(vx*vx + vy*vy) < len) { */
    /*     vx += ux; */
    /*     vy += uy; */
    for (int y = start_y; condition(y, end_y, dir_y); y += dir_y) {
        for (int x = start_x; condition(x, end_x, dir_x); x += dir_x) {
            if (grid[x+y*gw].object == object) {
                int rx = x + vx;
                int ry = y + vy;
                if (rx < 0 || ry < 0 || rx >= gw || ry >= gh || grid[rx+ry*gw].type) {
                    // Abort
                    memcpy(grid, result_grid, sizeof(struct Cell) * gw * gh);
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

    return (int) (ux+uy*gw);
}

void convert_object_to_dust(int object) {
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (grid[x+y*gw].object != object) continue;
            set(x, y, 0, -1);
            set_array(fg_grid, x, y, CELL_DUST, -1);
            dust_set_random_velocity(x, y);
        }
    }
}

void draw_blobs() {
    if (!do_draw_blobs) return;

    for (int i = 0; i < object_count; i++) {
        for (int y = 0; y < gh; y++) {
            for (int x = 0; x < gw; x++) {
                Uint32 b = objects[i].blob_data[chisel->size].blobs[x+y*gw];
                b *= b;
                if (b == 0) continue;
                srand(b);
                SDL_SetRenderDrawColor(renderer, rand()%255, rand()%255, rand()%255, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
    srand(time(NULL));
}

void draw_objects() {
	if (!do_draw_objects) return;

	for (int y = 0; y < gh; y++) {
		for (int x = 0; x < gw; x++) {
			if (grid[x+y*gw].object == -1) continue;
			int b = grid[x+y*gw].object + 10;
            b *= b;
			srand(b);
			SDL_SetRenderDrawColor(renderer, rand()%255, rand()%255, rand()%255, 255);
			SDL_RenderDrawPoint(renderer, x, y);
		}
	}
}

// Returns the index of the closest cell to the point (px, py)
int clamp_to_grid(int px, int py, bool outside, bool on_edge, bool set_current_object, bool must_be_hard) {
    int closest_index = -1;
    float closest_distance = gw*gh; // Arbitrarily high number

    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {

            if (on_edge) {
                if (!grid[x+y*gw].type) continue;
                if (must_be_hard && !is_cell_hard(grid[x+y*gw].type)) continue;
            } else if (outside) {
                if (grid[x+y*gw].type) continue;
            } else {
                if (!grid[x+y*gw].type) continue;
            }

            int condition;
            if (outside) {
                if (on_edge) {
                    if (must_be_hard) {
                        condition = get_neighbour_count(x, y, 1) < 8 && is_any_neighbour_hard(x, y);
                    } else {
                        condition = get_neighbour_count(x, y, 1) < 8;
                    }
                } else {
                    if (must_be_hard) {
                        condition = (get_neighbour_count(x, y, 1) >= 1) && is_any_neighbour_hard(x, y);
                    } else {
                        condition = get_neighbour_count(x, y, 1) >= 1;
                    }
                }
            } else {
                condition = 1;
            }

            if (condition) {
                float dx = px - x;
                float dy = py - y;
                float dist = sqrt(dx*dx + dy*dy);

                if (dist < closest_distance) {
                    closest_distance = dist;
                    closest_index = x+y*gw;
                }
            }
        }
    }

    if (set_current_object) {
        if (on_edge) {
            object_current = grid[closest_index].object;
        } else {
            // NOTE: Can (possibly?) cause fuckup if the point is directly in between two objects.
            object_current = get_any_neighbour_object(closest_index%gw, closest_index/gw);
        }
        if (object_current < -1) {
            printf("Object Current was < -1. You are permitted to panic! D:\n"); fflush(stdout);
            object_current = -1;
        }
    }
    return closest_index;
}

int clamp_to_grid_angle(int x, int y, float rad_angle, bool set_current_object) {
    int l = gw;
    float x1 = x;
    float y1 = y;
    float x2 = x1 + l * cos(rad_angle);
    float y2 = y1 + l * sin(rad_angle);

    float dx = x2-x1;
    float dy = y2-y1;
    float len = sqrt(dx*dx + dy*dy);
    float ux = dx/len;
    float uy = dy/len;

    // Loop until one of the values goes off-screen.
    // This is so that we expand the line out so that it reaches
    // until the edge of the screen.
    while (x1 >= 0 && x1 < gw && y1 >= 0 && y1 < gh) {
        x1 -= ux;
        y1 -= uy;
    }

    float closest_distance = gw*gh;
    int closest_index = -1;

    for (int yy = 0; yy < gh; yy++) {
        for (int xx = 0; xx < gw; xx++) {
            if (!grid[xx+yy*gw].type && is_point_on_line((SDL_Point){xx, yy}, (SDL_Point){x1, y1}, (SDL_Point){x2, y2}) && get_neighbour_count(xx, yy, 1) >= 1) {
                float dist = distance(xx, yy, x, y);
                if (dist < closest_distance) {
                    closest_distance = dist;
                    closest_index = xx+yy*gw;
                }
            }
        }
    }

    if (set_current_object) {
        object_current = get_any_neighbour_object(closest_index%gw, closest_index/gw);
    }
    
    return closest_index;
}

int is_cell_hard(int type) {
    return
        type == CELL_MARBLE      ||
        type == CELL_QUARTZ      ||
        type == CELL_COBBLESTONE ||
        type == CELL_WOOD_LOG    ||
        type == CELL_WOOD_PLANK  ||
        type == CELL_GLASS       ||
        type == CELL_COAL        ||
        type == CELL_DIAMOND     ||
        type == CELL_ICE;
}

int is_cell_liquid(int type) {
    return type == CELL_WATER;
}

int is_cell_gas(int type) {
    return
        type == CELL_SMOKE ||
        type == CELL_STEAM;
}
