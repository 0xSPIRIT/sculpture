#ifndef GRID_H_
#define GRID_H_

// Maximum amounts of pressure for each chisel size.
#define MAX_PRESSURE (chisel->size == 2 ? 48.0 : 120.0)

#define MAX_OBJECTS 32
#define NUM_GRID_LAYERS 4

#define DRAW_PRESSURE false

#define GRAV 0.5
#define MAX_GRAV 4

#include <stdbool.h>
#include <SDL2/SDL.h>

struct Cell {
    enum Cell_Type type;  // All types are found in globals.h
    int object;           // Object index the cell belongs. -1 for none
    int temp;             // Temporary variable for algorithms
    bool updated;         // Updated for the frame yet?
    Uint8 depth;          // Z-depth. Controls brightness of the cell.
    int time;             // Time since set
    float vx, vy;         // Velocity
    float vx_acc, vy_acc; // When vel < 1, we need to keep track of that
    int px, py;           // Previous positions
    int rand;             // Random value per cell
};

extern struct Cell *grid_layers[NUM_GRID_LAYERS]; 
extern struct Cell *grid, *fg_grid, *pickup_grid; // Pointers into grid_layers.

extern int gw, gh; // Grid size; gets set when goto_level() is called.
extern int grid_show_ghost;

struct Blob_Data {
    Uint32 *blobs; // Grid (gw, gh) with blob IDs as ints
    int *blob_pressures; // Index into this using the blob index.
    int blob_count;
};

struct Object {
    struct Blob_Data blob_data[3]; // 3 Blob sizes for 3 chisels. 
    int cell_count;
};
extern struct Object objects[MAX_OBJECTS];
extern int object_count, object_current;

extern int do_draw_blobs, do_draw_objects;
extern bool paused;
extern int frames, step_one;

void grid_init(int w, int h);
void grid_deinit();
void set(int x, int y, int val, int object);
void set_array(struct Cell *arr, int x, int y, int val, int object);
int grid_array_tick(struct Cell *array, int x_direction, int y_direction);
void grid_array_draw(struct Cell *array);
void simulation_tick();
void grid_draw(void);
SDL_Color pixel_from_index(struct Cell *cells, int i);

int is_in_bounds(int x, int y);
int is_in_boundsf(float x, float y);

void move_by_velocity(struct Cell *arr, int x, int y);
bool move_by_velocity_gas(struct Cell *arr, int x, int y);

void swap_array(struct Cell *grid, int x1, int y1, int x2, int y2);
void swap(int x1, int y1, int x2, int y2);
void switch_blob_to_array(struct Cell *from, struct Cell *to, int obj, int blob, int chisel_size);

float get_pressure_threshold(int chisel_size);
void print_blob_data(struct Object *object, int chisel_size);
bool blob_can_destroy(int obj, int chisel_size, int blob);

void object_tick(int obj);
int object_does_exist(int obj);
void object_get_cell_count(int obj);
void object_blobs_set_pressure(int obj, int chisel_size);
void object_generate_blobs(int object_index, int chisel_size);
bool object_remove_blob(int object, Uint32 blob, int chisel_size, bool replace_dust);
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

int is_cell_hard(int type);
int is_cell_liquid(int type);
int is_cell_gas(int type);

#endif  /* GRID_H_ */
