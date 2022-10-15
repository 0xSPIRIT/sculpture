#define MAX_UNDO 64

struct Save_State {
    struct Cell *grid_layers[NUM_GRID_LAYERS];
};
