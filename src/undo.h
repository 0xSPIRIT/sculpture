#define MAX_UNDO 128

struct Save_State {
    struct Cell *grid_layers[NUM_GRID_LAYERS];
    // TODO: Save placer type & amount status.
};
