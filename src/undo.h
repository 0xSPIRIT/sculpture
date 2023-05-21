#define MAX_UNDO 1024

// We don't want to save the entire memory of the game since
// that will destroy RAM usage.
typedef struct Save_State {
    Cell *grid_layers[NUM_GRID_LAYERS];
    
    Item placer_items[TOTAL_SLOT_COUNT+1];
    // Format: Inventory Slots(5),
    //         Material Converter Slots(4),
    //         Fuel Converter Slots(3)
    //         Item Holding(1)
    
    Source_Cell source_cell[SOURCE_CELL_MAX];
    int source_cell_count;
} Save_State;

void undo_system_init(void);
void undo_system_reset(void);
Save_State *current_state(void);
void save_state_to_next(void);
void set_state(int num);
bool is_current_grid_same_as(Save_State *state);
bool is_current_slots_same_as(Save_State *state);
void set_state_to_string_hook(const char *string);
void undo(void);
