#define MAX_UNDO 1024

// We don't want to save the entire memory of the game since
// that will destroy RAM usage.
struct Save_State {
    struct Cell *grid_layers[NUM_GRID_LAYERS];
    struct Placer placers[PLACER_COUNT];
    // TODO: Save placer type & amount status.
};
