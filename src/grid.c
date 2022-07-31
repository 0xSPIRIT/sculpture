#include "grid.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <time.h>

#include "globals.h"
#include "util.h"
#include "chisel.h"
#include "grabber.h"
#include "level.h"
#include "chisel_blocker.h"

struct Cell *grid, *fg_grid;
int gw, gh;
int grid_show_ghost = 0;

struct Object objects[MAX_OBJECTS];
int object_count = 0, object_current = 0;

int do_draw_blobs = 0, do_draw_objects = 0;
int paused = 0, step_one = 0;
int frames = 0;

static SDL_Surface *bark_surface, *glass_surface, *wood_plank_surface, *diamond_surface, *ice_surface, *grass_surface, *triangle_blob_surface;

static float damp(int i) {
    srand(i);
    return 0.5 + ((float)rand())/RAND_MAX * 0.4;
}

static float water_spread() {
    return rand()%3 + ((float)rand())/RAND_MAX;
}

void grid_init(int w, int h) {
    gw = w;
    gh = h;

    grid = calloc(w*h, sizeof(struct Cell));
    fg_grid = calloc(w*h, sizeof(struct Cell));

    printf("Memory used: %lld\n", w*h*sizeof(struct Cell)); fflush(stdout);

    memset(objects, 0, sizeof(struct Object)*MAX_OBJECTS);
    for (int i = 0; i < MAX_OBJECTS; i++) {
        for (int j = 0; j < 3; j++) {
            objects[i].blob_data[j].blobs = calloc(w*h, sizeof(int));
            objects[i].blob_data[j].blob_pressures = calloc(w*h, sizeof(int));
        }
    }

    for (int i = 0; i < w*h; i++) {
        grid[i] = (struct Cell){.type = 0, .object = -1, .depth = 255};
        fg_grid[i] = (struct Cell){.type = 0, .object = -1, .depth = 255};
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

void grid_deinit(void) {
    free(grid);
    free(fg_grid);
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
        r = my_rand(i*i) % 100 < 5;
        amt = 25;
        color = get_pixel(glass_surface, i%gw, i/gw);
        if (r || get_neighbour_count_of_object(i%gw, i/gw, 1, cells[i].object) < 8) {
            color.a = 240;
        }
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
        color = (SDL_Color){32, 32, 32, 255};
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
    if (cells[i].type != CELL_GLASS && cell_is_hard(cells[i].type))
        color.a = cells[i].depth;
    return color;
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
    
    while (sqrt((xx-x)*(xx-x) + (yy-y)*(yy-y)) < len) {
        xx += ux;
        yy += uy;
        if (xx < 0 || xx >= gw) {
            p->vx *= -damp(x+y*gw);
            xx -= ux;
            yy -= uy;
            break;
        }
        if (yy < 0 || yy >= gh) {
            p->vy = 0;
            xx -= ux;
            yy -= uy;
            break;
        }
        if (((int)xx != x || (int)yy != y) && grid[(int)xx+((int)yy)*gw].type) {
            if (ux)
                p->vx *= -0.5;
            if (uy)
                p->vy *= -0.5;
            xx -= ux;
            yy -= uy;
            break;
        }
    }

    swap_arr(arr, x, y, (int)xx, (int)yy);
}

void swap_arr(struct Cell *arr, int x1, int y1, int x2, int y2) {
    if (x1 < 0 || x1 >= gw || y1 < 0 || y1 >= gh) return;
    if (x2 < 0 || x2 >= gw || y2 < 0 || y2 >= gh) return;

    // The blob data isn't swapped here so after calling this,
    // you should update the blob dkta.
    
    struct Cell temp = arr[x2+y2*gw];
    arr[x2+y2*gw] = arr[x1+y1*gw];
    arr[x1+y1*gw] = temp;
}

void swap(int x1, int y1, int x2, int y2) {
    swap_arr(grid, x1, y1, x2, y2);
}

void set_arr(struct Cell *arr, int x, int y, int val, int object) {
    if (x < 0 || x >= gw || y < 0 || y >= gh) return;
    
    arr[x+y*gw].type = val;
    arr[x+y*gw].time = 0;

    if (object != -1) {
        arr[x+y*gw].object = object;
    } else if (cell_is_hard(val)) { // Automatically set if the cell should be an obj
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

    if (obj == -1) return;
    if (obj >= MAX_OBJECTS) return;

    if (val == 0) {
        objects[obj].blob_data[chisel->size].blobs[x+y*gw] = 0;
        arr[x+y*gw].object = -1;
    }
}

// Sets an index in the grid array. Obj can be left to -1 to automatically
// find an object to assign to the cell, or it can be any value.
void set(int x, int y, int val, int object) {
    set_arr(grid, x, y, val, object);
}

// Can we destroy this blob or is it too far inside?
int blob_can_destroy(int obj, int chisel_size, int blob) {
    return (float)objects[obj].blob_data[chisel_size].blob_pressures[blob] / MAX_PRESSURE < 0.75;
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

void grid_tick(void) {
    if (!step_one)
        if (paused) return;

    if (step_one) step_one = 0;

    frames++;
   
    for (int y = gh-1; y >= 0; y--) {
        for (int q = 0; q < gw; q++) {
            int x = q;
            if (frames%2 == 0) {
                x = gw - 1 - x;
            }

            struct Cell *c = &grid[x+y*gw];

            c->time++;

            // Make sure we're only dealing with non-objects.
            if (!c->type || c->object != -1 || c->updated) continue;

            c->updated = 1;

            switch (c->type) {
            case CELL_WATER:
                if (y+1 < gh && !grid[x+(y+1)*gw].type) {
                    c->vy += GRAV;
                    if (c->vy > MAX_GRAV) c->vy = MAX_GRAV;
                } else if (x+1 < gw && y+1 < gh && !grid[x+1+(y+1)*gw].type) {
                    c->vx = 1;
                    c->vy = 1;
                } else if (x-1 >= 0 && y+1 < gh && !grid[x-1+(y+1)*gw].type) {
                    c->vx = -1;
                    c->vy = 1;
                } else if (x+1 < gw && !grid[x+1+y*gw].type && x-1 >= 0 && !grid[x-1+y*gw].type) {
                    float dx = water_spread() * ((rand()%2==0)?1:-1);
                    c->vx = dx;
                    c->vy = 0;
                } else if (x+1 < gw && !grid[x+1+y*gw].type) {
                    c->vx = 1;
                    c->vy = 0;
                } else if (x-1 >= 0 && !grid[x-1+y*gw].type) {
                    c->vx = -1;
                    c->vy = 0;
                } else {
                    c->vx = c->vy = 0;
                }
                break;
            case CELL_SAND:
                if (y+1 < gh && !grid[x+(y+1)*gw].type) {
                    c->vy += GRAV;
                    if (c->vy > MAX_GRAV) c->vy = MAX_GRAV;
                    c->vx *= damp(x+y*gw);
                } else if (y+1 < gh && x+1 < gw && !grid[x+1+(y+1)*gw].type) {
                    c->vx = 1;
                    c->vy = 1;
                } else if (y+1 < gh && x-1 >= 0 && !grid[x-1+(y+1)*gw].type) {
                    c->vx = -1;
                    c->vy = 1;
                } else {
                    c->vx = c->vy = 0;
                }
                break;
            case CELL_STEAM: case CELL_SMOKE: case CELL_DUST:
                break;
            default:
                if (y+1 < gh && !grid[x+(y+1)*gw].type) {
                    c->vy += 1;
                    c->vx *= damp(x+y*gw);
                }
                break;
            }

            move_by_velocity(grid, x, y);
        }
    }

    for (int i = 0; i < gw*gh; i++)
        grid[i].updated = 0;

    grid_fg_tick();
}

void grid_fg_tick(void) {
    /* for (int y = 0; y < gh; y++) { */
    /*     for (int x = 0; x < gw; x++) { */
    /*         if (!fg_grid[x+y*gw].type || fg_grid[x+y*gw].updated) continue; */
    /*         fg_grid[x+y*gw].updated = 1; */
    /*         fg_grid[x+y*gw].time++; */

    /*         if (fg_grid[x+y*gw].time >= 60 + rand()%80) { */
    /*             set(x, y, 0, -1); */
    /*             continue; */
    /*         } */

    /*         int t = fg_grid[x+y*gw].type; */
    /*         if (t == CELL_DUST) { */
    /*             struct Cell *c = &fg_grid[x+y*gw]; */
    /*             c->vy += GRAV; */
    /*             c->vy = clamp(c->vy, -MAX_GRAV, MAX_GRAV); */
    /*             c->vx *= .8; */
    /*             move_by_velocity(fg_grid, x, y); */
    /*         } */
    /*     } */
    /* } */
    /* for (int i = 0; i < gw*gh; i++) */
    /*     fg_grid[i].updated = 0; */
}

static int outlines_total[256*256] = {0}; // 256x256 seems to be more than the max level size.
static int outlines_total_count = 0;

void grid_draw(int draw_lines) {
    // Draw main grid
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (!grid[x+y*gw].type) continue;

            SDL_Color col = pixel_from_index(grid, x+y*gw);
            if (DRAW_PRESSURE && objects[object_count-1].blob_data[chisel->size].blobs[x+y*gw]) {
                Uint8 c = (Uint8)(255 * ((float)objects[object_count-1].blob_data[chisel->size].blob_pressures[objects[object_count-1].blob_data[chisel->size].blobs[x+y*gw]] / MAX_PRESSURE));
                SDL_SetRenderDrawColor(renderer, c, c, c, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
            }
            struct Line l = {x, y, x-grid[x+y*gw].vx, y-grid[x+y*gw].vy};
            SDL_RenderDrawLine(renderer, l.x1, l.y1, l.x2, l.y2);
        }
    }

    for (int i = 0; i < outlines_total_count; i++) {
        int o = outlines_total[i];
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawPoint(renderer, o%gw, o/gw);
    }

    /* for (int y = 0; y < triangle_blob_surface->h; y++) { */
    /*     for (int x = 0; x < triangle_blob_surface->w; x++) { */
    /*         SDL_Color c = get_pixel(triangle_blob_surface, x, y); */
    /*         SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a); */
    /*         SDL_RenderDrawPoint(renderer, x, y); */
    /*     } */
    /* } */

    // Draw smoke grid
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (!fg_grid[x+y*gw].type) continue;
            SDL_Color col = pixel_from_index(fg_grid, x+y*gw);
            SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
            SDL_RenderDrawPoint(renderer, x, y);
        }
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
            if (xx == 0 && yy == 0) continue;
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
            if (x+xx < 0 || x+xx >= gw || y+yy < 0 || y+yy >= gh) continue;
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
            if (x+xx < 0 || x+xx >= gw || y+yy < 0 || y+yy >= gh) continue;
            if (cell_is_hard(grid[x+xx+(y+yy)*gw].type)) return 1;
        }
    }
    return 0;
}

int get_neighbour_count_of_object(int x, int y, int r, int obj) {
    int c = 0;
    for (int xx = -r; xx <= r; xx++) {
        for (int yy = -r; yy <= r; yy++) {
            if (xx == 0 && yy == 0) continue;
            if (x+xx < 0 || x+xx >= gw || y+yy < 0 || y+yy >= gh) continue;
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
    pressure /= count;
    return pressure;
}

// Try to move us down 1 cell.
// If any cell is unable to move, undo the work.
void object_tick(int obj) {
    if (current_tool == TOOL_GRABBER && grabber.object_holding == obj) return;

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
    
    object_set_blobs(obj, 0);
    object_set_blobs(obj, 1);
    object_set_blobs(obj, 2);
}

void object_blobs_set_pressure(int obj, int chisel_size) {
    int temp_blobs[gw*gh];
    struct Object *object = &objects[obj];
    memcpy(temp_blobs, object->blob_data[chisel_size].blobs, sizeof(int) * gw * gh);

    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (!object->blob_data[chisel_size].blobs[x+y*gw] || temp_blobs[x+y*gw] == -1) continue;

            Uint32 blob = object->blob_data[chisel_size].blobs[x+y*gw];

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
    if (!cell_is_hard(grid[x + y*gw].type) ||
        !cell_is_hard(grid[x+1 + y*gw].type) ||
        !cell_is_hard(grid[x-1 + y*gw].type) ||
        !cell_is_hard(grid[x + (y+1)*gw].type) ||
        !cell_is_hard(grid[x + (y-1)*gw].type)) {
        return;
    }
    if (x < 0 || y < 0 || x >= gw || y >= gh) return;

    if (rand() % 100 < perc) {
        obj->blob_data[chisel_size].blobs[x + (y)*gw] = blob;
    }
}

// Loop through 3x3 squares on the grid.
// (yes technically we are going through each cell, not just
// the ones in our object. We remove these after this loop.)
// Note: Since w,h doesn't always divide evenly to gw and gh,
//       we loop an extra one.
static void blob_set_squares(struct Object *obj, int chisel_size, int percentage, int w, int h, Uint32 *blob_count) {
    for (int y = 0; y < gh/h + 1; y++) {
        for (int x = 0; x < gw/w + 1; x++) {
            int rx = x*w;
            int ry = y*h;

            int did_fill = 0;

            // Fill in the squares
            for (int yy = 0; yy < h; yy++) {
                for (int xx = 0; xx < w; xx++) {
                    // Check in case we're on the extra loop at the end. (+1)
                    if (rx+xx >= gw || ry+yy >= gh) continue;
                    if (cell_is_hard(grid[rx+xx + (ry+yy)*gw].type)) {
                        obj->blob_data[chisel_size].blobs[rx+xx + (ry+yy)*gw] = *blob_count;
                        did_fill = 1;
                    }
                }
            }
            // Go through the outline of the square, and randomly add more cells.
            if (w != 1 || h != 1) {
                for (int yy = -1; yy < -1+(h+1)*2; yy += h+1) {
                    for (int xx = -1; xx <= w; xx++) {
                        // Not diagonals
                        if (xx == -1 && yy == -1) continue;
                        if (xx == 3 && yy == -1)  continue;
                        if (xx == 3 && yy == 3)   continue;
                        if (xx == -1 && yy == 3)  continue;
                        if (rx+xx < 0 || ry+yy < 0 || rx+xx >= gw || ry+yy >= gh) continue;
                        random_set_blob(obj, rx+xx, ry+yy, *blob_count, chisel_size, percentage);
                    }
                }
                for (int yy = -1; yy <= h; yy++) {
                    for (int xx = -1; xx < -1+(w+1)*2; xx += w+1) {
                        // Not diagonals
                        if (xx == -1 && yy == -1) continue;
                        if (xx == 3 && yy == -1)  continue;
                        if (xx == 3 && yy == 3)   continue;
                        if (xx == -1 && yy == 3)  continue;
                        if (rx+xx < 0 || ry+yy < 0 || rx+xx >= gw || ry+yy >= gh) continue;
                        random_set_blob(obj, rx+xx, ry+yy, *blob_count, chisel_size, percentage);
                    }
                }
            }
            
            if (did_fill)
                (*blob_count)++;
        }
    }
}

// Returns if it worked.
static int blob_surface_flood_fill(int x, int y, Uint32 current, struct Object *obj, SDL_Surface *surf, int chisel_size, Uint32 blob_count) {
    if (x < 0 || y < 0 || x >= gw || y >= gh) return 0;
    if (!grid[x+y*gw].type) return 0;
    if (grid[x+y*gw].temp) return 0;
    if (!cell_is_hard(grid[x+y*gw].type)) return 0;
    if (obj->blob_data[chisel_size].blobs[x+y*gw] != 0) return 0;

    grid[x+y*gw].temp = 1; // We've updated this cell already.

    int did_fill = 0;

    SDL_Color c = get_pixel(surf, x, y);
    Uint32 ci = SDL_MapRGB(surf->format, c.r, c.g, c.b);
    
    if (ci == current) {
        obj->blob_data[chisel_size].blobs[x+y*gw] = blob_count;
        did_fill = 1;
    }

    did_fill = did_fill || blob_surface_flood_fill(x+1, y, current, obj, surf, chisel_size, blob_count);
    did_fill = did_fill || blob_surface_flood_fill(x-1, y, current, obj, surf, chisel_size, blob_count);
    did_fill = did_fill || blob_surface_flood_fill(x, y+1, current, obj, surf, chisel_size, blob_count);
    did_fill = did_fill || blob_surface_flood_fill(x, y-1, current, obj, surf, chisel_size, blob_count);

    return did_fill;
}

// The texture must have alternating black and white components.
static void blob_set_shape_from_surface(struct Object *obj, SDL_Surface *surf, int chisel_size, int percentage, Uint32 *blob_count) {
    for (int i = 0; i < gw*gh; i++) {
        grid[i].temp = 0;
    }

    for (int y = 0; y < surf->h; y++) {
        for (int x = 0; x < surf->w; x++) {
            if (x >= gw || y >= gh) continue;
            if (grid[x+y*gw].temp) continue;

            SDL_Color c = get_pixel(surf, x, y);
            Uint32 current = SDL_MapRGB(surf->format, c.r, c.g, c.b);

            int did_fill = blob_surface_flood_fill(x, y, current, obj, surf, chisel_size, *blob_count);
            if (did_fill) {
                (*blob_count)++;
            }
        }
    }

    for (int i = 0; i < gw*gh; i++) {
        grid[i].temp = 0;
    }
}

// Sets up all blobs whose cells belongs to our obj.
void object_set_blobs(int object_index, int chisel_size) {
    if (object_index >= MAX_OBJECTS) return;
        
    int w = chisel_size*chisel_size+2;
    int h = w;

    Uint32 i = 1;

    int percentage = 40;

    if (chisel_size == 0) {
        w = h = 1;
        percentage = 30;
    }

    struct Object *obj = &objects[object_index];

    memset(obj->blob_data[chisel_size].blobs, 0, gw*gh*sizeof(int));

    if (chisel_size == 2) {
        blob_set_shape_from_surface(obj, triangle_blob_surface, chisel_size, percentage, &i);
    } else {
        blob_set_squares(obj, chisel_size, percentage, w, h, &i);

        // Remove single-celled blobs; join them with any neighbouring blob.
        if (w != 1 || h != 1) {
            for (int y = 0; y < gh; y++) {
                for (int x = 0; x < gw; x++) {
                    if (!grid[x+y*gw].type) continue;
                    Uint32 blob = obj->blob_data[chisel_size].blobs[x+y*gw];
                    if (!blob) continue;
            
                    Uint32 blob_neighbour;
                    // Check if any neighbours have the same blob as this.
                    for (int yy = -1; yy <= 1; yy++) {
                        for (int xx = -1; xx <= 1; xx++) {
                            if (xx == 0 && yy == 0) continue;
                            Uint32 b = obj->blob_data[chisel_size].blobs[x+xx+(y+yy)*gw];
                            if (b == 0) continue;
                            blob_neighbour = b;
                            if (b == blob) goto next_one; // This isn't a single-celled blob. Go to next.
                        }
                    }
                    // If not, we know this is a single-celled blob.
                    // So we join it with any neighbouring blob.
                    obj->blob_data[chisel_size].blobs[x+y*gw] = blob_neighbour;
                    i--;
                next_one:;
                }
            }
        }
    }

    // Remove the blob cells that aren't in our object.
    for (int j = 0; j < gw*gh; j++) {
        if (!grid[j].type || !obj->blob_data[chisel_size].blobs[j] || grid[j].object == object_index) continue;
        obj->blob_data[chisel_size].blobs[j] = 0;
        i--;
    }

        /* if (chisel_size == 0) { */
        /*     for (int y = 0; y < gh; y++) { */
        /*         for (int x = 0; x < gw; x++) { */
        /*             printf("%u ", obj->blob_data[chisel_size].blobs[x+y*gw]); */
        /*         } */
        /*         printf("\n"); */
        /*     } */
        /*     printf("\n");printf("\n"); fflush(stdout); */
        /* } */

    /* } */

    obj->blob_data[chisel_size].blob_count = i;

    object_blobs_set_pressure(object_index, chisel_size);
}

void object_remove_blob(int object, Uint32 blob, int chisel_size, int replace_dust) {
    struct Object *obj = &objects[object];
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if ((current_tool == TOOL_CHISEL_SMALL || current_tool == TOOL_CHISEL_MEDIUM) && chisel_blocker.pixels[x+y*gw] != chisel_blocker.side) {
                continue;
            }
            if (obj->blob_data[chisel_size].blobs[x+y*gw] == blob) {
                set(x, y, CELL_NONE, -1);
                if (replace_dust) { // If we're replacing with dust
                    set_arr(fg_grid, x, y, CELL_DUST, -1);
                    /* struct Cell *c = &fg_grid[x+y*gw]; */
                    /* float rad = 2 * M_PI * (chisel->angle/360.0); */
                    /* c->vx = 3 * cos(rad); */
                    /* c->vy = 3 * sin(rad); */
                    /* printf("%f, %f\n", c->vx, c->vy); fflush(stdout); */
                }
            }
        }
    }
    object_blobs_set_pressure(object, chisel_size);
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

static void mark_neighbours(int x, int y, int obj, int pobj) {
    if (x < 0 || y < 0 || x >= gw || y >= gh || !grid[x+y*gw].type || !cell_is_hard(grid[x+y*gw].type) || grid[x+y*gw].temp != -1 || grid[x+y*gw].object != pobj) return;
    grid[x+y*gw].temp = obj;

    mark_neighbours(x+1, y, obj, grid[x+y*gw].object);
    mark_neighbours(x-1, y, obj, grid[x+y*gw].object);
    mark_neighbours(x, y+1, obj, grid[x+y*gw].object);
    mark_neighbours(x, y-1, obj, grid[x+y*gw].object);

    mark_neighbours(x+1, y+1, obj, grid[x+y*gw].object);
    mark_neighbours(x-1, y-1, obj, grid[x+y*gw].object);
    mark_neighbours(x+1, y-1, obj, grid[x+y*gw].object);
    mark_neighbours(x-1, y+1, obj, grid[x+y*gw].object);
}

// Destroys old objects array, and creates new objects for all cells.
// This is useful when an existing object is split into two, and we want
// to split that into two separate objects.
// We don't want to bind together two objects which happen to be touching,
// though-- only break apart things.
void objects_reevaluate(void) {
    // Remember, we're resetting all blob data as well, so we will
    // have to reset that at the end.

    // Todo: free the blob data.
    memset(objects, 0, MAX_OBJECTS * sizeof(struct Object));
    for (int i = 0; i < MAX_OBJECTS; i++) {
        for (int j = 0; j < 3; j++) {
            objects[i].blob_data[j].blobs = calloc(gw*gh, sizeof(int));
            objects[i].blob_data[j].blob_pressures = calloc(gw*gh, sizeof(int));
        }
    }
    object_count = 0;
    object_current = 0;
    for (int i = 0; i < gw*gh; i++) {
        grid[i].temp = -1;
    }
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (!grid[x+y*gw].type || !cell_is_hard(grid[x+y*gw].type) || grid[x+y*gw].temp != -1) continue;
            mark_neighbours(x, y, object_count, grid[x+y*gw].object);
            object_count++;
        }
    }
    for (int i = 0; i < gw*gh; i++) {
        grid[i].object = grid[i].temp;
    }
    for (int i = 0; i < object_count; i++) {
        object_set_blobs(i, 0);
        object_set_blobs(i, 1);
        object_set_blobs(i, 2);
    }
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

    object_set_blobs(object, 0);
    object_set_blobs(object, 1);
    object_set_blobs(object, 2);

    return (int) (ux+uy*gw);
}

void draw_blobs(void) {
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

void draw_objects(void) {
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

// Returns the closest index to the point (px, py)
int clamp_to_grid(int px, int py, int outside, int on_edge, int set_current_object, int must_be_hard) {
    int closest_index = -1;
    float closest_distance = gw*gh; // Arbitrarily high number
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            if (on_edge) {
                if (!grid[x+y*gw].type) continue;
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
            // NOTE: Can cause fuckup if the point is directly in between two objects.
            object_current = get_any_neighbour_object(closest_index%gw, closest_index/gw);
        }
    }
    return closest_index;
}

int clamp_to_grid_angle(int x, int y, float rad_angle, int set_current_object) {
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

int cell_is_hard(int type) {
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
