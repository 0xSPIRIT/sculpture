#ifndef GRID_H_
#define GRID_H_

#define MAX_BLOB_CELLS   512
#define MAX_PRESSURE (chisel->size == 2 ? 56.5 : 121.0) /* Artificially raised max pressure from 48 to 56.5 as hack to decrease the final percentage. */
#define MAX_OBJECTS 32
#define MAX_CONTAINS 3

#define DRAW_PRESSURE 0

#include <SDL2/SDL.h>

struct Container {
    int types[MAX_CONTAINS];
    int amounts[MAX_CONTAINS];
};

struct BlobData {
    int *blobs; // Grid with numbers corresponding to different cells
    int *blob_pressures; // Index into this using the blob index.
    int blob_count;
};

struct Cell {
    int type;
    int object; // Object index the cell belongs. This should always be set.
    int temp; // Temporary variable for algorithms.
    int updated;
    Uint8 depth;
    float vx, vy;
};
extern struct Cell *grid;
extern int gw, gh;
extern int grid_show_ghost;

struct Object {
    struct BlobData blob_data[3]; // 3 Blob sizes for 3 chisels. 
};
extern struct Object objects[MAX_OBJECTS];
extern int object_count, object_current;

extern int do_draw_blobs, do_draw_objects;

void grid_init(int w, int h);
void grid_deinit(void);
void set(int x, int y, int val, int object);
void grid_tick(void);
void grid_draw(int draw_lines);
SDL_Color pixel_from_index(int i);

void move_by_velocity(int x, int y);

void swap(int x1, int y1, int x2, int y2);
void object_tick(int obj);
int object_does_exist(int obj);
void object_blobs_set_pressure(int obj, int chisel_size);
void object_set_blobs(int object_index, int chisel_size);
void object_remove_blob(int object, int blob, int chisel_size);
void object_darken_blob(struct Object *obj, int blob, int amt, int chisel_size);
void objects_reevalute(void);
int object_attempt_move(int object, int dx, int dy);

int get_any_neighbour_object(int x, int y);
int get_neighbour_count_of_object(int x, int y, int r, int obj);
int get_neighbour_count(int x, int y, int r);
int clamp_to_grid(int px, int py, int outside, int on_edge, int set_current_object);
int clamp_to_grid_angle(int x, int y, float rad_angle, int set_current_object);

void draw_blobs(void);
void draw_objects(void);

int cell_is_hard(int type);

#endif  /* GRID_H_ */
