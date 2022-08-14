#ifndef GRID_H_
#define GRID_H_

#define MAX_BLOB_CELLS 512
#define MAX_PRESSURE (chisel->size == 2 ? 56.5 : 121.0) /* Artificially raised max pressure from 48 to 56.5 as hack to decrease the final percentage. */
#define MAX_OBJECTS 32
#define MAX_CONTAINS 3

#define DRAW_PRESSURE 0

#define GRAV 0.5
#define MAX_GRAV 4

#include <stdbool.h>
#include <SDL2/SDL.h>

struct Cell {
    int type;
    int object;           // Object index the cell belongs. -1 for none
    int temp;             // Temporary variable for algorithms
    bool updated;          // Updated for the frame yet?
    Uint8 depth;          // Z-depth. More darkness.
    int time;             // Time since set
    float vx, vy;         // Velocity
    float vx_acc, vy_acc; // When vel < 05, we need to keep track of that
    int px, py;           // Previous positions
    int rand;             // Random value per cell
};

extern struct Cell *grid, *fg_grid;
extern int gw, gh;
extern int grid_show_ghost;

struct BlobData {
    Uint32 *blobs; // Grid (gw, gh) with blob IDs as ints
    int *blob_pressures; // Index into this using the blob index.
    int blob_count;
};

struct Object {
    struct BlobData blob_data[3]; // 3 Blob sizes for 3 chisels. 
    int cell_count;
};
extern struct Object objects[MAX_OBJECTS];
extern int object_count, object_current;

extern int do_draw_blobs, do_draw_objects;
extern bool paused;
extern int frames, step_one;

struct SourceCell {
    int x, y;
    int type;
};
extern struct SourceCell source_cell[256];
extern int source_cell_count;

void grid_init(int w, int h);
void grid_deinit();
void set(int x, int y, int val, int object);
void grid_tick();
void fg_grid_tick();
void grid_draw(int draw_lines);
SDL_Color pixel_from_index(struct Cell *cells, int i);

int is_in_bounds(int x, int y);
int is_in_boundsf(float x, float y);

void move_by_velocity(struct Cell *arr, int x, int y);

void swap_arr(struct Cell *grid, int x1, int y1, int x2, int y2);
void swap(int x1, int y1, int x2, int y2);

int blob_can_destroy(int obj, int chisel_size, int blob);

void object_tick(int obj);
int object_does_exist(int obj);
void object_get_cell_count(int obj);
void object_blobs_set_pressure(int obj, int chisel_size);
void object_set_blobs(int object_index, int chisel_size);
void object_remove_blob(int object, Uint32 blob, int chisel_size, int replace_dust);
void object_darken_blob(struct Object *obj, Uint32 blob, int amt, int chisel_size);
void objects_reevaluate();
int object_attempt_move(int object, int dx, int dy);

void convert_object_to_dust(int object);

int get_any_neighbour_object(int x, int y);
int get_neighbour_count_of_object(int x, int y, int r, int obj);
int get_neighbour_count(int x, int y, int r);
int clamp_to_grid(int px, int py, bool outside, bool on_edge, bool set_current_object, bool must_be_hard);
int clamp_to_grid_angle(int x, int y, float rad_angle, bool set_current_object);

void draw_blobs();
void draw_objects();

int cell_is_hard(int type);

#endif  /* GRID_H_ */
