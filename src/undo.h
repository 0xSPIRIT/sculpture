#define MAX_UNDO 1024

// We don't want to save the entire memory of the game since
// that will destroy RAM usage.
struct Save_State {
    struct Cell *grid_layers[NUM_GRID_LAYERS];
    
    struct Item placer_items[TOTAL_SLOT_COUNT+1];
    // Format: Inventory Slots(5),
    //         Material Converter Slots(4),
    //         Fuel Converter Slots(3)
    //         Item Holding(1)
    
    struct Source_Cell source_cell[SOURCE_CELL_MAX];
    int source_cell_count;
};
