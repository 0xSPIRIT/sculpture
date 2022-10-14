// This undo system stores the state of all grid layers
// in a linked list. Upon saving a state, all previously
// created next pointers from the current state is destroyed.

#define MAX_UNDO 256

struct Save_State {
    struct Cell *grid_layers[NUM_GRID_LAYERS];
};
